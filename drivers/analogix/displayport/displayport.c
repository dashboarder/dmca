/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#include <debug.h>
#include <arch.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <sys.h>
#include <sys/task.h>
#include <sys/callout.h>
#include <lib/devicetree.h>

#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#include <drivers/lpdp_phy/lpdp_phy.h>
#include "regs.h"

#if WITH_HW_DISPLAY_DISPLAYPORT
#include <drivers/process_edid.h>
#endif

#ifndef DISPLAYPORT_VERSION
#error "DISPLAYPORT_VERSION must be defined"
#endif

/////////////////////////////////////////
////////// debug support

#define DP_DEBUG_MASK ( 		\
		DP_DEBUG_INIT |			\
		DP_DEBUG_ERROR |	\
		DP_DEBUG_INFO |			\
		DP_DEBUG_INT |		  \
		DP_DEBUG_VIDEO |	\
		DP_DEBUG_AUX |			\
		DP_DEBUG_DPCD |			\
		DP_DEBUG_PLL |			\
		DP_DEBUG_PHY |			\
		0)

#undef DP_DEBUG_MASK
#define DP_DEBUG_MASK 		(DP_DEBUG_INIT | DP_DEBUG_ERROR)

#define DP_DEBUG_INIT			(1<<16)  // initialisation
#define DP_DEBUG_ERROR			(1<<17)  // errors
#define DP_DEBUG_INFO			(1<<18)  // info
#define DP_DEBUG_INT			(1<<19)  // interrupts
#define DP_DEBUG_VIDEO			(1<<20)  // video
#define DP_DEBUG_AUX			(1<<21)  // aux channel
#define DP_DEBUG_DPCD			(1<<22)  // dpcp read/write
#define DP_DEBUG_I2C			(1<<23)  // i2c read/write
#define DP_DEBUG_PLL			(1<<24)  // PLL 
#define DP_DEBUG_PHY			(1<<25)  // PLL 
#define DP_DEBUG_ALWAYS			(1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)										\
	do {													\
		if ((DP_DEBUG_ ## _fac) & (DP_DEBUG_MASK | DP_DEBUG_ALWAYS))				\
			dprintf(DEBUG_CRITICAL, "DP: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)


/////////////////////////////////////////
////////// consts

// Crude screensaver: Turn off displayport after 3 minutes.
#define	kScreenBurnTimeout		(3 * 60 * 1000000)
#define	kDPAuxRetryDelayUS		5000
#define	kAUXMaxBurstLengthStandard	16

#define	kDisplayPortStackSize		8192

#define	kLSClock_81mhz			81000000ULL
#define	kLSClock_135mhz			135000000ULL
#define	kLSClock_171mhz			171000000UL

#define	kLinkRatePhysical_162gbps	1620000000ULL
#define	kLinkRatePhysical_270gps	2700000000ULL
#define	kLinkRatePhysical_324gps	3240000000ULL

#define	kAUXRetryIntervalUS		5000
#define	kMaxAuxTransactionRetry		100
#define	kMaxI2CTransactionRetry		5

#define LPTX_BUF_DATA_COUNT		16

#define kDPHPDTimeout			(70 * 1000)

//The display spec for edp requires 134ms from HPD to backlight ON.
//The following time accounts for HPD occurring upto before programming the backlight.
//Depending on the backlight solution, its programming time varies. Best guess is around 10ms.
#define kDPDeviceHPD2BLON (170 * 1000)

/////////////////////////////////////////
////////// typedefs, enums, structs

enum {
	kDPAuxTranscationStatus_None = -1,
	kDPAuxTranscationStatus_Success,
	kDPAuxTranscationStatus_IODefer,
	kDPAuxTranscationStatus_IOError,
	kDPAuxTranscationStatus_OtherError
};

enum {
	kBaseVoltageType_Neg_120mV,
	kBaseVoltageType_Neg_80mV,
	kBaseVoltageType_Neg_40mV,
	kBaseVoltageType_Pos_0mV,
	kBaseVoltageType_Pos_40mV,
	kBaseVoltageType_Pos_80mV,
	kBaseVoltageType_Pos_120mV,
	kBaseVoltageType_Pos_160mV
};

struct eq_calibration {
	uint32_t		swing;
	uint32_t		swing_max;
	uint32_t		eq;
	uint32_t		eq_max;
};

struct swing_calibration {
	int32_t				 base;
	uint32_t				boost;
	struct eq_calibration   eq_levels[4];
};

struct port_calibration {
	struct swing_calibration		swing[4];
};

enum {
	kI2COptionsNone			= 0,
	kI2COptionsTransactionStart	= (1 << 0),
	kI2COptionsTransactionEnd	= (1 << 1),
	kI2COptionsRetryOnDeferOnly	= (1 << 2),
};

/////////////////////////////////////////
////////// local variables

static const uint32_t __s_bpp_coeff[3] = {3, 2, 3};  // RGB:3, YCBR422:2, YCBR444:3

static addr_t __base_address;

static bool dp_voltage_boost;

static volatile bool dp_aux_transaction_pending;
static volatile int  dp_aux_transaction_status;

static bool dp_started;
static bool dp_video_started;
#if WITH_HW_DISPLAY_DISPLAYPORT
static bool dp_aux_abort;
#endif

static struct task *dp_task;
static struct task_event dp_controller_task_event;
static bool dp_interrupt_occurred;

static dp_t *dp_link_config_ptr;

static uint32_t dp_controller_common_sta_4_val;
static uint32_t dp_controller_common_sta_3_val;
static uint32_t dp_controller_common_sta_2_val;
static uint32_t dp_controller_common_sta_1_val;
static uint32_t dp_controller_sta_val;

static struct callout dp_controller_screensaver_callout;

static struct task_event dp_controller_hpd_event;

static uint32_t dp_min_lane_count;

/////////////////////////////////////////
////////// local functions declaration

static int dp_controller_task(void *arg);
static void reset();
static int configure_video(struct video_link_data *data);
static void init_video();
static void configure_video_m_n(struct video_link_data *data);
static int configure_video_color(struct video_link_data *data);
static int configure_video_mode(struct video_link_data *data);
static int validate_video(struct video_link_data *data, uint32_t *m_vid, uint32_t *n_vid);
static void configure_video_bist(struct video_link_data *data);
static void interrupt_filter(void *arg);
static void handle_interrupt(void);
static int commit_aux_transaction(bool use_interrupts, bool deferRetryOnly, uint32_t max_retry_count);
static int wait_for_aux_idle();
static int read_bytes_i2c_internal(uint32_t device_addr, uint32_t addr, u_int8_t *data, uint32_t length);
static void uninit_video();
static void disable_video(void);
static void init_aux();

static void handle_screensaver_callout(struct callout *co, void *args);

static uint32_t read_reg(uint32_t offset);
static void write_reg(uint32_t offset, uint32_t val);
static void and_reg(uint32_t offset, uint32_t val);
static void or_reg(uint32_t offset, uint32_t val);
static void set_bits_in_reg(uint32_t offset, uint32_t pos, uint32_t mask, uint32_t value);
static inline uint32_t ulmin(uint32_t a, uint32_t b)
{
	return (a < b ? a : b);
}

struct dp_controller_config {
	addr_t		base_address;
	uint32_t 	clock;
	uint32_t	irq;
	const char 	*dt_path;
};

static const struct dp_controller_config dp_config[] = {
#if WITH_HW_DISPLAY_DISPLAYPORT
	{ DP_BASE_ADDR, CLK_DPLINK, INT_DPORT0, DP_DTPATH },
#else
	{ 0, 0, 0, 0 },
#endif // WITH_HW_DISPLAY_DISPLAYPORT
#if WITH_HW_DISPLAY_EDP
	{ EDP_BASE_ADDR, CLK_EDPLINK, INT_EDP0, DP_DTPATH },
#else
	{ 0, 0, 0, 0 },
#endif // WITH_HW_DISPLAY_EDP	
};

/////////////////////////////////////////
////////// controller global functions

int dp_controller_start(dp_t *dp_link_config)
{
	DTNodePtr	node;
	char *		prop_name;
	void *		prop_data;
	uint32_t	prop_size;

	if (dp_started)
		return 0;

	dp_link_config_ptr = dp_link_config;

	if (dp_link_config_ptr->type > kDPControllerType_EDP) {
		debug(INIT, "dp_controller_start: bad controller type: %d\n", dp_link_config->type);
		return -1;
	}

	if (dp_config[dp_link_config_ptr->type].dt_path == 0) {
		debug(INIT, "dp_controller_start: controller type not configured: %d\n", dp_link_config->type);
		return -1;
	}

	__base_address = dp_config[dp_link_config_ptr->type].base_address;
	// turn on the clock
	clock_gate(dp_config[dp_link_config_ptr->type].clock, true);

	//reset Device
	clock_reset_device(dp_config[dp_link_config_ptr->type].clock);
	spin(100); //TODO: how long?

	dp_voltage_boost		= false;

	lpdp_init(DPPHY_DTPATH);

	// gather calibration data from DeviceTree
	if ( FindNode(0, dp_config[dp_link_config_ptr->type].dt_path, &node) ) {
		// max lane count
		prop_name = "max_lane_count";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			dp_link_config_ptr->lanes = *((uint32_t*)prop_data);
		}
		
		// max link rate
		prop_name = "max_link_rate";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			dp_link_config_ptr->max_link_rate = *((uint32_t*)prop_data);
		}
		
		// min lane count
		prop_name = "min_lane_count";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			dp_min_lane_count = *((uint32_t*)prop_data);
		}
		
		// min link rate
		prop_name = "min_link_rate";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			dp_link_config_ptr->min_link_rate = *((uint32_t*)prop_data);
		}
		
		// downspread
		prop_name = "downspread_support";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			dp_link_config_ptr->ssc = *((uint32_t*)prop_data);
		}
	}
	
	// register interrupt handler
	install_int_handler(dp_config[dp_link_config_ptr->type].irq, interrupt_filter, NULL);
	
	event_init(&dp_controller_task_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	dp_task = task_create("displayport", &dp_controller_task, NULL, kDisplayPortStackSize);
	task_start(dp_task);
	
	event_init(&dp_controller_hpd_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	
	reset();
	
	dp_started = true;
	
	unmask_int(dp_config[dp_link_config_ptr->type].irq);
	
	debug(INIT, "dp inited\n");
	debug(INIT, "dp_link_config_ptr->type	%d\n", dp_link_config_ptr->type);
	debug(INIT, "dp_link_config_ptr->mode	%d\n", dp_link_config_ptr->mode);
	debug(INIT, "dp_link_config_ptr->lanes 	%d\n", dp_link_config_ptr->lanes);
	debug(INIT, "dp_link_config_ptr->max_link_rate 0x%x\n", dp_link_config_ptr->max_link_rate);
	debug(INIT, "dp_link_config_ptr->min_link_rate 0x%x\n", dp_link_config_ptr->min_link_rate);
	debug(INIT, "dp_link_config_ptr->ssc  %d\n", dp_link_config_ptr->ssc);
	
	return 0;
}

void dp_controller_stop()
{
	if (!dp_started || !dp_video_started)
		return;

#if WITH_HW_DISPLAY_DISPLAYPORT
	dp_aux_abort = true;

#if WITH_HW_MCU
	// Stop background EDID polling.
	abort_edid();
#endif
#endif
	
	displayport_enable_alpm(false);
	dp_controller_stop_video();
	
	// disable interrupt source
	mask_int(dp_config[dp_link_config_ptr->type].irq);
	
	dp_video_started = false;
	
	dp_device_stop();								
	
	write_reg(DPTX_INTERRUPT_MASK_1, 0);
	write_reg(DPTX_INTERRUPT_MASK_4, 0);
	write_reg(DPTX_INTERRUPT_ENABLE, 0);
	
	write_reg(DPTX_COMMON_INTERRUPT_STATUS_1, 0xff);
	write_reg(DPTX_COMMON_INTERRUPT_STATUS_4, 0xff);
	write_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS, 0xff);
	
	if (dp_link_config_ptr->type == kDPControllerType_DP) {
		callout_dequeue(&dp_controller_screensaver_callout);
	}
	
	// Turn off all functions.
	write_reg(DPTX_FUNCTION_ENABLE_2, DPTX_SSC_FUNC_EN_N |
		  DPTX_AUX_FUNC_EN_N | 
		  DPTX_SERDES_FIFO_FUNC_EN_N | 
		  DPTX_LS_CLK_DOMAIN_FUNC_EN_N );
	
	lpdp_quiesce();

	// disable the clock
	clock_gate(dp_config[dp_link_config_ptr->type].clock, false);
	
	dp_started = false;
	
	// exit task and free memory
	event_signal(&dp_controller_task_event);
	task_wait_on(dp_task);
	task_destroy(dp_task);
}

