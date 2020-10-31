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
#include <drivers/mipi.h>
#include <drivers/mipi/mipi.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/chipid.h>
#include <target/gpiodef.h>

#include "dsim.h"

#define CEIL_DIV(a,b)			(((a)/(b)) + (((a) % (b)) ? 1 : 0))

static u_int32_t lane_count;

static struct phy_settings *phy_settings_info;

static void mipi_program_phy_test_codes(struct phy_settings *phy_settings_info);
static void mipi_program_phy_hsfreqrange(struct phy_settings *phy_settings_info);
static void mipi_phy_write(uint32_t address, uint8_t *data, uint32_t length);
static void mipi_phy_read_mod_write(uint32_t address, uint8_t mask, uint8_t shift, uint8_t val);
static void mipi_phy_test_reset(bool reset);
static void mipi_phy_toggle_shutdownz(void);
static bool mipi_is_interface_enabled();

/*
 * The Programming sequences of MIPI DSI can be broken down in the following parts:
 * Static configurations
 * Main sequence
 * Sequence for generic packets
 */
int mipi_dsim_init(struct display_timing *timing, enum colorspace color)
{
	const char		*env;
	uint32_t		dpi_color_coding, vid_size, bytes_per_pixel;
	uint64_t		lane_byte_clock_period, lane_byte_clock, pixel_clock;
	uint64_t		esc_clock;
	uint32_t		total_pixels_per_line;
	uint32_t		status_mask;
	uint32_t		cmd_mode;
	uint32_t		outvact_lpcmd_time, invact_lpcmd_time;
	uint32_t		hsa_time, hbp_time, hline_time, hss_time;
	uint64_t		time_calculation;
	uint64_t		hline_time_ns;
	uint32_t		max_rd_time;
	uint32_t		pll_config;
	uint32_t		hs_freq_range;
	mipi_t			*display_config;

	dprintf(DEBUG_CRITICAL, "mipi_dsim_init()\n");

	env = env_get("display-timing");
	if (env == 0) env = "";

	pll_config = 0;
	hs_freq_range = 0;

	display_config = (mipi_t *)(timing->display_config);
	assert(display_config != NULL);

	phy_settings_info = &display_config->target_phy_settings;
	if (phy_settings_info == 0) {
		panic("Failed to find display timing info");
	}

	// Extract Lane configuration
	lane_count    = display_config->lanes;

#if DSIM_VERSION > 1
	// Extract PLL configuration
	pll_config    = DSIM_TOP_PLL_PARAM_P(display_config->pll_p);
	pll_config    |= DSIM_TOP_PLL_PARAM_M(display_config->pll_m);
	pll_config    |= DSIM_TOP_PLL_PARAM_N(display_config->pll_n);

	dprintf(DEBUG_CRITICAL,"pll_config 0x%x\n", pll_config);

	// Extract frequence range
	hs_freq_range = display_config->hsfreq;
#endif //DSIM_VERSION

	switch (timing->display_depth){
	case 16:
		dpi_color_coding = DPI_COLOR_CODING_BIT16_CONF1;
		vid_size = timing->h_active;
		break;
	case 18:
		dpi_color_coding = DPI_COLOR_CODING_BIT18_CONF1;
		vid_size = timing->h_active & ~3;
		break;
	case 24:
		dpi_color_coding = DPI_COLOR_CODING_BIT24;
		vid_size = timing->h_active & ~1;
		break;
	case 30:
		dpi_color_coding = DPI_COLOR_CODING_BIT30;
		vid_size = timing->h_active;
		break;
	default:
		panic("unexpected display depth");
	}
	bytes_per_pixel = timing->display_depth / 8;

	clock_gate(CLK_MIPI, true);

#if DSIM_VERSION > 1
	rDSIM_TOP_PLL_CTRL = DSIM_TOP_PLL_CTRL_PLL_CFG_SEL_PHY;
#endif //DSIM_VERSION

	//Reset the block if we detect HW errors
	uint32_t tmp = rDSIM_CORE_INT_ST1;
	if ((tmp & DSIM_CORE_INT_ST1_ERRORS) != 0) {
		clock_reset_device(CLK_MIPI);
	}

	rDSIM_CORE_INT_MSK0 = 0;
	rDSIM_CORE_INT_MSK1 = 0;

	//Configure byte clock dividers
	rDSIM_CORE_CLKMGR_CFG = (DSIM_CORE_CLKMGR_CFG_TO_CLK_DIVISION(1) | DSIM_CORE_CLKMGR_CFG_TX_ESC_CLK_DIVISION(display_config->esc_div));

	rDSIM_CORE_MODE_CFG = DSIM_CORE_MODE_CFG_COMMAND_MODE;

#ifndef TARGET_PHY_STOP_WAIT_TIME
#define TARGET_PHY_STOP_WAIT_TIME	0xf
#endif //TARGET_PHY_STOP_WAIT_TIME

	//Configure PHY lanes
	rDSIM_CORE_PHY_IF_CFG = DSIM_CORE_PHY_IF_CFG_PHY_STOP_WAIT_TIME(TARGET_PHY_STOP_WAIT_TIME) | DSIM_CORE_PHY_IF_CFG_N_LANES(lane_count - 1);

	//Configure packet handler
	rDSIM_CORE_PCKHDL_CFG = DSIM_CORE_PCKHDL_CFG_BTA_EN | DSIM_CORE_PCKHDL_CFG_ECC_RX_EN | DSIM_CORE_PCKHDL_CFG_CRC_RX_EN;

	//Don't support virtual channels
	rDSIM_CORE_DPI_VCID = 0;

	//Configure pixel map
	rDSIM_CORE_DPI_COLOR_CODING =  dpi_color_coding;
	rDSIM_TOP_PIXEL_REMAP = dpi_color_coding; 

	//rDSIM_CORE_DPI_CFG_POL = (timing->neg_hsync << 2) | (timing->neg_vsync << 1) | (timing->neg_vden << 0);
	rDSIM_CORE_DPI_CFG_POL = 0;

	//Configure low power mode
	rDSIM_CORE_VID_MODE_CFG =  (DSIM_CORE_VID_MODE_CFG_LP_CMD_EN | DSIM_CORE_VID_MODE_CFG_LP_VFP_EN | DSIM_CORE_VID_MODE_CFG_LP_VACT_EN | DSIM_CORE_VID_MODE_CFG_LP_HFP_EN | DSIM_CORE_VID_MODE_CFG_LP_HBP_EN); 
#if DSIM_VERSION > 1
	rDSIM_CORE_VID_MODE_CFG |=  (DSIM_CORE_VID_MODE_CFG_LP_VBP_EN | DSIM_CORE_VID_MODE_CFG_LP_VSA_EN); 
#endif
	rDSIM_CORE_VID_MODE_CFG |= DSIM_CORE_VID_MODE_CFG_VID_MODE_TYPE_HORIZONTALEXPANSION;

	//Configure video packetizing
	rDSIM_CORE_VID_PKT_SIZE = timing->h_active;
	// LEAVE DEFAULT  VID_NUM_CHUNKS VID_NULL_SIZE; 

	//Configure video timing
	pixel_clock = clock_get_frequency(CLK_VCLK0);
	lane_byte_clock = CEIL_DIV(clock_get_frequency(CLK_MIPI), 8);
	lane_byte_clock_period = CEIL_DIV(1000000000, lane_byte_clock);
	total_pixels_per_line = timing->h_active + timing->h_front_porch + timing->h_pulse_width + timing->h_back_porch;

	time_calculation = (timing->h_pulse_width * lane_byte_clock);
	time_calculation = CEIL_DIV(time_calculation, pixel_clock);
	hsa_time = time_calculation & UINT32_MAX;

	time_calculation = (timing->h_back_porch * lane_byte_clock);
	time_calculation = CEIL_DIV(time_calculation, pixel_clock);
	hbp_time = time_calculation & UINT32_MAX;

	time_calculation = (total_pixels_per_line * lane_byte_clock);
	time_calculation = CEIL_DIV(time_calculation, pixel_clock);
	hline_time = time_calculation & UINT32_MAX;

	rDSIM_CORE_VID_HSA_TIME = hsa_time;
	rDSIM_CORE_VID_HBP_TIME = hbp_time;
	rDSIM_CORE_VID_HLINE_TIME = hline_time;

	rDSIM_CORE_VID_VSA_LINES = timing->v_pulse_width;
	rDSIM_CORE_VID_VBP_LINES = timing->v_back_porch;
	rDSIM_CORE_VID_VACTIVE_LINES = timing->v_active;

	// Configure LP time to send commands
	//<rdar://problem/15320588> Synopsys Mipi dsi: outvact_lpcmd_time programming recommendation seems incorrect
	//The synopsys recommended values do not support sending commands while video is active. SEG has provided a WA as explained on the above
	//radar
	hline_time_ns = CEIL_DIV(pixel_clock, 1000);
	hline_time_ns = CEIL_DIV(1000000000, hline_time_ns);
	hline_time_ns *= total_pixels_per_line;
	hline_time_ns = CEIL_DIV(hline_time_ns, 1000); //ns

	esc_clock = lane_byte_clock_period * display_config->esc_div; 

	hss_time = 4 * lane_byte_clock_period;

	outvact_lpcmd_time = (hline_time_ns) - hss_time - (TARGET_HS_2_LP_DATA_TIME * lane_byte_clock_period)
		- (TARGET_LP_2_HS_DATA_TIME * lane_byte_clock_period) - (22 * esc_clock) - (2 * esc_clock);
	outvact_lpcmd_time = CEIL_DIV(outvact_lpcmd_time, (2 * 8 * esc_clock));

	rDSIM_CORE_VID_VFP_LINES = timing->v_front_porch  - CEIL_DIV(256, outvact_lpcmd_time);

	max_rd_time =   hline_time;

	//Set outvact_lpcmd_time to 0 to force sending the command on the last line
	outvact_lpcmd_time = 0;
	invact_lpcmd_time = 0;
	rDSIM_CORE_DPI_LP_CMD_TIM = (outvact_lpcmd_time << 16) | (invact_lpcmd_time << 0);

	//Disable 3D capabilities
	rDSIM_CORE_SDF_3D = 0;

	rDSIM_CORE_LPCLK_CTRL = 0;

	//Configure PHY timing
	rDSIM_CORE_PHY_TMR_CFG = ((TARGET_HS_2_LP_DATA_TIME & 0xFF)  << 24) | ((TARGET_LP_2_HS_DATA_TIME  & 0xFF)  << 16) | max_rd_time;

	rDSIM_CORE_PHY_TMR_LPCLK_CFG = (TARGET_HS_2_LP_CLOCK_TIME << 16) | (TARGET_LP_2_HS_CLOCK_TIME << 0);

	//Configure packet transmission type
	cmd_mode = DSIM_CORE_CMD_MODE_CFG_GEN_LW_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_GEN_SR_2P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_GEN_SR_1P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_GEN_SR_0P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_GEN_SW_2P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_GEN_SW_1P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_GEN_SW_0P_TX_LP;
#if DSIM_VERSION == 1
	cmd_mode |= DSIM_CORE_CMD_MODE_CFG_MAX_RD_PKT_SIZE_LP |
		   DSIM_CORE_CMD_MODE_CFG_DCS_LW_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_DCS_SR_0P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_DCS_SW_1P_TX_LP |
		   DSIM_CORE_CMD_MODE_CFG_DCS_SW_0P_TX_LP;
#endif //DSIM_VERSION

	rDSIM_CORE_CMD_MODE_CFG = cmd_mode;

#if DSIM_VERSION == 1
	//PHY power up 
	rDSIM_CORE_PWR_UP = DSIM_CORE_PWR_UP_SHUTDOWNZ;
#endif

	mipi_phy_test_reset( false );
	mipi_phy_test_reset( false );
	mipi_phy_test_reset( true );
#if DSIM_VERSION > 1

	// Configure PLL if needed
	if (pll_config != 0) {
		rDSIM_TOP_PLL_CTRL |= DSIM_TOP_PLL_CTRL_SEQ_OW_ON;
		//PHY power up 
		rDSIM_CORE_PWR_UP = DSIM_CORE_PWR_UP_SHUTDOWNZ;

		//PHY reset 
		rDSIM_CORE_PHY_RSTZ = DSIM_CORE_PHY_RSTZ_PHY_ENABLECLK;

		rDSIM_TOP_PLL_CTRL |= DSIM_TOP_PLL_CTRL_CLKSELPLL_PLLCLK |  DSIM_TOP_PLL_CTRL_HSFREQRANGE(hs_freq_range);

		rDSIM_TOP_PLL_CTRL |= DSIM_TOP_PLL_CTRL_SHADOW_CLEAR;

		//typically becomes 0 in the first read
		while ((rDSIM_TOP_PLL_CTRL & DSIM_TOP_PLL_CTRL_SHADOW_CLEAR) != 0);

		rDSIM_TOP_PLL_PARAM =  DSIM_TOP_PLL_PARAM_VCOCAP(TARGET_PLL_VCO_CAP) |
					DSIM_TOP_PLL_PARAM_VCORANGE(TARGET_VCO_RANGE) |
					DSIM_TOP_PLL_PARAM_LPFCTRL(TARGET_LPF_CTRL) |
					DSIM_TOP_PLL_PARAM_ICPCTRL(TARGET_ICP_CTR) |
					pll_config;

		//wait at least one microsecond (requirement between shadow_clear and updatepll)
		spin(2);

		rDSIM_TOP_PLL_CTRL |= DSIM_TOP_PLL_CTRL_UPDATEPLL_ON;

		// wait at least 200 nanoseconds (for updatepll to remain asserted at least four 24 MHz cycles) 8) clear MIPI_DSI_TOP_PLL_CTRL.updatepll
		// wait at least 200 nanoseconds (for phylock_hw to clear)
		spin(1);

		rDSIM_TOP_PLL_CTRL &= ~DSIM_TOP_PLL_CTRL_UPDATEPLL_ON;
		spin(1);
#if WITH_HW_AGC_MIPI_V2
		rDSIM_TOP_AGILE_SEQ1 = TARGET_AGILE_SEQ1;
		rDSIM_TOP_AGILE_SEQ2 = TARGET_AGILE_SEQ2;
		rDSIM_TOP_PLL_CTRL &= ~DSIM_TOP_PLL_CTRL_SEQ_OW_ON;

		//set up agilie clocking constants
		//<rdar://problem/22524373> Maui/Malta MIPI-DSI screen shift due to first first agile command after display on
		rDSIM_TOP_AGILE_LINECOUNT |= DSIM_TOP_AGILE_LINECOUNT_CONFIG_MAX;
		rDSIM_TOP_AGILE_CTRL &= ~((0x3 << 18) | (0x3 << 16)); //VFRONT
		rDSIM_TOP_AGILE_CTRL |= DSIM_TOP_AGILE_CTRL_ENABLE;

#endif //WITH_HW_AGC_MIPI_V2
	}

#endif //DSIM_VERSION

	rDSIM_CORE_PHY_RSTZ = DSIM_CORE_PHY_RSTZ_PHY_FORCEPLL | DSIM_CORE_PHY_RSTZ_PHY_ENABLECLK | DSIM_CORE_PHY_RSTZ_PHY_SHUTDOWNZ;

	spin(5);

	rDSIM_CORE_PHY_RSTZ = DSIM_CORE_PHY_RSTZ_PHY_FORCEPLL | DSIM_CORE_PHY_RSTZ_PHY_ENABLECLK | DSIM_CORE_PHY_RSTZ_PHY_SHUTDOWNZ | DSIM_CORE_PHY_RSTZ_PHY_RSTZ;

#if DSIM_VERSION > 1
	while ((rDSIM_GENERAL_CTRL & DSIM_GENERAL_CTRL_PHYLOCK_HW_LOCK) == 0);
#endif //DSIM_VERSION

#if DSIM_VERSION == 1
	//Program PHY hsfreqrange registers
	mipi_program_phy_hsfreqrange(phy_settings_info);
#endif //DSIM_VERSION

	rDSIM_CORE_LP_RD_TO_CNT = 0x19;
	rDSIM_CORE_BTA_TO_CNT = 0x19;

	//Monitor PHY status
	status_mask = (DSIM_CORE_PHY_STATUS_PHY_STOPSTATECLKLANE | DSIM_CORE_PHY_STATUS_PHY_STOPSTATE0LANE);
	status_mask |= (lane_count >= 2) ? DSIM_CORE_PHY_STATUS_PHY_STOPSTATE1LANE : 0;
	status_mask |= (lane_count >= 3) ? DSIM_CORE_PHY_STATUS_PHY_STOPSTATE2LANE : 0;
	status_mask |= (lane_count == 4) ? DSIM_CORE_PHY_STATUS_PHY_STOPSTATE3LANE : 0;

	while ((rDSIM_CORE_PHY_STATUS & status_mask) != status_mask);

	return 0;
}

