/*
 * Copyright (C) 2010-2013, 2015 Apple Inc. All rights reserved.
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
#include "regs.h"

#if WITH_HW_MCU
#include <drivers/mcu.h>
#include <drivers/process_edid.h>
#endif

#ifndef DISPLAYPORT_VERSION
#error "DISPLAYPORT_VERSION must be defined"
#endif

/////////////////////////////////////////
////////// debug support

#define DP_DEBUG_MASK ( 		\
		DP_DEBUG_INIT |	        \
		DP_DEBUG_ERROR |	\
		DP_DEBUG_INFO |	        \
		DP_DEBUG_INT |          \
		DP_DEBUG_VIDEO |	\
		DP_DEBUG_AUX |	        \
		DP_DEBUG_DPCD |	        \
		DP_DEBUG_PLL |	        \
		0)

#undef DP_DEBUG_MASK
#define DP_DEBUG_MASK 		(DP_DEBUG_INIT | DP_DEBUG_ERROR)

#define DP_DEBUG_INIT           (1<<16)  // initialisation
#define DP_DEBUG_ERROR          (1<<17)  // errors
#define DP_DEBUG_INFO           (1<<18)  // info
#define DP_DEBUG_INT            (1<<19)  // interrupts
#define DP_DEBUG_VIDEO          (1<<20)  // video
#define DP_DEBUG_AUX            (1<<21)  // aux channel
#define DP_DEBUG_DPCD           (1<<22)  // dpcp read/write
#define DP_DEBUG_I2C            (1<<23)  // i2c read/write
#define DP_DEBUG_PLL            (1<<23)  // PLL 
#define DP_DEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)	        							\
	do {					        						\
		if ((DP_DEBUG_ ## _fac) & (DP_DEBUG_MASK | DP_DEBUG_ALWAYS))    			\
			dprintf(DEBUG_CRITICAL, "DP: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)


/////////////////////////////////////////
////////// consts

// Crude screensaver: Turn off displayport after 3 minutes.
#define kScreenBurnTimeout	    (3 * 60 * 1000000)
#define kDPAuxRetryDelayUS	    5000

#define kDisplayPortStackSize	    8192

#define kLSClock_81mhz              81000000ULL
#define kLSClock_135mhz             135000000ULL
#define kLSClock_171mhz             171000000UL

#define kLinkRatePhysical_162gbps   1620000000ULL
#define kLinkRatePhysical_270gps    2700000000ULL
#define kLinkRatePhysical_324gps    3240000000ULL

#define kAUXRetryIntervalUS         5000
#define kMaxAuxTransactionRetry     100
#define kMaxI2CTransactionRetry     5

#if WITH_HW_DISPLAY_EDP 
#define kMaxLaneCount   4
#elif WITH_HW_HOOVER
#define kMaxLaneCount   4
#else
#define kMaxLaneCount   2
#endif


#define kDPHPDTimeout               (50 * 1000)
#ifdef TARGET_HPD_TO_BL_TIMEOUT
#undef	kDPHPDTimeout
#define kDPHPDTimeout			TARGET_HPD_TO_BL_TIMEOUT
#endif

// Calibration settings
#if DISPLAYPORT_VERSION < 3
#ifndef DPTX_PLL_CTL_VALUE
	#define DPTX_PLL_CTL_VALUE 	0x15
#endif

#ifndef DPTX_ANALOG_CTL_2_VALUE
	#define DPTX_ANALOG_CTL_2_VALUE	0x0c
#endif

#ifndef DPTX_ANALOG_CTL_3_VALUE
	#define DPTX_ANALOG_CTL_3_VALUE	0x80
#endif

#ifndef DPTX_FINE_PRE_EMPHASIS_VOLTAGE_VALUE
        #define DPTX_FINE_PRE_EMPHASIS_VOLTAGE_VALUE 0
#endif
#else
#ifndef DPTX_PLL_CTL_VALUE
	#define DPTX_PLL_CTL_VALUE 	0x1d
#endif

#ifndef DPTX_ANALOG_CTL_2_VALUE
	#define DPTX_ANALOG_CTL_2_VALUE	0x0d
#endif

#ifndef DPTX_ANALOG_CTL_3_VALUE
	#define DPTX_ANALOG_CTL_3_VALUE	0x80
#endif
#endif //DISPLAYPORT_VERSION

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
    uint32_t        swing;
    uint32_t        swing_max;
    uint32_t        eq;
    uint32_t        eq_max;
};

struct swing_calibration {
    int32_t                 base;
    uint32_t                boost;
    struct eq_calibration   eq_levels[4];
};

struct port_calibration {
    struct swing_calibration        swing[4];
};

/////////////////////////////////////////
////////// local variables

static const uint32_t __s_bpp_coeff[3] = {3, 2, 3};  // RGB:3, YCBR422:2, YCBR444:3

static addr_t __base_address;

static struct port_calibration *dp_port_calibration_table;

static bool dp_voltage_boost;
static u_int32_t dp_voltage_base[kMaxLaneCount];
static u_int32_t dp_eq_levels[kMaxLaneCount];
static u_int32_t dp_voltage_levels[kMaxLaneCount];

static volatile bool dp_aux_transaction_pending;
static volatile int  dp_aux_transaction_status;

static bool dp_started;
static bool dp_video_started;

static struct task_event dp_controller_task_event;
static bool dp_interrupt_occurred;

static u_int8_t dp_controller_type;
static u_int8_t dp_controller_mode;

static uint32_t dp_controller_common_sta_4_val;
static uint32_t dp_controller_common_sta_3_val;
static uint32_t dp_controller_common_sta_2_val;
static uint32_t dp_controller_common_sta_1_val;
static uint32_t dp_controller_sta_val;

static struct callout dp_controller_screensaver_callout;

struct task_event dp_controller_hpd_event;

static uint32_t dp_max_lane_count;
static uint32_t dp_max_link_rate;
static uint32_t dp_min_lane_count;
static uint32_t dp_min_link_rate; 

static bool dp_supports_downspread;

/////////////////////////////////////////
////////// local functions declaration

static int dp_controller_task(void *arg);
static void init_hw();
static int configure_video(struct video_link_data *data);
static void init_video(bool master);
static void configure_video_timing_mode(struct video_link_data *data);
static void configure_video_mute(struct video_link_data *data, bool mute);
static void configure_video_m_n(struct video_link_data *data);
static int configure_video_color(struct video_link_data *data);
static int configure_video_mode(struct video_link_data *data);
static int validate_video(struct video_link_data *data, u_int32_t *m_vid, u_int32_t *n_vid);
static void configure_video_bist(struct video_link_data *data);
static void interrupt_filter(void *arg);
static void handle_interrupt(void);
static int stop_i2c_transaction(bool read);
static int start_i2c_transaction(uint32_t device_addr, uint32_t addr);
static int commit_aux_transaction(bool use_interrupts, bool deferRetryOnly, u_int32_t max_retry_count);
static int wait_for_aux_done();
static int read_bytes_i2c_internal(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length);
static void uninit_video();
static int set_voltage_base(u_int32_t lane, int32_t base);

static void handle_screensaver_callout(struct callout *co, void *args);

static uint32_t read_reg(u_int32_t offset);
static void write_reg(u_int32_t offset, uint32_t val);
static void and_reg(u_int32_t offset, uint32_t val);
static void or_reg(u_int32_t offset, uint32_t val);
static void set_bits_in_reg(u_int32_t offset, u_int32_t pos, u_int32_t mask, u_int32_t value);


struct dp_controller_config {
	addr_t		base_address;
	u_int32_t 	clock;
	u_int32_t	irq;
	const char 	*dt_path;
};

static const struct dp_controller_config dp_config[] = {
#if WITH_HW_DISPLAY_DISPLAYPORT	
#if DISPLAYPORT_VERSION == 0
	{ DISPLAYPORT_BASE_ADDR, CLK_DPLINK, INT_DISPLAYPORT, DP_DTPATH },
#elif DISPLAYPORT_VERSION >= 1	
	{ DP_BASE_ADDR, CLK_DPLINK, INT_DPORT0, DP_DTPATH },
#elif
#error "Not expected"	
#endif // DISPLAYPORT_VERSION
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
	DTNodePtr   node;
	char *      prop_name;
	void *      prop_data;
	u_int32_t   prop_size;
	int         i;
	
	if (dp_started)
		return 0;
    
	dp_controller_type	= dp_link_config->type;
	dp_controller_mode	= dp_link_config->mode;
	dp_max_lane_count 	= dp_link_config->lanes;
	dp_min_lane_count 	= dp_link_config->lanes;
	dp_max_link_rate	= dp_link_config->max_link_rate;
	dp_min_link_rate	= dp_link_config->min_link_rate; 
	dp_supports_downspread  = dp_link_config->ssc;

	__base_address = dp_config[dp_controller_type].base_address;
	
	// turn on the clock
	clock_gate(dp_config[dp_controller_type].clock, true);
    
#if DISPLAYPORT_VERSION == 0
	// reset the hw from PMGR
	u_int32_t val;
    
	val = *(volatile u_int32_t *)(rPMGR_DPLINK_PS) & ~(1 << 8);
	*(volatile u_int32_t *)(rPMGR_DPLINK_PS) = (1 << 31) | val;
	spin(1);        
	*(volatile u_int32_t *)(rPMGR_DPLINK_PS) = val;
#else
	write_reg(DPTX_SW_RESET, DPTX_SW_RESET_OPERATION);
	while (read_reg(DPTX_SW_RESET) != 0) {}
#endif // DISPLAYPORT_VERSION == 0
    
    dp_voltage_boost        = false;
    for (i = 0; i < kMaxLaneCount; i++)
        dp_voltage_base[i] = kBaseVoltageType_Pos_0mV;
    
    // gather calibration data from DeviceTree
    if ( FindNode(0, dp_config[dp_controller_type].dt_path, &node) ) {
        // link calibration
        
        prop_name = "calibration_data";
        if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
            if ( prop_size != sizeof(struct port_calibration) ) {
                debug(ERROR, "calibration-data size mismatch, expected:%zu, read:%u \n",
                      sizeof(struct port_calibration), prop_size);
                return -1;
            }
            
            dp_port_calibration_table = (struct port_calibration *)malloc(prop_size);
            
            bcopy(prop_data, (void *)dp_port_calibration_table, prop_size);
        }
        
        // max lane count
        prop_name = "max_lane_count";
        if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
            dp_max_lane_count = *((uint32_t*)prop_data);
        }
        
        // max link rate
        prop_name = "max_link_rate";
        if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
            dp_max_link_rate = *((uint32_t*)prop_data);
        }
        
        // min lane count
        prop_name = "min_lane_count";
        if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
            dp_min_lane_count = *((uint32_t*)prop_data);
        }
        
        // min link rate
        prop_name = "min_link_rate";
        if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
            dp_min_link_rate = *((uint32_t*)prop_data);
        }
        
        // downspread
        prop_name = "downspread_support";
        if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
            dp_supports_downspread = *((uint32_t*)prop_data);
        }
    }
    
	// register interrupt handler
	install_int_handler(dp_config[dp_controller_type].irq, interrupt_filter, NULL);
    
	event_init(&dp_controller_task_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	task_start(task_create("displayport", &dp_controller_task, NULL, kDisplayPortStackSize));
    
	event_init(&dp_controller_hpd_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
    
    init_hw();
    
	dp_started = true;
    
    // enable interrupts
    write_reg(DPTX_COMMON_INT_MASK_4, DPTX_COMMON_INT_4_HP_CHG | 
              DPTX_COMMON_INT_4_PLUG |  
              DPTX_COMMON_INT_4_HPD_LOST);
    
	unmask_int(dp_config[dp_controller_type].irq);
	
	debug(INIT, "dp inited\n");
	debug(INIT, "dp_controller_type	%d\n", dp_controller_type);
	debug(INIT, "dp_controller_mode	%d\n", dp_controller_mode);
	debug(INIT, "dp_max_lane_count 	%d\n", dp_max_lane_count);
	debug(INIT, "dp_max_link_rate 0x%x\n", dp_max_link_rate);
	debug(INIT, "dp_min_link_rate 0x%x\n", dp_min_link_rate);
	debug(INIT, "dp_supports_downspread  %d\n", dp_supports_downspread);
    
    return 0;
}

void dp_controller_stop()
{
    if (!dp_started)
        return;

    dp_controller_stop_video();
   
    // disable interrupt source
	mask_int(dp_config[dp_controller_type].irq);
    
#if WITH_HW_MCU
#if WITH_HW_BS
	// stop pass through mode
	mcu_set_passthrough_mode(false);
#endif
    
	// Stop background EDID polling.
	abort_edid();
    
#endif
    
	dp_video_started = false;
    
    dp_device_stop();                                
    
    write_reg(DPTX_COMMON_INT_MASK_1, 0);
#if DISPLAYPORT_VERSION < 3
    write_reg(DPTX_COMMON_INT_MASK_2, 0);
    write_reg(DPTX_COMMON_INT_MASK_3, 0);
#endif //DISPLAYPORT_VERSION
    write_reg(DPTX_COMMON_INT_MASK_4, 0);
    write_reg(DPTX_DP_INT_STA_MASK, 0);
    
    write_reg(DPTX_COMMON_INT_STA_1, 0xff);
#if DISPLAYPORT_VERSION < 3
    write_reg(DPTX_COMMON_INT_STA_2, 0xff);
    write_reg(DPTX_COMMON_INT_STA_3, 0xff);
#endif //DISPLAYPORT_VERSION
    write_reg(DPTX_COMMON_INT_STA_4, 0xff);
    write_reg(DPTX_DP_INT_STA, 0xff);
    
	if (dp_controller_type == kDPControllerType_DP) {
		callout_dequeue(&dp_controller_screensaver_callout);
	}
	
	// Turn off all functions.
	u_int32_t flags;
		
#if DISPLAYPORT_VERSION == 0
	flags = DPTX_FUNC_EN_1_VIDCAP | DPTX_FUNC_EN_1_VIDFIF0;
#elif DISPLAYPORT_VERSION >= 1
	flags = DPTX_FUNC_EN_1_SLAVE | DPTX_FUNC_EN_1_MASTER;
#endif // DISPLAYPORT_VERSION
	    
	write_reg(DPTX_FUNC_EN_1, flags |
	          DPTX_FUNC_EN_1_AUDFIF0 | 
	          DPTX_FUNC_EN_1_AUDCAP | 
	          DPTX_FUNC_EN_1_HDCP | 
	          DPTX_FUNC_EN_1_SW_ALL);
	    
	write_reg(DPTX_FUNC_EN_2, DPTX_FUNC_EN_2_SSC |
	          DPTX_FUNC_EN_2_AUX | 
	          DPTX_FUNC_EN_2_SERDES_FIFO | 
	          DPTX_FUNC_EN_2_LS_CLK_DOMAIN );
    
    // power down the PLL
    uint8_t pllLockState;
    while(((pllLockState = read_reg(DPTX_DP_DEBUG_CTL)) & DPTX_DP_DEBUG_CTL_PLL_LOCK) == 0)
        debug(INFO, "before pllLockState=%d\n", pllLockState);
    
    and_reg(DPTX_DP_PLL_CTL, DPTX_DP_PLL_CTL_PWR_DN);
    debug(INFO, "powered down DP PLL lockstate=%d\n",pllLockState);
    
    while(((pllLockState = read_reg(DPTX_DP_DEBUG_CTL)) & DPTX_DP_DEBUG_CTL_PLL_LOCK) == 0)
        debug(INFO, "after pllLockState=%d\n", pllLockState);
    
    // clear PLL 
    write_reg(DPTX_COMMON_INT_STA_1, DPTX_COMMON_INT_1_PLL_LCK_CHG);
    and_reg(DPTX_DP_DEBUG_CTL, ~(DPTX_DP_DEBUG_CTL_F_PLL_LOCK | DPTX_DP_DEBUG_CTL_PLL_LOCK_CTRL));
    
    write_reg(DPTX_DP_PHY_PD, DPTX_DP_PHY_PWR_DWN); // power down PHY
    
    // disable the clock
    clock_gate(dp_config[dp_controller_type].clock, false);
    
    if ( dp_port_calibration_table ) {
        free(dp_port_calibration_table);
        dp_port_calibration_table = NULL;
    }
    
    dp_started = false;
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

u_int32_t dp_controller_get_max_lane_count()
{
    return dp_max_lane_count;
}

u_int32_t dp_controller_get_max_link_rate()
{
    return dp_max_link_rate;
}

u_int32_t dp_controller_get_min_lane_count()
{
    return dp_min_lane_count;
}

u_int32_t dp_controller_get_min_link_rate()
{
    return dp_min_link_rate;
}

int dp_controller_set_link_rate(u_int32_t link_rate)
{
    write_reg(DPTX_LINK_BW_SET, link_rate);
    
    return 0;
}

int dp_controller_get_link_rate(u_int32_t *link_rate)
{
    *link_rate = read_reg(DPTX_LINK_BW_SET);
    
    return 0;
}

int dp_controller_set_lane_count(uint32_t lane_count)
{
    uint8_t     pll_lock_state;
    uint8_t     reg_val;
    uint32_t    func_enable_2;
    
    func_enable_2 = DPTX_FUNC_EN_2_SERDES_FIFO | DPTX_FUNC_EN_2_LS_CLK_DOMAIN;
    
    if ( dp_supports_downspread )
        func_enable_2 |= DPTX_FUNC_EN_2_SSC;
	
    // clear PLL 
    write_reg(DPTX_COMMON_INT_STA_1, DPTX_COMMON_INT_1_PLL_LCK_CHG);
    and_reg(DPTX_DP_DEBUG_CTL, ~(DPTX_DP_DEBUG_CTL_F_PLL_LOCK | DPTX_DP_DEBUG_CTL_PLL_LOCK_CTRL));
    
    // power up the PLL
    write_reg(DPTX_DP_PLL_CTL, DPTX_PLL_CTL_VALUE);
    debug(INFO, "powered up DP PLL\n");
    
    while(((pll_lock_state = read_reg(DPTX_DP_DEBUG_CTL)) & DPTX_DP_DEBUG_CTL_PLL_LOCK) == 0) {
        debug(INFO, "after pllLockState=%d\n", pll_lock_state);
    }
    
    // Since we are adjusting the lane power, we need to hold the phy in reset
    or_reg(DPTX_FUNC_EN_2, func_enable_2);
    or_reg(DPTX_DP_PHY_TEST, DPTX_DP_PHY_TEST_RST_MACRO);
    
    // turn off all lanes
	reg_val = (DPTX_DP_PHY_CH1_PWR_DWN | DPTX_DP_PHY_CH0_PWR_DWN);
#if DISPLAYPORT_VERSION >= 2
	reg_val |= (DPTX_DP_PHY_CH3_PWR_DWN | DPTX_DP_PHY_CH2_PWR_DWN);
#endif // DISPLAYPORT_VERSION >= 2
#if DISPLAYPORT_VERSION > 2  && LOW_POWER_MODE == 1
	//to use 300mv we need to enable low-power mode
    reg_val |= DPTX_DP_PHY_SEL_LOWPOWER;
#endif // DISPLAYPORT_VERSION > 2 && LOW_POWER_MODE == 1
    write_reg(DPTX_DP_PHY_PD, reg_val);
    
    switch(lane_count) {
#if DISPLAYPORT_VERSION >= 2
        case 4:
            and_reg(DPTX_DP_PHY_PD, ~(DPTX_DP_PHY_CH3_PWR_DWN));
            and_reg(DPTX_DP_PHY_PD, ~(DPTX_DP_PHY_CH2_PWR_DWN));
#endif // DISPLAYPORT_VERSION >= 2
            
        case 2:
            and_reg(DPTX_DP_PHY_PD, ~(DPTX_DP_PHY_CH1_PWR_DWN));
            and_reg(DPTX_DP_PHY_PD, ~(DPTX_DP_PHY_CH0_PWR_DWN));
            
        case 1:
            and_reg(DPTX_DP_PHY_PD, ~(DPTX_DP_PHY_CH0_PWR_DWN));	
	        break;
    }
    
    debug(INFO, "Setting lane count to %d\n", lane_count);
    write_reg(DPTX_LANE_COUNT_SET, lane_count);
    
    // wait for 100us so that phy pll settles
    spin(100);
    
    and_reg(DPTX_DP_PHY_TEST, ~(DPTX_DP_PHY_TEST_RST_MACRO));
    and_reg(DPTX_FUNC_EN_2, ~func_enable_2);
    
    return 0;
}

int dp_controller_get_lane_count(uint32_t * lane_count)
{
    *lane_count = read_reg(DPTX_LANE_COUNT_SET);
    
    return 0;
}

bool dp_controller_get_supports_fast_link_training()
{
#ifdef WITH_HW_HOOVER
	return false;
#else
	return (true);
#endif
}

bool dp_controller_get_supports_downspread()
{
    return dp_supports_downspread;
}

int dp_controller_set_downspread(bool state)
{
    uint32_t regval = DPTX_DP_DN_SPREAD_MOD_FREQ_33Khz;
    
    if ( state )
        regval |= DPTX_DP_DN_SPREAD_ENABLE;
    
    write_reg(DPTX_DP_DN_SPREAD_CTL, regval);
    
    return 0;
}

int dp_controller_get_downspread(bool *state)
{
    *state = read_reg(DPTX_DP_DN_SPREAD_CTL) & DPTX_DP_DN_SPREAD_ENABLE;
    
    return 0;
}

int dp_controller_set_enhanced_mode(bool mode)
{
    if ( mode )
        or_reg(DPTX_SYS_CTL_4, DPTX_SYS_CTL_4_ENHANCED);
    else 
        and_reg(DPTX_SYS_CTL_4, ~(DPTX_SYS_CTL_4_ENHANCED));
    
    return 0;
}

int dp_controller_get_enhanced_mode(bool * mode)
{
    *mode = read_reg(DPTX_SYS_CTL_4) & DPTX_SYS_CTL_4_ENHANCED;
    
    return 0;
}

int dp_controller_set_ASSR(bool mode)
{		
	if ( mode )
		or_reg(DPTX_DP_TRAINING_PTRN_SET, DPTX_DP_TRAINING_PTRN_SET_SCRM_EDP);
	else
		and_reg(DPTX_DP_TRAINING_PTRN_SET, ~DPTX_DP_TRAINING_PTRN_SET_SCRM_EDP);

	return 0;
}

int dp_controller_get_ASSR(bool * mode)
{
	*mode = read_reg(DPTX_DP_TRAINING_PTRN_SET) & DPTX_DP_TRAINING_PTRN_SET_SCRM_EDP;

	return 0;
}

int dp_controller_get_adjustment_levels(u_int32_t lane, u_int32_t *voltage_swing, u_int32_t *eq)
{
    if ( lane > kMaxLaneCount )
        return -1;
    
    *eq = dp_eq_levels[lane];
    *voltage_swing = dp_voltage_levels[lane];
    
    return 0;        
}

int dp_controller_set_adjustment_levels(u_int32_t lane, u_int32_t voltage_swing, u_int32_t eq, 
                                        bool *voltage_max_reached, bool *eq_max_reached)
{
    uint32_t h3_swing;
    uint32_t  h3_eq;
    bool h3_boost;
    uint32_t h3_base;
    
    debug(INFO, "lane=%d maxLaneCount=%d voltage=%d eq=%d\n", lane, kMaxLaneCount, voltage_swing, eq);
    
    if ( lane > kMaxLaneCount )
        return -1;
    
    dp_voltage_levels[lane] = voltage_swing;
    dp_eq_levels[lane] = eq;
    
    h3_swing = voltage_swing;
    h3_eq = eq;
    h3_boost = false;
    h3_base = 0;
    
    if ( dp_port_calibration_table ) {
        h3_swing = dp_port_calibration_table->swing[voltage_swing].eq_levels[eq].swing;
        h3_eq = dp_port_calibration_table->swing[voltage_swing].eq_levels[eq].eq;
        h3_boost = dp_port_calibration_table->swing[voltage_swing].boost;
        h3_base = dp_port_calibration_table->swing[voltage_swing].base;
        
        debug(INFO, "h3Swing=%d h3Eq=%d h3Base=%d hd3Boost=%d \n", h3_swing, h3_eq, h3_base, h3_boost);
        
        if ( h3_boost )
            or_reg(DPTX_ANALOG_CTL_1, DPTX_ANALOG_CTL_1_TX_SWING_INC_30PER);
        else
            and_reg(DPTX_ANALOG_CTL_1, ~DPTX_ANALOG_CTL_1_TX_SWING_INC_30PER);
        
        set_voltage_base(lane, h3_base);
        
        if ( eq_max_reached )
            *eq_max_reached = dp_port_calibration_table->swing[voltage_swing].eq_levels[eq].eq_max;
        if ( voltage_max_reached )
            *voltage_max_reached = dp_port_calibration_table->swing[voltage_swing].eq_levels[eq].swing_max;
    }                        
    
#if DISPLAYPORT_VERSION < 3
    write_reg(DPTX_DP_LN0_LINK_TRAINING_CTL + (lane<<2),
              (DPTX_FINE_PRE_EMPHASIS_VOLTAGE_VALUE << DPTX_DP_LN_LINK_TRAINING_CTL_FINE_ADJ_PRE_EMPHASIS_SHIFT) | 
              ((h3_eq << DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_SHIFT) & DPTX_DP_LN_LINK_TRAINING_CTL_PRE_SET_MASK) | 
              ((h3_swing << DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_SHIFT) & DPTX_DP_LN_LINK_TRAINING_CTL_DRV_SET_MASK) );
#else //DISPLAYPORT_VERSION
#if LOW_POWER_MODE == 1
	//14059240: down to 300mv for H6
	write_reg(DPTX_DP_LN0_LINK_TRAINING_CTL + (lane<<2), CHX_SWING_300mV_PRE_EMP1_0dB_PRE_EMP2_0dB);
#else //LOW_POWER_MODE == 1
	write_reg(DPTX_DP_LN0_LINK_TRAINING_CTL + (lane<<2), CHX_SWING_400mV_PRE_EMP1_0dB_PRE_EMP2_0dB_PC0);
#endif //LOW_POWER_MODE == 1
#endif //DISPLAYPORT_VERSION
    
    return 0;        
}

int dp_controller_set_training_pattern(u_int32_t value, bool scramble)
{
    uint32_t scram_setting = 0;
    
#if WITH_HW_DISPLAY_EDP
    scram_setting |= DPTX_DP_TRAINING_PTRN_SET_SCRM_EDP;
#endif
    
    debug(INFO, "training pattern=%d scrambled=%d\n", value, scramble);
    
    scram_setting |= scramble ? 0 : DPTX_DP_TRAINING_PTRN_SET_SCRM_DIS;
    write_reg(DPTX_DP_TRAINING_PTRN_SET, scram_setting | value);
    
    return 0;
}

int dp_controller_get_training_pattern(u_int32_t *pattern, bool *scramble)
{
    uint32_t value = read_reg(DPTX_DP_TRAINING_PTRN_SET);
    
    if ( scramble )
        *scramble = (value & DPTX_DP_TRAINING_PTRN_SET_SCRM_DIS) == 0;
    
    if ( pattern )
        *pattern = (value & DPTX_DP_TRAINING_PTRN_SET_SW_MASK);
    
    return 0;
}

int dp_controller_read_bytes_dpcd(u_int32_t addr, u_int8_t *data, u_int32_t length)
{
    uint32_t startOffset = 0;
    int ret = 0;
    
    debug(DPCD, "addr=0x%08x data=%p length=%d payload:\n", addr, data, length);
    
    while (startOffset < length) {
        uint32_t actualDataCount;
        uint32_t currentDataCount;
        uint32_t currentDataIndex;
        
        currentDataCount = ((length - startOffset) > DPTX_BUF_DATA_COUNT) ? DPTX_BUF_DATA_COUNT: (length - startOffset);
        
        //Select DPCD device address
        write_reg(DPTX_AUX_ADDR_7_0, (addr + startOffset) & 0xFF); 
        write_reg(DPTX_AUX_ADDR_15_8, ((addr + startOffset)>>8) & 0xFF);
        write_reg(DPTX_AUX_ADDR_19_16, ((addr + startOffset)>>16) & 0x0F);
        
        //Set DPCD read cmd mot = 0 and read 1 bytes
        write_reg(DPTX_AUX_CH_CTL_1, ((currentDataCount-1) << DPTX_AUX_CH_CTL_1_AUX_LENGTH_SHIFT) | 
                  DPTX_AUX_CH_CTL_1_AUX_TX_COMM_NATIVE | DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ);
        
        //Clear Buffer
        write_reg(DPTX_BUFFER_DATA_CTL, DPTX_BUFFER_DATA_CTL_CLR); 
        
        //Start Aux transaction
        ret = commit_aux_transaction(false, false, kMaxAuxTransactionRetry);
        if ( ret != 0 )
            return -1;
        
        actualDataCount = read_reg(DPTX_BUFFER_DATA_CTL) & 0x1f;
        //debug_dpcd("Bytes: length=%d data=", actualDataCount);
        debug(DPCD, "Aux data length %d, expecting %d\n", actualDataCount, currentDataCount);
        debug(DPCD, "Bytes: ");
        
        if ( currentDataCount != actualDataCount )
            currentDataCount = __min(actualDataCount, currentDataCount);
        
        for(currentDataIndex = 0; currentDataIndex < currentDataCount; currentDataIndex++) {
            data[startOffset + currentDataIndex] = (uint8_t)read_reg(DPTX_BUF_DATA_0 + (currentDataIndex*sizeof(uint32_t)));
            if ( DP_DEBUG_MASK & DP_DEBUG_DPCD )
                printf("%02x ", data[startOffset + currentDataIndex]);
        }
        if ( DP_DEBUG_MASK & DP_DEBUG_DPCD )
            printf("\n");
        
        startOffset += currentDataCount;
    }
    
    return 0;
}

int dp_controller_read_bytes_i2c(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length)
{
    uint32_t startOffset = 0;
    int ret = 0;
    
    // try the 128 byte read first
    if ( read_bytes_i2c_internal(device_addr, addr, data, length) == 0 )
		return 0;
    
        debug(I2C, "retrying with segmented reads\n");
    
    // if that fails, fall back to individual 16 byte reads
    while ( startOffset < length ) {
        uint32_t currentDataCount = ((length - startOffset) > DPTX_BUF_DATA_COUNT) ? DPTX_BUF_DATA_COUNT: (length - startOffset);
        uint32_t retryCount = 0;
        
        ret = read_bytes_i2c_internal(device_addr, addr + startOffset, data + startOffset, currentDataCount);
        
                debug(I2C, "segmented read result=0x%08x with %d retries\n", ret, retryCount);
        
        if ( ret != 0 )
			break;
        
        startOffset += currentDataCount;
    }
    return ret;
}

int dp_controller_write_bytes_dpcd(u_int32_t addr, u_int8_t *data, u_int32_t length)
{
    uint32_t startOffset = 0;
    int ret;
    
    ret = -1;
    
    debug(DPCD, "addr=0x%08x data=%p length=%d \n", addr, data, length);
    
    while (startOffset < length) {
        uint32_t    currentDataCount;
        uint32_t    currentDataIndex;
        
        //Clear Buffer
        write_reg(DPTX_BUFFER_DATA_CTL, DPTX_BUFFER_DATA_CTL_CLR); 
        
        //Select DPCD device address
        write_reg(DPTX_AUX_ADDR_7_0, (addr + startOffset) & 0xFF); 
        write_reg(DPTX_AUX_ADDR_15_8, ((addr + startOffset)>>8) & 0xFF);
        write_reg(DPTX_AUX_ADDR_19_16, ((addr + startOffset)>>16) & 0x0F);
        
        currentDataCount = ((length - startOffset) > DPTX_BUF_DATA_COUNT) ? 
    DPTX_BUF_DATA_COUNT: (length - startOffset);
        
        debug(DPCD, "Bytes: ");
        for(currentDataIndex = 0; currentDataIndex < currentDataCount; currentDataIndex++) {
            if ( DP_DEBUG_MASK & DP_DEBUG_DPCD )
                printf("%2.2x ", data[startOffset + currentDataIndex]);
            write_reg(DPTX_BUF_DATA_0 + (currentDataIndex*sizeof(uint32_t)), 
                      data[startOffset + currentDataIndex]);
        }
        
        //Set DPCD write cmd mot = 0 and write 1 bytes
        write_reg(DPTX_AUX_CH_CTL_1, ((currentDataCount - 1) << DPTX_AUX_CH_CTL_1_AUX_LENGTH_SHIFT) | 
                  DPTX_AUX_CH_CTL_1_AUX_TX_COMM_NATIVE | 
                  DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE);
        
        //Start Aux transaction
        ret = commit_aux_transaction(false, false, kMaxAuxTransactionRetry);
        if ( ret != 0 )
            break;
        
        startOffset += currentDataCount;
    }
    if ( DP_DEBUG_MASK & DP_DEBUG_DPCD )
        printf("\n");
    
    return ret;
}

int dp_controller_write_bytes_i2c(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length)
{
    uint32_t startOffset = 0;    
    int ret;
    
    // start i2c transaction
    ret = start_i2c_transaction(device_addr, addr);
    if ( ret != 0 )
        goto exit;
    
        debug(I2C, "addr=0x%08x data=%p length=%d\n", addr, data, length);
    
    while (startOffset < length) {
        uint32_t currentDataCount;
        uint32_t currentDataIndex;
        
        currentDataCount = ((length - startOffset) > DPTX_BUF_DATA_COUNT) ? 
    DPTX_BUF_DATA_COUNT: (length - startOffset);
        
        for(currentDataIndex = 0; currentDataIndex < currentDataCount; currentDataIndex++) {
            if ( DP_DEBUG_MASK & DP_DEBUG_I2C )
                printf("%2.2x ", data[startOffset + currentDataIndex]);
            write_reg(DPTX_BUF_DATA_0 + (currentDataIndex*sizeof(uint32_t)), 
                      data[startOffset + currentDataIndex]);
        }
        
        //Set I2C write cmd 0x04 mot = 1
        write_reg(DPTX_AUX_CH_CTL_1, ((currentDataCount - 1) << DPTX_AUX_CH_CTL_1_AUX_LENGTH_SHIFT) |
                  DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT | 
                  DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE);
        
        //Start Aux transaction
        ret = commit_aux_transaction(false, true, kMaxI2CTransactionRetry);
        if ( ret != 0 )
            break;
        
        startOffset += currentDataCount;
    }
    if ( DP_DEBUG_MASK & DP_DEBUG_I2C )
        printf("\n");
    
    if ( ret != 0 )
        goto exit;
    
    ret = stop_i2c_transaction(false);
    
exit:
    return ret;
}

int dp_controller_start_video(struct video_link_data *data)
{
#if WITH_HW_MCU
	struct video_link_data edid_data;
#endif
	struct video_link_data *timings = NULL;
    if ( !dp_started )
        return -1;
#if WITH_HW_MCU
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

    and_reg(DPTX_VIDEO_CTL_1, ~DPTX_VIDEO_CTL_1_VIDEO_EN);

    dp_video_started = false;

    return 0;
}

bool dp_controller_video_configured()
{
	return dp_video_started;
}

//Not supported 
bool dp_controller_get_supports_alpm(void)
{
	return false;
}

int dp_controller_enable_alpm(bool enable, struct video_link_data *data)
{
	return -1;
}

/////////////////////////////////////////
////////// controller local functions

static void handle_screensaver_callout(struct callout *co, void *args)
{
	debug(ALWAYS, "Samsung Screensaver\n");
	
	display_clear();
}

static int dp_controller_task(void *arg)
{
#if WITH_HW_BS
	// Perform this on the displayport task to prevent blocking startup.
# if PRODUCT_IBEC
    mcu_set_passthrough_mode(false);
# else
    mcu_set_passthrough_mode(true);
# endif
#endif
    
	// External display, we would like to put screensaver
	if (dp_controller_type == kDPControllerType_DP) {
        callout_enqueue(&dp_controller_screensaver_callout, kScreenBurnTimeout, handle_screensaver_callout, NULL);
	}	
	
	for (;;) {
		
		event_wait(&dp_controller_task_event);
        
		// if controller was stopped before dp task got chance to run, exit
		if (dp_started == false) {
			printf("dp not started\n");
			return 0;
		}
		
		mask_int(dp_config[dp_controller_type].irq);
		
		if (dp_interrupt_occurred) {
			dp_interrupt_occurred = false;
			
			if (dp_controller_type == kDPControllerType_DP) {
				callout_reset(&dp_controller_screensaver_callout, 0);
			}
			// Invoke bottom half.
			handle_interrupt();
		}
		
		unmask_int(dp_config[dp_controller_type].irq);
	}
    
	return 0;
}

static void init_hw()
{
    int i;
    u_int32_t reg_val;
	
    ////////////////
    // Analog init
    
    write_reg(DPTX_ANALOG_CTL_1, (DPTX_ANALOG_CTL_1_TX_TERM_CTL_45_OHM_VALUE<<DPTX_ANALOG_CTL_1_TX_TERM_CTL_SHIFT));
    write_reg(DPTX_ANALOG_CTL_2, DPTX_ANALOG_CTL_2_VALUE);
    write_reg(DPTX_ANALOG_CTL_3, DPTX_ANALOG_CTL_3_VALUE); 
    write_reg(DPTX_PLL_FILTER_CTL_1, DPTX_PLL_FILTER_CTL_1_PD_RING_OSC_PWR_DWN |
              DPTX_PLL_FILTER_CTL_1_AUX_TERM_CTL_52_OHM |
              DPTX_PLL_FILTER_CTL_1_TX_CUR_MULT_3 |
              DPTX_PLL_FILTER_CTL_1_TX_CUR_4mA);
    for (i = 0; i < kMaxLaneCount; i++) {
        set_voltage_base(i, 0);					// set to 0mV
        write_reg(DPTX_DP_LN0_LINK_TRAINING_CTL + (i<<2), 0);
    }
    
    if ( dp_voltage_boost ) 
        or_reg(DPTX_ANALOG_CTL_1, DPTX_ANALOG_CTL_1_TX_SWING_INC_30PER);
    else
        and_reg(DPTX_ANALOG_CTL_1, ~DPTX_ANALOG_CTL_1_TX_SWING_INC_30PER);
    
    ////////////////
    // Interrupts init
    
    write_reg(DPTX_COMMON_INT_MASK_1, 0);
#if DISPLAYPORT_VERSION < 3
    write_reg(DPTX_COMMON_INT_MASK_2, 0);
    write_reg(DPTX_COMMON_INT_MASK_3, 0);
#endif //DISPLAYPORT_VERSION
    write_reg(DPTX_COMMON_INT_MASK_4, 0);
    write_reg(DPTX_DP_INT_STA_MASK, 0);
    
    write_reg(DPTX_COMMON_INT_STA_1, 0xff);
#if DISPLAYPORT_VERSION < 3
    write_reg(DPTX_COMMON_INT_STA_2, 0xff);
    write_reg(DPTX_COMMON_INT_STA_3, 0xff);
#endif //DISPLAYPORT_VERSION
    write_reg(DPTX_COMMON_INT_STA_4, 0xff);
    write_reg(DPTX_DP_INT_STA, 0xff);
    
    ////////////////
    // more init 
    
    write_reg(DPTX_TRAINIG_DEBUG, 0); // reset
    
    ///////////////////////////
    // enable all SW functions
    
    and_reg(DPTX_FUNC_EN_1, ~( DPTX_FUNC_EN_1_SW_ALL));        
    
    ///////////////////////////
    // enable Analog functions
    
    // power up phy
    write_reg(DPTX_DP_PHY_PD, 0);
    // power down ch1 ch0
    reg_val = (DPTX_DP_PHY_CH1_PWR_DWN | DPTX_DP_PHY_CH0_PWR_DWN);
#if DISPLAYPORT_VERSION >= 2
    // power down ch3 ch2
    reg_val |= (DPTX_DP_PHY_CH3_PWR_DWN | DPTX_DP_PHY_CH2_PWR_DWN);
#endif // DISPLAYPORT_VERSION >= 2
#if DISPLAYPORT_VERSION > 2 && LOW_POWER_MODE == 1
	//to use 300mv we need to enable low-power mode
    reg_val |= DPTX_DP_PHY_SEL_LOWPOWER;
#endif // DISPLAYPORT_VERSION > 2 && LOW_POWER_MODE == 1
    write_reg(DPTX_DP_PHY_PD, reg_val);
	
    // clear PLL 
    write_reg(DPTX_COMMON_INT_STA_1, DPTX_COMMON_INT_1_PLL_LCK_CHG);
    and_reg(DPTX_DP_DEBUG_CTL, ~(DPTX_DP_DEBUG_CTL_F_PLL_LOCK | DPTX_DP_DEBUG_CTL_PLL_LOCK_CTRL));
    
    // power up the PLL
    uint8_t pllLockState;
    while(((pllLockState = read_reg(DPTX_DP_DEBUG_CTL)) & DPTX_DP_DEBUG_CTL_PLL_LOCK) == 0)
        debug(INFO, "before pllLockState=%d\n", pllLockState);
    
    and_reg(DPTX_DP_PLL_CTL, ~(DPTX_DP_PLL_CTL_PWR_DN));
    debug(INFO, "powered up DP PLL lockstate=%d\n", pllLockState);
    
    while(((pllLockState = read_reg(DPTX_DP_DEBUG_CTL)) & DPTX_DP_DEBUG_CTL_PLL_LOCK) == 0)
        debug(INFO, "after pllLockState=%d\n", pllLockState);
    
    and_reg(DPTX_FUNC_EN_2, ~(DPTX_FUNC_EN_2_SERDES_FIFO | 
                              DPTX_FUNC_EN_2_LS_CLK_DOMAIN | 
                              DPTX_FUNC_EN_2_AUX));
    
    ////////////////
    // enable Aux
    
    // power off aux to reset
    or_reg(DPTX_FUNC_EN_2, DPTX_FUNC_EN_2_AUX);
    
    // configure channel defer
    write_reg(DPTX_AUX_CH_DEFER_CTL, DPTX_AUX_CH_DEFER_CTL_EN | (0x0<<DPTX_AUX_CH_DEFER_CTL_DEFER_COUNT_SHIFT));

#if WITH_HW_DISPLAY_EDP
    write_reg(DPTX_AUX_HW_RETRY_CTL, (5 << DPTX_AUX_HW_RETRY_BIT_PERIOD_DELAY_SHIFT) | DPTX_AUX_HW_RETRY_INTERVAL_600us | 7);
#else
    write_reg(DPTX_AUX_HW_RETRY_CTL, (5 << DPTX_AUX_HW_RETRY_BIT_PERIOD_DELAY_SHIFT) | DPTX_AUX_HW_RETRY_INTERVAL_600us);
#endif 
    // light it back up
    and_reg(DPTX_FUNC_EN_2, ~(DPTX_FUNC_EN_2_AUX));
}

static void interrupt_filter(void *arg)
{
    dp_controller_common_sta_4_val = read_reg(DPTX_COMMON_INT_STA_4);
    write_reg(DPTX_COMMON_INT_STA_4, dp_controller_common_sta_4_val);
    
#if DISPLAYPORT_VERSION < 3
    dp_controller_common_sta_3_val = read_reg(DPTX_COMMON_INT_STA_3);
    write_reg(DPTX_COMMON_INT_STA_3, dp_controller_common_sta_3_val);
    
    dp_controller_common_sta_2_val = read_reg(DPTX_COMMON_INT_STA_2);
    write_reg(DPTX_COMMON_INT_STA_2, dp_controller_common_sta_2_val);
#endif //DISPLAYPORT_VERSION
    
    dp_controller_common_sta_1_val = read_reg(DPTX_COMMON_INT_STA_1);
    write_reg(DPTX_COMMON_INT_STA_1, dp_controller_common_sta_1_val);
    
    dp_controller_sta_val = read_reg(DPTX_DP_INT_STA);
    write_reg(DPTX_DP_INT_STA, dp_controller_sta_val);
    
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
        
        if ( (read_reg(DPTX_AUX_CH_CTL_2) & DPTX_AUX_CH_CTL_2_AUX_EN) == 0 ) {
            
            if ( DPTX_DP_INT_STA_RPLY_RCV & sta_val ) {
                dp_aux_transaction_status = kDPAuxTranscationStatus_Success;
                debug(AUX, "waking pending dp_aux_transaction_status\n");
                // wake up dp_aux_transaction_status
            }
            
            if ( DPTX_DP_INT_STA_AUX_ERR & sta_val ) {
                uint8_t aux_err = read_reg(DPTX_AUX_CH_STA) & DPTX_AUX_CH_STA_STATUS_MASK;
                dp_aux_transaction_status = ( aux_err == DPTX_AUX_CH_STA_DEFER ) ? kDPAuxTranscationStatus_IODefer : kDPAuxTranscationStatus_IOError;
                debug(AUX, "waking pending dp_aux_transaction_status\n");
                // wake up dp_aux_transaction_status
            }
        }
    }
    
    if ( DPTX_DP_INT_STA_INT_HPD & sta_val ) {
        debug(INT, "DPTX_DP_INT_STA_INT_HPD\n");
    }
    
#if DISPLAYPORT_VERSION < 3
    if ( DPTX_COMMON_INT_2_HDCP_CHG & common_sta_2_val ) {
        debug(INT, "DPTX_COMMON_INT_2_HDCP_CHG\n");
    }
    
    if ( DPTX_COMMON_INT_2_HW_AUTH_CHG & common_sta_2_val ) {        
        debug(INT, "Auth Change, HDCP_STATUS=0x%02X\n", read_reg(DPTX_HDCP_STA));
    }
    
    if ( DPTX_COMMON_INT_2_HW_AUTH_DONE & common_sta_2_val ) {
        debug(INT, "DPTX_COMMON_INT_2_HW_AUTH_DONE\n");
    }
#endif //DISPLAYPORT_VERSION
    
    if ( DPTX_COMMON_INT_4_HP_CHG & common_sta_4_val ) {
        debug(INT, "DPTX_SYS_CTL_3=%d\n", read_reg(DPTX_SYS_CTL_3));
    }
    
    if ( DPTX_COMMON_INT_4_HPD_LOST & common_sta_4_val ) {
        if ( !(read_reg(DPTX_SYS_CTL_3) & DPTX_SYS_CTL_3_HPD_STATUS) ) {
            dp_device_stop();
        }
    } 
    
    if ( DPTX_COMMON_INT_4_HP_CHG & common_sta_4_val ) {
        if ( read_reg(DPTX_SYS_CTL_3) & DPTX_SYS_CTL_3_HPD_STATUS ) {
			event_signal(&dp_controller_hpd_event);
            dp_device_start((dp_controller_type == kDPControllerType_EDP) ? true : false);
        }
    }
    
}

static int set_voltage_base(u_int32_t lane, int32_t base)
{
	u_int8_t voltage = kBaseVoltageType_Pos_0mV;
	u_int8_t shift;
	
	debug(INFO, "set lane %d voltage base=%d\n", lane, base);
    
	shift = 0;
	
	switch (base) {
		case -120:
			voltage = kBaseVoltageType_Neg_120mV;
			break;
		case -80:
			voltage = kBaseVoltageType_Neg_80mV;
			break;
		case -40:
			voltage = kBaseVoltageType_Neg_40mV;
			break;
		case 0:
			voltage = kBaseVoltageType_Pos_0mV;
			break;
		case 40:
			voltage = kBaseVoltageType_Pos_40mV;
			break;
		case 80:
			voltage = kBaseVoltageType_Pos_80mV;
			break;
		case 120:
			voltage = kBaseVoltageType_Pos_120mV;
			break;
		case 160:
			voltage = kBaseVoltageType_Pos_160mV;
			break;
	}
    
	dp_voltage_base[lane] = voltage;
    
#if DISPLAYPORT_VERSION < 3
	switch (lane) {
		case 0:
			shift = DPTX_PLL_FILTER_CTL_2_CH0_AMP_SHIFT;
			break;
		case 1:
			shift = DPTX_PLL_FILTER_CTL_2_CH1_AMP_SHIFT;
			break;
#if DISPLAYPORT_VERSION >= 2
		case 2:
			shift = DPTX_PLL_FILTER_CTL_2_CH2_AMP_SHIFT;
			break;
		case 3:
			shift = DPTX_PLL_FILTER_CTL_2_CH3_AMP_SHIFT;
			break;
#endif // DISPLAYPORT_VERSION >= 2				
	}
	
	set_bits_in_reg(DPTX_PLL_FILTER_CTL_2, shift, 0x7 << shift, voltage);
#endif //DISPLAYPORT_VERSION < 3
    
    // pre charge delay
    spin(10);
    
	return 0;
}

static int configure_video(struct video_link_data *data)
{
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
			debug(ALWAYS, "    total: %u\n", data->timing.axis[i].total);
			debug(ALWAYS, "    active: %u\n", data->timing.axis[i].active);
			debug(ALWAYS, "    sync_width: %u\n", data->timing.axis[i].sync_width);
			debug(ALWAYS, "    back_porch: %u\n", data->timing.axis[i].back_porch);
			debug(ALWAYS, "    front_porch: %u\n", data->timing.axis[i].front_porch);
			debug(ALWAYS, "    sync_rate: %u (%uHz)\n",
			      data->timing.axis[i].sync_rate, data->timing.axis[i].sync_rate >> 16);
			debug(ALWAYS, "    sync_polarity: %u\n", data->timing.axis[i].sync_polarity);
		}
	}
#endif
    
    init_video((data->mirror_mode ? false : true));
    
    if ( configure_video_mode(data) != 0 )
        goto exit;
    
    if ( configure_video_color(data) != 0 )
        goto exit;
    
    configure_video_bist(data);
    
    uint8_t pllLockState;
    while(((pllLockState = read_reg(DPTX_DP_DEBUG_CTL)) & DPTX_DP_DEBUG_CTL_PLL_LOCK) == 0)
        debug(PLL, "before pllLockState=%d\n", pllLockState);
    
    configure_video_m_n(data);
    
    configure_video_mute(data, false);
    
    configure_video_timing_mode(data);
    
	// Setup InfoFrame
	// RY:Please refer to page 65 of the CEA861 spec
	write_reg(DPTX_AVI_DB1, 0);
	write_reg(DPTX_AVI_DB2, 0x08);
	write_reg(DPTX_AVI_DB3, 0x00);
	write_reg(DPTX_AVI_DB4, 0x00);
	write_reg(DPTX_AVI_DB5, 0x00);
	write_reg(DPTX_AVI_DB6, 0x00);
	write_reg(DPTX_AVI_DB7, 0x00);
	write_reg(DPTX_AVI_DB8, 0x00);
	write_reg(DPTX_AVI_DB9, 0x00);
	write_reg(DPTX_AVI_DB10, 0x00);
	write_reg(DPTX_AVI_DB11, 0x00);
	write_reg(DPTX_AVI_DB12, 0x00);
	write_reg(DPTX_AVI_DB13, 0x00);    
	or_reg(DPTX_PKT_SEND_CTL, DPTX_BF_C_AVI_EN | DPTX_BF_C_AVI_UP);
    
    // start video
    or_reg(DPTX_VIDEO_CTL_1, DPTX_VIDEO_CTL_1_VIDEO_EN);
    
	dp_video_started = true;
    
    debug(VIDEO, "finished configuring video\n");
    
    return 0;
    
exit:
    debug(ERROR, "failed to configure video\n");
	dp_video_started = false;
    return -1;
}

static void init_video(bool master)
{
    // configure video
    write_reg(DPTX_SYS_CTL_1, 0); // clear force_det and det_ctrl
    write_reg(DPTX_VIDEO_CTL_8, (2 << 4) | (0 << 0)); // vid_hres_th and vid_vres_th
    write_reg(DPTX_SYS_CTL_2, (0xf << 4));	// clear cha_sta, force_cha and cha_ctrl
    write_reg(DPTX_VIDEO_CTL_10, 0);
    
    and_reg(DPTX_SYS_CTL_3, ~(DPTX_SYS_CTL_3_F_VALID | DPTX_SYS_CTL_3_VALID_CTRL));
    
#if DISPLAYPORT_VERSION == 0        
    
    // enable vid capture and vid fifo
    and_reg(DPTX_FUNC_EN_1, ~(DPTX_FUNC_EN_1_VIDCAP | DPTX_FUNC_EN_1_VIDFIF0));
    
#elif DISPLAYPORT_VERSION >= 1
    
    if ( !master ) {
        and_reg(DPTX_FUNC_EN_1, ~(DPTX_FUNC_EN_1_SLAVE));
        or_reg(DPTX_FUNC_EN_1, DPTX_FUNC_EN_1_MASTER);
    } else {
        and_reg(DPTX_FUNC_EN_1, ~(DPTX_FUNC_EN_1_MASTER));
        or_reg(DPTX_FUNC_EN_1, DPTX_FUNC_EN_1_SLAVE);
    }
    
#endif // DISPLAYPORT_VERSION
}

static void configure_video_timing_mode(struct video_link_data *data)
{
    if ( data->mirror_mode ) {
        // enable video slave mode
        or_reg(DPTX_SOC_GENERAL_CTL, DPTX_SOC_GENERAL_CTL_VIDEO_MODE); // slave mode
        and_reg(DPTX_SOC_GENERAL_CTL, ~(DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_MODE_EN)); // enable video timing generation
    }
    else {
        // enable video master mode
        and_reg(DPTX_SOC_GENERAL_CTL, ~(DPTX_SOC_GENERAL_CTL_VIDEO_MODE)); // master mode
        or_reg(DPTX_SOC_GENERAL_CTL, (DPTX_SOC_GENERAL_CTL_VIDEO_MASTER_MODE_EN)); // enable video timing generation
    }
}

static void configure_video_mute(struct video_link_data *data, bool mute)
{
    if ( mute )
        or_reg(DPTX_VIDEO_CTL_1, DPTX_VIDEO_CTL_1_VIDEO_MUTE);
    else 
        and_reg(DPTX_VIDEO_CTL_1, ~(DPTX_VIDEO_CTL_1_VIDEO_MUTE));
}

static void configure_video_m_n(struct video_link_data *data)
{
    // using register calcuated M/N
    // we are defaulting to operating in master mode
    // therefore we are ensuring that async clock
    // is being used to calculate M/N
    and_reg(DPTX_SYS_CTL_4, ~(DPTX_SYS_CTL_4_FIX_M_VID)); 
    write_reg(DPTX_N_VID_0, 0);
    write_reg(DPTX_N_VID_1, 0x80);
    write_reg(DPTX_N_VID_2, 0);
}

static int configure_video_color(struct video_link_data *data)
{
    uint32_t val;
    
    // set video color format
    switch (data->color.depth) {
        case 12:
            val = DPTX_VIDEO_CTL_2_IN_BPC_12BIT;
            break;
        case 10:
            val = DPTX_VIDEO_CTL_2_IN_BPC_10BIT;
            break;
        case 8:
            val = DPTX_VIDEO_CTL_2_IN_BPC_8BIT;
            break;
        case 6:
            val = DPTX_VIDEO_CTL_2_IN_BPC_6BIT;
            break;
        default:
            return -1;
    }
    
    // Set color range / space
    write_reg(DPTX_VIDEO_CTL_2, (data->color.range << DPTX_VIDEO_CTL_2_IN_D_RANGE_SHIFT) | 
              val | 
              data->color.space);
    
    // set color coeff
    set_bits_in_reg(DPTX_VIDEO_CTL_3, DPTX_VIDEO_CTL_3_IN_YC_COEFFI_SHIFT, 
                    DPTX_VIDEO_CTL_3_IN_YC_COEFFI_MASK, data->color.coefficient);

    return 0;
}

static int configure_video_mode(struct video_link_data *data)
{
    if ( data->mirror_mode ) {
        debug(VIDEO, "configuring mirror mode (as slave)\n");
        
        //Configure Interlaced for slave mode video
        if ( data->timing.interlaced )
            or_reg(DPTX_VIDEO_CTL_10, DPTX_VIDEO_CTL_10_SLAVE_I_SCAN_CFG);
        else
            and_reg(DPTX_VIDEO_CTL_10, ~DPTX_VIDEO_CTL_10_SLAVE_I_SCAN_CFG);
    }
    else {
        uint32_t        mVid;
        uint32_t        nVid;
        
        debug(VIDEO, "configuring video as master\n");
        
        // The register is inverted: 0->positive, 1->negative sync.
        if ( data->timing.axis[kDisplayAxisTypeVertical].sync_polarity )
            and_reg(DPTX_VIDEO_CTL_10, ~DPTX_VIDEO_CTL_10_VSYNC_P_CFG);
        else
            or_reg(DPTX_VIDEO_CTL_10, DPTX_VIDEO_CTL_10_VSYNC_P_CFG);
        
        if ( data->timing.axis[kDisplayAxisTypeHorizontal].sync_polarity )
            and_reg(DPTX_VIDEO_CTL_10, ~DPTX_VIDEO_CTL_10_HSYNC_P_CFG);
        else
            or_reg(DPTX_VIDEO_CTL_10, DPTX_VIDEO_CTL_10_HSYNC_P_CFG);
        
        if ( validate_video(data, &mVid, &nVid) != 0 )
            return -1;
        
        // setup video format
        write_reg(DPTX_H_TOTAL_MASTER,   data->timing.axis[kDisplayAxisTypeHorizontal].total);
        write_reg(DPTX_H_F_PORCH_MASTER, data->timing.axis[kDisplayAxisTypeHorizontal].front_porch);
        write_reg(DPTX_H_B_PORCH_MASTER, data->timing.axis[kDisplayAxisTypeHorizontal].back_porch);
        write_reg(DPTX_H_ACTIVE_MASTER,  data->timing.axis[kDisplayAxisTypeHorizontal].active);
        
        write_reg(DPTX_V_TOTAL_MASTER,   data->timing.axis[kDisplayAxisTypeVertical].total);
        write_reg(DPTX_V_F_PORCH_MASTER, data->timing.axis[kDisplayAxisTypeVertical].front_porch);
        write_reg(DPTX_V_B_PORCH_MASTER, data->timing.axis[kDisplayAxisTypeVertical].back_porch);
        write_reg(DPTX_V_ACTIVE_MASTER,  data->timing.axis[kDisplayAxisTypeVertical].active);
        
        if ( data->timing.interlaced ) {
            or_reg(DPTX_SOC_GENERAL_CTL, DPTX_SOC_GENERAL_CTL_MASTER_VIDEO_INTERLACE_EN);
        } else {
            and_reg(DPTX_SOC_GENERAL_CTL, ~(DPTX_SOC_GENERAL_CTL_MASTER_VIDEO_INTERLACE_EN));
        }
        
        write_reg(DPTX_M_VID_MASTER, mVid);
        write_reg(DPTX_N_VID_MASTER, nVid);
        write_reg(DPTX_M_VID_GEN_FILTER_TH, 0);  // Video Filter Thresold
        and_reg(DPTX_DP_M_CAL_CTL, ~(1 << 2));   // disable low-pass filter for m-vid
        // enable this if we have filter thresold set
    }
    
    return 0;        
}

static int validate_video(struct video_link_data *data, u_int32_t *m_vid, u_int32_t *n_vid)
{
    uint32_t        lr;
    uint32_t        lc;
    uint64_t        streamClk, maxStreamClk;
    uint32_t        bpp;
    uint32_t        vTotalBppPerFrame;
    uint64_t        maxAvailableVSyncRate;
    uint32_t        mVid;
    uint32_t        nVid;
    
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
    lr = read_reg(DPTX_LINK_BW_SET);
    lc = read_reg(DPTX_LANE_COUNT_SET);
    
	if (lc == 0) {
		debug(ERROR, "Lane count programmed to zero\n");
		return -1;
	}
    
    if (lr == kLinkRate270Gbps)
        maxAvailableVSyncRate = kLinkRatePhysical_270gps;
    else                        
        maxAvailableVSyncRate = kLinkRatePhysical_162gbps;
    
    maxAvailableVSyncRate /= 10;
    maxAvailableVSyncRate *= 8;
    maxAvailableVSyncRate *= lc;
    maxAvailableVSyncRate /= vTotalBppPerFrame;
    
    // setup M and N, master mode
    switch (lr) {
        case kLinkRate270Gbps:
#if DISPLAYPORT_VERSION == 0
            maxStreamClk = kLSClock_135mhz;
            nVid = kLSClock_135mhz;
#elif DISPLAYPORT_VERSION >= 1
            maxStreamClk = kLSClock_171mhz;
            nVid = kLSClock_171mhz;
#endif // DISPLAYPORT_VERSION
            break;
        case kLinkRate162Gbps:
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
        write_reg(DPTX_VIDEO_CTL_4, DPTX_VIDEO_CTL_4_BIST_EN | 
                  ((data->test_mode - 1) & DPTX_VIDEO_CTL_4_BIST_TYPE_MASK));
        debug(VIDEO, "Finished Video BIST\n");
    } else {
        and_reg(DPTX_VIDEO_CTL_4, ~(DPTX_VIDEO_CTL_4_BIST_EN));
    }
}

static int read_bytes_i2c_internal(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length)
{
    int         ret             = 0;
    uint32_t    startOffset     = 0;
    uint32_t    attemptCount    = 0;
    uint32_t    maxAttempts     = kMaxI2CTransactionRetry + 1;
    
        debug(I2C, "device_addr=0x%08x addr=0x%08x data=%p length=%d payload=\n", device_addr, addr, 
          data, length);
    
    // Since we don't allow any retries on commitAuxTransaction, we should really
    // retry on the whole transaction, which includes: start, commmit, and stop
    while ( attemptCount++ < maxAttempts ) {
        int commitRet = 0;

        if ( ret != 0 ) {
            debug(I2C, "waiting %dus till next attempt\n", kAUXRetryIntervalUS);
            task_sleep(kAUXRetryIntervalUS);
        }

        ret = start_i2c_transaction(device_addr, addr);
        if ( ret != 0 )
            goto check;
        
        while (startOffset < length) {
            uint32_t actualDataCount;
            uint32_t currentDataCount;
            uint32_t currentDataIndex;
            
            currentDataCount = ((length - startOffset) > DPTX_BUF_DATA_COUNT) ? 
            DPTX_BUF_DATA_COUNT : (length - startOffset);
            
            //Set I2C write cmd 0x04 mot = 1        
                debug(I2C, "startOffset=%d requesting=%d bytes\n", startOffset, currentDataCount);
            
            write_reg(DPTX_AUX_CH_CTL_1, ((currentDataCount - 1) << DPTX_AUX_CH_CTL_1_AUX_LENGTH_SHIFT) |
                      DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT | 
                      DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ);
            
            write_reg(DPTX_BUFFER_DATA_CTL, DPTX_BUFFER_DATA_CTL_CLR); 
            
            //Start Aux transaction
            commitRet = commit_aux_transaction(false, true, kMaxAuxTransactionRetry);
            if ( commitRet != 0 )
                break;
            
            actualDataCount = read_reg(DPTX_BUFFER_DATA_CTL) & 0x1f;
            debug(I2C, "i2C data length %d, expecting %d\n", actualDataCount, currentDataCount);
            debug(I2C, "Bytes: ");
            if ( currentDataCount != actualDataCount ) {
                commitRet = kDPAuxTranscationStatus_IOError;
                debug(I2C, "Short RX!!!\n");
                break;
            }
            
            for(currentDataIndex = 0; currentDataIndex < currentDataCount; currentDataIndex++) {
                data[startOffset + currentDataIndex] = (uint8_t)read_reg(DPTX_BUF_DATA_0 + (currentDataIndex*sizeof(uint32_t)));
                if ( DP_DEBUG_MASK & DP_DEBUG_I2C )
                    printf("%02x ", data[startOffset + currentDataIndex]);
            }
            if ( DP_DEBUG_MASK & DP_DEBUG_I2C )
                printf("\n");
            
            startOffset += currentDataCount;
        }
        
    check:
        // need to call stop regardless of transaction result
        ret = stop_i2c_transaction(true);
        // if stop and commit was successful, break
        // if stop was successfull, but commit was not, assign commit return value to ret
        if ( ret == 0 ) {
            if ( commitRet == 0 )
                break;
            
            ret = commitRet;
        }
    }

    debug(I2C, "result=%d\n", ret);
    return ret;
}

static int wait_for_aux_done()
{
	uint32_t timeout_time = 20;
	while (timeout_time-- && (read_reg(DPTX_AUX_CH_STA) & DPTX_AUX_CH_STA_BUSY))
		task_sleep(50);
	return (timeout_time == 0) ? -1 : 0;
}

static int commit_aux_transaction(bool useInterrupts, bool deferRetryOnly, u_int32_t maxRetryCount)
{
    uint32_t    retryCount = 0;
    int         ret;
    
    ret = -1;
    
    if ( useInterrupts ) {
        debug(AUX, "dp_aux_transaction_pending=%d sleeping\n", dp_aux_transaction_pending);
        
        // sleep on dp_aux_transaction_pending
        while( dp_aux_transaction_pending )
            task_yield();
        
        debug(AUX, "dp_aux_transaction_pending=%d waking\n", dp_aux_transaction_pending);
        
        dp_aux_transaction_pending = true;
    }
    
    for ( retryCount=0; retryCount<= maxRetryCount; retryCount++ ) {

        if ( retryCount ) {
            debug(AUX, "waiting %dus till next attempt\n", kAUXRetryIntervalUS);
            task_sleep(kAUXRetryIntervalUS);
        }

        ret = wait_for_aux_done();
        if ( ret != 0 ) {
            debug(ERROR, "Timeout waiting for AUX done\n");
            continue;
        }

        // enable interrupt
        or_reg(DPTX_DP_INT_STA, DPTX_DP_INT_STA_RPLY_RCV|DPTX_DP_INT_STA_AUX_ERR);
        or_reg(DPTX_DP_INT_STA_MASK, DPTX_DP_INT_STA_RPLY_RCV|DPTX_DP_INT_STA_AUX_ERR);
        
        // start aux
        or_reg(DPTX_AUX_CH_CTL_2, DPTX_AUX_CH_CTL_2_AUX_EN);
        
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
                int_sta = read_reg(DPTX_DP_INT_STA) & (DPTX_DP_INT_STA_RPLY_RCV | DPTX_DP_INT_STA_AUX_ERR);
                write_reg(DPTX_DP_INT_STA, int_sta);
                if (int_sta != 0 && (read_reg(DPTX_AUX_CH_CTL_2) & DPTX_AUX_CH_CTL_2_AUX_EN) == 0)
                    break;
                task_yield();
            }
            
            if (dp_controller_type == kDPControllerType_DP) {
                callout_reset(&dp_controller_screensaver_callout, 0);
            }
            
            if (int_sta & DPTX_DP_INT_STA_AUX_ERR) {
                uint8_t aux_err = read_reg(DPTX_AUX_CH_STA) & DPTX_AUX_CH_STA_STATUS_MASK;
                debug(AUX, "AUX_STATUS error: %d\n", aux_err);

                dp_aux_transaction_status = ( aux_err == DPTX_AUX_CH_STA_DEFER ) ? kDPAuxTranscationStatus_IODefer : kDPAuxTranscationStatus_IOError;

            } else {
                dp_aux_transaction_status = kDPAuxTranscationStatus_Success;
            }
        }
        
        // make sure the aux channel is not busy
        ret = wait_for_aux_done();
        if ( ret != 0 ) {
            and_reg(DPTX_AUX_CH_CTL_2, ~DPTX_AUX_CH_CTL_2_AUX_EN);
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
    or_reg(DPTX_DP_INT_STA, DPTX_DP_INT_STA_RPLY_RCV|DPTX_DP_INT_STA_AUX_ERR);
    and_reg(DPTX_DP_INT_STA_MASK, ~(DPTX_DP_INT_STA_RPLY_RCV|DPTX_DP_INT_STA_AUX_ERR));
    
    if ( useInterrupts ) {
        dp_aux_transaction_pending = false;
        // wake up dp_aux_transaction_pending
        debug(AUX, "waking pending dp_aux_transaction_pending\n");
    }
    
	if ( ret == kDPAuxTranscationStatus_IOError)
		debug(AUX, "AUX transaction failed\n");
    
    return ret;
}

static int start_i2c_transaction(uint32_t device_addr, uint32_t addr)
{
    //Clear Buffer
    write_reg(DPTX_BUFFER_DATA_CTL, DPTX_BUFFER_DATA_CTL_CLR); 
    
    write_reg(DPTX_AUX_CH_CTL_2, 0);
    
    write_reg(DPTX_BUFFER_DATA_CTL, DPTX_BUFFER_DATA_CTL_CLR); 
    
    //Select EDID device address
    write_reg(DPTX_AUX_ADDR_7_0, (device_addr) & 0xFF); 
    write_reg(DPTX_AUX_ADDR_15_8, ((device_addr)>>8) & 0xFF);
    write_reg(DPTX_AUX_ADDR_19_16, ((device_addr)>>16) & 0x0F);
    
    //Set reg addr of I2C device.
    write_reg(DPTX_BUF_DATA_0, addr); 
    
    //Set I2C write com 0x04 mot = 1
    write_reg(DPTX_AUX_CH_CTL_1, DPTX_AUX_CH_CTL_1_AUX_TX_COMM_MOT | 
              DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE);
    
    //Start Aux transaction
    return commit_aux_transaction(false, false, kMaxAuxTransactionRetry);
}

static int stop_i2c_transaction(bool read)
{
    int ret;
    
    //Turn off MOT bit
    write_reg(DPTX_AUX_CH_CTL_1, read ? 
              DPTX_AUX_CH_CTL_1_AUX_TX_COMM_READ : DPTX_AUX_CH_CTL_1_AUX_TX_COMM_WRITE);
    
    //Set address only transaction bit
    write_reg(DPTX_AUX_CH_CTL_2, DPTX_AUX_CH_CTL_2_AUX_ADDR_ONLY);
    
    //Terminate Aux transaction
    ret = commit_aux_transaction(false, false, kMaxAuxTransactionRetry);
    if ( ret != 0 )
        goto exit;
    
    //Clear address only transaction bit
    write_reg(DPTX_AUX_CH_CTL_2, 0);
    
exit:
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
	*(volatile uint32_t *)(__base_address + offset) &= value;
}

static void or_reg(uint32_t offset, uint32_t value)
{
	*(volatile uint32_t *)(__base_address + offset) |= value;
}

static void set_bits_in_reg(u_int32_t offset, u_int32_t pos, u_int32_t mask, u_int32_t value)
{
    uint32_t set = read_reg(offset);
    
    set &= ~mask;
    set |= (value << pos);
    
    write_reg(offset, set);
}
