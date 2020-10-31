/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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
#include <platform/gpiodef.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/soc/hwregbase.h>
#include <platform/soc/pmgr.h>
#include <sys.h>
#include <sys/task.h>
#include <sys/callout.h>
#include <lib/devicetree.h>

#include <drivers/display.h>
#include <drivers/hdmi.h>
#include "hdmi.h"
#include "regs.h"

#include <drivers/process_edid.h>
#include <drivers/iic.h>


/////////////////////////////////////////
////////// debug support

#define HDMI_DEBUG_MASK (		\
		HDMI_DEBUG_INIT |       \
		HDMI_DEBUG_ERROR |	\
		HDMI_DEBUG_INFO |	\
		HDMI_DEBUG_INT |	\
		HDMI_DEBUG_VIDEO |	\
		HDMI_DEBUG_PLL |	\
		HDMI_DEBUG_REG |	\
		0)

#undef HDMI_DEBUG_MASK
#define HDMI_DEBUG_MASK	(HDMI_DEBUG_INIT | HDMI_DEBUG_ERROR)

#define HDMI_DEBUG_INIT           (1<<16)  // initialisation
#define HDMI_DEBUG_ERROR          (1<<17)  // errors
#define HDMI_DEBUG_INFO           (1<<18)  // info
#define HDMI_DEBUG_INT            (1<<19)  // interrupts
#define HDMI_DEBUG_VIDEO          (1<<20)  // video
#define	HDMI_DEBUG_REG		  (1<<22)  // register
#define HDMI_DEBUG_I2C            (1<<23)  // i2c read/write
#define HDMI_DEBUG_PLL            (1<<24)  // PLL 
#define HDMI_DEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)								\
	do {											\
		if ((HDMI_DEBUG_ ## _fac) & (HDMI_DEBUG_MASK | HDMI_DEBUG_ALWAYS))		\
			dprintf(DEBUG_CRITICAL, "HDMI: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);\
	} while(0)

/////////////////////////////////////////
////////// consts, macros

#define HDMI_DTPATH       "arm-io/hdmi"

// Crude screensaver: Turn off hdmi after 3 minutes.
#define kScreenBurnTimeout	    (3 * 60 * 1000000)
#define kTMDSMinimumPowerOffIntervalMS  100
#define kHDMICoreResetIntervalMS        1               // Core reset asserted time: Arbitrary large value
#define kPHYResetIntervalMS             1               // PHY reset signal high time: Minimum 100 microseconds per HDMI 1.4 PHY manual

#define kHDMIStackSize       8192

#define require(assertion, exception_label) \
        do {                                            \
                if (__builtin_expect(!(assertion), 0))  \
                {                                       \
                        goto exception_label;           \
                }                                       \
        } while (0)
        
#define require_action(assertion, exception_label, action) \
        do {                                            \
                if (__builtin_expect(!(assertion), 0))  \
                {                                       \
                        {                               \
                                action;                 \
                        }                               \
                        goto exception_label;           \
                }                                       \
        } while (0)
        
#define require_noerr(error_code, exception_label) \
        do {                                                    \
                if (__builtin_expect(0 != (error_code), 0))     \
                {                                               \
                        goto exception_label;                   \
                }                                               \
        } while (0)
        
#define require_noerr_action(error_code, exception_label, action) \
        do {                                                    \
                if (__builtin_expect(0 != (error_code), 0))     \
                {                                               \
                        {                                       \
                                action;                         \
                        }                                       \
                        goto exception_label;                   \
                }                                               \
        } while (0)



/////////////////////////////////////////
////////// typedefs, enums, structs

uint32_t _registers[] = {
	0x00000000,
	0x00010000,
	0x00030000,
	0x00040000,
	0x00050000,
	0x00060000,
	0x00070000,
	0x00017000,
	0x00080000,
};

uint32_t _phyConfigRegs[kPHYConfigRegCount/4 + 1] = {
	HDMI_BASE_ADDR + HDMIPHYCON0,
	HDMI_BASE_ADDR + HDMIPHYCON1,
	HDMI_BASE_ADDR + HDMIPHYCON2,
	HDMI_BASE_ADDR + HDMIPHYCON3,
	HDMI_BASE_ADDR + HDMIPHYCON4,
	HDMI_BASE_ADDR + HDMIPHYCON5,
	HDMI_BASE_ADDR + HDMIPHYCON6,
	HDMI_BASE_ADDR + HDMIPHYCON7,
	HDMI_BASE_ADDR + HDMIPHYCON8,
};

enum Module {
	kCtrlModule	= 0,    // CTRL_BASE:       Controller
	kHDMIModule	= 1,    // HDMI_CORE_BASE:  HDMI Core
	kSPDIFModule	= 2,   // SPDIF_BASE:      SPDIF Receiver
	kI2SModule	= 3,     // I2S_BASE:        I2S Receiver
	kTGModule	= 4,      // TG_BASE:         Timing Generator
	kEFuseModule	= 5,   // EFUSE_BASE:      E-Fuse
	kCECModule	= 6,     // CEC_BASE:        HDMI CEC
	kHDCPModule	= 7,    //                  HDCP Registers
	kI2CModule	= 8,     // I2C_BASE:        I2C to HDMI PHY
	kModuleCount	= 9
};


/////////////////////////////////////////
////////// local variables

static const char hdmi_dt_path[]  = HDMI_DTPATH;

static addr_t __base_address;

static AppleSamsungHDMITXPHYConfigTableEntry *hdmi_port_phy_cfg_table;
size_t hdmi_port_phy_cfg_table_size;

const HDMITXPHYConfig *phyConfig;

uint32_t frame_interval_MS;
struct video_link_data _current_video_link;

static bool hdmi_started;
static bool hdmi_video_started;

static u_int8_t hdmi_controller_type;
static u_int8_t hdmi_controller_mode;

static struct task_event hdmi_controller_task_event;

static struct callout hdmi_controller_screensaver_callout;

static bool hdmi_interrupt_occurred;
static uint32_t ctrl_int_status;
static uint32_t iic_int_status;

/////////////////////////////////////////
////////// local functions declaration

static int hdmi_controller_task(void *arg);
static int hdmi_phy_regs_task(void *arg);
static int configure_video(struct video_link_data *data, uint32_t pixel_clock);
static void init_video(bool master);
static void configure_video_mute(struct video_link_data *data, bool mute);
static int configure_video_color(const struct video_link_data * data);
static int configure_video_mode(struct video_link_data *data, uint32_t pixel_clock);
static void configure_video_bist(struct video_link_data *data);
static void hdmi_interrupt_filter(void *arg);
static void handle_interrupt(void);
static void uninit_video();
static int reset(void);
static void power_down_phy(bool fullPowerDown);
static int validate_video_link(struct video_link_data * data, uint32_t pixel_clock, const HDMITXPHYConfig ** pPHYConfig);

static int set_tx_output_enabled(bool enabled);
static int set_tx_output_mode(int mode);

static void handle_screensaver_callout(struct callout *co, void *args);

void set_av_mute_enabled(bool enabled);

static uint8_t read_reg(uint8_t module, u_int32_t offset);
static uint32_t read_reg32(uint8_t module, u_int32_t offset);
static void write_reg(uint8_t module, u_int32_t offset, uint8_t val);
static void write_reg32(uint8_t module, u_int32_t offset, uint32_t val);
static void and_reg(uint8_t module, u_int32_t offset, uint8_t val);
static void or_reg(uint8_t module, u_int32_t offset, uint8_t val);
static void set_bits_in_reg(uint8_t module, u_int32_t offset, uint8_t pos, uint8_t mask, uint8_t value);
static uint32_t get_bits_in_reg(uint8_t module, uint32_t reg, uint32_t pos, uint32_t mask);
static uint8_t read_phy_reg(uint8_t offset);
static int write_phy_reg(u_int32_t offset, const uint8_t data);
static void or_phy_reg(u_int32_t offset, uint8_t data);
static void and_phy_reg(u_int32_t offset, uint8_t data);
void write_regs(uint8_t module, uint32_t basereg, uint32_t, unsigned int length);
void write_regs_with_data(uint8_t module, uint32_t basereg, uint8_t * data, unsigned int length);
static int write_phy_config_reg(PHYConfigReg reg, uint32_t data);
static int stop_video_link();
static int complete_video_link();
static void set_TMDS_power_enabled(bool enabled);
static int bypass_wait = 0;


struct hdmi_controller_config {
	addr_t		base_address;
	u_int32_t	clock;
	u_int32_t	irq;
	const char	*dt_path;
};

static const struct hdmi_controller_config hdmi_config[] = {
#if WITH_HW_DISPLAY_HDMI	
	{ HDMI_BASE_ADDR, CLK_HDMI, INT_HDMI_LINK, hdmi_dt_path },
#else
	{ 0, 0, 0, 0 },
#endif // WITH_HW_DISPLAY_HDMI
};

struct display_infoframe   aviInfoFrame =  { 
				kDisplayInfoFrameTypeAVI, 
				0x02,	//Version
				0x0d,	//length 
				0xf3,	//checksum 
				{0x10, 0x68, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 } //data
};


extern AppleSamsungHDMITXPHYConfigTableEntry hdmi_static_config[];
extern size_t hdmi_static_config_size;

/////////////////////////////////////////
////////// controller global functions

int hdmi_controller_start(u_int8_t type, u_int8_t mode)
{
	u_int32_t   hpdstate;


	if (hdmi_started)
		return 0;
    
	hdmi_controller_type = type;
	hdmi_controller_mode = mode;
    
	__base_address = hdmi_config[hdmi_controller_type].base_address;

	// register interrupt handler
	install_int_handler(hdmi_config[hdmi_controller_type].irq, hdmi_interrupt_filter, NULL);
	event_init(&hdmi_controller_task_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	hdmi_port_phy_cfg_table = (AppleSamsungHDMITXPHYConfigTableEntry *)hdmi_static_config;
	hdmi_port_phy_cfg_table_size = hdmi_static_config_size;

	clock_gate(hdmi_config[hdmi_controller_type].clock, true);

	unmask_int(hdmi_config[hdmi_controller_type].irq);

	reset();

	//power down HDMI PHY, keeping HPD enabled
	power_down_phy(false);

	hdmi_started = true;
	task_start(task_create("hdmi", &hdmi_controller_task, NULL, kHDMIStackSize));

	hpdstate = read_reg(kCtrlModule, CTRL_HPD_STATUS);
	debug(INIT,"hpdstate=0x%08x\n", hpdstate);
	if (hpdstate == 0x00000001) {
		hdmi_device_start();
	}

	// clear any pending HPD interrupts (Note: status bits are write-one-to-clear)
	write_reg(kCtrlModule, CTRL_INTC_FLAG, CTRL_INTC_FLAG_HPD_CHG);

	// unmask HPD
	or_reg(kCtrlModule, CTRL_INTC_CON, CTRL_INTC_CON_EN_GLOBAL|CTRL_INTC_CON_EN_HPD_CHG);

	write_reg(kCtrlModule, CTRL_INTC_FLAG, CTRL_INTC_FLAG_HPD_CHG);

	return 0;
}

void hdmi_controller_stop()
{
	if (!hdmi_started) {
		debug(INT, "hdmi not started. Nothing to do\n");
		return;
	}

	hdmi_controller_stop_video();
   
	// disable interrupt source
	mask_int(hdmi_config[hdmi_controller_type].irq);

	// Stop background EDID polling.
	abort_edid();

	hdmi_video_started = false;
    
	hdmi_device_stop();                                

	if (hdmi_controller_type == kHDMIControllerType_HDMI) {
		callout_dequeue(&hdmi_controller_screensaver_callout);
	}

	hdmi_started = false;
}
	
int hdmi_controller_validate_video(struct video_timing_data *timings)
{
	uint64_t        pixel_clock;

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
    
	pixel_clock = data.timing.axis[kDisplayAxisTypeVertical].total * 
		data.timing.axis[kDisplayAxisTypeHorizontal].total;
	pixel_clock *= data.timing.axis[kDisplayAxisTypeVertical].sync_rate; // vTotal * hTotal * vSyncRate
		pixel_clock >>= 16;
    
	debug(VIDEO, "stream clock=%lld\n", pixel_clock);

	return validate_video_link(&data, pixel_clock,  NULL);
}

int hdmi_controller_read_bytes_i2c(u_int32_t device_addr, u_int8_t addr, u_int8_t *data, u_int32_t length)
{
	uint8_t offset[1];

	offset[0] = addr;
	return (iic_read(HDMI_DDC_IIC_BUS, device_addr, offset, 1, data, length, IIC_COMBINED));
}

int hdmi_controller_write_bytes_i2c(u_int32_t device_addr, u_int32_t addr, u_int8_t *data, u_int32_t length)
{
	return -1;
}

int hdmi_controller_start_video(struct video_link_data *data, uint32_t pixel_clock)
{
	struct video_link_data edid_data;
	struct video_link_data *timings = NULL;

	if ( !hdmi_started )
		return -1;

	debug(INIT,"Pixel clock %d\n", pixel_clock);
	// Overwriting timings with EDID preference.
	bcopy(data, &edid_data, sizeof(edid_data));
	if (get_edid_timings(&edid_data.timing) == 0) timings = &edid_data;

	if (timings == NULL) timings = data;
	
	return configure_video(timings, pixel_clock);
}

int hdmi_controller_stop_video()
{
	if ( !hdmi_video_started )
		return -1;

	stop_video_link();
	complete_video_link();

	// unmask HPD
	or_reg(kCtrlModule, CTRL_INTC_CON, CTRL_INTC_CON_EN_GLOBAL|CTRL_INTC_CON_EN_HPD_CHG);

	write_reg(kCtrlModule, CTRL_INTC_FLAG, CTRL_INTC_FLAG_HPD_CHG);

	hdmi_video_started = false;

	return 0;
}

bool hdmi_controller_video_configured()
{
	return hdmi_video_started;
}

uint8_t hdmi_read_reg(uint32_t offset)
{
	return( read_reg(0, offset) );
}

void hdmi_write_reg(uint32_t offset, uint8_t value)
{
	write_reg(0, offset, value);
}
/////////////////////////////////////////
////////// controller local functions


static void handle_screensaver_callout(struct callout *co, void *args)
{
	debug(ALWAYS, "HDMI Screensaver\n");
	// set_TMDS_power_enabled calls  write_phy_reg which endup calling event_wait_timeout. Such call is not safe when performed in the handling of the callout. 
	bypass_wait = 1;
	set_TMDS_power_enabled(false);
	bypass_wait = 0;
}

static int hdmi_controller_task(void *arg)
{
	if (hdmi_controller_type == kHDMIControllerType_HDMI) {
		callout_enqueue(&hdmi_controller_screensaver_callout, kScreenBurnTimeout, handle_screensaver_callout, NULL);
	}

	// Perform this on the hdmi task to prevent blocking startup.
	for (;;) {

		event_wait(&hdmi_controller_task_event);
        
		// if controller was stopped before hdmi task got chance to run, exit
		if (hdmi_started == false) {
			debug(ERROR, "controller task stopped since hdmi stopped\n");
			return 0;
		}
		
		mask_int(hdmi_config[hdmi_controller_type].irq);

		if (hdmi_interrupt_occurred) {
			hdmi_interrupt_occurred = false;

			if (hdmi_controller_type == kHDMIControllerType_HDMI) {
				callout_reset(&hdmi_controller_screensaver_callout, 0);
			}

			// Invoke bottom half.
			handle_interrupt();
		}
		unmask_int(hdmi_config[hdmi_controller_type].irq);
	}

	return 0;
}

static bool get_hot_plug_detect()
{
	return read_reg(kCtrlModule, CTRL_HPD_STATUS) & CTRL_HPD_STATUS_PLUGGED;
}

static void hdmi_interrupt_filter(void *arg)
{
	ctrl_int_status = read_reg(kCtrlModule, CTRL_INTC_FLAG);
	write_reg(kCtrlModule, CTRL_INTC_FLAG, ctrl_int_status);

	iic_int_status = read_reg32(kI2CModule, IICINT);
	write_reg32(kI2CModule, IICINT, iic_int_status);

	debug(INT, "Interrupt received. ctrl_int_status=0x%08x\n", ctrl_int_status);
	debug(INT, "Interrupt received. ctrl_int_status=0x%08x iic_int_status=0x%08x\n", ctrl_int_status, iic_int_status);

	hdmi_interrupt_occurred = true;

	event_signal(&hdmi_controller_task_event);
}

static void handle_interrupt(void)
{
	// Handle unplug event
	if ( (ctrl_int_status & CTRL_INTC_FLAG_HPD_UNPLUG) ) {
		debug(INT, "HPD_UNPLUG\n");

		debug(INT, "MUST HANDLE TEAR DOWN\n");
		hdmi_device_stop();
	}

	// Handle plug event
	if ( (ctrl_int_status & CTRL_INTC_FLAG_HPD_PLUG) ) {
		debug(INT, "HPD_PLUG\n");

		if ( get_hot_plug_detect() ) {
			debug(INT, "MUST HANDLE PLUG IN \n");
			hdmi_device_start();

		} else {
			debug(INT, "MUST HANDLE UNPLUG\n");
			hdmi_device_stop();
		}
	}

	// Handle HDCP interrupts
	if ((ctrl_int_status & CTRL_INTC_FLAG_HDCP)) {
		uint32_t    hdcp_int_status;

		debug(INT, "HDCP REALLY!!!\n");
		hdcp_int_status = read_reg(kHDMIModule, HDMI_STATUS);

		// Clear active HDCP interrupts (Note: bit 7 is read-only and not an interrupt source)
		write_reg(kHDMIModule, HDMI_STATUS, hdcp_int_status);
	}

}

static void reset_HDMI_core(void)
{
	// From H4I Power Manager (PMGR) Microarchitecture Specification
	// section 7.2 Block Resets (see H4i_UM):
	//
	//  NOTE: Some of the blocks may have CSR bits defined as reset bits, however some of these have
	//        been disabled. The only valid supported block reset mechanism in H4I is the reset bit in the
	//        block's power state register.

	//Reset device
	clock_reset_device(hdmi_config[hdmi_controller_type].clock);
}

static void reset_PHY(void)
{
	// Ensure that PHY is powered on (if the PHY power off signal is connected)
	and_reg(kCtrlModule, CTRL_PHY_CON_0, ~CTRL_PHY_CON_0_PHY_PWR_OFF);

	// Reset PHY
	or_reg(kCtrlModule, CTRL_PHY_RSTOUT, CTRL_HDMI_PHY_RSTOUT_RESET);
	task_sleep(kPHYResetIntervalMS);
	and_reg(kCtrlModule, CTRL_PHY_RSTOUT, ~(CTRL_HDMI_PHY_RSTOUT_RESET));

}

static void set_TMDS_power_enabled(bool enabled)
{
	uint8_t	reg;
	uint8_t	mask;
	static bool tmds_enabled = 0;

	if (tmds_enabled == enabled)
		return;

	tmds_enabled = enabled;
	reg  = PHY_CONFIG_REG1D;
	mask = PHY_V1P4_CONFIG_REG1D_I2C_PDEN | PHY_V1P4_CONFIG_REG1D_TX_DRV_PD;

	if (enabled ) {
		and_phy_reg(reg, ~mask);
	} else {
		or_phy_reg(reg, mask);
	}
}

static void power_down_phy(bool fullPowerDown)
{

	or_phy_reg(PHY_CONFIG_REG1D, PHY_V1P4_CONFIG_REG1D_I2C_PDEN | (fullPowerDown ? PHY_V1P4_CONFIG_REG1D_FULL_PD : PHY_V1P4_CONFIG_REG1D_PD_WITH_HPD));
	debug(INIT,"PHY %s power down\n", fullPowerDown ? "full" : "partial");

	if ( fullPowerDown ) {
		// Power down PHY (if the PHY power off signal is connected)
		or_reg(kCtrlModule, CTRL_PHY_CON_0, CTRL_PHY_CON_0_PHY_PWR_OFF);
	}
}

bool get_is_PHY_ready(void)
{
	    return ((read_reg(kCtrlModule, CTRL_PHY_STATUS_0) & CTRL_PHY_STATUS_0_PHY_READY) ? true : false) ;
}

static int reset(void)
{
	debug(INIT,"PHY_CON_0=0x%08x\n", read_reg(kCtrlModule, CTRL_PHY_CON_0));
	// Reset HDMI core and PHY
	debug(INIT,"PHYReady=%u\n", get_is_PHY_ready());

        // Reset HDMI core (Note: core reset disables PHY power in case of 1.4 TX)
	reset_HDMI_core();

	// Reset HDMI PHY
	reset_PHY();

	// Set TMDS bit order
	set_bits_in_reg(kHDMIModule, HDMI_CON_0, 0, HDMI_CON_0_ENCODING_RETAIN_BIT_ORDER, 0);

	debug(INIT,"PHYReady=%u\n", get_is_PHY_ready());

	debug(INIT,"PHY_CON_0=0x%08x\n", read_reg(kCtrlModule, CTRL_PHY_CON_0));
	// Initialize the I2C bridge
	//
	// - Clear pending interrupts
	// - Set configuration options from device tree
	// - Unmask Stop interrupt
	// - Enable ACK generation
	// - Set mode to master transmitter and enable I2C output

	write_reg32(kI2CModule, SW_RESET, kIICSwReset);
	do {
		write_reg32(kI2CModule, HDMIPHY_ID, PHY_I2C_ADDRESS);
	} while (read_reg32(kI2CModule, HDMIPHY_ID) != PHY_I2C_ADDRESS);

	write_reg32(kI2CModule, IICINT, kIICIntAll);  // Clear pending interrupts
	write_reg32(kI2CModule, IICCON, (IICCONFIG_IICCON | kIICConAckGen));
	write_reg32(kI2CModule, FIFOCON, IICCONFIG_FIFOCON);
	write_reg32(kI2CModule, IICBUSCON, IICCONFIG_IICBUSCON);
	write_reg32(kI2CModule, TIMEOUT_CON, IICCONFIG_TIMEOUTCON);
	write_reg32(kI2CModule, IICSTAT, (kIICStatMaster | kIICStatTx | kIICStatSOE));

	return 0;
}


static int configure_video(struct video_link_data *data, uint32_t pixel_clock)
{
	debug(INIT,"starting to configure video\n");
    
	if (hdmi_video_started) return 0;
    
	// Uncomment to show the timings we finally decide to use.
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

	init_video((data->mirror_mode ? false : true));

	if ( configure_video_mode(data, pixel_clock) != 0 ) {
		debug(ERROR,"failed configure_video_mode\n");
		goto exit;
	}

	if ( configure_video_color(data) != 0 )
		goto exit;

	configure_video_bist(data);

	configure_video_mute(data, false);

	hdmi_video_started = true;
    
	debug(VIDEO, "finished configuring video\n");
    
	return 0;
    
exit:
	debug(ERROR, "failed to configure video\n");
	hdmi_video_started = false;

	return -1;
}

static void init_video(bool master)
{
	debug(INIT,"%s:nothing to do yet\n", __FUNCTION__);
}

static HDMITXPHYConfigDepth phy_config_depth(uint32_t depth)
{
	HDMITXPHYConfigDepth depthIndex;
	switch ( depth ) {
	case 12:
		depthIndex = kHDMITXPHYConfigDepth12;
		break;

	case 10:
		depthIndex = kHDMITXPHYConfigDepth10;
		break;

	default:
		depthIndex = kHDMITXPHYConfigDepth8;
		break;
	}
	return depthIndex;
}

const HDMITXPHYConfig * find_PHY_config(uint32_t pixelClockHz, uint32_t pixelClockMinHz, uint32_t pixelClockMaxHz, uint32_t colorDepth)
{
    const HDMITXPHYConfig * bestConfig      = NULL;
    const HDMITXPHYConfigDepth		depthIndex = phy_config_depth(colorDepth);

    if ( hdmi_port_phy_cfg_table ) {
        const AppleSamsungHDMITXPHYConfigTableEntry *   table;
        unsigned int                                    rowCount;
        uint32_t                                        bestError;

        // Search for an entry with a matching pixel clock
        //
        //  Note: The table must be sorted by pixel clock in ascending order.
        //
        table     = (const AppleSamsungHDMITXPHYConfigTableEntry *)hdmi_port_phy_cfg_table;
        rowCount  = hdmi_port_phy_cfg_table_size / sizeof table[0];
        bestError = UINT32_MAX;

        for (unsigned int i = 0; i < rowCount; i++) {
            uint32_t    configPixelClockHz = table[i].pixelClockHz;
            uint32_t    error;

	    debug(INFO,"pixelClockMaxHz %d pixel_clock %d configPixelClockHz %d\n", pixelClockMaxHz, pixelClockHz, configPixelClockHz);
            if (configPixelClockHz < pixelClockMinHz) {
		    goto next;
	    }
            if (configPixelClockHz > pixelClockMaxHz) {
		    goto exit;
	    }

            if (table[i].depth[depthIndex].invalid) {
		    goto next;
	    }

            if ( configPixelClockHz < pixelClockHz ) {
                error = pixelClockHz - configPixelClockHz;
            } else {
                error = configPixelClockHz - pixelClockHz;
            }

            if ( error < bestError ) {
                bestError   = error;
                bestConfig  = &table[i].depth[depthIndex].data;
            }

        next:
            continue;
        }
    }

exit:
    return bestConfig;
}

static void configure_video_mute(struct video_link_data *data, bool mute)
{
	set_av_mute_enabled(mute);
}

static int validate_video_color(const struct video_color_data * color)
{
	int    ret = 0;

	if (color->depth < 8 || color->depth > 12) {
		ret = -1;
		debug(ERROR,"Color depth not supported\n");
		goto exit;
	}


	if (color->space == kDisplayColorSpacesYCbCr422) {
		// It is invalid to have a depth other than 8 with YCbCr4:2:2 [see HDMI v1.3 section 6.5].
		if (color->depth != 8) {
			ret = -1;
			debug(ERROR,"YCbCr4:2:2 requires depth = 8 bits\n");
			goto exit;
		}
	}

exit:
	return ret;
}

static int validate_video_timing(const struct video_timing_axis * h, const struct video_timing_axis * v)
{
	int    ret = 0;

	if (h->total <= 0 || h->total > kHorizontalTotalLimit) {
		ret = -1;
		goto exit;
	}

	if (v->total <= 0 && v->total >  kVerticalTotalLimit) {
		ret = -1;
		goto exit;
	}


	if (h->active > kHorizontalActiveLimit) {
		ret = -1;
		goto exit;
	}

	if (v->active > kVerticalActiveLimit) {
		ret = -1;
		goto exit;
	}


	if ((h->total < h->active) || ((h->total - h->active) > kHorizontalBlankLimit)) {
		ret = -1;
		goto exit;
	}

	if ((v->total < v->active) || ((v->total - v->active) > kVerticalBlankLimit)) {
		ret = -1;
		goto exit;
	}

exit:
	return ret;
}

#define kVideoPixelClkTolleranceNumerator   5
#define kVideoPixelClkTolleranceDenomiator  1000
					  
static inline u_int32_t
ulmin(u_int32_t a, u_int32_t b)
{
		return (a < b ? a : b);
}

uint32_t _pixelClockLimit = UINT32_MAX;

static int validate_video_link(struct video_link_data * data, uint32_t pixel_clock, const HDMITXPHYConfig ** pPHYConfig)
{
	const struct video_timing_axis * h     = &data->timing.axis[kDisplayAxisTypeHorizontal];
	const struct video_timing_axis * v     = &data->timing.axis[kDisplayAxisTypeVertical];
	const struct video_color_data  * color = &data->color;
	const HDMITXPHYConfig * phyConfig = NULL;
	uint32_t    pixelClockToleranceHz, pixelClockMinHz, pixelClockMaxHz;
	int64_t pixelClockToleranceFixed = pixel_clock;
	int    ret = 0;
				        
	pixelClockToleranceFixed *= kVideoPixelClkTolleranceNumerator;
	pixelClockToleranceFixed /= kVideoPixelClkTolleranceDenomiator;
	pixelClockToleranceHz = (uint32_t) pixelClockToleranceFixed;

	// Uncomment to show the timings we finally decide to use.
	debug(VIDEO,"Validating link\n");
	debug(VIDEO,"mirror_mode: %d\n", data->mirror_mode);
	debug(VIDEO,"test_mode: 0x%08x\n", data->test_mode);
	debug(VIDEO,"color:\n");
	debug(VIDEO,"  depth: %u\n", data->color.depth);
	debug(VIDEO,"  space: %u\n", data->color.space);
	debug(VIDEO,"  range: %u\n", data->color.range);
	debug(VIDEO,"  coefficient: %u\n", data->color.coefficient);
	debug(VIDEO,"timing:\n");
	debug(VIDEO,"  interlaced: %d\n", data->timing.interlaced);
	{
		int i;
		for (i = 0; i < kDisplayAxisCount; ++i) {
			debug(VIDEO,"  Axis %d:\n", i);
			debug(VIDEO,"    total: %u\n", data->timing.axis[i].total);
			debug(VIDEO,"    active: %u\n", data->timing.axis[i].active);
			debug(VIDEO,"    sync_width: %u\n", data->timing.axis[i].sync_width);
			debug(VIDEO,"    back_porch: %u\n", data->timing.axis[i].back_porch);
			debug(VIDEO,"    front_porch: %u\n", data->timing.axis[i].front_porch);
			debug(VIDEO,"    sync_rate: %u (%uHz)\n",
			      data->timing.axis[i].sync_rate, data->timing.axis[i].sync_rate >> 16);
			debug(VIDEO,"    sync_polarity: %u\n", data->timing.axis[i].sync_polarity);
		}
	}

	// Interlaced video not supported
	if (data->timing.interlaced) {
		ret = -1;
		debug(VIDEO,"interlaced mode not supported\n");
		goto exit;
	}

	// Mirror mode is not supported
	if (data->mirror_mode) {
		ret = -1;
		debug(VIDEO,"Mirror mode not supported\n");
		goto exit;
	 }

	// Verify that color mode is supported
	ret = validate_video_color(color);
	if (ret < 0) {
		goto exit;
	}

	// Calculate target pixel clock range, clamping max value to _pixelClockLimit
	pixelClockMinHz = pixel_clock - pixelClockToleranceHz;
	pixelClockMaxHz = ulmin(pixel_clock + pixelClockToleranceHz, _pixelClockLimit);

	// Verify that the target pixel clock is within the valid range
	if (pixel_clock > _pixelClockLimit) {
		ret = -1;
		debug(VIDEO,"pixel clock value is too high, %lu\n", (unsigned long)pixel_clock);
		goto exit;
	}

	// Verify that there is a PHY configuration for the pixel clock
	phyConfig = find_PHY_config(pixel_clock, pixelClockMinHz, pixelClockMaxHz, data->color.depth);
	if (phyConfig == NULL) {
		ret = -1;
		debug(VIDEO,"No PHY configuration for required pixel clock: %lu\n", (unsigned long)pixel_clock);
		goto exit;
	}

	// Return the PHY configuration if requested
	if ( pPHYConfig ) {
		*pPHYConfig = phyConfig;
	}

	// Verify that timing generator parameters are supported
	ret = validate_video_timing(h, v);
	if (ret < 0) {
		debug(VIDEO,"Timing not supported\n");
		goto exit;
	}

exit:
	debug(ERROR,"ret=0x%08x\n", ret);

	return ret;
}

void set_horizontal_blanking(uint32_t value)
{
	// H_BLANK_0:1
	write_regs(kHDMIModule, HDMI_H_BLANK_0, value, 2);
}

void set_vertical_blanking(uint32_t blanking, uint32_t total)
{
	// V1_BLANK_0:1
	write_regs(kHDMIModule, HDMI_V1_BLANK_0, blanking, 2);

	// V2_BLANK_0:1
	write_regs(kHDMIModule, HDMI_V2_BLANK_0, total, 2);
}

void set_line_lengths(uint32_t h_line, uint32_t v_line)
{
	debug(VIDEO,"h_line %d, v_line %d\n", h_line, v_line);
	// V_LINE_0:1
	write_regs(kHDMIModule, HDMI_V_LINE_0, v_line, 2);

	// H_LINE_0:1
	write_regs(kHDMIModule, HDMI_H_LINE_0, h_line, 2);
}

void set_bottom_field_extents(uint32_t start, uint32_t end)
{
	// V_BLANK_F0_0:1
	write_regs(kHDMIModule, HDMI_V_BLANK_F0_0, start, 2);

	// V_BLANK_F1_0:1
	write_regs(kHDMIModule, HDMI_V_BLANK_F1_0, end, 2);
}

void set_horizontal_sync_shape(bool active_high, uint32_t start, uint32_t end)
{
	// HSYNC_POL
	write_reg(kHDMIModule, HDMI_HSYNC_POL, !active_high); 

	// H_SYNC_START_0:1
	write_regs(kHDMIModule, HDMI_H_SYNC_START_0, start, 2);

	// H_SYNC_END_0:1
	write_regs(kHDMIModule, HDMI_H_SYNC_END_0, end, 2);
}

void set_vertical_sync_shape(bool active_high, uint32_t vsync1_start, uint32_t vsync1_end, uint32_t vsync2_start, uint32_t vsync2_end)
{
	// VSYNC_POL
	write_reg(kHDMIModule, HDMI_VSYNC_POL, !active_high);

	// V_SYNC_LINE_BEF_1_0:1  - top field vsync start
	write_regs(kHDMIModule, HDMI_V_SYNC_LINE_BEF_1_0, vsync1_start, 2);

	// V_SYNC_LINE_BEF_2_0:1  - top field vsync end
	write_regs(kHDMIModule, HDMI_V_SYNC_LINE_BEF_2_0, vsync1_end, 2);

	// V_SYNC_LINE_AFT_1_0:1  - bottom field vsync start
	write_regs(kHDMIModule, HDMI_V_SYNC_LINE_AFT_1_0, vsync2_start, 2);

	// V_SYNC_LINE_AFT_2_0:1  - bottom field vsync end
	write_regs(kHDMIModule, HDMI_V_SYNC_LINE_AFT_2_0, vsync2_end, 2);
}

static void set_bottom_field_transition_point(uint32_t start, uint32_t end)
{
	// V_SYNC_LINE_AFT_PXL_1_0:1
	write_regs(kHDMIModule, HDMI_V_SYNC_LINE_AFT_PXL_1_0, start, 2);

	// V_SYNC_LINE_AFT_PXL_2_0:1
	write_regs(kHDMIModule, HDMI_V_SYNC_LINE_AFT_PXL_2_0, end, 2);
}

static int configure_timing_generator(const struct video_link_data * data)
{
	const struct video_timing_axis * h = &data->timing.axis[kDisplayAxisTypeHorizontal];
	const struct video_timing_axis * v = &data->timing.axis[kDisplayAxisTypeVertical];

	uint32_t h_blanking     = h->total - h->active;
	uint32_t v_blanking     = v->total - v->active;

	uint32_t v_bot_start    = v->total + v_blanking + 1;    // interlaced only

	// Make "line 1" coincident with the first VSYNC, as in CEA formats
	uint32_t vsync1_lineno  = 1;
	uint32_t vsync2_lineno  = vsync1_lineno + v->total;     // interlaced only

	// Configure Timing Generator related registers

	// TG_CMD
	write_reg(kTGModule, TG_CMD, data->timing.interlaced ? TG_CMD_FIELD_EN : 0);

	// TG_CBGEN
	//   Pixel repititon is not supported <rdar://problem/10712907>

	// TG_H_FSZ_L:TG_H_FSZ_H
	write_regs(kTGModule, TG_H_FSZ_L, h->total, 2);

	// TG_HACT_ST_L:TG_HACT_ST_H
	write_regs(kTGModule, TG_HACT_ST_L, h_blanking, 2);

	// TG_HACT_SZ_L:TG_HACT_SZ_H
	write_regs(kTGModule, TG_HACT_SZ_L, h->active, 2);

	// TG_V_FSZ_L:TG_V_FSZ_H
	write_regs(kTGModule, TG_V_FSZ_L, v->total, 2);

	// TG_VSYNC_L:TG_VSYNC_H
	write_regs(kTGModule, TG_VSYNC_L, vsync1_lineno, 2);

	// TG_VSYNC2_L:TG_VSYNC2_H (interlaced only)
	write_regs(kTGModule, TG_VSYNC2_L, vsync2_lineno, 2);

	// TG_VACT_ST_L:TG_VACT_ST_H
	write_regs(kTGModule, TG_VACT_ST_L, v_blanking, 2);

	// TG_VACT_SZ_L:TG_VACT_SZ_H
	write_regs(kTGModule, TG_VACT_SZ_L, v->active, 2);

	// TG_FIELD_CHG_L:TG_FIELD_CHG_H (interlaced only)
	write_regs(kTGModule, TG_FIELD_CHG_L, vsync2_lineno, 2);

	// TG_VACT_ST2_L:TG_VACT_ST2_H (interlaced only)
	write_regs(kTGModule, TG_VACT_ST2_L, v_bot_start, 2);

	// Note: 3D (HDMI 1.4 TX only) not supported

	// TG_VSYNC_TOP_HDMI_L:TG_VSYNC_TOP_HDMI_H
	write_regs(kTGModule, TG_VSYNC_TOP_HDMI_L, vsync1_lineno, 2);  // same as TG_VSYNC_L:TG_VSYNC_H

	// TG_VSYNC_BOT_HDMI_L:TG_VSYNC_BOT_HDMI_H (interlaced only)
	write_regs(kTGModule, TG_VSYNC_BOT_HDMI_L, vsync2_lineno, 2);  // same as TG_VSYNC2_L:TG_VSYNC2_H

	// TG_FIELD_TOP_HDMI_L:TG_FIELD_TOP_HDMI_H
	write_regs(kTGModule, TG_FIELD_TOP_HDMI_L, vsync1_lineno, 2);  // same as TG_VSYNC_L:TG_VSYNC_H

	// TG_FIELD_BOT_HDMI_L:TG_FIELD_BOT_HDMI_H
	write_regs(kTGModule, TG_FIELD_BOT_HDMI_L, vsync2_lineno, 2);  // same as TG_VSYNC2_L:TG_VSYNC2_H

	// TG_HDMI_TPGEN
	//   Pixel repititon is not supported <rdar://problem/10712907>

	return 0;
}

static int configure_video_format(struct video_link_data * data)
{
	const struct video_timing_axis * h = &data->timing.axis[kDisplayAxisTypeHorizontal];
	const struct video_timing_axis * v = &data->timing.axis[kDisplayAxisTypeVertical];

	uint32_t h_blanking     = h->total - h->active;
	uint32_t v_blanking     = v->total - v->active;

	uint32_t v_line         = data->timing.interlaced ? (v->total * 2 + 1) : v->total;
	uint32_t v_bot_start    = v->total + v_blanking + 1;    // interlaced only

	uint32_t hsync_start    = h->front_porch - 2;            // There is apparently an implicit +2 added to HSYNC_START (compare manual recommended values to CEA-861-D formats)
	uint32_t hsync_end      = hsync_start + h->sync_width;

	uint32_t vsync1_start   = v->front_porch;
	uint32_t vsync1_end     = vsync1_start + v->sync_width;

	uint32_t vsync2_start   = v->total + vsync1_start;      // interlaced only
	uint32_t vsync2_end     = v->total + vsync1_end;        // interlaced only

	// The +0.5 vertical lines offset, not included in vsync2_start/end
	uint32_t vsync2_h_start = (h->total / 2) + h->front_porch; // interlaced only
	uint32_t vsync2_h_end   = vsync2_h_start;                 // interlaced only
	uint64_t const64 = 1000;


	// Calculate frame interval rounded (half up) to nearest millisecond
	frame_interval_MS = (((const64 << 32) / v->sync_rate) + 0x8000) >> 16;

	// Configure registers related to video format

	set_horizontal_blanking(h_blanking);

	set_vertical_blanking(v_blanking, v->total);

	set_line_lengths(h->total, v_line);

	// INT_PRO_MODE
	write_reg(kHDMIModule, HDMI_INT_PRO_MODE, data->timing.interlaced);

	set_bottom_field_extents(v_bot_start, v_line);  // interlaced only

	set_horizontal_sync_shape(h->sync_polarity, hsync_start, hsync_end);

	set_vertical_sync_shape(v->sync_polarity, vsync1_start, vsync1_end, vsync2_start, vsync2_end);

	set_bottom_field_transition_point(vsync2_h_start, vsync2_h_end);

	// HDMI_CON_1[Pxl_rep_num]
	//   Pixel repititon is not supported <rdar://problem/10712907>

	return 0;
}

static void set_mute_pixel(struct pixel pixel)
{
    // BLUE_SCREEN_[RGB]_0 - upper half of the least significant 8 bits
    write_reg(kHDMIModule, HDMI_BLUE_SCREEN_R_0, (pixel.r >> 4) & 0x0F);
    write_reg(kHDMIModule, HDMI_BLUE_SCREEN_G_0, (pixel.g >> 4) & 0x0F);
    write_reg(kHDMIModule, HDMI_BLUE_SCREEN_B_0, (pixel.b >> 4) & 0x0F);

    // BLUE_SCREEN_[RGB]_1 - the most significant 8 bits
    write_reg(kHDMIModule, HDMI_BLUE_SCREEN_R_1, pixel.r >> 8);
    write_reg(kHDMIModule, HDMI_BLUE_SCREEN_G_1, pixel.g >> 8);
    write_reg(kHDMIModule, HDMI_BLUE_SCREEN_B_1, pixel.b >> 8);
}

static void set_color_space(int space)
{
    // Enable or disable YCbCr4:2:2 mode (RGB4:4:4/YCbCr4:4:4 are handled identically)
    set_bits_in_reg(kHDMIModule, HDMI_CON_0, 0, HDMI_CON_0_YCBCR422_SEL, space == kDisplayColorSpacesYCbCr422 ? HDMI_CON_0_YCBCR422_SEL : 0);
}

static int configure_video_pattern(const struct video_link_data * data)
{
    // enable video pattern generation
    if ( data->test_mode ) {
        or_reg(kHDMIModule, HDMI_VIDEO_PATTERN_GEN, HDMI_VIDEO_PATTERN_GEN_PATTERN_ENABLE);
        debug(VIDEO,"Video pattern generation enabled.\n");
    } else {
        and_reg(kHDMIModule, HDMI_VIDEO_PATTERN_GEN, ~(HDMI_VIDEO_PATTERN_GEN_PATTERN_ENABLE));
    }

    return 0;
}

static int set_tx_output_mode(int mode)
{
	uint32_t dvi_mask = HDMI_CON_2_VID_PERIOD_EN | HDMI_CON_2_DVI_BAND_EN;
	uint8_t transtaled_mode = (mode == kHDMI_tx_mode_HDMI) ? HDMI_MODE_SEL_MODE_HDMI : HDMI_MODE_SEL_MODE_DVI; 

	debug(VIDEO,"%s %d:, Mode %s\n", __FUNCTION__, __LINE__, (mode == kHDMI_tx_mode_HDMI) ? "HDMI" : "DVI");
	// MODE_SEL[1:0] = mode
	set_bits_in_reg(kHDMIModule, HDMI_MODE_SEL, HDMI_MODE_SEL_MODE_SHIFT, HDMI_MODE_SEL_MODE_MASK, transtaled_mode);

	// Configure video preamble and guard band
	//  HDMI_CON_2[Vid_Period_En]	= 1 if DVI mode, 0 otherwise
	//  HDMI_CON_2[Dvi_Band_En]	  = 1 if DVI mode, 0 otherwise
	set_bits_in_reg(kHDMIModule, HDMI_CON_2, 0, dvi_mask, (mode == kHDMI_tx_mode_DVI) ? dvi_mask : 0);

	return 0;
}

static inline int set_PHY_config_done(bool done)
{
    write_phy_config_reg(kPHYConfigRegModeSet, done ? PHY_CONFIG_MODE_SET_DONE : PHY_CONFIG_MODE_SET_IN_PROGRESS);
    return 0;
}

static int configure_PHY(const HDMITXPHYConfig * config)
{
	uint8_t    buf[kPHYConfigRegCount];
	int    ret = 0;
	int reg;

	// Sanity check (if this fails, it is a programming error)
	if (sizeof (*config) != (sizeof (buf) - 2)) {
		ret = -1;
		goto exit;
	}

	// Copy config into data buffer; initialize first & last bytes from registers
	buf[kPHYConfigReg00] = read_phy_reg(kPHYConfigReg00);
	buf[kPHYConfigReg07] = read_phy_reg(kPHYConfigReg07);
	memcpy((uint8_t *)buf + 1, config, sizeof (*config));

	debug(VIDEO,"PHY_STATUS=0x%08x\n", read_reg(kCtrlModule, CTRL_PHY_STATUS_0));

	// Configure the PHY
	ret = set_PHY_config_done(false);
	if (ret < 0) {
		debug(ERROR,"set_PHY_config_done: ret %d\n", ret);
		goto exit;
	}

	for (reg = kPHYConfigReg00; reg != kPHYConfigRegCount; reg++) {
		ret = write_phy_config_reg(reg, buf[reg]);
		if (ret < 0) {
			debug(ERROR,"write_phy_config_reg: ret %d\n", ret);
			goto exit;
		}
	}

	ret = set_PHY_config_done(true);
	if (ret < 0) {
		debug(ERROR,"set_PHY_config_done: ret %d\n", ret);
		goto exit;
	}

	debug(VIDEO,"PHY_STATUS=0x%08x\n", read_reg(kCtrlModule, CTRL_PHY_STATUS_0));

exit:
	return ret;
}
static int wait_for_PHY_ready()
{
	//TODO
	while (get_is_PHY_ready()  == false) {
		task_sleep(kPHYReadyPollIntervalMS);
	}

    return 0;
}

static int set_timing_generator_enabled(bool enabled)
{
    // TG_CMD[TG_EN] = enabled
    set_bits_in_reg(kTGModule, TG_CMD, 0, TG_CMD_TG_EN, enabled ? TG_CMD_TG_EN : 0);

    return 0;
}

static int configure_video_color(const struct video_link_data * data)
{
    static const struct pixel kBlackPixelRGB =
    {
        .r = 0,
        .g = 0,
        .b = 0
    };

    static const struct pixel kBlackPixelYCbCr =
    {
        .g =  16 << 8,  // Y
        .b = 128 << 8,  // Cb
        .r = 128 << 8   // Cr
    };

    const struct video_color_data *  color = &data->color;
    uint32_t limit_mode;
    uint32_t deep_color_mode;

    // Configure the color space
    set_color_space(color->space);

    // Configure "blue screen" (actually black, not blue) pixel value
    set_mute_pixel((color->space == kDisplayColorSpacesRGB) ? kBlackPixelRGB : kBlackPixelYCbCr);

    // Configure Color space and Pixel limitation
    //
    //  Note: This allows a full-range YCbCr configuration, though HDMI v1.3
    //        section 6.6 says "YCBCR components shall always be Limited Range"
    //
    if ( color->range == kDisplayColorDynamicRangeFull ) {
        limit_mode = HDMI_CON_1_PXL_LMT_CTRL_BYPASS_MODE;
    } else {
        write_reg(kHDMIModule, HDMI_YMIN, kVideoColorLimitedRangeYMin);
        write_reg(kHDMIModule, HDMI_YMAX, kVideoColorLimitedRangeYMax);

        if ( color->space == kDisplayColorSpacesRGB ) {
            limit_mode = HDMI_CON_1_PXL_LMT_CTRL_RGB_MODE;
        } else {
            limit_mode = HDMI_CON_1_PXL_LMT_CTRL_YCBCR_MODE;
            
            write_reg(kHDMIModule, HDMI_CMIN, kVideoColorLimitedRangeCMin);
            write_reg(kHDMIModule, HDMI_CMAX, kVideoColorLimitedRangeCMax);
        }
    }
    set_bits_in_reg(kHDMIModule, HDMI_CON_1, HDMI_CON_1_PXL_LMT_CTRL_SHIFT, HDMI_CON_1_PXL_LMT_CTRL_MASK, limit_mode);

    // Configure Color depth
    switch ( color->depth ) {
        case 12:
            deep_color_mode = HDMI_DC_CONTROL_DEEP_COLOR_MODE_12_BPC;
            break;
            
        case 10:
            deep_color_mode = HDMI_DC_CONTROL_DEEP_COLOR_MODE_10_BPC;
            break;

        default:
            deep_color_mode = HDMI_DC_CONTROL_DEEP_COLOR_MODE_8_BPC;
            break;
    }
    set_bits_in_reg(kHDMIModule, HDMI_DC_CONTROL, HDMI_DC_CONTROL_DEEP_COLOR_MODE_SHIFT, HDMI_DC_CONTROL_DEEP_COLOR_MODE_MASK, deep_color_mode);

    return 0;
}

static int prepare_video_link(struct video_link_data *data, uint32_t pixel_clock)
{
	const HDMITXPHYConfig * phyConfig;
	int tx_mode = kHDMI_tx_mode_HDMI;
	int ret;

	ret = validate_video_link(data, pixel_clock,  &phyConfig);
	require_noerr(ret, exit);

	// Use DVI mode if downstream port type is not HDMI
	tx_mode = get_edid_downstream_type();

	ret = configure_video_format(data);
	require_noerr(ret, exit);

	ret = configure_timing_generator(data);
	require_noerr_action(ret, exit, debug(ERROR,"configure_timing_generator: ret %d\n", ret));

	ret = configure_video_color(data);
	require_noerr_action(ret, exit, debug(ERROR,"configure_timing_generator: ret %d\n", ret));

	ret = configure_video_pattern(data);
	require_noerr_action(ret, exit, debug(ERROR,"configure_video_pattern: ret %d\n", ret));

	ret = set_tx_output_mode(tx_mode);
	require_noerr_action(ret, exit, debug(ERROR,"set_tx_output_mode: ret %d\n", ret));

	// Note: PHY configuration includes enabling TMDS power.
	ret = configure_PHY(phyConfig);
	require_noerr_action(ret, exit, debug(ERROR,"configure_PHY: ret %d\n", ret));

	ret = wait_for_PHY_ready();
	require_noerr_action(ret, exit, debug(ERROR,"wait_for_PHY_ready: ret %d\n", ret));

	set_TMDS_power_enabled(false);

	// Enable timing generator
	ret = set_timing_generator_enabled(true);
	require_noerr_action(ret, exit, debug(ERROR,"set_timing_generator_enabled: ret %d\n", ret));

	return 0;
exit:
	debug(ERROR,"error\n");
	return -1;
}


static void start_info_frame(struct display_infoframe *infoFrame)
{
	 // Configure the InfoFrame header, if needed.
	uint8_t header[] =  {infoFrame->type, infoFrame->version, infoFrame->length} ;
	write_regs_with_data(kHDMIModule, HDMI_AVI_HEADER0, header, sizeof header);
	// Set the InfoFrame checksum
	write_reg(kHDMIModule, HDMI_AVI_CHECK_SUM, infoFrame->checksum);
	// Set the InfoFrame data
	write_regs_with_data(kHDMIModule, HDMI_AVI_BYTE01, infoFrame->data, infoFrame->length);
	// Configure InfoFrame to be sent at every VSYNC
	set_bits_in_reg(kHDMIModule, HDMI_AVI_CON, HDMI_PKT_CON_PKT_TX_CON_SHIFT, HDMI_PKT_CON_PKT_TX_CON_MASK, HDMI_PKT_CON_PKT_TX_CON_SEND_ALWAYS);

}

void stop_info_frame(struct display_infoframe * infoFrame)
{
	// Disable transmission of the InfoFrame
	set_bits_in_reg(kHDMIModule, HDMI_AVI_CON, HDMI_PKT_CON_PKT_TX_CON_SHIFT, HDMI_PKT_CON_PKT_TX_CON_MASK, HDMI_PKT_CON_PKT_TX_CON_DISABLED);
}


static int get_tx_output_mode()
{
	uint32_t reg;
	int mode;

	reg = get_bits_in_reg(kHDMIModule, HDMI_MODE_SEL, HDMI_MODE_SEL_MODE_SHIFT, HDMI_MODE_SEL_MODE_MASK);

	mode = reg == HDMI_MODE_SEL_MODE_HDMI ? kHDMI_tx_mode_HDMI : kHDMI_tx_mode_DVI;
	debug(VIDEO,"%s %d:, Mode %s\n", __FUNCTION__, __LINE__, (mode == kHDMI_tx_mode_HDMI) ? "HDMI" : "DVI");

	return mode;
}

static int start_general_control_packet(const struct video_link_data * data)
{
    const struct video_color_data *  color = &data->color;
    uint32_t gcp_cd;
    bool setSupportsDeepColor = false;

    // Clear GCP data registers
    write_regs(kHDMIModule, HDMI_GCP_BYTE1, (uint32_t)0, 3);

    // Note: HDMI v1.3, section 6.5.3 says:
    //
    // Sources shall only send GCPs with non-zero CD to Sinks that indicate
    // support for Deep Color...
    //
    // Once a Source sends a GCP with non-zero CD to a sink, it should continue
    // sending GCPs with non-zero CD at least once per video field even if
    // reverting to 24-bit color, as long as the Sink continues to support
    // Deep Color.

    // Set color depth (CD) field of General Control Packet
    switch ( color->depth ) {
        case 12:
            gcp_cd = GCP_SB1_CD_36_BPP;
            setSupportsDeepColor = true;
            break;
            
        case 10:
            gcp_cd = GCP_SB1_CD_30_BPP;
            setSupportsDeepColor = true;
            break;

        default:
            gcp_cd = GCP_SB1_CD_24_BPP;
            break;
    }
    // Set Color Depth
    set_bits_in_reg(kHDMIModule, HDMI_GCP_BYTE2, GCP_SB1_CD_SHIFT, GCP_SB1_CD_MASK,
                 setSupportsDeepColor ? gcp_cd : GCP_SB1_CD_NONE);

    // Set Transmission Frequency
    //   Always set both VSYNC enables <rdar://problem/11672667>
    set_bits_in_reg(kHDMIModule, HDMI_GCP_CON, HDMI_GCP_CON_VSYNC_SHIFT, HDMI_GCP_CON_VSYNC_MASK, HDMI_GCP_CON_VSYNC_BOTH_FIELDS);

    // Configure GCP to be sent at every VSYNC
    //  GCP_CON[GCP_CON] = (every VSYNC)
    set_bits_in_reg(kHDMIModule, HDMI_GCP_CON, HDMI_PKT_CON_PKT_TX_CON_SHIFT, HDMI_PKT_CON_PKT_TX_CON_MASK, HDMI_PKT_CON_PKT_TX_CON_SEND_ALWAYS);

    return 0;
}

void stop_general_control_packet()
{
    set_bits_in_reg(kHDMIModule, HDMI_GCP_CON, HDMI_PKT_CON_PKT_TX_CON_SHIFT, HDMI_PKT_CON_PKT_TX_CON_MASK, HDMI_PKT_CON_PKT_TX_CON_DISABLED);

    // Clear GCP data registers
    write_regs(kHDMIModule, HDMI_GCP_BYTE1, (uint32_t)0, 3);
}

void set_av_mute_enabled(bool enabled)
{
    set_bits_in_reg(kHDMIModule, HDMI_GCP_BYTE1, 0, GCP_SB0_CLR_AVMUTE|GCP_SB0_SET_AVMUTE, enabled ? GCP_SB0_SET_AVMUTE : GCP_SB0_CLR_AVMUTE);

    // Allow time for updated GCP to be transmitted
    task_sleep(frame_interval_MS + 1);

    debug(VIDEO,"AV MUTE %s after %u ms\n", enabled ? "enabled" : "disabled", (unsigned int)(frame_interval_MS + 1));
}


static int start_video_link(struct video_link_data *data)
{
    int tx_mode = get_tx_output_mode();
    int ret;

    if ( tx_mode != kHDMI_tx_mode_DVI ) {

        // start the AVI InfoFrame
        start_info_frame(&aviInfoFrame);
    
        // start the general control packet (GCP)
        ret = start_general_control_packet(data);
	require_noerr(ret, exit);
        
        // Enable Set_AVMUTE before enabling HDMI TX output
        set_av_mute_enabled(true);
    }

    // Enable HDMI core output (to PHY)
    ret = set_tx_output_enabled(true);
    require_noerr(ret, exit);

    task_sleep(kTMDSMinimumPowerOffIntervalMS);

    set_TMDS_power_enabled(true);

    ret = wait_for_PHY_ready();
    require_noerr(ret, exit);

    if ( tx_mode != kHDMI_tx_mode_DVI ) {
        // Allow time for video to stabilize before unmute
        task_sleep(kVideoStabilizationIntervalMS);

        // Send Clear_AVMUTE
        set_av_mute_enabled(false);
    }

    debug(VIDEO,"Finished Configuring Video\n");
    
    _current_video_link = *data;

exit:
    return ret;
}


static int complete_video_link()
{
	int ret;

	// Disable timing generator
	ret = set_timing_generator_enabled(false);
	require_noerr(ret, exit);

	// power down HDMI PHY, keeping HPD enabled
	power_down_phy(false);

	// Reset TX mode
	ret = set_tx_output_mode(kHDMI_tx_mode_None);
	require_noerr(ret, exit);

exit:
	return ret;
}

static int stop_video_link()
{
	int	tx_mode = get_tx_output_mode();
	int	ret = 0;
    
	if ( tx_mode != kHDMI_tx_mode_DVI ) {
		// Enable AVMUTE
		set_av_mute_enabled(true);
	}

	bzero(&_current_video_link, sizeof(_current_video_link));

	// Disable PHY TMDS power (see POWER DOWN SEQUENCE in HDMI TX V1.4 manual)
	set_TMDS_power_enabled(false);

	task_sleep(kTMDSMinimumPowerOffIntervalMS);

	if ( tx_mode != kHDMI_tx_mode_DVI ) {
		// Clear TX internal AVMUTE state
		set_av_mute_enabled(false);
	}

	ret = set_tx_output_enabled(false);
	require_noerr_action(ret, exit, debug(ERROR,"for set_tx_output_enabled: ret %d\n", ret));

	if ( tx_mode != kHDMI_tx_mode_DVI ) {
		struct display_infoframe local_aviInfoFrame; // dummy

		// stop the general control packet (GCP)
		stop_general_control_packet();
    
		// Stop info frame
		stop_info_frame(&local_aviInfoFrame);
	}


exit:
	return ret;
}

static int set_tx_output_enabled(bool enabled)
{
	// HDMI_CON_0[SYSTEM_EN] = enabled
	set_bits_in_reg(kHDMIModule, HDMI_CON_0, 0, HDMI_CON_0_SYSTEM_EN, enabled ? HDMI_CON_0_SYSTEM_EN : 0);

	return 0;
}

static int configure_video_mode(struct video_link_data *data, uint32_t pixel_clock)
{
	int ret;

	ret = prepare_video_link(data, pixel_clock);
	if (ret < 0) {
		debug(ERROR,"failed to prepare_video_link\n");
		return -1;
	}
	ret = start_video_link(data);
	if (ret < 0) {
		debug(ERROR,"failed to start_video_link\n");
		return -1;
	}

    return 0;
}

static void configure_video_bist(struct video_link_data *data)
{
	// enable bist mode
	if ( data->test_mode ) {
		or_reg(kHDMIModule, HDMI_VIDEO_PATTERN_GEN, HDMI_VIDEO_PATTERN_GEN_PATTERN_ENABLE);
		debug(VIDEO, "Finished Video BIST\n");
	} else {
		and_reg(kHDMIModule, HDMI_VIDEO_PATTERN_GEN, ~(HDMI_VIDEO_PATTERN_GEN_PATTERN_ENABLE));
	}
}

#define	HDMI_REG_OFFSET(size, m, o)	\
	*(volatile size *)(__base_address +_registers[m] + o)

static uint8_t read_reg(uint8_t module, uint32_t offset)
{
	uint8_t val;
	uint32_t addr;

	addr = __base_address + _registers[module] + offset;
	val = HDMI_REG_OFFSET(uint8_t, module, offset);
	debug(REG, "module=%d reg=0x%08x addr=0x%08x value=0x%08x\n", module, offset, addr, val);
	return(val);
}

static uint32_t read_reg32(uint8_t module, u_int32_t offset)
{
	uint32_t val;
	uint32_t addr;

	addr = __base_address + _registers[module] + offset;
	val = HDMI_REG_OFFSET(uint32_t, module, offset);
	debug(REG,"module=%d addr=0x%08x reg=0x%08x value=0x%08x\n", module, addr, offset, val);
	return(val);
}

static void write_reg(uint8_t module, uint32_t offset, uint8_t value)
{
	uint32_t addr;
	addr = __base_address + _registers[module] + offset;
	HDMI_REG_OFFSET(uint8_t, module, offset) = value;
	debug(REG,"module=%d reg=0x%08x addr=0x%08x value=0x%08x\n", module, offset, addr, value);
}

static void write_reg32(uint8_t module, u_int32_t offset, uint32_t value)
{
	uint32_t addr;
	addr = __base_address + _registers[module] + offset;
	HDMI_REG_OFFSET(uint32_t, module, offset) = value;
	debug(REG,"module=%d reg=0x%08x addr=0x%08x value=0x%08x\n", module, offset, addr, value);
}

static void and_reg(uint8_t module, uint32_t offset, uint8_t value)
{
	uint32_t addr;
	uint8_t reg;
	addr = __base_address + _registers[module] + offset;
	reg = read_reg(module, offset);
	reg &= value;
	write_reg(module, offset, reg);
}

static void or_reg(uint8_t module, uint32_t offset, uint8_t value)
{
	uint32_t addr;
	uint8_t reg;
	addr = __base_address + _registers[module] + offset;
	reg = read_reg(module, offset);
	reg |= value;
	write_reg(module, offset, reg);
}

static void set_bits_in_reg(uint8_t module, uint32_t offset, uint8_t pos, uint8_t mask, uint8_t value)
{
    uint8_t set = read_reg(module, offset);
    
    set &= ~mask;
    set |= (value << pos);
    
    write_reg(module, offset, set);
}

static uint32_t get_bits_in_reg(uint8_t module, uint32_t reg, uint32_t pos, uint32_t mask)
{
	    return (read_reg(module, reg) & mask) >> pos;
}
static uint8_t read_phy_reg(uint8_t reg)
{
	uint32_t Register;
	uint8_t data;
	uint8_t offset;
	uint8_t shift;


	offset = reg / 4;
	shift = reg % 4;

	Register = *(volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset]);

	//Now normalize its value
	data = Register >> shift;

	debug(REG, "read_phy_reg: hdmiphycon%x := 0x%08x addr 0x%08x\n", offset, data, _phyConfigRegs[offset]);

	return data;
}

#define HDMI_IO_RETRIES	20
bool get_is_PHY_write_in_progress(void)
{
	uint32_t	status;
	int		i;

	for (i = 0; i < HDMI_IO_RETRIES; i++) {
		task_sleep(1); //MS
		status = read_reg(kI2CModule, HDMIPHYCON_STAT);
		if (!(status & HDMIPHYCON_STAT_IN_PROGRESS))
			break;
		debug(REG, "Retry: %d HDMIPHYCON_STAT 0x%08x\n", i, status);

	}

	return (status & HDMIPHYCON_STAT_IN_PROGRESS);
}

#define HDMI_IO_TIMEOUT_US (10 * 1000)

static int wait_for_PHY_write_completion()
{
    return (get_is_PHY_write_in_progress() ? -1 : 0);
}

int write_phy_reg(uint32_t reg, const uint8_t data)
{
	int ret = 0;
	uint8_t offset;
	uint8_t shift;

	require_action((reg < PHY_CONFIG_REG_COUNT), exit, ret = -1); 

	require_action(!get_is_PHY_write_in_progress(), exit, ret = -1);

	if (reg == kPHYConfigRegModeSet) {
		offset = kPHYConfigRegModeSet;
		shift = 0;
	} else {
		offset = reg / 4;
		shift = (reg % 4) * 8;
	}

	//Put it in position
	*(volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset]) = ((*(volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset])) & ~(0xFF << shift)) | (data << shift);

	if (!bypass_wait && wait_for_PHY_write_completion()) {
		debug(INT, "%s %d: done waiting for stop event\n", __FUNCTION__, __LINE__);
	}
	debug(REG, "%s: reg 0x%08x := 0x%08x\n", __FUNCTION__, reg,  *(volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset]));

exit:
	return ret;
}

static int write_phy_config_reg(PHYConfigReg reg, uint32_t data)
{
	int    ret = 0;
	uint32_t Register;

	if (get_is_PHY_write_in_progress()) {
		debug(ERROR, "%s: Failed cuz write is in progress\n", __FUNCTION__);
		ret = -1;
		goto exit;
	}
	uint8_t offset;
	uint8_t shift;

	offset = reg / 4;
	shift = (reg % 4) * 8;

	//Put it in position
	Register = *(volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset]);
	Register &= ~(0xFF << shift);
	Register |= (data << shift);

	*(volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset]) = Register;

	debug(REG, "HDMIPHYCON%u := 0x%08x\n", offset, Register);
	debug(REG, "reg 0x%08x addr 0x%p := 0x%08x\n", reg, (volatile uint32_t *)((uintptr_t)_phyConfigRegs[offset]), Register);

	if (wait_for_PHY_write_completion()) {
		debug(REG, "%s %d: done waiting for stop event\n", __FUNCTION__, __LINE__);
	}

exit:
	return ret;
}

static void and_phy_reg(uint32_t offset, uint8_t data)
{
	uint8_t value;

	value = read_phy_reg(offset);
	value &= data;
	write_phy_reg(offset, value);
}

static void or_phy_reg(uint32_t offset, uint8_t data)
{
	uint8_t value;

	value = read_phy_reg(offset);
	value |= data;
	write_phy_reg(offset, value);
}

void write_regs_with_data(uint8_t module, uint32_t basereg, uint8_t * data, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++) {
		write_reg(module, basereg + (4 * i), data[i]);
	}
}

void write_regs(uint8_t module, uint32_t basereg, uint32_t value, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++) {
		write_reg(module, basereg + (4 * i), value & 0xff);
		value >>= 8;
	}
}