int mipi_dsim_quiesce(void)
{
	dprintf(DEBUG_CRITICAL, "mipi_dsim_quiesce()\n");

	if (!mipi_is_interface_enabled()) return 0;

	//Disable interrupts
	rDSIM_CORE_INT_MSK0 = 0x1FFFFF;
	rDSIM_CORE_INT_MSK1 = 0x1FFF;

	//Need to shutdown core before the phy
	// Power off the core
	rDSIM_CORE_PWR_UP = 0;

	// Power off PHY
	mipi_phy_test_reset( true );

	rDSIM_CORE_PHY_RSTZ &= ~(DSIM_CORE_PHY_RSTZ_PHY_FORCEPLL | DSIM_CORE_PHY_RSTZ_PHY_SHUTDOWNZ | DSIM_CORE_PHY_RSTZ_PHY_ENABLECLK);

	// Delay from SHUTDOWNZ assertion to RSTZ assertion (T4)
	spin( 5 );

	rDSIM_CORE_PHY_RSTZ &= ~(DSIM_CORE_PHY_RSTZ_PHY_RSTZ);

	clock_gate(CLK_MIPI, false);

	return 0;
}

void mipi_dsim_enable_high_speed(bool enable)
{
	if (!mipi_is_interface_enabled()) return;

	if (enable) {
		//Enable MIPI operation
		rDSIM_CORE_LPCLK_CTRL |= DSIM_CORE_LPCLK_CTRL_PHY_TXREQUESTCLKHS;

#if DSIM_VERSION == 1
		//Program PHY test registers
		mipi_program_phy_test_codes(phy_settings_info);

		//handle termination
		mipi_phy_read_mod_write(0x21, 0x1f, 2, 2);
#endif
	} else {
		rDSIM_CORE_LPCLK_CTRL &= ~DSIM_CORE_LPCLK_CTRL_PHY_TXREQUESTCLKHS;
	}
}