#if WITH_HW_DISPLAY_EDP
// Ensure enough time has passed from HPD and turning the backlight ON
void dp_controller_wait_for_HPD_to_BL(utime_t hpd_time)
{
	utime_t hpd2blON = hpd_time + kDPDeviceHPD2BLON;
	utime_t now = system_time();
	utime_t timeleft = 0;

	if (now >= hpd2blON) {
		debug(INIT, "no need to wait hpd2blON 0x%llx  hpd_time 0x%llx  now 0x%llx timeleft 0x%llx\n", hpd2blON,  hpd_time, now, timeleft);
		return;
	}
	timeleft = hpd2blON - now;
	spin(timeleft);
	debug(INIT, "waited hpd2blON 0x%llx  hpd_time 0x%llx  now 0x%llx timeleft 0x%llx\n", hpd2blON,  hpd_time, now, timeleft);
}

bool dp_controller_wait_for_edp_hpd(utime_t *hpd_time)
{
	bool res;

	res = event_wait_timeout(&dp_controller_hpd_event, kDPHPDTimeout);
	//We might have missed the interrupt yet HPD is high. Check one last time before giving up
	if (!res) {
		if ( read_reg(DPTX_SYSTEM_CONTROL_3) & DPTX_HPD_STATUS ) {
			dp_device_start((dp_link_config_ptr->type == kDPControllerType_EDP) ? true : false);
			res = true;
		}
	}
	*hpd_time = system_time();

	return res;
}
#endif // WITH_HW_DISPLAY_EDP

int dp_controller_validate_video(struct video_timing_data *timings)
{
	// Assume basic settings.
	struct video_link_data data;
	bzero(&data, sizeof(data));
	bcopy(timings, &data.timing, sizeof(*timings));
	data.mirror_mode = false;
	data.test_mode = 0;
	data.color.depth = 8;
	data.color.space = kDisplayColorSpacesRGB;
	data.color.range = kDisplayColorDynamicRangeVESA;
	data.color.coefficient = kDisplayColorCoefficientITU601;
	return validate_video(&data, NULL, NULL);
}

uint32_t dp_controller_get_max_lane_count()
{
	return dp_link_config_ptr->lanes;
}

uint32_t dp_controller_get_max_link_rate()
{
	return dp_link_config_ptr->max_link_rate;
}

uint32_t dp_controller_get_min_lane_count()
{
	return dp_min_lane_count;
}

uint32_t dp_controller_get_min_link_rate()
{
	return dp_link_config_ptr->min_link_rate;
}

int dp_controller_set_link_rate(uint32_t link_rate)
{
	
	return lpdp_set_link_rate(link_rate);
}

int dp_controller_get_link_rate(uint32_t *link_rate)
{
	return lpdp_get_link_rate(link_rate);
}

int dp_controller_set_lane_count(uint32_t lane_count)
{
	bool is_downspread_supported = false;

	// disable SSC, SERDES FIFO, and link symbol clock domain modules
	or_reg(DPTX_FUNCTION_ENABLE_2, DPTX_SSC_FUNC_EN_N | DPTX_SERDES_FIFO_FUNC_EN_N | DPTX_LS_CLK_DOMAIN_FUNC_EN_N);

	lpdp_phy_set_lane_count(lane_count);

	// Note: DPTX_LANE_COUNT_SET == 0 is an illegal value; see <rdar://problem/15816689>
	write_reg(DPTX_MAIN_LINK_LANE_COUNT, lane_count);

	dp_controller_get_downspread(&is_downspread_supported);

	// enable SSC (if applicable), SERDES FIFO, and link symbol clock domain modules
	and_reg(DPTX_FUNCTION_ENABLE_2, ~((is_downspread_supported ? DPTX_SSC_FUNC_EN_N : 0) | DPTX_SERDES_FIFO_FUNC_EN_N | DPTX_LS_CLK_DOMAIN_FUNC_EN_N));

	write_reg(DPTX_FUNCTION_ENABLE_1, DPTX_LINK_CONTROLLER_RESET);
	and_reg(DPTX_FUNCTION_ENABLE_1, ~DPTX_LINK_CONTROLLER_RESET);

	return 0;
}

int dp_controller_get_lane_count(uint32_t * lane_count)
{
	*lane_count = read_reg(DPTX_MAIN_LINK_LANE_COUNT);
	
	return 0;
}

bool dp_controller_get_supports_fast_link_training()
{
	return dp_link_config_ptr->fast_link_training;
}

bool dp_controller_get_supports_downspread()
{
	return (dp_link_config_ptr->ssc && lpdp_get_supports_downspread());
}

int dp_controller_set_downspread(bool state)
{
	return lpdp_set_downspread(state);
}

int dp_controller_get_downspread(bool *state)
{
	*state = lpdp_get_downspread();
	
	return 0;
}

int dp_controller_set_enhanced_mode(bool mode)
{
	if ( mode )
		or_reg(DPTX_SYSTEM_CONTROL_4, DPTX_ENHANCED);
	else 
		and_reg(DPTX_SYSTEM_CONTROL_4, ~(DPTX_ENHANCED));
	
	return 0;
}

int dp_controller_get_enhanced_mode(bool * mode)
{
	*mode = read_reg(DPTX_SYSTEM_CONTROL_4) & DPTX_ENHANCED;
	
	return 0;
}

int dp_controller_set_ASSR(bool mode)
{	
	if ( mode )
		or_reg(DPTX_DP_EDP_CONTROL, DPTX_ALTERNATE_SR_EN);
	else
		and_reg(DPTX_DP_EDP_CONTROL, ~DPTX_ALTERNATE_SR_EN);
	
	return 0;
}

