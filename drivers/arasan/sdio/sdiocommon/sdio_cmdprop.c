/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#include "sdio_cmdprop.h"

#include "sdio_types.h"


enum CommandRegFieldOffsets
{
	kResponseTypeOffset         = 0,
	kCmdCRCCheckEnabledOffset   = 3,
	kCmdIndexCheckEnabledOffset = 4,
	kDataPresentSelectOffset    = 5,
	kCommandTypeOffset          = 6,
	kCommandIndexOffset         = 8,
};

enum CommandRegFieldMasks
{
	kResponseTypeSelectMask = (0x3 << kResponseTypeOffset),
	kCmdCRCCheckEnabled     = (1 << kCmdCRCCheckEnabledOffset),
	kCmdIndexCheckEnabled   = (1 << kCmdIndexCheckEnabledOffset),
	kDataPresentSelect      = (1 << kDataPresentSelectOffset),
	kCommandTypeMask        = (0x3 << kCommandTypeOffset),
	kCommandIndexMask       = (0x3F << kCommandIndexOffset),
};

enum CommandRegCommandType
{
	kSDIOCommandNormal  = (0x0 << kCommandTypeOffset),
	kSDIOCommandSuspend = (0x1 << kCommandTypeOffset),
	kSDIOCommandResume  = (0x2 << kCommandTypeOffset),
	kSDIOCommandAbort   = (0x3 << kCommandTypeOffset),
};

enum CommandRegResponseTypeSelect
{
	kNoResponse                = (0x0 << kResponseTypeOffset),
	kResponseLength136         = (0x1 << kResponseTypeOffset),
	kResponseLength48          = (0x2 << kResponseTypeOffset),
	kResponseLength48CheckBusy = (0x3 << kResponseTypeOffset),
};


bool
sdio_isDataPresent(UInt16 cmdIndex)
{
	// TODO: Support other data commands
	return kSDIOCmd53_IORWExtended == cmdIndex;
}

static bool
sdio_shouldCheckIndex(UInt16 cmdIndex)
{
	return cmdIndex != kSDIOCmd5_IOSendOpCond;
}

static bool
sdio_shouldCheckCRC(UInt16 cmdIndex)
{
	return cmdIndex != kSDIOCmd5_IOSendOpCond;
}

static enum CommandRegResponseTypeSelect
sdio_getResponseType(UInt16 cmdIndex)
{
	// TODO: Support other response types
	return kResponseLength48;
}

UInt16 sdio_generateCommand(UInt16 cmdIndex)
{
	UInt16 cmd = 0;
	
	// Command Index
	cmd |= (cmdIndex << kCommandIndexOffset) & kCommandIndexMask;
	
	// Command Type
	// TODO: Support abort & suspend / resume commands
	cmd |= kSDIOCommandNormal;

	// Data present select
	if(sdio_isDataPresent(cmdIndex)) {
		cmd |= kDataPresentSelect;
	}

	// Command Index Check
	if(sdio_shouldCheckIndex(cmdIndex)) {
		cmd |= kCmdIndexCheckEnabled;
	}
	
	// Command CRC Check
	if(sdio_shouldCheckCRC(cmdIndex)) {
		cmd |= kCmdCRCCheckEnabled;
	}
	
	// Response type select
	cmd |= sdio_getResponseType(cmdIndex);
	
	return cmd;
}