void mipi_dsim_enable_video(bool enable)
{
#if DSIM_VERSION == 1
	uint32_t		status_mask;
#endif

	if (!mipi_is_interface_enabled()) return;

	if (enable) {
#if DSIM_VERSION == 1
		//<rdar://problem/15548340> [N61/N56] Fiji MIPI DSI PHY requires shutdownz toggle during power up sequence
		//validate no commands are in flight
		status_mask = DSIM_CORE_PHY_STATUS_PHY_STOPSTATE0LANE;
		status_mask |= (lane_count >= 2) ? DSIM_CORE_PHY_STATUS_PHY_STOPSTATE1LANE : 0;
		status_mask |= (lane_count >= 3) ? DSIM_CORE_PHY_STATUS_PHY_STOPSTATE2LANE : 0;
		status_mask |= (lane_count == 4) ? DSIM_CORE_PHY_STATUS_PHY_STOPSTATE3LANE : 0;

		while ((rDSIM_CORE_PHY_STATUS & status_mask) != status_mask);

		mipi_phy_toggle_shutdownz();
#endif
		
		//Set Video Mode
		rDSIM_CORE_MODE_CFG = DSIM_CORE_MODE_CFG_VIDEO_MODE;
	} else {
		//Set Video Mode
		rDSIM_CORE_MODE_CFG = DSIM_CORE_MODE_CFG_COMMAND_MODE;
	}
}