int dp_controller_get_ASSR(bool * mode)
{
	*mode = read_reg(DPTX_DP_EDP_CONTROL) & DPTX_ALTERNATE_SR_EN;
	
	return 0;
}

int dp_controller_get_adjustment_levels(uint32_t lane, uint32_t *voltage_swing, uint32_t *eq)
{
	uint32_t	lane_count;
	int		ret = 0;

	if ( lane > dp_link_config_ptr->lanes )
		return -1;
	
	ret = dp_controller_get_lane_count(&lane_count);
	if (ret != 0)
		return ret;

	return (lpdp_phy_get_adjustment_levels(lane, voltage_swing, eq));
}

int dp_controller_set_adjustment_levels(uint32_t lane, uint32_t voltage_swing, uint32_t eq, 
										bool *voltage_max_reached, bool *eq_max_reached)
{
	return (lpdp_phy_set_adjustment_levels(lane, voltage_swing, eq, voltage_max_reached, eq_max_reached));
}

int dp_controller_set_training_pattern(uint32_t value, bool scramble)
{
	uint32_t scram_setting = 0;
	
	debug(INFO, "training pattern=%d scrambled=%d\n", value, scramble);
	//TODO: fix need for scrambling via settings rdar://problem/15466504>
	
	scram_setting |= scramble ? 0 : DPTX_SCRAMBLING_DISABLE;
	write_reg(DPTX_DP_TRAINING_PATTERN_SET, scram_setting | value);
	
	return 0;
}

int dp_controller_get_training_pattern(uint32_t *pattern, bool *scramble)
{
	uint32_t value = read_reg(DPTX_DP_TRAINING_PATTERN_SET);
	
	if ( scramble )
		*scramble = (value & DPTX_SCRAMBLING_DISABLE) == 0;
	
	if ( pattern )
		*pattern = (value & DPTX_SW_TRAINING_PTRN_SET_SW_MASK);
	
	return 0;
}

void setup_aux_transaction(uint32_t address, uint32_t length, uint32_t command)
{
	write_reg(DPTX_DP_AUX_CHANNEL_LENGTH, DPTX_AUX_LENGTH(length - 1));

	write_reg(DPTX_DP_AUX_CH_ADDRESS_0,  0xFF & address);
	write_reg(DPTX_DP_AUX_CH_ADDRESS_1, 0xFF & (address >> 8));
	write_reg(DPTX_DP_AUX_CH_CONTROL_1,  (command & DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MASK) | (0x0F & (address >> 16)));

	// Ensure that AUX_CH_CTL_2[ADDR_ONLY] is set appropriately for each transaction
	write_reg(DPTX_DP_AUX_CH_CONTROL_2, (length > 0) ? 0 : DPTX_ADDR_ONLY);
}

int dp_controller_read_bytes_dpcd(uint32_t addr, u_int8_t *data, uint32_t length)
{
	uint32_t startOffset = 0;
	uint32_t maxBurstLength = kAUXMaxBurstLengthStandard;
	int ret = 0;
	
	debug(DPCD, "addr=0x%08x data=%p length=%d payload:\n", addr, data, length);
	
	while (startOffset < length) {
		uint32_t actualDataCount;
		uint32_t currentDataCount = __min(maxBurstLength, (length - startOffset));
		
		setup_aux_transaction(addr+startOffset, currentDataCount,
					DPTX_AUX_CH_CTL_1_AUX_TX_COMM_NATIVE |
					DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ);

		//Clear Buffer
		write_reg(DPTX_DP_BUFFER_DATA_COUNT, DPTX_BUF_CLR); 

		//Start Aux transaction
		ret = commit_aux_transaction(false, false, kMaxAuxTransactionRetry);
		if ( ret != 0 )
			return -1;

		actualDataCount = read_reg(DPTX_DP_BUFFER_DATA_COUNT) & 0xff;
		debug(DPCD, "Aux data length %d, expecting %d\n", actualDataCount, currentDataCount);

		if (actualDataCount == 0) {
			debug(ERROR, "actual count is 0\n");
			return -1;
		}

		if ( currentDataCount != actualDataCount )
			currentDataCount = __min(actualDataCount, currentDataCount);

		 debug(DPCD, "read: Bytes:\n");
		
		for(uint32_t currentDataIndex = 0; currentDataIndex < currentDataCount; currentDataIndex++) {
			data[startOffset + currentDataIndex] = (uint8_t)read_reg(DPTX_AUX_CH_BUFFER_DATA_0 + (currentDataIndex*sizeof(uint32_t)));
				 debug(DPCD, "read: " "\t0x%08x: %02x\n", addr + currentDataIndex, data[startOffset + currentDataIndex]);
		}
		
		startOffset += currentDataCount;
	}

	return 0;
}

int dp_controller_read_bytes_i2c(uint32_t device_addr, uint32_t addr, u_int8_t *data, uint32_t length)
{
	uint32_t startOffset = 0;
	int ret = 0;
	
	// try the 128 byte read first
	if ( read_bytes_i2c_internal(device_addr, addr, data, length) == 0 )
		return 0;
	
		debug(I2C, "retrying with segmented reads\n");
	
	// if that fails, fall back to individual 16 byte reads
	while ( startOffset < length ) {
		uint32_t currentDataCount = ((length - startOffset) > LPTX_BUF_DATA_COUNT) ? LPTX_BUF_DATA_COUNT: (length - startOffset);
		uint32_t retryCount = 0;
		
		ret = read_bytes_i2c_internal(device_addr, addr + startOffset, data + startOffset, currentDataCount);
		
		debug(I2C, "segmented read result=0x%08x with %d retries\n", ret, retryCount);
		
		if ( ret != 0 )
			break;
		
		startOffset += currentDataCount;
	}
	return ret;
}

int dp_controller_write_bytes_dpcd(uint32_t addr, u_int8_t *data, uint32_t length)
{
	uint32_t startOffset = 0;
	int ret;
	
	ret = -1;
	
	debug(DPCD, "write: addr=0x%08x data=%p length=%d \n", addr, data, length);
	
	while (startOffset < length) {
		uint32_t	currentDataCount;
		uint32_t	currentDataIndex;

		currentDataCount = ((length - startOffset) > LPTX_BUF_DATA_COUNT) ?  LPTX_BUF_DATA_COUNT: (length - startOffset);
		
		setup_aux_transaction(addr+startOffset, currentDataCount,
					DPTX_AUX_CH_CTL_1_AUX_TX_COMM_NATIVE |
					DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE);

		//Clear Buffer
		write_reg(DPTX_DP_BUFFER_DATA_COUNT, DPTX_BUF_CLR); 
		
		debug(DPCD, "write: Bytes:\n");
		for(currentDataIndex = 0; currentDataIndex < currentDataCount; currentDataIndex++) {
			debug(DPCD, "write:\t 0x%08x: %2.2x\n", addr + currentDataIndex, data[startOffset + currentDataIndex]);
			write_reg(DPTX_AUX_CH_BUFFER_DATA_0 + (currentDataIndex*sizeof(uint32_t)), 
					  data[startOffset + currentDataIndex]);
		}
		
		//Start Aux transaction
		ret = commit_aux_transaction(false, false, kMaxAuxTransactionRetry);
		if ( ret != 0 )
			break;
		
		startOffset += currentDataCount;
	}
	debug(DPCD,"\n");
	
	return ret;
}

int issue_i2c_write_request(uint32_t options, uint32_t address, const uint8_t * data, unsigned int length)
{
	//If not end set mot = 1
	uint32_t mot = (options & kI2COptionsTransactionEnd) ? 0 : DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT;
	int ret;

	if (length > DPTX_BUF_DATA_COUNT)
		return -1;

	//Clear Buffer
	write_reg(DPTX_DP_BUFFER_DATA_COUNT, DPTX_BUF_CLR);
	debug(I2C, "writing %d bytes @ I2C 0x%02x:\n", length, address);

	//Fill buffer
	for (unsigned int i = 0; i < length; i++ ) {
		write_reg(DPTX_AUX_CH_BUFFER_DATA_0 + (i * sizeof(uint32_t)), data[i]);
	}
	setup_aux_transaction(address, length, mot | DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE);

	//Start Aux transaction
	ret = commit_aux_transaction(false, false, options & kI2COptionsRetryOnDeferOnly);
	if ( ret )
		debug(I2C, "ret=0x%08x\n", ret);

	return ret;
}

