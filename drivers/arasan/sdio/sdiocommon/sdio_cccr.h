/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#ifndef _SDIO_CCCR_H_
#define _SDIO_CCCR_H_


#ifdef __cplusplus
extern "C"  {
#endif


enum SDIOCccrRegisterAddress
{
	kSDIOCccrRevisionAddr			= 0x00,
	kSDIOCccrSpecRevAddr			= 0x01,
	kSDIOCccrIOEnableAddr			= 0x02,
	kSDIOCccrIOReadyAddr			= 0x03,
	kSDIOCccrIntEnableAddr			= 0x04,
	kSDIOCccrIntPendingAddr			= 0x05,
	kSDIOCccrIOAbortAddr			= 0x06,
	kSDIOCccrBusInterfaceControlAddr= 0x07,
	kSDIOCccrCardCapabilityAddr		= 0x08,
	kSDIOCccrCommonCISPtrLowAddr	= 0x09,
	kSDIOCccrCommonCISPtrMidAddr	= 0x0A,
	kSDIOCccrCommonCISPtrHighAddr	= 0x0B,
	kSDIOCccrBusSuspendAddr			= 0x0C,
	kSDIOCccrFunctionSelectAddr		= 0x0D,
	kSDIOCccrExecFlagsAddr			= 0x0E,
	kSDIOCccrReadyFlagsAddr			= 0x0F,
	kSDIOCccrFn0BlockSizeLSBAddr	= 0x10,
	kSDIOCccrFn0BlockSizeMSBAddr	= 0x11,
	kSDIOCccrPowerControlAddr		= 0x12,
	kSDIOCccrHighSpeedAddr			= 0x13,
};


// Bus Interface Control
enum SDIOCccrBusInterfaceControlMask
{
	kSDIOCccrBusWidthMask = (1 << 1) | (1 << 0),
};

enum SDIOCccrBusInterfaceControl
{
	kSDIOCccrBusIfWidth1Bit = ((0 << 1)|(0 << 0)),
	kSDIOCccrBusIfWidth4Bit = ((1 << 1)|(0 << 0)),

	kSDIOCccrBusIfContinuousSPIIntEnabled	= (1 << 5),
	kSDIOCccrBusIfContinuousSPIInt		= (1 << 6),
	kSDIOCccrBusIfCardDetectDisable		= (1 << 7),
};


// Card Capabilities

enum SDIOCccrCardCapability
{
	kSDIOCccrCapDirectCommands			= (1 << 0),
	kSDIOCccrCapMultiBlock				= (1 << 1),
	kSDIOCccrCapReadWait				= (1 << 2),
	kSDIOCccrCapSuspendResume			= (1 << 3),
	kSDIOCccrCap4BitIntraBlockInt		= (1 << 4),
	kSDIOCccrCap4BitIntraBlockIntEnable	= (1 << 5),
	kSDIOCccrCapLowSpeed				= (1 << 6),
	kSDIOCccrCap4BitLowSpeed			= (1 << 7),
};


// High Speed

enum SDIOCccrHighSpeed
{
	kSDIOCccrHighSpeedSupported = (1 << 0),
	kSDIOCccrHighSpeedEnabled	= (1 << 1),
};


#ifdef __cplusplus
}
#endif

#endif /* _SDIO_CCCR_H_ */