//Programming of generic packets
int mipi_dsim_send_short_command(u_int8_t cmd, u_int8_t data0, u_int8_t data1)
{
	if (!mipi_is_interface_enabled()) return 0;

	//Write packet header
	rDSIM_CORE_GEN_HDR = DSIM_CORE_GEN_HDR_GEN_DT(cmd) | DSIM_CORE_GEN_HDR_GEN_WC_LSBYTE(data0) | DSIM_CORE_GEN_HDR_GEN_WC_MSBYTE(data1);

	//Read queue status
	while (!(rDSIM_CORE_CMD_PKT_STATUS & DSIM_CORE_CMD_PKT_STATUS_GEN_CMD_EMPTY));

	return 0;
}

int mipi_dsim_send_long_command(u_int8_t cmd, const u_int8_t *data, u_int32_t length)
{
	u_int32_t cnt, payload = 0;

	if (!mipi_is_interface_enabled()) return 0;

	for (cnt = 0; cnt < length; cnt++) {
		payload = (payload >> 8) | ((u_int32_t)data[cnt] << 24);
		if ((cnt & 3) == 3) {
			//Write packet data
			rDSIM_CORE_GEN_PLD_DATA = payload;
			payload = 0;
		}
	}
	if (payload) {
		payload >>= 32 - ((length & 3) * 8);
	}

	//Need to write to the fifo even if the values are 0's
	if (length % 4)
		rDSIM_CORE_GEN_PLD_DATA = payload;

	return mipi_dsim_send_short_command(cmd, length & 0xff, (length >> 8) & 0xff);
}