int dp_controller_write_bytes_i2c(uint32_t device_addr, uint32_t addr, u_int8_t *data, uint32_t length)
{
	int		ret		= 0;
	uint32_t	offset		= 0;
	uint32_t	attemptCount	= 0;
	uint32_t	maxAttempts	= kMaxI2CTransactionRetry + 1;
	uint32_t	maxBurstLength	= kAUXMaxBurstLengthStandard;
	int		transfer_status = 0;

	// Since we don't allow any retries on commitAuxTransaction, we should really
	// retry on the whole transaction, which includes: start, commmit, and stop
	while( attemptCount++ < maxAttempts ) {
		if ( ret != 0 ) {
			debug(I2C, "waiting %dms till next attempt\n", kAUXRetryIntervalUS);
			task_sleep(kAUXRetryIntervalUS);
		}

		debug(I2C, "retry=%d device_addr=0x%08x addr=0x%08x data=%p length=%d\n", attemptCount, device_addr, addr, data, length);

		ret = issue_i2c_write_request(kI2COptionsTransactionStart, device_addr, NULL, 0);
		if ( ret != 0 )
			goto check;

		while (offset < length) {
			uint32_t chunk = ulmin(maxBurstLength, (length - offset));

			debug(I2C,"offset=%d requesting=%d bytes\n", offset, chunk);

			ret = issue_i2c_write_request(kI2COptionsRetryOnDeferOnly, device_addr, data + offset, chunk);
			if (ret != 0 )
				goto check;
			offset += chunk;
		}
check:
		// save transfer status
		transfer_status = ret;

		// terminate transaction regardless of transfer errors
		ret = issue_i2c_write_request(kI2COptionsTransactionEnd, device_addr, NULL, 0);

		// if stop was successful, use transfer error as return status
		ret = transfer_status;
	}
	debug(I2C,"result=0x%08x\n",ret);
	return ret;
}

int dp_controller_start_video(struct video_link_data *data)
{
#if WITH_HW_MCU && WITH_HW_DISPLAY_DISPLAYPORT
	struct video_link_data edid_data;
#endif
	struct video_link_data *timings = NULL;
	if ( !dp_started )
		return -1;
#if WITH_HW_MCU && WITH_HW_DISPLAY_DISPLAYPORT
	// Overwriting timings with EDID preference.
	bcopy(data, &edid_data, sizeof(edid_data));
	if (get_edid_timings(&edid_data.timing) == 0) timings = &edid_data;
#endif
	if (timings == NULL) timings = data;
	
	return configure_video(timings);
}

int dp_controller_stop_video()
{
	if ( !dp_video_started )
		return -1;

	and_reg(DPTX_SYSTEM_CONTROL_3, ~DPTX_VIDEO_SENDING_EN);

	and_reg(DPTX_VIDEO_CONTROL_1, ~DPTX_VIDEO_EN);

	dp_video_started = false;

	return 0;
}

bool dp_controller_video_configured()
{
	return dp_video_started;
}

bool dp_controller_get_supports_alpm(void)
{
	return dp_link_config_ptr->alpm;
}

int dp_controller_enable_alpm(bool enable, struct video_link_data *data)
{
	if (enable) {
		// Force PLL lock to high all the time to avoid un-necessary reset during ALPM mode
		or_reg(DPTX_DP_DEBUG_CONTROL_1,  ( DPTX_F_PLL_LOCK | DPTX_PLL_LOCK_CTRL));

#if DISPLAYPORT_VERSION < 5
		write_reg(DPTX_DP_VIDEO_DATA_FIFO_THRESHOLD, DPTX_VIDEO_TH_VALUE(0x8));
		
		// To avoid Video FIFO overrun
		or_reg(DPTX_DP_VIDEO_DATA_FIFO_THRESHOLD, DPTX_VIDEO_TH_CTRL);
#endif

		lpdp_phy_configure_alpm(enable);
		
#if DISPLAYPORT_VERSION >= 5
		// Program AUX_LINES_START/AUX_LINES_END registers
		write_reg(DPTX_AUX_LINES_START, 
			0); 
		write_reg(DPTX_AUX_LINES_END, 
			data->timing.axis[kDisplayAxisTypeVertical].active + 
			data->timing.axis[kDisplayAxisTypeVertical].sync_width + 
			data->timing.axis[kDisplayAxisTypeVertical].back_porch);
#endif

		// Program ALPM control registers
		// Clear PHY_SLEEP_DIS (use PHY_SLEEP only)
		and_reg(DPTX_ADVANCED_LINK_POWER_MANAGEMENT_CONTROL_2, ~(DPTX_PHY_SLEEP_EN));
		write_reg(DPTX_WAKEUP_LINES, dp_link_config_ptr->rx_n1);
		write_reg(DPTX_WAKEUP_CR_SYMBOLS, dp_link_config_ptr->rx_n2);
		write_reg(DPTX_SYMBOL_LOCK_PATTERN, dp_link_config_ptr->rx_n3);

#if DISPLAYPORT_VERSION >= 4
		write_reg(DPTX_SLEEP_STANDBY_DELAY_REG, DPTX_SLEEP_STANDBY_DELAY(dp_link_config_ptr->rx_n5));
#endif
		// Enable ALPM. ML_PHY_SLEEP_EN is used only
		// <rdar://problem/17835736> why need to set AUX_WAKE_UP_EN instead of WAKE_F_CHANGE_EN
		or_reg(DPTX_ADVANCED_LINK_POWER_MANAGEMENT_CONTROL_1, (DPTX_ALPM_PARA_UPDATE_EN | DPTX_ALPM_MODE | DPTX_ML_PHY_SLEEP_EN | DPTX_WAKE_F_CHANGE_EN));
	} else {
		// Disable ALPM..
		and_reg(DPTX_ADVANCED_LINK_POWER_MANAGEMENT_CONTROL_1, ~(DPTX_ML_PHY_SLEEP_EN | DPTX_ML_PHY_STANDBY_EN));
		spin(17 * 1000);

#if DISPLAYPORT_VERSION >= 5
		// Program AUX_LINES_START/AUX_LINES_END registers
		write_reg(DPTX_AUX_LINES_START, 0); 
		write_reg(DPTX_AUX_LINES_END, 0);
#endif
	}

	return 0;

}
/////////////////////////////////////////
////////// controller local functions

static void handle_screensaver_callout(struct callout *co, void *args)
{
	debug(ALWAYS, "Analogix Screensaver\n");
	
	display_clear();
}

static int dp_controller_task(void *arg)
{
	// External display, we would like to put screensaver
	if (dp_link_config_ptr->type == kDPControllerType_DP) {
		callout_enqueue(&dp_controller_screensaver_callout, kScreenBurnTimeout, handle_screensaver_callout, NULL);
	}	
	
	for (;;) {
		
		event_wait(&dp_controller_task_event);
		
		// if controller was stopped before dp task got chance to run, exit
		if (dp_started == false) {
			printf("dp not started\n");
			return 0;
		}
		
		mask_int(dp_config[dp_link_config_ptr->type].irq);
		
		if (dp_interrupt_occurred) {
			dp_interrupt_occurred = false;
			
			if (dp_link_config_ptr->type == kDPControllerType_DP) {
				callout_reset(&dp_controller_screensaver_callout, 0);
			}
			// Invoke bottom half.
			handle_interrupt();
		}
		
		unmask_int(dp_config[dp_link_config_ptr->type].irq);
	}
	
	return 0;
}

static void init_registers()
{
	write_reg(DPTX_FUNCTION_ENABLE_2, DPTX_SSC_FUNC_EN_N |
					DPTX_AUX_FUNC_EN_N | 
					DPTX_SERDES_FIFO_FUNC_EN_N | 
					DPTX_LS_CLK_DOMAIN_FUNC_EN_N );
	disable_video();

	write_reg(DPTX_VIDEO_CONTROL_4, 0);

	write_reg(DPTX_SYSTEM_CONTROL_1, 0x0);
	write_reg(DPTX_SYSTEM_CONTROL_2, DPTX_CHA_CRI(1));
	write_reg(DPTX_SYSTEM_CONTROL_3, 0x0);
	write_reg(DPTX_SYSTEM_CONTROL_4, 0x0);

	write_reg(DPTX_PACKET_SEND_CONTROL, 0x0);

	write_reg(DPTX_DP_HPD_DE_GLITCH, DPTX_HPD_DEGLITCH_DEFAULT);

	write_reg(DPTX_DP_LINK_DEBUG_CONTROL, DPTX_NEW_PRBS7);

	write_reg(DPTX_THRESHOLD_OF_M_VID_GENERATION_FILTER, 0x4);    // register default

#if DISPLAYPORT_VERSION < 5	
	write_reg(DPTX_DP_VIDEO_DATA_FIFO_THRESHOLD, 0x0);

	// set video FIFO threshold
	//
	// Reference:
	//  Fiji/Capri LPDP-TX Programming Sequence Guideline Document, Revision A.3
	set_bits_in_reg(DPTX_DP_VIDEO_DATA_FIFO_THRESHOLD,
			DPRX_VIDEO_TH_VALUE_SHIFT,
			DPRX_VIDEO_TH_VALUE_MASK, 8);   // TODO: allow configuration via device tree
	
	or_reg(DPTX_DP_VIDEO_DATA_FIFO_THRESHOLD, DPTX_VIDEO_TH_CTRL);
#endif
}

static void reset()
{
	// initialize PHY/PL
	lpdp_initialize_phy_and_pll();

	// initialize registers
	init_registers();

	//Initialize controller
	and_reg(DPTX_DP_ANALOG_POWER_DOWN, ~DPTX_DP_PHY_PD);
	and_reg(DPTX_FUNCTION_ENABLE_2, ~(DPTX_SERDES_FIFO_FUNC_EN_N | DPTX_LS_CLK_DOMAIN_FUNC_EN_N));
	and_reg(DPTX_FUNCTION_ENABLE_1, ~(DPTX_VID_CAP_FUNC_EN_N | DPTX_VID_FIFO_FUNC_EN_N | DPTX_SW_FUNC_EN_N));

	// Initially, let's only register for HPD changes
	write_reg(DPTX_INTERRUPT_MASK_4, DPTX_HOTPLUG_CHG | DPTX_PLUG | DPTX_HPD_LOST);
	write_reg(DPTX_INTERRUPT_ENABLE, DPTX_INT_HPD);

	init_aux();
	 
	//TODO
	//<rdar://problem/16655733> Fiji/Capri LPDP: Implement eDP ALTERNATE SCRAMBLER RESET
}

