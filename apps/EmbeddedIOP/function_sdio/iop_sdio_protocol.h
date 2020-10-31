/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _IOP_SDIO_PROTOCOL_H_
#define _IOP_SDIO_PROTOCOL_H_

#include <sys/types.h>

#ifdef PLATFORM_VARIANT_IOP
#include <sdiocommon/sdio_types.h>
#else
#include <IOKit/sdio/sdio_types.h>
#endif


/*
 * Command size is (somewhat) tunable.
 *
 * The principal consideration here is the maximum scatter/gather list size
 * this permits.
 */
#define kIOPSDIO_MESSAGE_SIZE   (512)


typedef UInt32 IOPSDIO_opcode_t;

enum IOPSDIOOpcode
{
	kIOPSDIOOpcodeUnknown      = 0,
	kIOPSDIOOpcodePing         = 1,
	kIOPSDIOOpcodeInit         = 2,
	kIOPSDIOOpcodeFree         = 3,
	kIOPSDIOOpcodeReset        = 4,
	kIOPSDIOOpcodeSetBusParam  = 5,
	kIOPSDIOOpcodeSendCommand  = 6,
	kIOPSDIOOpcodeTransferData = 7,
};


typedef UInt32 IOPSDIO_flags_t;

enum IOPSDIOFlags
{
	kIOPSDIOFlagsNone            = 0,
};


typedef UInt32 IOPSDIO_status_t;

enum IOPSDIOStatus
{
// IOP Messaging Errors
	
	/** @brief Command succeeded. */
	kIOPSDIOStatusSuccess    = 0,
	
	/** @brief Status is not yet known (e.g. message has not been sent or processed). */
	kIOPSDIOStatusUnknown    = 1,
	
	/** @brief One of the IOP parameters was invalid. */
	kIOPSDIOParameterInvalid = 2,

// Common IOReturn Values
	// If the IOP Status is in the range [0x200 0x300) then it is a standard
	// IOKit IOReturn value without the "iokit_common_err(   )" bits set
	
// SDIOReturn_t Values
	// If the IOP status is in the range [0x300 0x800) it is an SDIOReturn_t
	// value. This value is the same as the base value of IOSDIOReturn in the
	// IOKit SDIO family, without the "iokit_family_err(sub_iokit_sdio,   )"
	// bits set.
};



/* this must match <drivers/dma.h>::struct dma_segment */
struct IOPSDIO_dma_segment {
	u_int32_t	paddr;
	u_int32_t	length;
};


/** @brief Standard header on all IOP SDIO Commands. */
struct IOPSDIOHeader
{
    IOPSDIO_opcode_t  opcode;
    IOPSDIO_flags_t   flags;
    IOPSDIO_status_t  status;
};


/** @brief Info about the target SDIO Host Controller. */
struct IOPSDIOTargetSDHC
{
	/** @brief The Base physical address of the SDIO block's register file. */
	UInt32 basePhysicalAddr;

	/** @brief Wake event. */
	UInt32 dmaCompleteEventAddr;
};


// SDIO INIT


/** @brief Bus Parameter Command. */
struct IOPSDIOInitCmd
{
	/** @brief The capabilities of the SDHC. */
	UInt64 sdhcCapabilities;
	
	/** @brief The Maximum current capabilities of the SDHC. */
	UInt64 sdhcMaxCurrentCapabilities;
};


// SDIO FREE


/** @brief Bus Parameter Command. */
struct IOPSDIOFreeCmd
{
	// Nothing at the moment
};


// SDIO RESET

/** @brief SDHC Reset Command. */
struct IOPSDIOResetCmd
{
	/** @brief Reset flags. */
	UInt32 resetFlags;
};


// BUS PARAMETERS

/** @brief Bus Parameter Command. */
struct IOPSDIOSetBusParamCmd
{
	/** @brief Base clock rate input to the SD Block.
	 * If 0, the value from the SDHC capabilities register is used instead.
	 */
	UInt32 baseClockRateHz;

	/** @brief New SDIO clock rate in Hz. No change if 0.
	 * On return, this is set the value that was actually programmed into the 
	 * SDHC block.
	 */
	UInt32 clockRateHz;

	/** @brief New SDIO bus width (1 or 4). No change if 0. */
	UInt8 busWidth;

	/** @brief 1 if the SDIO clock should be on, -1 if off, 0 to leave unchanged */
	int clockMode;

	/** @brief 2 for high speed mode, 1 for normal speed, 0 to leave unchanged. */
	int busSpeedMode;
};


// SDIO COMMAND

/** @brief Send an SDIO Command using the command line.
 * Note that the status of the command will be reflected by the iopsdio.status field.
 */
struct IOPSDIOCommandCmd
{
	/** @brief The SDIO Command. */
	struct SDIOCommand command;
	
	/** @brief The command response. */
	struct SDIOCommandResponse response;
};


// SDIO DATA COMMAND

/** @brief Send an SDIO Command with data (including the command line)
 * Note that the status of the command will be reflected by the iopsdio.status field.
 */
struct IOPSDIOTransferCmd
{
	/** @brief The SDIO Command. */
	struct SDIOCommand command;
	
	/** @brief The command response. */
	struct SDIOCommandResponse response;

	/** @brief The SDIO transfer parameters. */
	struct SDIOTransfer transfer;
	
	/** @brief Information about the memory segments involved in the transfer. */
	struct SDIOMemorySegments memory;

	/** @brief Physical segments for DMA engine. Variable Size. */	
	struct IOPSDIO_dma_segment segment[];
};



// IOP MESSAGES

/** @brief IOP SDIO Message. */
union IOPSDIOMessage
{
	struct {
		/** @brief IOP Header. */
		struct IOPSDIOHeader header;

		/** @brief Target SDIO block. */
		struct IOPSDIOTargetSDHC targetSDHC;
		
		/** @brief Command Data. */
		union {
			struct IOPSDIOInitCmd        initCmd;
			struct IOPSDIOFreeCmd        freeCmd;
			struct IOPSDIOResetCmd       resetCmd;
			struct IOPSDIOSetBusParamCmd setBusParamCmd;
			struct IOPSDIOCommandCmd     commandCmd;
			struct IOPSDIOTransferCmd    transferCmd;
		};
	};
	
	UInt8 _pad[kIOPSDIO_MESSAGE_SIZE] SDIO_RESERVED;
};


union _IOPSDIO_sgl_sizing_tool
{
	struct {
		/** @brief IOP Header. */
		struct IOPSDIOHeader header;

		/** @brief Target SDIO block. */
		struct IOPSDIOTargetSDHC targetSDHC;
		
		/** @brief Command Data. */
		union {
			struct IOPSDIOTransferCmd     transferCmd;
			/* XXX add other sgl-using commands */
		};
	};	
};

#define kIOPSDIO_MAX_SEGMENTS	((kIOPSDIO_MESSAGE_SIZE - sizeof(union _IOPSDIO_sgl_sizing_tool)) / sizeof(IOPSDIO_dma_segment))


#endif // _IOP_SDIO_PROTOCOL_H_