int mipi_dsim_read_short_command(u_int8_t cmd, u_int8_t *data)
{
	uint32_t tmp, i = 0, status_mask;

	if (!mipi_is_interface_enabled()) return 0;

	// Clear pending interrupts
	rDSIM_CORE_INT_ST1 = 0x1FFF;

	mipi_dsim_send_short_command(cmd, data[0], data[1]);

	while (1) {
		tmp = rDSIM_CORE_INT_ST1;
		
		if ((tmp & DSIM_CORE_INT_ST1_ERRORS) != 0) {
			dprintf(DEBUG_CRITICAL, "mipi cmd error 0x%x\n", tmp);
			return -1;
		}

		if ((tmp & DSIM_CORE_INT_ST1_TO_LP_RX) || (tmp & DSIM_CORE_INT_ST1_TO_HS_TX)) {
			dprintf(DEBUG_CRITICAL, "mipi: timing out\n");
			return -1;
		}

		tmp = rDSIM_CORE_CMD_PKT_STATUS;
		status_mask = DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_R_EMPTY | DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_W_EMPTY |
			DSIM_CORE_CMD_PKT_STATUS_GEN_CMD_EMPTY;
		if (tmp == status_mask) {
			break;
		}

		// Hardware timeout depends on having a receiver, and
		// completing BTA.  This isn't guaranteed for dev boards.
		if (++i == 5) {
			dprintf(DEBUG_CRITICAL, "mipi: timing out\n");
			return -1;
		}
		spin(50 * 1000);
	}

	// Wait for the read data
	while ((rDSIM_CORE_CMD_PKT_STATUS & DSIM_CORE_CMD_PKT_STATUS_GEN_RD_CMD_BUSY) != DSIM_CORE_CMD_PKT_STATUS_GEN_RD_CMD_BUSY);


	tmp = rDSIM_CORE_GEN_PLD_DATA;

	data[0] = (tmp >> 0) & 0xff;
	data[1] = (tmp >> 8) & 0xff;

	return 0;
}

