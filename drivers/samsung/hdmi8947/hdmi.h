/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef HDMI_H
#define HDMI_H	1
typedef enum {
	kPHYConfigReg00,	//HDMIPHYCON0
	kPHYConfigReg01,	//HDMIPHYCON0
	kPHYConfigReg02,	//HDMIPHYCON0
	kPHYConfigReg03,	//HDMIPHYCON0
	kPHYConfigReg04,	//HDMIPHYCON1
	kPHYConfigReg05,	//HDMIPHYCON1
	kPHYConfigReg06,	//HDMIPHYCON1
	kPHYConfigReg07,	//HDMIPHYCON1
	kPHYConfigReg08,	//HDMIPHYCON2
	kPHYConfigReg09,	//HDMIPHYCON2
	kPHYConfigReg0a,	//HDMIPHYCON2
	kPHYConfigReg0b,	//HDMIPHYCON2
	kPHYConfigReg0c,	//HDMIPHYCON3
	kPHYConfigReg0d,	//HDMIPHYCON3
	kPHYConfigReg0e,	//HDMIPHYCON3
	kPHYConfigReg0f,	//HDMIPHYCON3
	kPHYConfigReg10,	//HDMIPHYCON4
	kPHYConfigReg11,	//HDMIPHYCON4
	kPHYConfigReg12,	//HDMIPHYCON4
	kPHYConfigReg13,	//HDMIPHYCON4
	kPHYConfigReg14,	//HDMIPHYCON5
	kPHYConfigReg15,	//HDMIPHYCON5
	kPHYConfigReg16,	//HDMIPHYCON5
	kPHYConfigReg17,	//HDMIPHYCON5
	kPHYConfigReg18,	//HDMIPHYCON6
	kPHYConfigReg19,	//HDMIPHYCON6
	kPHYConfigReg1a,	//HDMIPHYCON6
	kPHYConfigReg1b,	//HDMIPHYCON6
	kPHYConfigReg1c,	//HDMIPHYCON7
	kPHYConfigReg1d,	//HDMIPHYCON7
	kPHYConfigReg1e,	//HDMIPHYCON7
	kPHYConfigRegRSVD,	//HDMIPHYCON7
	kPHYConfigRegCount,     // count excludes the mode set control register
	kPHYConfigRegModeSet = 0x1f, //HDMIPHYCON8
} PHYConfigReg;

typedef uint8_t HDMITXPHYConfig[30];

typedef enum HDMITXPHYConfigDepth {
	kHDMITXPHYConfigDepth8,
	kHDMITXPHYConfigDepth10,
	kHDMITXPHYConfigDepth12,
	kHDMITXPHYConfigDepthCount
} HDMITXPHYConfigDepth;

typedef struct {
	HDMITXPHYConfig data;
	uint8_t                     pad;        // Align to multiple of 4 bytes for device tree alignment
	uint8_t                     invalid;    // If nonzero, entry is invalid
} HDMITXPHYConfigDepthTableEntry;

typedef struct HDMITXPHYConfigTableEntry {
	uint32_t                                    pixelClockHz;   // target pixel clock
	HDMITXPHYConfigDepthTableEntry  depth[kHDMITXPHYConfigDepthCount];
} AppleSamsungHDMITXPHYConfigTableEntry;
#endif
