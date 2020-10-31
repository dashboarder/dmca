/*
 * Copyright (C) 2008-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/mipi.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <target/gpiodef.h>

#include "dsim.h"

#define DISPLAY_CONFIG_PLL_P(x)		(((x) >> 26) & 0x03F)
#define DISPLAY_CONFIG_PLL_M(x)		(((x) >> 16) & 0x3FF)
#define DISPLAY_CONFIG_PLL_S(x)		(((x) >> 12) & 0x007)
#define DISPLAY_CONFIG_PLL_B(x)		(((x) >>  8) & 0x00F)
#define DISPLAY_CONFIG_ESC_DIV(x)	(((x) >>  4) & 0x00F)
#define DISPLAY_CONFIG_LANES(x)		(((x) >>  0) & 0x00F)

#define DSIM_LANE_MASK			((1 << DSIM_LANE_COUNT) - 1)

static u_int32_t lane_count;
static u_int32_t lane_mask;
static uint32_t  esc_clock_div;

static bool dsim_enabled;

int mipi_dsim_init(struct display_timing *timing, enum colorspace color)
{
	bool video_mode = true;
	u_int32_t tmp, pll_config, pix_format, afc_code = 0;
	u_int32_t config = rDSIM_CONFIG_HfpMode;
	bool afc_enable = false;
	uint32_t display_config;

	dprintf(DEBUG_CRITICAL, "mipi_dsim_init()\n");

	memcpy(&display_config, timing->display_config, sizeof(display_config));
	// Extract Lane configuration
	lane_count    = DISPLAY_CONFIG_LANES(display_config);
	lane_mask     = (1 << lane_count) - 1;

	// Extract PLL configuration
	pll_config    = (rDSIM_PLLCTRL_P(DISPLAY_CONFIG_PLL_P(display_config)) |
			 rDSIM_PLLCTRL_M(DISPLAY_CONFIG_PLL_M(display_config)) |
			 rDSIM_PLLCTRL_S(DISPLAY_CONFIG_PLL_S(display_config)));

	// Extract Pixel Format
	if (timing->display_depth <= 18) pix_format = 5;
	else pix_format = 7;

	//Extract escape clock divisor
	esc_clock_div = DISPLAY_CONFIG_ESC_DIV(display_config);

	clock_gate(CLK_MIPI, true);

	// Configure clock selections
	rDSIM_CLKCTRL = (rDSIM_CLKCTRL_EscClkEn |
			 ((pll_config == 0) ? rDSIM_CLKCTRL_PllBypass : 0) |
			 rDSIM_CLKCTRL_ByteClkSrc((pll_config == 0) ? 1 : 0) |	// SOC or DSIM PLL
			 rDSIM_CLKCTRL_EscPrescalar(DISPLAY_CONFIG_ESC_DIV(display_config)));

	// Select frequency band
	rDSIM_PLLCTRL = rDSIM_PLLCTRL_FreqBand(DISPLAY_CONFIG_PLL_B(display_config));

#ifdef TARGET_DSIM_THS_PREPARE
	//Adjust tHS-PREPARE on the phy
	rDSIM_PLLCTRL |= rDSIM_PLLCTRL_PreprCtl(TARGET_DSIM_THS_PREPARE);
#endif //TARGET_DSIM_THS_PREPARE

	// Configure PLL if needed
	if (pll_config != 0) {
#if DSIM_VERSION == 1
		u_int32_t fin_pll;

		// Assume that AFC will be used
		afc_enable = true;

		// Set the AFC code in PHYACCHR
		fin_pll = clock_get_frequency(CLK_NCLK) / (1000000 * DISPLAY_CONFIG_PLL_P(display_config));

		// These code values are only for the 45nm PHY (i.e. DSIM_VERSION == 1)
		switch (fin_pll) {
		case 6:  afc_code = 1; break;
		case 7:  afc_code = 0; break;
		case 8:  afc_code = 3; break;
		case 9:  afc_code = 2; break;
		case 10: afc_code = 5; break;
		case 11: afc_code = 4; break;
		case 12: afc_code = 4; break;
		default : afc_enable = false; break;
		}
#endif

		// If needed, enable AFC
		if (afc_enable) {
			rDSIM_PHYACCHR = rDSIM_PHYACCHR_AFC(afc_code) | rDSIM_PHYACCHR_AFC_ENABLE;
		}

		rDSIM_PLLTMR = 300000; // 1ms - HACK: assuming 300MHz hperf2

		rDSIM_PLLCTRL |= pll_config;
		rDSIM_PLLCTRL |= rDSIM_PLLCTRL_PllEn;

		while ((rDSIM_STATUS & rDSIM_STATUS_PllStable) == 0);
	}

	// Issue software reset
	rDSIM_SWRST   = rDSIM_SWRST_SwRst;

	// If needed, enable AFC again
	if (afc_enable) {
		// PLL must be off to change AFC setting
		rDSIM_PLLCTRL &= ~rDSIM_PLLCTRL_PllEn;

		rDSIM_PHYACCHR = rDSIM_PHYACCHR_AFC(afc_code) | rDSIM_PHYACCHR_AFC_ENABLE;

		// Turn PLL back on
		rDSIM_PLLCTRL |= rDSIM_PLLCTRL_PllEn;
	}

	// Wait for software reset to complete
	while ((rDSIM_STATUS & rDSIM_STATUS_SwRstRls) == 0);

#ifdef TARGET_DSIM_DPHYCTL
	rDSIM_PHYACCHR |= rDSIM_PHYACCHR_DPHYCTL(TARGET_DSIM_DPHYCTL);
#endif // TARGET_DSIM_DPHYCTL

#if TARGET_DSIM_DOWN_CODE
	rDSIM_PHYACCHR |= rDSIM_PHYACCHR_DOWN_CODE(TARGET_DSIM_DOWN_CODE);
	rDSIM_PHYACCHR2 |= rDSIM_PHYACCHR2_DOWN_CODE(TARGET_DSIM_DOWN_CODE);
#endif // TARGET_DSIM_DOWN_CODE

#ifdef TARGET_DSIM_UP_CODE
	rDSIM_PHYACCHR |= rDSIM_PHYACCHR_UP_CODE(TARGET_DSIM_UP_CODE);
	rDSIM_PHYACCHR2 |= rDSIM_PHYACCHR2_UP_CODE(TARGET_DSIM_UP_CODE);
#endif // TARGET_DSIM_UP_CODE

#ifdef TARGET_DSIM_SUPPRESS
	rDSIM_SUPPRESS |= TARGET_DSIM_SUPPRESS;
#endif // TARGET_DSIM_SUPPRESS

	// Use the same resolution as CLCD
	rDSIM_MDRESOL = (rDSIM_MDRESOL_MainVResol(timing->v_active) |
			 rDSIM_MDRESOL_MainHResol(timing->h_active));

#ifdef TARGET_DISP_VIDEO_MODE
	video_mode = TARGET_DISP_VIDEO_MODE;
#endif //TARGET_DISP_VIDEO_MODE

	if (video_mode) {
		// The vertical blanking settings are the same as the CLCD settings
		rDSIM_MVPORCH = (rDSIM_MVPORCH_CmdAllow(13) |
			 rDSIM_MVPORCH_StableVfp(timing->v_front_porch) |
			 rDSIM_MVPORCH_MainVbp(timing->v_back_porch));
		rDSIM_MSYNC   = (timing->v_pulse_width << 22);

		// The horizontal blanking settings are constants which are
		// sufficent for all lane counts, frequencies and CLCD settings
		//	Hfp = 15
		//	Hbp = 14
		//	Hsa = 1
		rDSIM_MHPORCH = (15 << 16) | (14 << 0);
		rDSIM_MSYNC   |= (1 << 0);
	} 
#if DSIM_VERSION == 3
	else {
		rDSIM_I80_LPVALID_ST = (timing->h_active * 3) + 100;
	}
#endif // DSIM_VERSION == 3

	rDSIM_SSCNT   = 0xa;

#ifdef TARGET_DSIM_CONFIG
	config = TARGET_DSIM_CONFIG;
#endif

	rDSIM_CONFIG  = (rDSIM_CONFIG_BurstMode |
			 rDSIM_CONFIG_AutoMode |
			 config |
			 rDSIM_CONFIG_MainPixFormat(pix_format) |
			 rDSIM_CONFIG_NumOfDatLane(DSIM_LANE_COUNT - 1) |
			 rDSIM_CONFIG_LaneEn(DSIM_LANE_MASK) |
			 rDSIM_CONFIG_LaneClkEn);
	if (video_mode) {
		rDSIM_CONFIG  |= rDSIM_CONFIG_VideoMode;
	}

#ifdef TARGET_NO_BURST_MODE
	if (TARGET_NO_BURST_MODE) {
		rDSIM_CONFIG &= ~rDSIM_CONFIG_BurstMode;
	}
#endif
	rDSIM_CLKCTRL |= (rDSIM_CLKCTRL_ByteClkEn |
			  rDSIM_CLKCTRL_LaneEscClkEn(DSIM_LANE_MASK) |
			  rDSIM_CLKCTRL_LaneEscClkEnClk);

	rDSIM_FIFOCTRL = 0x1F;
	rDSIM_FIFOTHLD = 0x1FF;

	// Force Stop state on all lanes
	rDSIM_ESCMODE = (rDSIM_ESCMODE_CmdLpdt | rDSIM_ESCMODE_ForceStopstate);
	spin(1 * 1000);
	rDSIM_ESCMODE &= ~rDSIM_ESCMODE_ForceStopstate;

	// Wait for Stop state on all lanes
	tmp = rDSIM_STATUS_StopstateClk | rDSIM_STATUS_StopstateDat(DSIM_LANE_MASK);
	while ((rDSIM_STATUS & tmp) != tmp);

	// Request ULPS mode on all lanes
	rDSIM_ESCMODE = (rDSIM_ESCMODE_CmdLpdt | rDSIM_ESCMODE_TxUlpsClk | rDSIM_ESCMODE_TxUlpsDat);

	// Wait for ULPS mode on all lanes
	tmp = rDSIM_STATUS_UlpsClk | rDSIM_STATUS_UlpsDat(DSIM_LANE_MASK);
	while ((rDSIM_STATUS & tmp) != tmp);

	// Set the lane configuration
	rDSIM_CONFIG &= ~(rDSIM_CONFIG_NumOfDatLaneMsk | rDSIM_CONFIG_LaneEnMsk);
	rDSIM_CONFIG |= rDSIM_CONFIG_NumOfDatLane(lane_count - 1) | rDSIM_CONFIG_LaneEn(lane_mask);

	// Enable the configured lanes
	rDSIM_CLKCTRL &= ~rDSIM_CLKCTRL_LaneEscClkEnMsk;
	rDSIM_CLKCTRL |= rDSIM_CLKCTRL_LaneEscClkEn(lane_mask);

	// Request exit ULPS mode on configured lanes
	rDSIM_ESCMODE |= (rDSIM_ESCMODE_TxUlpsClkExit | rDSIM_ESCMODE_TxUlpsExit);

	// Wait for exit ULPS mode on configured lanes
	tmp = rDSIM_STATUS_UlpsClk | rDSIM_STATUS_UlpsDat(lane_mask);
	while ((rDSIM_STATUS & tmp) != 0);

	// Clear exit ULPS mode request and ULPS mode request
	rDSIM_ESCMODE &= ~(rDSIM_ESCMODE_TxUlpsClkExit | rDSIM_ESCMODE_TxUlpsExit);
	spin(1 * 1000);
	rDSIM_ESCMODE &= ~(rDSIM_ESCMODE_TxUlpsClk | rDSIM_ESCMODE_TxUlpsDat);

	// Wait for Stop state on configured lanes
	tmp = rDSIM_STATUS_StopstateClk | rDSIM_STATUS_StopstateDat(lane_mask);
	while ((rDSIM_STATUS & tmp) != tmp);

#if DSIM_VERSION == 3
	if (!video_mode) {
		rDSIM_ULPSIN =  TARGET_DSI_ULPS_IN_DELAY;
		rDSIM_ULPSEND = TARGET_DSI_ULPS_END_DELAY;
		rDSIM_ULPSOUT = TARGET_DSI_ULPS_OUT_DELAY;

		//<rdar://problem/18037708> SkiHillVail12S5356c: panic(cpu 0 caller 0x855223df): "_waitUntilPayloadFifoIsEmpty: Timeout Waiting for payload fifo to empty"
		rDSIM_ESCMODE |= rDSIM_ESCMODE_Auto_Ulps_Clk;
	}

#endif //DSIM_VERSION == 3

	dsim_enabled = true;

	return 0;
}

int mipi_dsim_quiesce(void)
{
	u_int32_t tmp, pllctrl, phyacchr;

	dprintf(DEBUG_CRITICAL, "mipi_dsim_quiesce()\n");

	if (!dsim_enabled) return 0;

	// Save the PLLCTRL and PHYACCHR settings before software reset
	pllctrl = rDSIM_PLLCTRL;
	phyacchr = rDSIM_PHYACCHR;

	// Issue software reset
	rDSIM_SWRST   = rDSIM_SWRST_SwRst;

	// Turn off the PLL if it was on
	rDSIM_PLLCTRL = pllctrl & ~rDSIM_PLLCTRL_PllEn;

	// Restore PHYACCHR setting
	rDSIM_PHYACCHR = phyacchr;

	// If the PLL was on, turn it back on
	rDSIM_PLLCTRL = pllctrl;

	// Wait for reset to complete
	while ((rDSIM_STATUS & rDSIM_STATUS_SwRstRls) == 0);

	// Set the lane configuration
	rDSIM_CONFIG &= ~(rDSIM_CONFIG_NumOfDatLaneMsk | rDSIM_CONFIG_LaneEnMsk);
	rDSIM_CONFIG |= (rDSIM_CONFIG_NumOfDatLane(DSIM_LANE_COUNT - 1) |
			 rDSIM_CONFIG_LaneEn(DSIM_LANE_MASK) |
			 rDSIM_CONFIG_LaneClkEn);

	// Enable the configured lanes
	rDSIM_CLKCTRL &= ~(rDSIM_CLKCTRL_TxRequestHsClk | rDSIM_CLKCTRL_LaneEscClkEnMsk);
	rDSIM_CLKCTRL |= rDSIM_CLKCTRL_LaneEscClkEn(DSIM_LANE_MASK);

	// Force Stop state on all lanes
	rDSIM_ESCMODE = (rDSIM_ESCMODE_CmdLpdt | rDSIM_ESCMODE_ForceStopstate);
	spin(1 * 1000);
	rDSIM_ESCMODE &= ~rDSIM_ESCMODE_ForceStopstate;

	// Wait for Stop state on all lanes
	tmp = rDSIM_STATUS_StopstateClk | rDSIM_STATUS_StopstateDat(DSIM_LANE_MASK);
	while ((rDSIM_STATUS & tmp) != tmp);

	// Request ULPS mode on all lanes
	rDSIM_ESCMODE = (rDSIM_ESCMODE_CmdLpdt | rDSIM_ESCMODE_TxUlpsClk | rDSIM_ESCMODE_TxUlpsDat);

	// Wait for ULPS mode on all lanes
	tmp = rDSIM_STATUS_UlpsClk | rDSIM_STATUS_UlpsDat(DSIM_LANE_MASK);
	while ((rDSIM_STATUS & tmp) != tmp);

	// Turn off internal clocks
	rDSIM_CLKCTRL = 0x00000000;
	rDSIM_CONFIG  = rDSIM_CONFIG_VideoMode;
	rDSIM_ESCMODE = 0x00000000;
	rDSIM_PLLCTRL = 0x00000000;

	clock_gate(CLK_MIPI, false);

	dsim_enabled = false;

	return 0;
}

void mipi_dsim_enable_high_speed(bool enable)
{
	if (enable) {
		rDSIM_CLKCTRL |= rDSIM_CLKCTRL_TxRequestHsClk;
		while ((rDSIM_STATUS & rDSIM_STATUS_TxReadyHsClk) == 0);
	} else {
		rDSIM_CLKCTRL &= ~rDSIM_CLKCTRL_TxRequestHsClk;
		while ((rDSIM_STATUS & rDSIM_STATUS_TxReadyHsClk) != 0);
	}
}

void mipi_dsim_enable_video(bool enable)
{
	if (enable) {
		rDSIM_MDRESOL |= rDSIM_MDRESOL_MainStandby;
	} else {
		rDSIM_MDRESOL &= ~rDSIM_MDRESOL_MainStandby;
	}
}

int mipi_dsim_send_short_command(u_int8_t cmd, u_int8_t data0, u_int8_t data1)
{
	uint32_t escape_clock_period_us;
	uint32_t mipi_byte_clock_frequency;

	mipi_byte_clock_frequency = clock_get_frequency(CLK_MIPI)/ 8; 
	escape_clock_period_us = (100000000 * esc_clock_div) /mipi_byte_clock_frequency;
	escape_clock_period_us = (escape_clock_period_us/100) < 1 ? 1 : (escape_clock_period_us/100);

	rDSIM_PKTHDR = (cmd & 0x3f) | (((u_int32_t)data0) << 8) | (((u_int32_t)data1) << 16);

	//10 ESC clocks before checking the status bit.
	spin (10 * escape_clock_period_us);

	//check for command header to have been sent
	while ((rDSIM_FIFOCTRL & rDSIM_FIFOCTRL_EmptyHSfr) == 0);
	//check for command payload to have been sent
	while ((rDSIM_FIFOCTRL & rDSIM_FIFOCTRL_EmptyLSfr) == 0);

	spin(22 + ((343 * 1000000 * esc_clock_div)/mipi_byte_clock_frequency)); 

	return 0;
}

int mipi_dsim_send_long_command(u_int8_t cmd, const u_int8_t *data, u_int32_t length)
{
	u_int32_t cnt, payload = 0;

	for (cnt = 0; cnt < length; cnt++) {
		payload = (payload >> 8) | ((u_int32_t)data[cnt] << 24);
		if ((cnt & 3) == 3) {
			rDSIM_PAYLOAD = payload;
			payload = 0;
		}
	}
	if (payload) {
		payload >>= 32 - ((length & 3) * 8);
		rDSIM_PAYLOAD = payload;
	}

	for (cnt = 0; cnt < 5; cnt++) {
		rDSIM_PAYLOAD = 0;
	}

	length += 20;
	return mipi_dsim_send_short_command(cmd, length & 0xff, (length >> 8) & 0xff);
}

int mipi_dsim_read_short_command(u_int8_t cmd, u_int8_t *data)
{
	u_int32_t tmp, i = 0;

	// Clear pending interrupts
	rDSIM_INTSRC = 0xffffffff;

	mipi_dsim_send_short_command(cmd, data[0], data[1]);

	while (1) {
		tmp = rDSIM_INTSRC;

		if (tmp & rDSIM_INTSRC_RxAck) return -1;
		if (tmp & rDSIM_INTSRC_LpdrTout) return -1;
		// Ignore errors for now (not all drivers do ECC)
//		if (tmp & rDSIM_INTSRC_AnyError) return -1;

		if (tmp & rDSIM_INTSRC_RxDatDone) break;

		// Hardware timeout depends on having a receiver, and
		// completing BTA.  This isn't guaranteed for dev boards.
		if (++i == 5) {
			dprintf(DEBUG_CRITICAL, "mipi: timing out\n");
			return -1;
		}
		spin(50 * 1000);
	}

	while (rDSIM_FIFOCTRL & rDSIM_FIFOCTRL_EmptyRx);

	tmp = rDSIM_RXFIFO;

	switch (tmp & 0x3f) {
	case DSIM_RSP_READ_1B:
	case DSIM_RSP_DSC_READ_1B:
		data[0] = (tmp >> 8) & 0xff;
		data[1] = 0;
		break;
	case DSIM_RSP_READ_2B:
	case DSIM_RSP_DSC_READ_2B:
		data[0] = (tmp >> 8) & 0xff;
		data[1] = (tmp >> 16) & 0xff;
		break;
	default:
		return -1;
	}

	return 0;
}

int mipi_dsim_read_long_command(u_int8_t cmd, u_int8_t *data, u_int32_t *length)
{
	u_int32_t tmp, len, maxlen, i = 0;

	maxlen = *length;

	// Clear pending interrupts
	rDSIM_INTSRC = 0xffffffff;

	mipi_dsim_send_short_command(cmd, data[0], data[1]);

	while (1) {
		tmp = rDSIM_INTSRC;

		if (tmp & rDSIM_INTSRC_RxAck) return -1;
		if (tmp & rDSIM_INTSRC_LpdrTout) return -1;
		// Ignore errors for now (not all drivers do ECC)
//		if (tmp & rDSIM_INTSRC_AnyError) return -1;

		if (tmp & rDSIM_INTSRC_RxDatDone) break;

		// Hardware timeout depends on having a receiver, and
		// completing BTA.  This isn't guaranteed for dev boards.
		if (++i == 5) {
			dprintf(DEBUG_CRITICAL, "mipi: timing out\n");
			return -1;
		}
		spin(50 * 1000);
	}

	while (rDSIM_FIFOCTRL & rDSIM_FIFOCTRL_EmptyRx);

	tmp = rDSIM_RXFIFO;

	switch (tmp & 0x3f) {
	case DSIM_RSP_LONG_READ:
	case DSIM_RSP_DSC_LONG_READ:
		break;
	default:
		dprintf(DEBUG_CRITICAL, "mipi: weird response %08x\n", tmp);
		return -1;
	}

	len = (tmp >> 8) & 0xffff;
	if (len < maxlen)
		*length = len;

	for (i = 0; i < len; i++) {
		if ((i % 4) == 0) {
			while (rDSIM_FIFOCTRL & rDSIM_FIFOCTRL_EmptyRx);
			tmp = rDSIM_RXFIFO;
		}
		if (i < maxlen)
			data[i] = tmp & 0xff;
		tmp >>= 8;
	}

	return 0;
}
