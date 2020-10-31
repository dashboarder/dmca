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


#ifndef _SDIO_CMDFIELDS_H_
#define _SDIO_CMDFIELDS_H_


#ifdef PLATFORM_VARIANT_IOP
#include <sdiocommon/sdio_types.h>
#include <sys/types.h>
#else
#include <IOKit/sdio/sdio_types.h>
#include <IOKit/IOTypes.h>
#endif


#ifdef __cplusplus
extern "C"  {
#endif


#pragma mark -
#pragma mark Common

enum SDIODirection sdio_getCommandDirection(enum SDIOCommandIndex index, UInt32 argument);



#pragma mark -
#pragma mark Cmd 5

enum SDIOCmd5OperationConditions {
	kSDIOOpCond_None	= (0x000000),
	
	kSDIOOpCond_20_21	= (1 << 8),
	kSDIOOpCond_21_22	= (1 << 9),
	kSDIOOpCond_22_23	= (1 << 10),
	kSDIOOpCond_23_24	= (1 << 11),
	kSDIOOpCond_24_25	= (1 << 12),
	kSDIOOpCond_25_26	= (1 << 13),
	kSDIOOpCond_26_27	= (1 << 14),
	kSDIOOpCond_27_28	= (1 << 15),
	kSDIOOpCond_28_29	= (1 << 16),
	kSDIOOpCond_29_30	= (1 << 17),
	kSDIOOpCond_30_31	= (1 << 18),
	kSDIOOpCond_31_32	= (1 << 19),
	kSDIOOpCond_32_33	= (1 << 20),
	kSDIOOpCond_33_34	= (1 << 21),
	kSDIOOpCond_34_35	= (1 << 22),
	kSDIOOpCond_35_36	= (1 << 23),
	
	kSDIOOpCond_All		= (0xFFFF00)
};

UInt32 sdio_getOCR(const struct SDIOCommandResponse *cmd5Response);

bool sdio_getNumIOFunctions(const struct SDIOCommandResponse *cmd5Response);

bool sdio_isMemoryPresent(const struct SDIOCommandResponse *cmd5Response);

bool sdio_isIOReady(const struct SDIOCommandResponse *cmd5Response);



#pragma mark -
#pragma mark Cmd 52


enum SDIOCmd52Flag
{
	kSDIOCmd52RAW = (1 << 27),
};

typedef UInt32 SDIOCmd52Flags;


UInt32 sdio_generateCmd52Arg(enum SDIODirection direction,
							 UInt8 sdioFunction, UInt32 cardAddress, UInt8 data,
							 SDIOCmd52Flags flags);



#pragma mark -
#pragma mark Cmd 53


enum SDIOCmd53Flag
{
	kSDIOCmd53ByteMode			= (0 << 27),
	kSDIOCmd53BlockMode			= (1 << 27),
	kSDIOCmd53FixedAddress		= (0 << 26),
	kSDIOCmd53IncrementAddress	= (1 << 26),
};

typedef UInt32 SDIOCmd53Flags;


UInt32 sdio_generateCmd53Arg(enum SDIODirection direction,
							 UInt8 sdioFunction, UInt32 cardAddress, UInt16 count,
							 SDIOCmd53Flags flags);

UInt16 sdio_getCmd53Count(UInt32 cmdArg);


	
#ifdef __cplusplus
}
#endif

#endif /* _SDIO_CMDFIELDS_H_ */