int mipi_dsim_read_long_command(u_int8_t cmd, u_int8_t *data, u_int32_t *length)
{
	u_int32_t tmp, maxlen, i = 0;

	if (!mipi_is_interface_enabled()) return 0;

	maxlen = *length;

	// Clear pending interrupts
	rDSIM_CORE_INT_ST1 = 0x1FFF;

	mipi_dsim_send_short_command(cmd, data[0], data[1]);

	while (1) {
		tmp = rDSIM_CORE_INT_ST1;

		if ((tmp & DSIM_CORE_INT_ST1_ERRORS) != 0) {
			dprintf(DEBUG_CRITICAL, "mipi cmd error 0x%x\n", tmp);
			return -1;
		}

		if ((tmp & DSIM_CORE_INT_ST1_TO_LP_RX) || (tmp & DSIM_CORE_INT_ST1_TO_HS_TX)) {
			dprintf(DEBUG_CRITICAL, "mipi: timing out\n");
			return -1;
		}

		// rd_cmd_busy should go low indicating the data is ready
		if ( !(rDSIM_CORE_CMD_PKT_STATUS & DSIM_CORE_CMD_PKT_STATUS_GEN_RD_CMD_BUSY))
			break;
		
		// Hardware timeout depends on having a receiver, and
		// completing BTA.  This isn't guaranteed for dev boards.
		if (++i == 5) {
			dprintf(DEBUG_CRITICAL, "mipi_dsim_read_long_command: timing out\n");
			return -1;
		}
		spin(50 * 1000);
	}

	//Unlike other IPs, the FIJI IP strips the response packet header. SW determines how many bytes were
	//sent by checking when the FIFO is emtpy and returning how many bytes were read
	for (i = 0; i < maxlen; i++) {
		if (rDSIM_CORE_CMD_PKT_STATUS & DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_R_EMPTY)
			break;

		if (!(i % 4)) {
			tmp = rDSIM_CORE_GEN_PLD_DATA;
		}
		data[i] = tmp & 0xff;
		tmp >>= 8;
	}


	while (!(rDSIM_CORE_CMD_PKT_STATUS & DSIM_CORE_CMD_PKT_STATUS_GEN_PLD_R_EMPTY)){
		//empty the fifo
		tmp = rDSIM_CORE_GEN_PLD_DATA;
	}

	if (i < maxlen)
		*length = i;

	return 0;
}