static void init_aux()
{
	// clear pending interrupts
	or_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS, DPTX_RPLY_RECEIV | DPTX_AUX_ERR);

	// power off aux to reset
	or_reg(DPTX_FUNCTION_ENABLE_2, DPTX_AUX_FUNC_EN_N);

	write_reg(DPTX_DP_AUX_CH_DEFER_CONTROL, DPTX_DEFER_CTRL_EN | DPTX_DEFER_COUNT(0));

	set_bits_in_reg(DPTX_BIST_AUX_CH_RELATED_REGISTER, 0,
					DPTX_BIST_AUX_AUX_TC_MASK  | DPTX_BIST_AUX_AUX_RETRY_TIMER_MASK,
					DPTX_AUX_TC_d599 | DPTX_BIST_AUX_AUX_RETRY_TIMER(0));

#if WITH_HW_DISPLAY_EDP
	set_bits_in_reg(DPTX_BIST_AUX_CH_RELATED_REGISTER, DPTX_BIST_AUX_AUX_RETRY_TIMER_SHIFT, DPTX_BIST_AUX_AUX_RETRY_TIMER_MASK, 7);
#endif

	and_reg(DPTX_FUNCTION_ENABLE_2, ~DPTX_AUX_FUNC_EN_N);
								
	write_reg(DPTX_DP_AUX_CH_CONTROL_2, 0);
	write_reg(DPTX_DP_AUX_CH_CONTROL_1, 0);
}

static void interrupt_filter(void *arg)
{
	dp_controller_common_sta_4_val = read_reg(DPTX_COMMON_INTERRUPT_STATUS_4);
	write_reg(DPTX_COMMON_INTERRUPT_STATUS_4, dp_controller_common_sta_4_val);
	
	dp_controller_common_sta_1_val = read_reg(DPTX_COMMON_INTERRUPT_STATUS_1);
	write_reg(DPTX_COMMON_INTERRUPT_STATUS_1, dp_controller_common_sta_1_val);
	
	dp_controller_sta_val = read_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS);
	write_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS, dp_controller_sta_val);
	
	debug(INT, "Interrupt received. common_sta_4_val=0x%08x common_sta_3_val=0x%08x common_sta_2_val=0x%08x common_sta_1_val=0x%08x sta_val=0x%08x\n", 
		  dp_controller_common_sta_4_val, dp_controller_common_sta_3_val, dp_controller_common_sta_2_val, dp_controller_common_sta_1_val, dp_controller_sta_val);
	
	dp_interrupt_occurred = true;
	
	event_signal(&dp_controller_task_event);
}

static void handle_interrupt(void)
{
	// This occurs in task context with interrupt masked.
	
	uint32_t common_sta_4_val;
	uint32_t common_sta_3_val;
	uint32_t common_sta_2_val;
	uint32_t common_sta_1_val;
	uint32_t sta_val;
	
	common_sta_4_val = dp_controller_common_sta_4_val;
	common_sta_3_val = dp_controller_common_sta_2_val;
	common_sta_2_val = dp_controller_common_sta_2_val;
	common_sta_1_val = dp_controller_common_sta_1_val;
	sta_val = dp_controller_sta_val;
	
	debug(INT, "Interrupt received. common_sta_4_val=0x%08x common_sta_3_val=0x%08x common_sta_2_val=0x%08x common_sta_1_val=0x%08x sta_val=0x%08x\n", 
		  common_sta_4_val, common_sta_3_val, common_sta_2_val, common_sta_1_val, sta_val);
	
	if ( dp_aux_transaction_pending ) {
		
		if ( (read_reg(DPTX_DP_AUX_CH_CONTROL_2) & DPTX_AUX_EN) == 0 ) {
			
			if ( DPTX_RPLY_RECEIV & sta_val ) {
				dp_aux_transaction_status = kDPAuxTranscationStatus_Success;
				debug(AUX, "waking pending dp_aux_transaction_status\n");
				// wake up dp_aux_transaction_status
			}
			
			if ( DPTX_AUX_ERR & sta_val ) {
				uint8_t aux_err = read_reg(DPTX_AUX_CHANNEL_ACCESS_STATUS) & 0xf;
				dp_aux_transaction_status = ( aux_err == DPTX_MUCH_DEFER_ERROR ) ? kDPAuxTranscationStatus_IODefer : kDPAuxTranscationStatus_IOError;
				debug(AUX, "waking pending dp_aux_transaction_status\n");
				// wake up dp_aux_transaction_status
			}
		}
	}
	
	if ( DPTX_INT_HPD & sta_val ) {
		debug(INT, "DPTX_DP_INT_STA_INT_HPD\n");
	}
	
	if ( DPTX_HOTPLUG_CHG & common_sta_4_val ) {
		debug(INT, "DPTX_SYSTEM_CONTROL_3=%d\n", read_reg(DPTX_SYSTEM_CONTROL_3));
	}
	
	if ( DPTX_HPD_LOST & common_sta_4_val ) {
		if ( !(read_reg(DPTX_SYSTEM_CONTROL_3) & DPTX_HPD_STATUS) ) {
			dp_device_stop();
		}
	} 
	
	if ( DPTX_HOTPLUG_CHG & common_sta_4_val ) {
		if ( read_reg(DPTX_SYSTEM_CONTROL_3) & DPTX_HPD_STATUS ) {
			event_signal(&dp_controller_hpd_event);

#if WITH_HW_DISPLAY_DISPLAYPORT
			dp_aux_abort = false;
#endif
			dp_device_start((dp_link_config_ptr->type == kDPControllerType_EDP) ? true : false);
		}
	}
	
}

#define USE_F_SEL	1


#if !USE_F_SEL
#define kSlaveClockRetry 200
#define kSlaveClockStreamValidationCycles 10
static int validate_video_capture()
{
	int ret    = -1;
	uint32_t valid  = 0;
	uint32_t retry  = 0;

	while (retry++ < kSlaveClockRetry ) {
		uint32_t value;

		spin(5 * 1000);

		value = read_reg(DPTX_SYSTEM_CONTROL_1);
		write_reg(DPTX_SYSTEM_CONTROL_1, value);
		value = read_reg(DPTX_SYSTEM_CONTROL_1);
		if ((value & DPTX_DET_STA) == 0) {
			printf("Error! Input stream clock not detected. 0x%08x\n", value);
			valid = 0;
			continue;
		}

		//To check whether input stream clock is stable.
		//To do that clear it first.
		value = read_reg(DPTX_SYSTEM_CONTROL_2);
		write_reg(DPTX_SYSTEM_CONTROL_2, value);
		value = read_reg(DPTX_SYSTEM_CONTROL_2);
		if (value & DPTX_DET_STA) {
			printf("Error! Input stream clock is changing 0x%08x\n", value);
			valid = 0;
			continue;
		}

		value = read_reg(DPTX_SYSTEM_CONTROL_3);
		write_reg(DPTX_SYSTEM_CONTROL_3, value);
		value = read_reg(DPTX_SYSTEM_CONTROL_3);
		if ((value & DPTX_STRM_VALID) == 0) {
			printf("Error! Input stream not valid. 0x%08x\n", value);
			valid = 0;
			continue;
		}

		if ( valid++ < kSlaveClockStreamValidationCycles )
			continue;

		ret = 0;
		break;
	}

	if ( ret )
		printf("Error! Timed out validating video capture. \n");
	else
		printf("Validated video capture\n");

	printf( "ret=0x%08x\n", ret);

	return ret;
}

