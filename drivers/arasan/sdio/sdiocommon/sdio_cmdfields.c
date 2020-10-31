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


#include "sdio_cmdfields.h"


#pragma mark -
#pragma mark Common

enum CommandOffset
{
	kSDIOCmdAddressOffset	= 9,
	kSDIOCmdFunctionOffset	= 28,
};

enum CommandMask
{
	kSDIOCmdFunctionMask	= 0x7,
	kSDIOCmdAddressMask		= 0x1FFFF,
};

enum SDIOCmdDirection
{
	kSDIOCmdDirectionRead  = (0 << 31),
	kSDIOCmdDirectionWrite = (1 << 31),
};


static inline UInt32
convertFunctionToArg(UInt8 function)
{
	return (function & kSDIOCmdFunctionMask) << kSDIOCmdFunctionOffset;
}

static inline UInt32
convertAddressToArg(UInt32 address)
{
	return (address & kSDIOCmdAddressMask) << kSDIOCmdAddressOffset;
}



enum SDIODirection
sdio_getCommandDirection(enum SDIOCommandIndex index, UInt32 argument)
{
	switch (index) {
		case kSDIOCmd52_IORWDirect:
		case kSDIOCmd53_IORWExtended:
			return (argument & kSDIOCmdDirectionWrite) ? kSDIODirectionWrite : kSDIODirectionRead;
		default:
			return kSDIODirectionNone;
	}
}


#pragma mark -
#pragma mark Cmd5

UInt32
sdio_getOCR(const struct SDIOCommandResponse *cmd5Response)
{
	return cmd5Response->response[0] & 0x00fffff;
}

bool
sdio_getNumIOFunctions(const struct SDIOCommandResponse *cmd5Response)
{
	return (cmd5Response->response[0] & 0x70000000) >> 28;
}

bool
sdio_isMemoryPresent(const struct SDIOCommandResponse *cmd5Response)
{
	return cmd5Response->response[0] & 0x08000000; // >> 27
}

bool
sdio_isIOReady(const struct SDIOCommandResponse *cmd5Response)
{
	return cmd5Response->response[0] & 0x80000000; // >> 31
}




#pragma mark -
#pragma mark Cmd52


UInt32
sdio_generateCmd52Arg(enum SDIODirection direction,
					  UInt8 sdioFunction, UInt32 cardAddress, UInt8 data,
					  SDIOCmd52Flags flags)
{
	UInt32 dir = (direction & kSDIODirectionWrite) ? kSDIOCmdDirectionWrite : kSDIOCmdDirectionRead;
	
	return dir | convertFunctionToArg(sdioFunction) | flags
		| convertAddressToArg(cardAddress) | data;
}



#pragma mark -
#pragma mark Cmd53

enum SDIOCmd53Mask
{
	kSDIOCmd53CountMask = 0x1FF
};


static inline UInt32
convertCmd53CountToArg(UInt32 count)
{
	return (count & kSDIOCmd53CountMask);
}


UInt32
sdio_generateCmd53Arg(enum SDIODirection direction,
					  UInt8 sdioFunction, UInt32 cardAddress, UInt16 count,
					  SDIOCmd53Flags flags)
{
	UInt32 dir = (direction & kSDIODirectionWrite) ? kSDIOCmdDirectionWrite : kSDIOCmdDirectionRead;
	
	return dir | convertFunctionToArg(sdioFunction) | flags 
		| convertAddressToArg(cardAddress) | convertCmd53CountToArg(count);
}

UInt16
sdio_getCmd53Count(UInt32 cmdArg)
{
	return cmdArg & kSDIOCmd53CountMask;
}


