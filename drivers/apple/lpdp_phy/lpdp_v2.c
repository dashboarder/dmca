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
#include <assert.h>
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

#include <drivers/displayport/displayport.h>
#include <drivers/displayport.h>
#include <drivers/lpdp_phy/lpdp_phy.h>
#include "regs_v2.h"

#if WITH_DEVICETREE
#include <lib/devicetree.h>
#endif

/////////////////////////////////////////
////////// debug support

#define LPDP_DEBUG_MASK ( 		\
		LPDP_DEBUG_INIT |			\
		LPDP_DEBUG_ERROR |	\
		LPDP_DEBUG_INFO |			\
		LPDP_DEBUG_PLL |			\
		LPDP_DEBUG_PHY |			\
		0)

#undef LPDP_DEBUG_MASK
#define LPDP_DEBUG_MASK 		(LPDP_DEBUG_INIT | LPDP_DEBUG_ERROR)

#define LPDP_DEBUG_INIT			(1<<16)  // initialisation
#define LPDP_DEBUG_ERROR		(1<<17)  // errors
#define LPDP_DEBUG_INFO			(1<<18)  // info
#define LPDP_DEBUG_PLL			(1<<24)  // PLL 
#define LPDP_DEBUG_PHY			(1<<25)  // PLL 
#define LPDP_DEBUG_ALWAYS		(1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)										\
	do {													\
		if ((LPDP_DEBUG_ ## _fac) & (LPDP_DEBUG_MASK | LPDP_DEBUG_ALWAYS))				\
			dprintf(DEBUG_CRITICAL, "DP: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)


#define kMaxLaneCount			4
#define kLinkRatePhysical_162gbpsi	1620000000ULL
#define kLinkRatePhysical_270gps	2700000000ULL

//ERRORS
#define	RET_SUCCESS		0
#define	RET_ERROR		-1

#ifndef LPDP_PHY_VERSION
#error LPDP_PHY_VERSION undefined
#endif

#ifndef LPDP_LINK_CAL_TABLE_VERSION
#error LPDP_LINK_CAL_TABLE_VERSION undefined
#endif

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

/*
    Device tree calibration data formats:

    link-calibration-type:  t700x-fixed
    link-calibration-data:  ${ VS DE }

    link-calibration-type:  t700x-training-table
    link-calibration-data:  ${ VS DE VS DE VS DE VS DE }    // voltage swing level 0, pre-emphasis levels 0~3
    link-calibration-data:  ${ VS DE VS DE VS DE 00 00 }    // voltage swing level 1, pre-emphasis levels 0~3
    link-calibration-data:  ${ VS DE VS DE 00 00 00 00 }    // voltage swing level 2, pre-emphasis levels 0~3
    link-calibration-data:  ${ VS DE 00 00 00 00 00 00 }    // voltage swing level 3, pre-emphasis level  0~3

    link-calibration-type:  s800x-fixed
    link-calibration-data:  $VS $R

    link-calibration-type:  s800x-training-table
    link-calibration-data:  $VS $R  $VS $R  $VS $R  $VS $R        // voltage swing level 0, pre-emphasis levels 0~3
    link-calibration-data:  $VS $R  $VS $R  $VS $R  $00 $00       // voltage swing level 1, pre-emphasis levels 0~3
    link-calibration-data:  $VS $R  $VS $R  $00 $00 $00 $00       // voltage swing level 2, pre-emphasis levels 0~3
    link-calibration-data:  $VS $R  $00 $00 $00 $00 $00 $00       // voltage swing level 3, pre-emphasis level  0~3
*/

#if LPDP_LINK_CAL_TABLE_VERSION < 2
struct lpdp_port_calibration {
	uint8_t			swing;
	uint8_t			deemphasis;
} __attribute__((packed));

static const char *training_table_text = "t700x-training-table";
static const char *fixed_text = "t700x-fixed";
#else
struct lpdp_port_calibration {
	uint32_t		swing;
	uint32_t		r;
} __attribute__((packed));

static const char *training_table_text = "s800x-training-table";
static const char *fixed_text = "s800x-fixed";
#endif

#if WITH_HW_DISPLAY_EDP
#include <target/lpdp_settings.h>

#ifndef LPDP_PORT_CALIBRATION_TABLE_FIXED
#error LPDP_PORT_CALIBRATION_TABLE_FIXED not defined
#endif

#else
static struct lpdp_port_calibration lpdp_port_calibration_table[kDPVoltageLevelMax+1][kDPEQLevelMax+1];
#endif

struct pll_timing {
	// reference clock frequency in MHz
	unsigned int    ref_mhz;

	// sequencer configuration
	unsigned int    ref_div         :  5;   // sequencer divisor
	unsigned int    setup_count     :  8;   // sequencer ticks
	unsigned int    start_count     :  8;   // sequencer ticks
	unsigned int    pwrdn_count     : 14;   // sequencer ticks
	unsigned int    reset_count     :  8;   // sequencer ticks
	unsigned int    update_count    :  8;   // sequencer ticks
	unsigned int    finish_count    :  8;   // sequencer ticks
	unsigned int    wakeup_count    :  8;   // sequencer ticks
	unsigned int    hold_count	:  8;   // sequencer ticks
};

typedef enum {
	lpdp_pll_state_off	= 0,
	lpdp_pll_state_on	= 1,
	lpdp_pll_state_unknown	= 2,
} lpdp_pll_state_t;

/////////////////////////////////////////
////////// PHY local variables
static addr_t __base_address = LPDP_PHY_BASE_ADDR;
static uint32_t _linkRate;
static bool lpdp_pll_state = true;
static uint32_t lpdp_voltage_levels[kMaxLaneCount];
static uint32_t lpdp_voltage_base[kMaxLaneCount];
static uint32_t lpdp_eq_levels[kMaxLaneCount];
static uint32_t lpdp_voltage_levels[kMaxLaneCount];
static bool lpdp_port_calibration_table_fixed;
static uint32_t	pll_vco_rctrl;
static struct pll_timing lpdp_pll_timing;
static uint32_t t_cal_duration_microseconds;

/////////////////////////////////////////
////////// PHY local functions
static void set_bias_power_enable(bool enable);
static void set_aux_power_enable(bool enable);
static void set_lane_power_controls(unsigned int first, unsigned int limit, uint32_t mask, uint32_t bits);
static void set_lane_power_enable(bool enable);
static bool lpdp_get_pll_is_locked();
static void set_aux_voltage_swing(uint32_t vreg_adj);
#if LPDP_PHY_VERSION < 2
static void set_phy_configure_ldos(uint32_t ldopre_vreg_adj, uint32_t ldoclk_vreg_adj, uint32_t auxvreg_adj);
#endif
static void enable_phy_ldos();
#if LPDP_LINK_CAL_TABLE_VERSION < 2
static void set_lane_adjustment_levels(unsigned int lane, uint8_t vreg_adj, uint8_t eq);
#else
static void set_lane_adjustment_levels(unsigned int lane, uint32_t vreg_adj, uint32_t r);
#endif
static int update_pll_dividers(uint32_t lr);
static int  lpdp_power_pll(bool poweron);
static int  lpdp_power_down_pll();
static int  lpdp_power_up_pll();
static int lpdp_phy_impedance_calibration(void);
static unsigned int micro_seconds_for_count(struct pll_timing *tm, unsigned int count);
static unsigned int get_sleep_to_power_down_duration(struct pll_timing *tm);
static unsigned int get_power_down_duration(struct pll_timing *tm);
static unsigned int get_reset_duration(struct pll_timing *tm);
static unsigned int get_reset_to_update_duration(struct pll_timing *tm);
static unsigned int get_lock_duration(struct pll_timing *tm);
static uint32_t read_reg(uint32_t offset);
static void write_reg(uint32_t offset, uint32_t val);
static void and_reg(uint32_t offset, uint32_t val);
static void or_reg(uint32_t offset, uint32_t val);
static void set_bits_in_reg(uint32_t offset, uint32_t pos, uint32_t mask, uint32_t value);

/////////////////////////////////////////
////////// PHY global functions

int lpdp_init(const char *dt_path)
{
	int		i;

	for (i = 0; i < kMaxLaneCount; i++)
		lpdp_voltage_base[i] = kBaseVoltageType_Pos_0mV;

	t_cal_duration_microseconds = 1000; // 1ms

	lpdp_pll_state = lpdp_pll_state_unknown;

	// Note: 200uS is the worst-case from pll_pwrdn to PLL locked under all
	//       conditions and settings per <rdar://problem/15238851>.
	//       The period from pll_pwrdn to PLL locked is:
	//          reset_count + update_count + finish_count

	lpdp_pll_timing.ref_mhz      = 24;
	lpdp_pll_timing.ref_div      = 24;   // ref clock ticks per (1µs) sequencer tick
	lpdp_pll_timing.setup_count  = 4;    // µs
	lpdp_pll_timing.start_count  = 1;    // µs
	lpdp_pll_timing.pwrdn_count  = 10;   // µs
	lpdp_pll_timing.reset_count  = 100;  // µs
	lpdp_pll_timing.update_count = 5;    // µs
	lpdp_pll_timing.finish_count = 50;  // µs

#if WITH_HW_DISPLAY_EDP
	lpdp_port_calibration_table_fixed = LPDP_PORT_CALIBRATION_TABLE_FIXED;
#else
#if WITH_DEVICETREE
	DTNodePtr	node;
	char *		prop_name;
	void *		prop_data;
	uint32_t	prop_size;

	// Copy DT settings in to local copy
	if ( FindNode(0, dt_path, &node) ) {
		
		prop_name = "link-calibration-type";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			if (strcmp(training_table_text, prop_data) == 0) {
				lpdp_port_calibration_table_fixed = false;
			} else if (strcmp(fixed_text, prop_data) == 0) {
				lpdp_port_calibration_table_fixed = true;
			} else {
				panic("Unknown DT LPDP Calibration Table Type.");
			}
		} else {
			panic("Missing DT LPDP Calibration Table Type.");
		}
		
		// gather calibration data from DeviceTree
		prop_name = "link-calibration-data";
		if ( FindProperty(node, &prop_name, &prop_data, &prop_size) ) {
			uint32_t table_size = (lpdp_port_calibration_table_fixed) ? 
				sizeof(struct lpdp_port_calibration) : sizeof(struct lpdp_port_calibration) * 16;
			if ( prop_size != table_size ) {
	  				debug(ERROR, "calibration-data size mismatch, expected:%d, read:%d \n",
					table_size, prop_size);
				return -1;
			}

			bcopy(prop_data, (void *)lpdp_port_calibration_table, prop_size);
		} else {
			panic("Missing DT LPDP Calibration Table Data.");
		}
	} else 	
#endif
	
	{
		debug(ERROR, "Missing DT LPDP Calibration Table. Using Defaults\n");
		lpdp_port_calibration_table_fixed = true;
#if LPDP_LINK_CAL_TABLE_VERSION < 2
		lpdp_port_calibration_table[0][0].swing = LPDP_PHY_LANE_x_VREG_ADJ_MAX;
		lpdp_port_calibration_table[0][0].deemphasis = 0;
#else
		lpdp_port_calibration_table[0][0].swing = LPDP_PHY_LANE_x_VREG_ADJ_MAX;
		lpdp_port_calibration_table[0][0].r = 0;
#endif
	}
#endif

	//the block should had been properly reset. validating such assumption
	assert(read_reg(rLPDP_PHY_GEN_CTRL) & (LPDP_PHY_GEN_CTRL_SEQ_OW | LPDP_PHY_GEN_CTRL_LANE_PD_OW));
        assert(read_reg(rLPDP_PLL_GEN) & LPDP_PLL_GEN_RST);

	//extract Configuration of PLL
	//PMGR should have programmed correctly
	//we save the value to pass to the OS.
	pll_vco_rctrl = read_reg(rLPDP_PLL_CLK);
	if (pll_vco_rctrl & LPDP_PLL_CLK_VCO_RCTRL_SEL_ENABLE)
		pll_vco_rctrl = ((pll_vco_rctrl >> LPDP_PLL_CLK_VCO_RCTRL_OW_SHIFT) & LPDP_PLL_CLK_VCO_RCTRL_OW_MASK);
	else
		pll_vco_rctrl = 0;

	return RET_SUCCESS;
}

int lpdp_initialize_phy_and_pll(void)
{
	int ret = RET_SUCCESS;
	int i;

	// ensure lane_pd_ow=1 (default) to allow software control of lane power
	or_reg(rLPDP_PHY_GEN_CTRL,  (LPDP_PHY_GEN_CTRL_SEQ_OW | LPDP_PHY_GEN_CTRL_SLEEP_SW ));

#if LPDP_PHY_VERSION < 2
	for (i = 0 ; i < kMaxLaneCount; i++) {
		set_lane_adjustment_levels(i, LPDP_PHY_LANE_VREG_ADJ_420_mV, 0);
	}

	//TODO!!
	set_aux_voltage_swing(0xA);

	set_phy_configure_ldos(0x4, 0x4, 0x3);
#elif LPDP_PHY_VERSION == 2
	for (i = 0 ; i < kMaxLaneCount; i++) {
		set_lane_adjustment_levels(i, LPDP_PHY_LANE_VREG_ADJ_450_mV, 0);
	}

	//TODO!!
	set_aux_voltage_swing(0x6);
#endif

	//reset calibration
	write_reg(rLPDP_PHY_CAL, 0);

#if LPDP_PHY_VERSION < 2
	write_reg(rLPDP_PHY_RESERVED, (1 << 4));
#endif

	write_reg(rLPDP_PHY_CLK_CTRL, LPDP_PHY_CLK_CTRL_CLK_ENABLE | LPDP_PHY_CLK_CTRL_CLKDIV5_RESET);

	or_reg(rLPDP_PHY_GEN_CTRL,  LPDP_PHY_GEN_CTRL_LANE_PD_OW);

	set_bias_power_enable(true);

	// power up all lanes
	set_lane_power_enable(true);

	enable_phy_ldos();

	set_aux_power_enable(true);

#if LPDP_PHY_VERSION < 2
	and_reg(rLPDP_PHY_BIST, ~LPDP_PHY_BIST_LOOPBACK_SRC_SEL(LPDP_PHY_BIST_LOOPBACK_SRC_SEL_MASK));
#endif

	spin(1000); //1 ms
	spin(10); //10 us

	write_reg(rLPDP_PHY_CAL, LPDP_PHY_CAL_RESET_N);
#if LPDP_PHY_VERSION < 2
	and_reg(rLPDP_PHY_RESERVED, ~(1 << 4));
#endif
	and_reg(rLPDP_PHY_CLK_CTRL, ~LPDP_PHY_CLK_CTRL_CLKDIV5_RESET);

	spin(10); //10 us

	// perform PHY impedance calibration
	ret = lpdp_phy_impedance_calibration();

	// panic if impedance calibration procedure failed
	if ( ret != 0 ) {
		panic("lpdp impedance calibration failed\n");
	}

	set_lane_power_controls(0, kMaxLaneCount, LPDP_PHY_LANE_HI_Z, 0);

	//configure PLL to an initial value
	lpdp_set_link_rate(kLinkRate162Gbps);

#if LPDP_PHY_VERSION < 2
	write_reg(rLPDP_GEN_DISPLAY_SPLIT, 0);
#endif

	return ret;
}

void lpdp_init_finalize(void)
{
	//remove the overrides
	and_reg(rLPDP_PHY_GEN_CTRL,  ~(LPDP_PHY_GEN_CTRL_SEQ_OW | LPDP_PHY_GEN_CTRL_LANE_PD_OW));
}

void lpdp_quiesce()
{
	// power down all lanes [LANE_x.hi_z=1, LANE_x.ldo_pwrdn=1, LANE_x.pwrdn=1]
	set_lane_power_enable(false);

	// power down AUX channel [AUX_CTRL.pwrdn=1]
	set_aux_power_enable(false);

	// power down PHY's central bias [GEN_CTRL.bias_pwrdn=1]
	set_bias_power_enable(false);

	// assert lpdp_sleep, pll_pwrdn, and pll_reset
	lpdp_power_pll(false);
	
}

int lpdp_set_link_rate(uint32_t lr)
{
	int		ret = 0;

	debug(PHY, "Setting link rate to 0x%02x\n", (uint8_t)lr);

#if LPDP_PHY_VERSION == 2
	//<rdar://problem/20158240> Bug Fix for Default Register Value of asg_adppll Calibration Reset
	or_reg(rLPDP_PLL_FCAL, LPDP_PLL_FCAL_RESET);
#endif
	// Block clock output during PLL setup
	or_reg(rLPDP_PLL_CLK, LPDP_PLL_CLK_VCO_BLK_VCLK);

	// assert lpdp_sleep, pll_pwrdn, and pll_reset
	lpdp_power_pll(false);

	// initialize link rate to 0
	_linkRate = kLinkRate000Gbps;

	// if the target link rate is nonzero, reconfigure and power up the PLL
	if ( lr > kLinkRate000Gbps ) {

		// update PLL dividers for new link rate
		if (update_pll_dividers(lr)) {
			ret = -1;
			goto exit;
		}

		// power up PLL and wait for lock,
		//  if this fails, power it back down and leave link rate at 0
		if (lpdp_power_pll(true)) {
			ret = -1;
			lpdp_power_pll(false);
			goto exit;
		}

		// finally, update the current link rate
		_linkRate = lr;
	}

exit:
	return ret;
}

int lpdp_get_link_rate(uint32_t *link_rate)
{
		*link_rate = _linkRate;
		return RET_SUCCESS;
}

int lpdp_phy_set_adjustment_levels(uint32_t lane, uint32_t voltage_swing, uint32_t eq, 
					bool *voltage_max_reached, bool *eq_max_reached)
{
	int		ret = 0;

	if ( lane > kMaxLaneCount )
		return -1;

	debug(PHY, "lane=%d voltage=%d eq=%d\n", lane, voltage_swing, eq);

	if (voltage_swing > kDPVoltageLevelMax) {
		voltage_swing = kDPVoltageLevelMax;
	}

	if (eq > kDPEQLevelMax) {
		eq = kDPEQLevelMax;
	}

	if (lpdp_port_calibration_table_fixed) {
#if LPDP_LINK_CAL_TABLE_VERSION < 2
		set_lane_adjustment_levels(lane, 
			lpdp_port_calibration_table[0][0].swing,
			lpdp_port_calibration_table[0][0].deemphasis);
#else
		set_lane_adjustment_levels(lane, 
			lpdp_port_calibration_table[0][0].swing,
			lpdp_port_calibration_table[0][0].r);
#endif
	} else {
#if LPDP_LINK_CAL_TABLE_VERSION < 2
		set_lane_adjustment_levels(lane, 
			lpdp_port_calibration_table[voltage_swing][eq].swing,
			lpdp_port_calibration_table[voltage_swing][eq].deemphasis);
#else
		set_lane_adjustment_levels(lane, 
			lpdp_port_calibration_table[voltage_swing][eq].swing,
			lpdp_port_calibration_table[voltage_swing][eq].r);
#endif			
	}
	
	if (voltage_max_reached) {
		*voltage_max_reached = (voltage_swing == kDPVoltageLevelMax);
	}
	
	if (eq_max_reached) {
		*eq_max_reached = (eq == kDPEQLevelMax);
	}
	
	return ret;
}

void lpdp_phy_reset()
{
	lpdp_phy_impedance_calibration();
}

bool lpdp_get_supports_downspread()
{
	return false;
}

int lpdp_set_downspread(bool value)
{
	//not supported but needs to succeed.. hence noop
	return RET_SUCCESS;
}

int lpdp_get_downspread(void)
{
	//not supported but needs to succeed.. hence noop
	return RET_SUCCESS;
}

int lpdp_phy_get_adjustment_levels(uint32_t lane, uint32_t *voltage_swing, uint32_t *eq)
{
	if (eq)
		*eq = lpdp_eq_levels[lane];
	
	if (voltage_swing)
		*voltage_swing = lpdp_voltage_levels[lane];

	return RET_SUCCESS;
}

void lpdp_phy_set_lane_count(const uint32_t lane_count)
{
	// enable active lanes
	set_lane_power_controls(0, lane_count, LPDP_PHY_LANE_HI_Z, 0);

	// disable inactive lanes
	set_lane_power_controls(lane_count, kMaxLaneCount, LPDP_PHY_LANE_HI_Z, LPDP_PHY_LANE_HI_Z);
}

#if WITH_DEVICETREE

#include <lib/devicetree.h>

int lpdp_phy_update_device_tree(DTNode *lpdp_node)
{
	u_int32_t	propSize;
	char		*propName;
	void		*propData;

	if (lpdp_node == NULL) {
		return RET_ERROR;
	}
	propName = "pll_vco_rctrl";
	if (FindProperty(lpdp_node, &propName, &propData, &propSize)) {
		if (pll_vco_rctrl == 0) {
			((char **)propData)[0] = "~";
		} else {
			((u_int32_t *)propData)[0] = pll_vco_rctrl;
		}
	}
		
#if WITH_HW_DISPLAY_EDP
	// Copy local settings to the DT
	propName = "link-calibration-type";
	if ( FindProperty(lpdp_node, &propName, &propData, &propSize) ) {
		memset(propData, 0, propSize);
		if (lpdp_port_calibration_table_fixed) {
			if (propSize < strlen(fixed_text) + 1) {
				panic("link-calibration-type DT entry too small.");
			}			
			snprintf(propData, propSize, "%s", fixed_text);
 		} else {
			if (propSize < strlen(training_table_text) + 1) {
				panic("link-calibration-type DT entry too small.");
			}
			snprintf(propData, propSize, "%s", training_table_text);
		}
	} else {
		panic("Missing DT LPDP Calibration Table Type.");
	}

	propName = "link-calibration-data";
	if ( FindProperty(lpdp_node, &propName, &propData, &propSize) ) {
		memset(propData, 0, propSize);
		uint32_t table_size = (lpdp_port_calibration_table_fixed) ? 
			sizeof(struct lpdp_port_calibration) : sizeof(struct lpdp_port_calibration) * 16;
		if (propSize < table_size) {
			panic("link-calibration-data table too small.");
		}
		bcopy((void *)lpdp_port_calibration_table, propData, table_size);
	} else {
		panic("Missing DT LPDP Calibration Table Data.");
	}
#endif

	return RET_SUCCESS;
}
#endif //WITH_DEVICETREE

/////////////////////////////////////////
////////// PHY local functions

static int lpdp_phy_impedance_calibration( void )
{
	int		result = RET_SUCCESS;
	uint32_t	reg;

	// Let the pull down calibration start
	or_reg(rLPDP_PHY_CAL, LPDP_PHY_CAL_START);

	// Wait for calibration to complete
	spin(t_cal_duration_microseconds);

	reg = read_reg(rLPDP_PHY_CAL);
	if (!(reg & LPDP_PHY_CAL_DONE) || (reg &  LPDP_PHY_CAL_FAIL)) {
		printf("impedance Calibration failed rLPDP_PHY_CAL 0x%x\n", reg);
		result = RET_ERROR;
	}

	spin(1);

	and_reg(rLPDP_PHY_CAL, ~LPDP_PHY_CAL_START);

	return result;
}

static void set_bias_power_enable(bool enable)
{
	uint32_t rmw = read_reg(rLPDP_PHY_GEN_CTRL);
	rmw &= ~( LPDP_PHY_GEN_CTRL_BIAS_PWRDN);
	rmw |= ( enable ? 0 : LPDP_PHY_GEN_CTRL_BIAS_PWRDN);
	write_reg(rLPDP_PHY_GEN_CTRL, rmw);
}

static void set_aux_power_enable(bool enable)
{
	uint32_t rmw = read_reg(rLPDP_PHY_AUX_CTRL);
	rmw &= ~(LPDP_PHY_AUX_CTRL_PWRDN);
	rmw |= ( enable ? 0 : LPDP_PHY_AUX_CTRL_PWRDN);
	write_reg(rLPDP_PHY_AUX_CTRL, rmw);
}

static void set_lane_power_enable(bool enable)
{
	uint32_t    bits = enable ? 0 : LPDP_PHY_LANE_x_FULL_DISABLE;

	set_lane_power_controls(0, kMaxLaneCount, LPDP_PHY_LANE_x_FULL_DISABLE, bits);
}

static void set_lane_power_controls(unsigned int first, unsigned int limit, uint32_t mask, uint32_t bits)
{
	for (unsigned int i = first; i < limit; i++) {
		// allow at least 5ns between lane power/enable state changes
		spin(1);

		// set power state of lane 'i'
		uint32_t rmw = read_reg(rLPDP_PHY_LANE(i));
		rmw &= ~mask;
		rmw |= bits;
		write_reg(rLPDP_PHY_LANE(i), rmw);
	}
}

static bool lpdp_get_pll_is_locked()
{
	return (read_reg(rLPDP_PLL_LOCK) & LPDP_PLL_LOCK_OUT_ON);
}

#if LPDP_PHY_VERSION < 2
static void set_phy_configure_ldos(uint32_t ldopre_vreg_adj, uint32_t ldoclk_vreg_adj, uint32_t auxvreg_adj)
{
	uint32_t reg;

	reg = read_reg(rLPDP_PHY_PRE_LDO_CTRL);
	reg &= ~(LPDP_PHY_PRE_LDO_CTR_LDOPRE_VREG_ADJ(0xf));
	reg |= LPDP_PHY_PRE_LDO_CTR_LDOPRE_VREG_ADJ(ldopre_vreg_adj);
	write_reg(rLPDP_PHY_PRE_LDO_CTRL, reg);

	reg = read_reg(rLPDP_PHY_CLK_LDO_CTRL);
	reg &= ~LPDP_PHY_CLK_LDO_CTRL_LDOCLK_VREG_ADJ(0xf);
	reg |= LPDP_PHY_CLK_LDO_CTRL_LDOCLK_VREG_ADJ(ldoclk_vreg_adj);
	write_reg(rLPDP_PHY_CLK_LDO_CTRL, reg);

	reg = read_reg(rLPDP_PHY_AUX_LDO_CTRL);
	reg &= ~LPDP_PHY_AUX_LDO_CTRL_AUXVREG_ADJ(0xf);
	reg |= LPDP_PHY_AUX_LDO_CTRL_AUXVREG_ADJ(auxvreg_adj);
	write_reg(rLPDP_PHY_AUX_LDO_CTRL, reg);
}
#endif

static void enable_phy_ldos()
{
#if LPDP_PHY_VERSION < 2
	and_reg(rLPDP_PHY_CLK_LDO_CTRL, ~LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN(LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_MAX));
	and_reg(rLPDP_PHY_PRE_LDO_CTRL, ~LPDP_PHY_PRE_LDO_CTR_LDOPRE_PWRDN(LPDP_PHY_PRE_LDO_CTR_LDOPRE_PWRDN_MAX));
	and_reg(rLPDP_PHY_AUX_LDO_CTRL, ~LPDP_PHY_AUX_LDO_CTRL_AUXLDO_PWRDN);
#elif LPDP_PHY_VERSION == 2
	and_reg(rLPDP_PHY_CLK_LDO_CTRL, ~(LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_SML(LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_SML_MAX)|
					  LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN(LPDP_PHY_CLK_LDO_CTRL_LDOCLK_PWRDN_MAX)));
	and_reg(rLPDP_PHY_AUX_LDO_CTRL, ~(LPDP_PHY_AUX_LDO_CTRL_LDOPOST_PWRDN_SML|LPDP_PHY_AUX_LDO_CTRL_LDOCLK_PWRDN_SML|
					  LPDP_PHY_AUX_LDO_CTRL_LDOPOST_PWRDN_BIG|LPDP_PHY_AUX_LDO_CTRL_LDOCLK_PWRDN_BIG));
#endif
}

static void set_aux_voltage_swing(uint32_t vreg_adj)
{
	uint32_t rmw = read_reg(rLPDP_PHY_AUX_CTRL);
	rmw &= ~LPDP_PHY_AUX_CTRL_VREG_ADJ_MASK;
	rmw |= vreg_adj << LPDP_PHY_AUX_CTRL_VREG_ADJ_SHIFT;
	write_reg(rLPDP_PHY_AUX_CTRL, rmw);
}


#if LPDP_LINK_CAL_TABLE_VERSION < 2
static void set_lane_adjustment_levels(unsigned int lane, uint8_t voltage_swing, uint8_t eq)
#else
static void set_lane_adjustment_levels(unsigned int lane, uint32_t voltage_swing, uint32_t r)
#endif
{
	uint32_t rmw = read_reg(rLPDP_PHY_LANE(lane));
	rmw &= ~(LPDP_PHY_LANE_x_VREG_ADJ_MASK);
	rmw |= LPDP_PHY_LANE_VREG_ADJ(voltage_swing);
	write_reg(rLPDP_PHY_LANE(lane), rmw);
	
	lpdp_voltage_levels[lane] = voltage_swing;
#if LPDP_LINK_CAL_TABLE_VERSION < 2
	lpdp_eq_levels[lane] = eq;
#else	
#if LPDP_LINK_CAL_TABLE_VERSION < 3
	// Maui
	lpdp_eq_levels[lane] = (((((rLPDP_PHY_CAL_TX & LPDP_PHY_CAL_TX_CODE_FSM_MASK) >> LPDP_PHY_CAL_TX_CODE_FSM_SHIFT) * r) / 2) + (1 << 15)) >> 16;
#else
	// Elba/Malta +
	lpdp_eq_levels[lane] = ((((((rLPDP_PHY_CAL_TX & LPDP_PHY_CAL_TX_CODE_FSM_MASK) >> LPDP_PHY_CAL_TX_CODE_FSM_SHIFT) + 24) * r) / 2) + (1 << 15)) >> 16;
#endif
#endif
}

//TODO: Get the correct values for gclk_div
static int update_pll_dividers(uint32_t lr)
{
	int		ret = 0;
	uint32_t	fb;
	uint32_t	pre;

	// determine feedback and pre-divider ratios
	switch ( lr ) {
	case kLinkRate000Gbps:
		fb = pre = 0;
		break;

	case kLinkRate162Gbps:
		fb  = 135;
		pre = 4;
		break;
			
	case kLinkRate270Gbps:
		fb  = 225;
		pre = 4;
		break;
			
	case kLinkRate324Gbps:
		fb  = 135;
		pre = 2;
		break;
			
	case kLinkRate540Gbps: // Not supported on Fiji/Capri
		fb  = 225;
		pre = 2;
		break;

	default:
		debug(ERROR, "unsupported link rate: %u", lr);
		ret = -1;
		goto exit;
	}

	// Set new PLL divider ratios
	uint32_t rmw = read_reg(rLPDP_PLL_IDIV);
	rmw &= ~LPDP_PLL_IDIV_FB_MASK;
	rmw |= (fb << LPDP_PLL_IDIV_FB_SHIFT);
	rmw &=  ~LPDP_PLL_IDIV_PRE_MASK;
	rmw |= (pre << LPDP_PLL_IDIV_PRE_SHIFT);
	write_reg(rLPDP_PLL_IDIV, rmw);
	and_reg(rLPDP_PLL_GEN, ~(LPDP_PLL_GEN_GCLK_DIV(LPDP_PLL_GEN_GCLK_DIV_MASK))); 

exit:
	return ret;
}

static int  lpdp_power_pll(bool poweron)
{
	int  result = RET_ERROR;

	if ((lpdp_pll_state != lpdp_pll_state_unknown) && (lpdp_pll_state == poweron))
		return RET_SUCCESS;

	lpdp_pll_state = poweron;
	if (poweron)
		result = lpdp_power_up_pll();
	else
		result = lpdp_power_down_pll();
	return result;
}

static int lpdp_power_down_pll()
{
	or_reg(rLPDP_PHY_GEN_CTRL,  (LPDP_PHY_GEN_CTRL_SEQ_OW | LPDP_PHY_GEN_CTRL_SLEEP_SW));

	spin(31); //> 30 ns

	// de-assert pll_pwrdn after at least 10µS
	and_reg(rLPDP_PLL_GEN, ~LPDP_PLL_GEN_PWRDN);

	//spin(get_reset_duration(&lpdp_pll_timing));
	spin(100);

	or_reg(rLPDP_PLL_CLK, LPDP_PLL_CLK_VCO_BLK_VCLK);
	// de-assert pll_reset after at least 100µS
	and_reg(rLPDP_PLL_GEN, ~LPDP_PLL_GEN_RST);
	or_reg(rLPDP_PLL_GEN,  (LPDP_PLL_GEN_PWRDN | LPDP_PLL_GEN_RST));

	return RET_SUCCESS;
}

static int lpdp_power_up_pll()
{
	// Note: powerDownPll() must be called first
	assert(read_reg(rLPDP_PHY_GEN_CTRL & LPDP_PHY_GEN_CTRL_SEQ_OW));
	assert(read_reg(rLPDP_PHY_GEN_CTRL & LPDP_PHY_GEN_CTRL_SLEEP_SW));
	assert(read_reg(rLPDP_PLL_GEN      & LPDP_PLL_GEN_PWRDN));
	assert(read_reg(rLPDP_PLL_GEN      & LPDP_PLL_GEN_RST));

	//spin(get_power_down_duration(&lpdp_pll_timing));
	spin(10);

	// de-assert pll_pwrdn after at least 10µS
	and_reg(rLPDP_PLL_GEN, ~LPDP_PLL_GEN_PWRDN);

	//spin(get_reset_duration(&lpdp_pll_timing));
	spin(100);

	// de-assert pll_reset after at least 100µS
	and_reg(rLPDP_PLL_GEN, ~LPDP_PLL_GEN_RST);

	//spin(get_reset_to_update_duration(&lpdp_pll_timing));
	spin(5);

	// assert pll_update_divn for at least 1µS
	or_reg(rLPDP_PLL_IDIV, LPDP_PLL_IDIV_UPDT);
	spin(1);
	and_reg(rLPDP_PLL_IDIV, ~LPDP_PLL_IDIV_UPDT);

	// wait for PLL lock
	//spin(get_lock_duration(&lpdp_pll_timing));
	spin(50);

	// check for PLL lock, bail if not locked
	if (!lpdp_get_pll_is_locked()) {
		printf("failed to lock lpdp pll\n");
		return RET_ERROR;
	}
	
	// Enable clock output after PLL is locked
	and_reg(rLPDP_PLL_CLK, ~LPDP_PLL_CLK_VCO_BLK_VCLK);

	// de-assert lpdp_sleep

	//  Note: The sleep signal is used by the LPDP PHY to gate-off PLL clock;
	//        see <rdar://problem/15238851>.
	and_reg(rLPDP_PHY_GEN_CTRL, ~LPDP_PHY_GEN_CTRL_SLEEP_SW);

	// wait at least 1µS for output driver ready
	spin(2);

	or_reg(rLPDP_PLL_GEN,  LPDP_PLL_GEN_CLKDIV2_RESET_N);

	return RET_SUCCESS;
}

void lpdp_phy_configure_alpm(bool enable)
{

	if (enable) {
		// Clear Sequencer overwrite and Lane power-down overwrite
		and_reg(rLPDP_PHY_GEN_CTRL, ~(LPDP_PHY_GEN_CTRL_SLEEP_SW | LPDP_PHY_GEN_CTRL_LANE_PD_OW));

		// Program ALPM PHY Sequence Timers
		write_reg(rLPDP_GEN_SEQ_1, LPDP_GEN_SEQ_1_START_COUNT(lpdp_pll_timing.start_count));
		or_reg(rLPDP_GEN_SEQ_1, LPDP_GEN_SEQ_1_SETUP_COUNT(lpdp_pll_timing.setup_count));
		//Hardcodede value to be fixed in <rdar://problem/16786136> Make display_config a void * for future flexibility
		write_reg(rLPDP_GEN_SEQ_2,  1873);
		write_reg(rLPDP_GEN_SEQ_3, lpdp_pll_timing.reset_count);
		write_reg(rLPDP_GEN_SEQ_4, LPDP_GEN_SEQ_4_HOLD_COUNT(lpdp_pll_timing.hold_count));
		write_reg(rLPDP_GEN_SEQ_5, LPDP_GEN_SEQ_5_FINISH_COUNT(lpdp_pll_timing.finish_count));
		or_reg(rLPDP_GEN_SEQ_4, LPDP_GEN_SEQ_4_UPDATE_COUNT(lpdp_pll_timing.update_count));
		write_reg(rLPDP_GEN_SEQ_5, LPDP_GEN_SEQ_5_WAKEUP_COUNT(lpdp_pll_timing.wakeup_count));
	}

}

// convenience methods for conversion to µs (for software use)

static unsigned int micro_seconds_for_count(struct pll_timing *tm, unsigned int count) {
	// Note: Normally, ref_mhz == ref_div, but it is not assumed.
	//       Values are rounded up.
	return ((count * tm->ref_div) + tm->ref_mhz - 1) / tm->ref_mhz;
}

static unsigned int get_sleep_to_power_down_duration(struct pll_timing *tm) {
	return micro_seconds_for_count(tm, tm->start_count);
}

static unsigned int get_power_down_duration(struct pll_timing *tm) {
	return micro_seconds_for_count(tm, tm->pwrdn_count);
}

static unsigned int get_reset_duration(struct pll_timing *tm) {
	return micro_seconds_for_count(tm, tm->reset_count);
}

static unsigned int get_reset_to_update_duration(struct pll_timing *tm) {
	return micro_seconds_for_count(tm, tm->update_count);
}

static unsigned int get_lock_duration(struct pll_timing *tm) {
	return micro_seconds_for_count(tm, tm->finish_count);
}

static uint32_t read_reg(uint32_t offset)
{
	volatile uint32_t reg;

	reg = (*(volatile uint32_t *)(__base_address + offset));
	return reg;
}

static void write_reg(uint32_t offset, uint32_t value)
{
	*(volatile uint32_t *)(__base_address + offset) = value;
}

static void and_reg(uint32_t offset, uint32_t value)
{
	uint32_t reg;
	reg = read_reg(offset);
	reg &= value;
	write_reg(offset, reg);
}

static void or_reg(uint32_t offset, uint32_t value)
{
	uint32_t reg;
	reg = read_reg(offset);
	reg |= value;
	write_reg(offset, reg);
}

static void set_bits_in_reg(uint32_t offset, uint32_t pos, uint32_t mask, uint32_t value)
{
	uint32_t set = read_reg(offset);
	
	set &= ~mask;
	set |= (value << pos);
	
	write_reg(offset, set);
}
