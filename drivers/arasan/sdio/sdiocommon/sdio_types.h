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


#ifndef _SDIO_TYPES_H_
#define _SDIO_TYPES_H_


#ifdef PLATFORM_VARIANT_IOP
#include <sys/types.h>
#else
#include <IOKit/IOTypes.h>
#endif


#ifdef __cplusplus
extern "C"  {
#endif

#ifdef __GNUC__
#define SDIO_PACKED     __attribute__((packed))
#define SDIO_DEPRECATED __attribute__((deprecated))
#define SDIO_RESERVED   __attribute__((deprecated))
#else
#error Unknown Compiler
#endif


typedef UInt32 IOSDIOClockRate;	

enum
{
	kIOSDIOMaxClockRate = 50000000
};

enum IOSDIOClockMode
{
	kIOSDIOClockOff = -1,
	kIOSDIOClockOn  = +1
};

enum IOSDIOBusWidth
{
	kIOSDIO1BitMode = 1,
	kIOSDIO4BitMode = 4
};

enum IOSDIOBusSpeedMode
{
	kIOSDIONormalSpeed = 1,
	kIOSDIOHighSpeed   = 2
};
	

enum SDIOCommandIndex
{
	kSDIOCmd0_GoIdleState          = 0,
	kSDIOCmd1_SendOpCond           = 1,
	kSDIOCmd2_AllSendCID           = 2,
	kSDIOCmd3_SendRelativeAddr     = 3,
	kSDIOCmd4_SetDSR               = 4,
	kSDIOCmd5_IOSendOpCond         = 5,
	kSDIOCmd6_SwitchFunc           = 6,
	kSDIOCmd7_SelectDeselectCard   = 7,
	kSDIOCmd9_SendCSD              = 9,
	kSDIOCmd10_SendCID             = 10,
	kSDIOCmd12_StopTransmission    = 12,
	kSDIOCmd13_SendStatus          = 13,
	kSDIOCmd15_GoInactiveState     = 15,
	kSDIOCmd16_SetBlockLength      = 16,
	kSDIOCmd17_ReadSingleBlock     = 17,
	kSDIOCmd18_ReadMultipleBlock   = 18,
	kSDIOCmd24_WriteBlock          = 24,
	kSDIOCmd25_WriteMultipleBlock  = 25,
	kSDIOCmd27_ProgramCSD          = 27,
	kSDIOCmd28_SetWriteProt        = 28,
	kSDIOCmd29_ClearWriteProt      = 29,
	kSDIOCmd30_SendWriteProt       = 30,
	kSDIOCmd32_EraseWrBlkStart     = 32,
	kSDIOCmd33_EraseWrBlkEnd       = 33,
	kSDIOCmd38_Erase               = 38,
	kSDIOCmd42_LockUnlock          = 42,
	kSDIOCmd52_IORWDirect          = 52,
	kSDIOCmd53_IORWExtended        = 53,
	kSDIOCmd55_AppCmd              = 55,
	kSDIOCmd56_GenCmd              = 56,
	kSDIOCmd58_ReadOCR             = 58,
	kSDIOCmd59_CRCOnOff            = 59,
	
	kSDIOAcmd6_SetBusWidth         = 6,
	kSDIOAcmd13_SDStatus           = 13,
	kSDIOAcmd22_SendNumWrBlocks    = 22,
	kSDIOAcmd23_SetWrBlkEraseCount = 23,
	kSDIOAcmd41_SDAppOpCond        = 41,
	kSDIOAcmd42_SetClrCardDetect   = 42,
	kSDIOAcmd51_SendSCR            = 51,
};

enum SDIODirection
{
	kSDIODirectionNone  = 0,        // Same as kIODirectionNone
	kSDIODirectionRead  = (1 << 0),	// Same as kIODirectionIn
	kSDIODirectionWrite = (1 << 1),	// Same as kIODirectionOut
};
	
struct SDIOCommand
{
	UInt16 index;		// SDIOCommandIndex
	UInt32 argument;
};

struct SDIOCommandResponse
{
	UInt32 response[4];
};

struct SDIOMemorySegments
{
	UInt64 dataLength;
	UInt32 segmentCount;
	void *segmentList;
};
	
struct SDIOTransfer
{
	struct SDIOCommand command;
	UInt8 direction;	// SDIODirection
	UInt16 blockSize;
	UInt16 blockCount;
};



enum SDIOReturn
{
	/** @brief Success. */
	kSDIOReturnSuccess              = 0,
	
// Generic IOReturn-derived Errors
	
	/** @brief Invalid Argument. */
	kSDIOReturnBadArgument          = 0x2c2,
	
	/** @brief Can't allocate memory. */
	kSDIOReturnNoMemory             = 0x2bd,

	
// General SDIO Errors

	/** @brief General SDIO Error. */
	kSDIOReturnSDIOError            = 0x300,
	
	
// Host Controller Errors
	
	/** @brief General Host Controller Error. */
	kSDIOReturnSDHCError            = 0x400,
	
	/** @brief Host controller physical address invalid. */
	kSDIOReturnNoHostController     = 0x401,
	
	/** @brief Couldn't perform command because SD Clock is disabled. */
	kSDIOReturnInternalClockUnstable= 0x402,
	
	/** @brief Couldn't perform command because SD Clock is disabled. */
	kSDIOReturnSDClockDisabled      = 0x403,
	
	/** @brief Failed due to errors in ADMA transfer. */
	kSDIOReturnADMAError            = 0x404,
	
	/** @brief Failed due to errors in SDMA transfer. */
	kSDIOReturnSDMAError            = 0x405,
	
	/** @brief Error performing Auto CMD12. */
	kSDIOReturnAutoCMD12Error       = 0x406,
	
	/** @brief HC not supplying power to card. */
	kSDIOReturnCurrentLimitError    = 0x407,
	
	/** @brief Vendor-specific error. */
	kSDIOReturnVendorError          = 0x408,
	
	/** @brief Device in reset. */
	kSDIOReturnInReset              = 0x409,
	
	/** @brief Block never signaled command complete. */
	kSDIOReturnNoCmdComplete        = 0x40A,
	
	/** @brief Block never signaled transfer complete. */
	kSDIOReturnNoTransferComplete   = 0x40B,
	
	/** @brief DMA transfer timed out. */
	kSDIOReturnDMATimeout           = 0x40C,
	
	
// Card Errors
	
	/** @brief General Card Error. */
	kSDIOReturnCardError            = 0x500,
	
	/** @brief Failed because no card is inserted. */
	kSDIOReturnNoCard               = 0x501,
	
	/** @brief Failed because the card was ejected. */
	kSDIOReturnCardEjected          = 0x502,
	
	/** @brief Failed because card is write protected. */
	kSDIOReturnWriteProtected       = 0x503,
	
	
// Problems on SDIO Command Line
	
	/** @brief General SDIO Cmd line error. */
	kSDIOReturnCmdError             = 0x600,
	
	/** @brief SDIO Cmd line is busy. */
	kSDIOReturnCmdLineBusy          = 0x601,
	
	/** @brief SDIO Cmd line's signal level is incorrect. */
	kSDIOReturnCmdSignalInvalid     = 0x602,
	
	/** @brief Command index error in response. */
	kSDIOReturnCmdIndexError        = 0x603,
	
	/** @brief Conflict on SDIO Command Line. */
	kSDIOReturnCmdLineConflict      = 0x604,
	
	/** @brief SDIO Cmd failed with a CRC error. */
	kSDIOReturnCmdCRCError          = 0x605,
	
	/** @brief SDIO Cmd failed with a End Bit error. */
	kSDIOReturnCmdEndBitError       = 0x606,
	
	/** @brief SDIO Cmd Timed out (no response. */
	kSDIOReturnCmdTimeout           = 0x607,
	
	
// Problems on SDIO Data Line
	
	/** @brief General SDIO Data Line Error. */
	kSDIOReturnDataError            = 0x700,
	
	/** @brief SDIO Data line is busy. */
	kSDIOReturnDataLineBusy         = 0x701,
	
	/** @brief SDIO Data line's signal level is incorrect. */
	kSDIOReturnDataSignalInvalid    = 0x702,
	
	/** @brief SDIO Data failed with a CRC error. */
	kSDIOReturnDataCRCError         = 0x703,
	
	/** @brief SDIO Data failed with a CRC error. */
	kSDIOReturnDataEndBitError      = 0x704,
	
	/** @brief SDIO Data Timed out (no response. */
	kSDIOReturnDataTimeout          = 0x705,
};

typedef enum SDIOReturn SDIOReturn_t;



#ifdef __cplusplus
}
#endif

#endif /* _SDIO_TYPES_H_ */