int get_link_data_capture_and_compare(struct video_link_data *data)
{
	uint32_t        dp_vid_ctl;
	int        ret = -1;
	 
	//allow enough time 2 frames at least
	spin(34 * 1000);
	ret = validate_video_capture();
	if (ret != 0) {
		printf("validate_video_capture failed\n");
		goto exit;
	}

	dp_vid_ctl = read_reg(DPTX_DP_VIDEO_CONTROL);

	uint32_t space        = ((dp_vid_ctl & (3<1)) >> 1);
	printf("cp space %d tm space %d: %s\n", space, data->color.space, space != data->color.space ? "differ" : "equal");
	uint32_t range        = ((dp_vid_ctl & DPTX_DP_VIDEO_CONTROL_D_RANGE_MASK) >> DPTX_DP_VIDEO_CONTROL_D_RANGE_SHIFT);
	printf("cp range %d tm range %d: %s\n", range, data->color.range, range != data->color.range ? "differ" : "equal");
	uint32_t coefficient  = ((dp_vid_ctl & DPTX_DP_VIDEO_CONTROL_YC_COEFF_MASK) >> DPTX_DP_VIDEO_CONTROL_YC_COEFF_SHIFT);
	printf("cp coefficient %d tm coefficient %d: %s\n", coefficient, data->color.coefficient, coefficient != data->color.coefficient ? "differ" : "equal");
	uint32_t depth        = (((dp_vid_ctl & DPTX_DP_VIDEO_CONTROL_BPC_MASK) >> DPTX_DP_VIDEO_CONTROL_BPC_SHIFT));
	switch(depth) {
	case 0:
		depth = 6;
		break;
	case 1:
		depth = 8;
		break;
	case 2:
		depth = 10;
		break;
	case 3:
		depth = 12;
		break;
	}
	printf("cp depth %d tm depth %d: %s\n", depth, data->color.depth, depth != data->color.depth ? "differ" : "equal");
		
	uint32_t v_total            = read_reg(DPTX_TOTAL_LINE_STATUS);
	printf("cp v_total %d tm v_total %d \n", v_total, data->timing.axis[kDisplayAxisTypeVertical].total);
	uint32_t v_active           = read_reg(DPTX_ACTIVE_LINE_STATUS);
	printf("cp v_active %d tm v_active %d \n", v_active, data->timing.axis[kDisplayAxisTypeVertical].active);
	uint32_t v_sync_width        = read_reg(DPTX_VERTICAL_SYNC_WIDTH_STATUS);
	printf("cp v_sync_width %d tm v_sync_width %d \n", v_sync_width, data->timing.axis[kDisplayAxisTypeVertical].sync_width);
	uint32_t v_back_porch        = read_reg(DPTX_VERTICAL_BACK_PORCH_STATUS);
	printf("cp v_back_porch %d tm v_back_porch %d \n", v_back_porch, data->timing.axis[kDisplayAxisTypeVertical].back_porch);
	uint32_t v_front_porch       = read_reg(DPTX_VERTICAL_FRONT_PORCH_STATUS);
	printf("cp v_front_porch %d tm v_front_porch %d \n", v_front_porch, data->timing.axis[kDisplayAxisTypeVertical].front_porch);
	uint32_t v_sync_polarity     = (read_reg(DPTX_VIDEO_STATUS) & DPTX_VSYNC_P_S) == 0;
	printf("cp v_sync_polarity %d tm v_sync_polarity %d \n", v_sync_polarity, data->timing.axis[kDisplayAxisTypeVertical].sync_polarity);
	
	uint32_t h_total          = read_reg(DPTX_TOTAL_PIXEL_STATUS);
	printf("cp h_total %d tm h_total %d \n", h_total, data->timing.axis[kDisplayAxisTypeHorizontal].total);
	uint32_t h_active         = read_reg(DPTX_ACTIVE_PIXEL_STATUS);
	printf("cp h_active %d tm h_active %d\n", h_active, data->timing.axis[kDisplayAxisTypeHorizontal].active);
	uint32_t h_sync_width      = read_reg(DPTX_HORIZON_SYNC_WIDTH_STATUS);
	printf("cp h_sync_width %d tm h_sync_width %d\n", h_sync_width, data->timing.axis[kDisplayAxisTypeHorizontal].sync_width);
	uint32_t h_back_porch      = read_reg(DPTX_HORIZON_BACK_PORCH_STATUS);
	printf("cp h_back_porch %d tm h_back_porch %d\n", h_back_porch, data->timing.axis[kDisplayAxisTypeHorizontal].back_porch);
	uint32_t h_front_porch     = read_reg(DPTX_HORIZON_FRONT_PORCH_STATUS);
	printf("cp h_front_porch %d tm h_front_porch %d\n", h_front_porch, data->timing.axis[kDisplayAxisTypeHorizontal].front_porch);
	uint32_t h_sync_polarity   = (read_reg(DPTX_VIDEO_STATUS) & DPTX_HSYNC_P_S) == 0;
	printf("cp h_sync_polarity %d tm h_sync_polarity %d\n", h_sync_polarity, data->timing.axis[kDisplayAxisTypeHorizontal].sync_polarity);
	
	uint32_t my_mVid = read_reg(DPTX_M_VID_VALUE_MONITOR);
	uint32_t my_nVid = (read_reg(DPTX_N_VID_CFG_2) << 16) | (read_reg(DPTX_N_VID_CFG_1) << 8) | (read_reg(DPTX_N_VID_CFG_0) << 0);
	
	uint32_t my_linkRate;
	dp_controller_get_link_rate(&my_linkRate);
	     
	printf("my_nvid %d my_mVid 0x%lx my_linkRate 0x%x\n", my_nVid, my_mVid, my_linkRate);
	uint64_t tmp = my_linkRate == 0xa ? 270000000ULL : 162000000ULL;
	uint64_t my_streamClk = my_nVid ? ((tmp * my_mVid)/my_nVid) : 0;

	uint32_t totalArea = h_total * v_total;
	 
	uint32_t syncRate = (my_streamClk<<16) / totalArea;
	 
	printf("Timing: syncRate=%d.%d\n", (int)(syncRate >> 16), (int)(((syncRate & 0xffff) * 10000)>>16));
	uint8_t sink_status;
	dp_controller_read_bytes_dpcd(0x205, &sink_status, 1);
	if (sink_status == 0)
		printf("SINK STATUS IS FAILED\n");
	 ret = 0;

exit:
	 printf("ret=0x%08x\n", ret);
	 return ret;
}
#endif //USE_F_SEL

static int configure_video(struct video_link_data *data)
{
	int downstream_type = kDPDownstreamTypeDP;
	debug(VIDEO, "starting to configure video\n");
	
	if (dp_video_started) return 0;
	
	// Uncomment to show the timings we finally decide to use.
#if 0
	debug(ALWAYS, "configure_video with:\n");
	debug(ALWAYS, "mirror_mode: %d\n", data->mirror_mode);
	debug(ALWAYS, "test_mode: 0x%08x\n", data->test_mode);
	debug(ALWAYS, "color:\n");
	debug(ALWAYS, "  depth: %u\n", data->color.depth);
	debug(ALWAYS, "  space: %u\n", data->color.space);
	debug(ALWAYS, "  range: %u\n", data->color.range);
	debug(ALWAYS, "  coefficient: %u\n", data->color.coefficient);
	debug(ALWAYS, "timing:\n");
	debug(ALWAYS, "  interlaced: %d\n", data->timing.interlaced);
	{
		int i;
		for (i = 0; i < kDisplayAxisCount; ++i) {
			debug(ALWAYS, "  Axis %d:\n", i);
			debug(ALWAYS, "	total: %u\n", data->timing.axis[i].total);
			debug(ALWAYS, "	active: %u\n", data->timing.axis[i].active);
			debug(ALWAYS, "	sync_width: %u\n", data->timing.axis[i].sync_width);
			debug(ALWAYS, "	back_porch: %u\n", data->timing.axis[i].back_porch);
			debug(ALWAYS, "	front_porch: %u\n", data->timing.axis[i].front_porch);
			debug(ALWAYS, "	sync_rate: %u (%uHz)\n",
				  data->timing.axis[i].sync_rate, data->timing.axis[i].sync_rate >> 16);
			debug(ALWAYS, "	sync_polarity: %u\n", data->timing.axis[i].sync_polarity);
		}
	}
#endif
	
	init_video();
	
#if USE_F_SEL
	if ( configure_video_mode(data) != 0 )
		goto exit;
#endif //USE_F_SEL
	
	if ( configure_video_color(data) != 0 )
		goto exit;
	
	configure_video_bist(data);
	
	configure_video_m_n(data);

	//the phy/pll epilogue
	lpdp_init_finalize();

	//Set the PLL lock override bit in display port controller to avoid link controller reset during ALPM 
	//(<rdar://problem/15876785> LPDP ALPM needs programming F_PLL_LOCK from DP_DEBUG_CONTROL_REGISTER_1 (0x2067006c0))
	or_reg(DPTX_DP_DEBUG_CONTROL_1, DPTX_PLL_LOCK_CTRL | DPTX_F_PLL_LOCK);

#if WITH_HW_MCU && WITH_HW_DISPLAY_DISPLAYPORT
	downstream_type = get_edid_downstream_type();
#endif
	// start video
	or_reg(DPTX_VIDEO_CONTROL_1, DPTX_VIDEO_EN);

	or_reg(DPTX_SYSTEM_CONTROL_3, DPTX_VIDEO_SENDING_EN);

#if !USE_F_SEL
	get_link_data_capture_and_compare(data);
#endif

	// Setup InfoFrame
	// RY:Please refer to page 65 of the CEA861 spec
	// do not send info frames if alpm is supported
	if ((downstream_type == kDPDownstreamTypeHDMI) && (!dp_link_config_ptr->alpm)) {
		debug(VIDEO, "downstream type is %d and alpm not set. Sending info frames\n", downstream_type);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 0), 0);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 1), 0x08);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 2), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 3), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 4), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 5), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 6), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 7), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 8), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 9), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 10), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 11), 0x00);
		write_reg(DPTX_AVI_INFOFRAME_PACKET_DATA_BYTE_1 + (4 * 12), 0x00);	
		or_reg(DPTX_PACKET_SEND_CONTROL, DPTX_AVI_INFO_UP);
		or_reg(DPTX_PACKET_SEND_CONTROL, DPTX_AVI_INFO_EN);
	}

	dp_video_started = true;
	
	debug(VIDEO, "finished configuring video\n");
	
	return 0;
	