static void mipi_program_phy_hsfreqrange(struct phy_settings *phy_settings_info)
{
	struct phy_setting_vals        *phy_setting_value_ptr;
	uint32_t			num_of_values;

	num_of_values = 1;

	phy_setting_value_ptr = &(phy_settings_info->phy_setting_values[0]);
	mipi_phy_write(phy_setting_value_ptr->test_code, phy_setting_value_ptr->test_data, phy_setting_value_ptr->num_of_data);
}

static void mipi_program_phy_test_codes(struct phy_settings *phy_settings_info)
{
	struct phy_setting_vals        *phy_setting_value_ptr;
	uint32_t			num_of_values, cnt;

	num_of_values = phy_settings_info->num_of_values;

	//skip first element as it was programmed in the hsfreqrange
	for(cnt = 1; cnt < num_of_values; cnt++) {
		phy_setting_value_ptr = &(phy_settings_info->phy_setting_values[cnt]);
		mipi_phy_write(phy_setting_value_ptr->test_code, phy_setting_value_ptr->test_data, phy_setting_value_ptr->num_of_data);
	}
}

static void mipi_phy_write(uint32_t address, uint8_t *data, uint32_t length)
{
	uint32_t i;

	// Clock High
	rDSIM_CORE_PHY_TST_CTRL0 = DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;

	//register address
	rDSIM_CORE_PHY_TST_CTRL1 = DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(address);

	// Mark this tx as an AddressWrite
	rDSIM_CORE_PHY_TST_CTRL1 |= DSIM_CORE_PHY_TST_CTRL1_PHY_TESTEN;

	//Wait 5ns
	spin(5);

	// Latch address
	rDSIM_CORE_PHY_TST_CTRL0 &= ~DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;

	spin(10);
	//Get ready for the Write Data
	rDSIM_CORE_PHY_TST_CTRL1 &= ~DSIM_CORE_PHY_TST_CTRL1_PHY_TESTEN;

	for (i = 0; i < length; i++) {
		rDSIM_CORE_PHY_TST_CTRL1 &= ~DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(0xFF);
		rDSIM_CORE_PHY_TST_CTRL1 |= DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(data[i]);
		spin(2);
		rDSIM_CORE_PHY_TST_CTRL0 |= DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;
		spin(10);
		rDSIM_CORE_PHY_TST_CTRL0 &= ~DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;
	}
}

static void mipi_phy_read_mod_write(uint32_t address, uint8_t mask, uint8_t shift, uint8_t val)
{
	uint32_t reg;

	// Clock High
	rDSIM_CORE_PHY_TST_CTRL0 = DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;

	//register address
	rDSIM_CORE_PHY_TST_CTRL1 = DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(address);

	// Mark this tx as an AddressWrite
	rDSIM_CORE_PHY_TST_CTRL1 |= DSIM_CORE_PHY_TST_CTRL1_PHY_TESTEN;

	//Wait 5ns
	spin(5);

	// Latch address
	rDSIM_CORE_PHY_TST_CTRL0 &= ~DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;

	spin(10);

	//Get ready for the Write Data
	rDSIM_CORE_PHY_TST_CTRL1 &= ~DSIM_CORE_PHY_TST_CTRL1_PHY_TESTEN;

	// From Fiji | MIPI DSI PHY Required test code settings & impact:
	// Once the termination resistance is  calibrated, read out the calibrated
	// value (bit[4:0]) and then force these values thereafter by writing them
	// back to bits[6:2] with  bits[1:0] = 10 (0x2). This prevents clock lanes
	// from recalibrating termination during each PHY reset.
	reg = rDSIM_CORE_PHY_TST_CTRL1;
	reg = DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDOUT(reg);
	reg &= mask;
	reg = (reg << shift) | val;

	// DataWrite
	rDSIM_CORE_PHY_TST_CTRL1 &= ~DSIM_CORE_PHY_TST_CTRL1_PHY_TESTEN;

	rDSIM_CORE_PHY_TST_CTRL1 &= ~DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(0xFF);
	rDSIM_CORE_PHY_TST_CTRL1 |= DSIM_CORE_PHY_TST_CTRL1_PHY_TESTDIN(reg);
				       
	spin(2);

	// Pulse clock to latch data
	rDSIM_CORE_PHY_TST_CTRL0 |= DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;

	spin(10);

	reg = rDSIM_CORE_PHY_TST_CTRL1;

	rDSIM_CORE_PHY_TST_CTRL0 &= ~DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLK;

	spin(2);
}

static void mipi_phy_test_reset(bool reset)
{
	rDSIM_CORE_PHY_TST_CTRL0  = reset ? DSIM_CORE_PHY_TST_CTRL0_PHY_TESTCLR : 0;

	spin(5);
}

static void mipi_phy_toggle_shutdownz(void)
{
	rDSIM_CORE_PHY_RSTZ &= ~DSIM_CORE_PHY_RSTZ_PHY_SHUTDOWNZ;
	spin(1);
	rDSIM_CORE_PHY_RSTZ |= DSIM_CORE_PHY_RSTZ_PHY_SHUTDOWNZ;
}


static bool mipi_is_interface_enabled()
{
	clock_gate(CLK_MIPI, true);

	if (rDSIM_CORE_PWR_UP == DSIM_CORE_PWR_UP_SHUTDOWNZ)
		return true;

	clock_gate(CLK_MIPI, false);
	return false;
}

#if WITH_DEVICETREE

#include <lib/devicetree.h>

int mipi_update_device_tree(DTNode *mipi_node)
{
	uint32_t	propSize;
	char		*propName;
	void		*propData;
	uint32_t	sizeofData;
	uint32_t	cnt;
	struct phy_setting_vals *phy_settings_ptr;
	struct phy_setting_vals *propdata_phy_settings;
	uint32_t	size_phy_settings;

	if (phy_settings_info == NULL) {
		printf("something is seriously bad\n");
		return -1;
	}

	size_phy_settings = sizeof(struct phy_setting_vals);

	propName = "phy-test-num";
	if (FindProperty(mipi_node, &propName, &propData, &propSize)) {
		((u_int32_t *)propData)[0] = phy_settings_info->num_of_values;
	}

	propName = "phy-test";
	if (FindProperty(mipi_node, &propName, &propData, &propSize)) {
		sizeofData = sizeof(struct phy_setting_vals) * phy_settings_info->num_of_values;
		if (propSize < sizeofData) {
			printf("device tree doesn't have enough space for data: propSize %d sizeofData %d\n", propSize, sizeofData);
			return -1;
		}
		for (cnt = 0; cnt < phy_settings_info->num_of_values; cnt++) {


			phy_settings_ptr = &(phy_settings_info->phy_setting_values[cnt]);
			propdata_phy_settings = (struct phy_setting_vals *)propData + (cnt * size_phy_settings);
			memcpy(propData + (cnt * size_phy_settings), phy_settings_ptr, size_phy_settings);
		}
	}

	return 0;
}

#endif