exit:
	debug(ERROR, "failed to configure video\n");
	dp_video_started = false;
	return -1;
}

static void disable_video(void)
{
	and_reg(DPTX_SYSTEM_CONTROL_3, ~(DPTX_VIDEO_SENDING_EN));
	
	and_reg(DPTX_VIDEO_CONTROL_1, ~DPTX_VIDEO_EN);
	
	// disable vid capture and vid fifo
	or_reg(DPTX_FUNCTION_ENABLE_1, (DPTX_VID_CAP_FUNC_EN_N | DPTX_VID_FIFO_FUNC_EN_N));
}

static void init_video()
{
	// configure video
	write_reg(DPTX_SYSTEM_CONTROL_1, 0); // clear force_det and det_ctrl
	write_reg(DPTX_VIDEO_CONTROL_8, DPTX_VID_HRES_TH(2) | DPTX_VID_VRES_TH(0)); // vid_hres_th and vid_vres_th
	write_reg(DPTX_VIDEO_CONTROL_10, 0);
	write_reg(DPTX_SYSTEM_CONTROL_2, (0xf << 4));

#if DISPLAYPORT_VERSION >= 4
	and_reg(DPTX_VIDEO_CONTROL_10, ~DPTX_F_SEL);
	and_reg(DPTX_VIDEO_CONTROL_10, DPTX_F_SEL);
#endif
	// enable vid capture and vid fifo
	and_reg(DPTX_FUNCTION_ENABLE_1, ~(DPTX_VID_CAP_FUNC_EN_N | DPTX_VID_FIFO_FUNC_EN_N | DPTX_SW_FUNC_EN_N));
}

static void configure_video_m_n(struct video_link_data *data)
{
	// using register calcuated M/N
	// we are defaulting to operating in master mode
	// therefore we are ensuring that async clock
	// is being used to calculate M/N
	and_reg(DPTX_SYSTEM_CONTROL_4, ~(DPTX_FIX_M_VID)); 
	write_reg(DPTX_N_VID_CFG_0, 0);
	write_reg(DPTX_N_VID_CFG_1, 0x80);
	write_reg(DPTX_N_VID_CFG_2, 0);
}

static int configure_video_color(struct video_link_data *data)
{
	uint32_t val;
	
	// set video color format
	switch (data->color.depth) {
		case 12:
			val = DPTX_IN_BPC_12_BITS;
			break;
		case 10:
			val = DPTX_IN_BPC_10_BITS;
			break;
		case 8:
			val = DPTX_IN_BPC_8_BITS;
			break;
		case 6:
			val = DPTX_IN_BPC_6_BITS;
			break;
		default:
			return -1;
	}
	
	// Set color range / space
	write_reg(DPTX_VIDEO_CONTROL_2, (data->color.range << DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT) | 
			  val | 
			  data->color.space);
	
	// set color coeff
	set_bits_in_reg(DPTX_VIDEO_CONTROL_3, DPTX_IN_YC_COEFFI_SHIFT, 
					DPTX_IN_YC_COEFFI_MASK, data->color.coefficient);
	
	return 0;
}

static int configure_video_mode(struct video_link_data *data)
{
	write_reg(DPTX_TOTAL_LINE_CFG,           data->timing.axis[kDisplayAxisTypeVertical].total);
	write_reg(DPTX_ACTIVE_LINE_CFG,          data->timing.axis[kDisplayAxisTypeVertical].active);
		
	write_reg(DPTX_VERTICAL_FRONT_PORCH_CFG, data->timing.axis[kDisplayAxisTypeVertical].front_porch);
	write_reg(DPTX_VERTICAL_SYNC_WIDTH_CFG,  data->timing.axis[kDisplayAxisTypeVertical].sync_width);
	write_reg(DPTX_VERTICAL_BACK_PORCH_CFG,  data->timing.axis[kDisplayAxisTypeVertical].back_porch);

	write_reg(DPTX_TOTAL_PIXEL_CFG,          data->timing.axis[kDisplayAxisTypeHorizontal].total);
	write_reg(DPTX_ACTIVE_PIXEL_CFG,         data->timing.axis[kDisplayAxisTypeHorizontal].active);
	write_reg(DPTX_HORIZON_FRONT_PORCH_CFG,  data->timing.axis[kDisplayAxisTypeHorizontal].front_porch);
	write_reg(DPTX_HORIZON_SYNC_WIDTH_CFG,   data->timing.axis[kDisplayAxisTypeHorizontal].sync_width);
	write_reg(DPTX_HORIZON_BACK_PORCH_CFG,   data->timing.axis[kDisplayAxisTypeHorizontal].back_porch);

	or_reg(DPTX_VIDEO_CONTROL_10, DPTX_F_SEL);
	     
	write_reg(DPTX_THRESHOLD_OF_M_VID_GENERATION_FILTER, 0);  // Video Filter Thresold
	and_reg(DPTX_DP_M_VID_CALCULATION_CONTROL, ~(1 << 2));   // disable low-pass filter for m-vid
	
	return 0;		
}

static int validate_video(struct video_link_data *data, uint32_t *m_vid, uint32_t *n_vid)
{
	uint32_t		lr;
	uint32_t		lc;
	uint64_t		streamClk, maxStreamClk;
	uint32_t		bpp;
	uint32_t		vTotalBppPerFrame;
	uint64_t		maxAvailableVSyncRate;
	uint32_t		mVid;
	uint32_t		nVid;
	
	nVid = 0;
	maxStreamClk = 0;
	
	streamClk = data->timing.axis[kDisplayAxisTypeVertical].total * 
		data->timing.axis[kDisplayAxisTypeHorizontal].total;
	streamClk *= data->timing.axis[kDisplayAxisTypeVertical].sync_rate; // vTotal * hTotal * vSyncRate
	streamClk >>= 16;
	
	bpp = data->color.depth * __s_bpp_coeff[data->color.space];
	
	vTotalBppPerFrame = bpp *
		data->timing.axis[kDisplayAxisTypeVertical].total * 
		data->timing.axis[kDisplayAxisTypeHorizontal].total;
	
	if (vTotalBppPerFrame == 0) {
		// Prevent divide-by-zero.
		debug(VIDEO, "Frame is zero sized\n");
		return -1;
	}
	
	// if interlaced
	if ( data->timing.interlaced ) {
		
		// XXX tempoorary hack to not support interlaced for the moment
		return -1;
		
		streamClk >>= 1;
		vTotalBppPerFrame >>= 1;
	}
	
	debug(VIDEO, "stream clock=%lld\n", streamClk);
	
	// get link rate and lane count		
	dp_controller_get_link_rate(&lr);
	dp_controller_get_lane_count(&lc);
	
	if (lc == 0) {
		debug(ERROR, "Lane count programmed to zero\n");
		return -1;
	}
	
	switch (lr) {
	case  kLinkRate270Gbps:
		maxAvailableVSyncRate = kLinkRatePhysical_270gps;
		break;
	case kLinkRate162Gbps:						
		maxAvailableVSyncRate = kLinkRatePhysical_162gbps;
		break;
	case kLinkRate324Gbps:
		maxAvailableVSyncRate = kLinkRatePhysical_324gps;
		break;
	default:
		debug(ERROR, "Unknown link rate %d\n", lr);
		return -1;
	}
	
	maxAvailableVSyncRate /= 10;
	maxAvailableVSyncRate *= 8;
	maxAvailableVSyncRate *= lc;
	maxAvailableVSyncRate /= vTotalBppPerFrame;
	
	// setup M and N, master mode
	switch (lr) {
		case kLinkRate270Gbps:
			maxStreamClk = kLSClock_171mhz;
			nVid = kLSClock_171mhz;
			break;
		case kLinkRate162Gbps:
			maxStreamClk = kLSClock_81mhz;
			nVid = kLSClock_81mhz;
			break;
		case kLinkRate324Gbps:
			maxStreamClk = kLSClock_81mhz;
			nVid = kLSClock_81mhz;
			break;
	}		
	
	debug(VIDEO, "max stream clock=%lld\n", maxStreamClk);
	
	if ( streamClk > maxStreamClk ) {
		debug(VIDEO, "stream-clk value is bad, %lld\n", streamClk);
		return -1;
	}
	
	mVid = (uint32_t)streamClk;
	
	debug(VIDEO, "maxAvailableVSyncRate=%lld mVid=%d nVid=%d\n", maxAvailableVSyncRate, mVid,nVid);
	
	if((uint64_t)data->timing.axis[kDisplayAxisTypeVertical].sync_rate > (maxAvailableVSyncRate<<16)) {
		debug(VIDEO, "vsync rate is too high, vSyncRate:%d, maxAvailableVSyncRate:%llu\n", 
			  data->timing.axis[kDisplayAxisTypeVertical].sync_rate, maxAvailableVSyncRate);
		return -1;
	}
	
	if ( m_vid )
		*m_vid = mVid;
	
	if ( n_vid )
		*n_vid = nVid;
	
	return 0;			   
}

static void configure_video_bist(struct video_link_data *data)
{
	// enable bist mode
	if ( data->test_mode ) {
		write_reg(DPTX_VIDEO_CONTROL_4, DPTX_BIST_EN | (data->test_mode - 1));
		debug(VIDEO, "Finished Video BIST\n");
	} else {
		and_reg(DPTX_VIDEO_CONTROL_4, ~(DPTX_BIST_EN));
	}
}

static int issue_i2c_read_request(uint32_t options, uint32_t address, uint8_t * data, unsigned int length)
{
	//If not end set mot = 1
	uint32_t mot = (options & kI2COptionsTransactionEnd) ? 0 : DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT;
	uint32_t bytesRead;
	int ret;

	if(length > DPTX_BUF_DATA_COUNT) {
		ret = -1;
		goto exit;
	}

	setup_aux_transaction(address, length, mot | DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ);

	//Clear Buffer
	write_reg(DPTX_DP_BUFFER_DATA_COUNT, DPTX_BUF_CLR);

	//Start Aux transaction
	ret = commit_aux_transaction(false, false, options & kI2COptionsRetryOnDeferOnly);
	if (ret != 0) {
		goto exit;
	}
	
	bytesRead = read_reg(DPTX_DP_BUFFER_DATA_COUNT) & DPTX_BUFFER_DATA_COUNT_MASK;
	if(bytesRead != length) {
	       	debug(ERROR, "SHORT RX!!! read %d, expecting %d\n", bytesRead, length);
		ret = -1;
		goto exit; 
	}

	//Copy out buffer
	for (unsigned int i = 0; i < length; i++ ) {
		data[i] = read_reg(DPTX_AUX_CH_BUFFER_DATA_0 + (i * sizeof(uint32_t)));
	}

exit:
	if ( ret )
		debug(I2C, "ret=0x%08x\n", ret);

	return ret;
}

static int read_bytes_i2c_internal(uint32_t device_addr, uint32_t addr, u_int8_t *data, uint32_t length)
{
	int		ret		= 0;
	uint32_t	offset		= 0;
	uint32_t	attemptCount	= 0;
	uint32_t	maxAttempts	= kMaxI2CTransactionRetry + 1;
	uint32_t	maxBurstLength	= kAUXMaxBurstLengthStandard;
	int		transfer_status = 0;
	
	debug(I2C, "device_addr=0x%08x addr=0x%08x data=%p length=%d payload=\n", device_addr, addr, 
	    data, length);
	
	// Since we don't allow any retries on commitAuxTransaction, we should really
	// retry on the whole transaction, which includes: start, commmit, and stop
	while ( attemptCount++ < maxAttempts ) {
		if ( ret != 0 ) {
			debug(I2C, "waiting %dus till next attempt\n", kAUXRetryIntervalUS);
			task_sleep(kAUXRetryIntervalUS);
		}

		ret = issue_i2c_read_request(kI2COptionsTransactionStart, device_addr, NULL, 0);
		if ( ret != 0 )
			goto check;
		
		while (offset < length) {
			uint32_t chunk = ulmin(maxBurstLength, (length - offset));

			debug(I2C, "offset=%d requesting=%d bytes\n", offset, chunk);

			ret = issue_i2c_read_request(kI2COptionsRetryOnDeferOnly, device_addr, data + offset, chunk);
			if (ret != 0 )
				goto check;
			offset += chunk;
		}
		
check:
		// save transfer status
		transfer_status = ret;

		// terminate transaction regardless of transfer errors
		ret = issue_i2c_read_request(kI2COptionsTransactionEnd, device_addr, NULL, 0);

		// if stop was successful, use transfer error as return status
		ret = transfer_status;
		break;
	}

	debug(I2C, "result=%d\n", ret);
	return ret;
}

static int wait_for_aux_idle()
{
	uint32_t timeout_time = 20;
	while (timeout_time-- && (read_reg(DPTX_AUX_CHANNEL_ACCESS_STATUS) & DPTX_AUX_BUSY))
		task_sleep(50);
	return (timeout_time == 0) ? -1 : 0;
}

static int commit_aux_transaction(bool useInterrupts, bool deferRetryOnly, uint32_t maxRetryCount)
{
	uint32_t	attemptCount	= 0;
	uint32_t	maxAttempts	= maxRetryCount + 1;
	int		ret		= 0;
	 
	if ( useInterrupts ) {
		debug(AUX, "dp_aux_transaction_pending=%d sleeping\n", dp_aux_transaction_pending);

		// sleep on dp_aux_transaction_pending
		while( dp_aux_transaction_pending )
			task_yield();

		debug(AUX, "dp_aux_transaction_pending=%d waking\n", dp_aux_transaction_pending);

		dp_aux_transaction_pending = true;
	}

	for ( attemptCount = 0; attemptCount <= maxAttempts; attemptCount ++ ) {

		if ( ret ) {
			debug(AUX, "waiting %dus till next attempt\n", kAUXRetryIntervalUS);
			task_sleep(kAUXRetryIntervalUS);
		}

		ret = wait_for_aux_idle();
		if ( ret != 0 ) {
			debug(ERROR, "Timeout waiting for AUX done\n");
			continue;
		}

		// enable interrupt
		write_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS, DPTX_RPLY_RECEIV | DPTX_AUX_ERR);
		or_reg(DPTX_INTERRUPT_ENABLE, DPTX_RPLY_RECEIV| DPTX_AUX_ERR);

		// start aux
		or_reg(DPTX_DP_AUX_CH_CONTROL_2, DPTX_AUX_EN);

		dp_aux_transaction_status = kDPAuxTranscationStatus_None;
		if ( useInterrupts ) {
			debug(AUX, "sleeping on dp_aux_transaction_status\n");
			// sleep on dp_aux_transaction_status
			while ( dp_aux_transaction_status == kDPAuxTranscationStatus_None )
				task_yield();
			debug(AUX, "waking on dp_aux_transaction_status\n");
		}
		else {
			// Wait until interrupt received and aux channel enable auto-clears.
			uint32_t int_sta;
			for (;;) {
				int_sta = read_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS) & (DPTX_RPLY_RECEIV | DPTX_AUX_ERR);
				write_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS, int_sta);
				if (int_sta != 0 && (read_reg(DPTX_DP_AUX_CH_CONTROL_2) & DPTX_AUX_EN) == 0)
					break;
#if WITH_HW_DISPLAY_DISPLAYPORT
				if (dp_aux_abort)
					return -1; //return error so abort_edid takes effect
#endif

				task_yield();
			}

			if (dp_link_config_ptr->type == kDPControllerType_DP) {
				callout_reset(&dp_controller_screensaver_callout, 0);
			}

			if (int_sta & DPTX_AUX_ERR) {
				uint8_t aux_err = read_reg(DPTX_AUX_CHANNEL_ACCESS_STATUS) & 0xF;
				debug(AUX, "AUX_STATUS error: %d\n", aux_err);

				dp_aux_transaction_status = ( aux_err == DPTX_MUCH_DEFER_ERROR ) ? kDPAuxTranscationStatus_IODefer : kDPAuxTranscationStatus_IOError;

			} else {
				dp_aux_transaction_status = kDPAuxTranscationStatus_Success;
			}
		}

		// make sure the aux channel is not busy
		ret = wait_for_aux_idle();
		if ( ret != 0 ) {
			and_reg(DPTX_DP_AUX_CH_CONTROL_2, ~DPTX_AUX_EN);
			debug(ERROR, "Timeout waiting for AUX done\n");
			continue;
		}

		debug(AUX, "dp_aux_transaction_status=0x%08x\n", dp_aux_transaction_status);

		ret = dp_aux_transaction_status;
		if (ret == kDPAuxTranscationStatus_Success) {
			break;
		} else if (ret!=kDPAuxTranscationStatus_IODefer && deferRetryOnly ) {
			debug(AUX, "Not retrying because error not defer.  ret=0x%08x\n", ret);
			break;
		}
	}

	// clear reply receive interrupt
	or_reg(DPTX_DISPLAYPORT_INTERRUPT_STATUS, (DPTX_RPLY_RECEIV | DPTX_AUX_ERR));
	and_reg(DPTX_INTERRUPT_ENABLE, ~(DPTX_RPLY_RECEIV | DPTX_AUX_ERR));

	if ( useInterrupts ) {
		dp_aux_transaction_pending = false;
		// wake up dp_aux_transaction_pending
		debug(AUX, "waking pending dp_aux_transaction_pending\n");
	}

	if ( ret == kDPAuxTranscationStatus_IOError)
		debug(AUX, "AUX transaction failed\n");

	return ret;
}

static uint32_t read_reg(uint32_t offset)
{
	return(*(volatile uint32_t *)(__base_address + offset));
}

static void write_reg(uint32_t offset, uint32_t value)
{
	*(volatile uint32_t *)(__base_address + offset) = value;
}

static void and_reg(uint32_t offset, uint32_t value)
{
	write_reg(offset,  read_reg(offset) & value);
}

static void or_reg(uint32_t offset, uint32_t value)
{
	write_reg(offset,  read_reg(offset) | value);
}

static void set_bits_in_reg(uint32_t offset, uint32_t pos, uint32_t mask, uint32_t value)
{
	uint32_t set = read_reg(offset);
	
	set &= ~mask;
	set |= (value << pos);
	
	write_reg(offset, set);
}
