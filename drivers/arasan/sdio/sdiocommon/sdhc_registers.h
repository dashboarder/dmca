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


#ifndef _SDIO_SDHC_H
#define _SDIO_SDHC_H


#ifdef PLATFORM_VARIANT_IOP
#include <sdiocommon/sdio_types.h>
#include <sys/types.h>
#else
#include <IOKit/sdio/sdio_types.h>
#include <IOKit/IOTypes.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#pragma mark -
#pragma mark SDHC Register File

/** Safe typedef for use of SDHCRegisters register file. */
typedef volatile struct SDHCRegisters SDHCRegisters_t;

/** Standard SDIO Host Controller 2.0 Register File. */
struct SDHCRegisters
{
// SD Command Generation: 0x00 - 0x0F
	volatile UInt32 sdmaSystemAddr;
	volatile UInt16 blockSize;
	volatile UInt16 blockCount;
	volatile UInt32 argument;
	volatile UInt16 transferMode;
	volatile UInt16 command;

// Response: 0x10 - 0x1F
	volatile UInt32 response[4];

// Buffer Data Port: 0x20 - 0x23
	volatile UInt32 bufferDataPort;

// Host Controls: 0x24 - 0x2F
	volatile UInt32 presentState;
	volatile UInt8 hostControl;
	volatile UInt8 powerControl;
	volatile UInt8 blockGapControl;
	volatile UInt8 wakeupControl;
	volatile UInt16 clockControl;
	volatile UInt8 timeoutControl;
	volatile UInt8 softwareReset;

// Interrupt Controls: 0x30 - 0x3D
	volatile UInt16 normalInterruptStatus;
	volatile UInt16 errorInterruptStatus;
	volatile UInt16 normalInterruptStatusEnable;
	volatile UInt16 errorInterruptStatusEnable;
	volatile UInt16 normalInterruptSignalEnable;
	volatile UInt16 errorInterruptSignalEnable;
	volatile UInt16 autoCmd12ErrorStatus;
	volatile UInt8 __reservedIntArea[2] SDIO_RESERVED; // Do not use, will change

// Capabilities: 0x40 - 0x4F
	volatile UInt64 capabilities;
	volatile UInt64 maxCurrentCapabilities;

// Force Event: 0x50 - 0x53
	volatile UInt16 forceEventAutoCmd12ErrorStatus;
	volatile UInt16 forceEventErrorInterruptStatus;

// ADMA: 0x54 - 0x5F
	volatile UInt8 admaErrorStatus;
	volatile UInt8 __reservedCapArea[3] SDIO_RESERVED;  // Do not use, will change
	volatile UInt64 admaSystemAddr;

// Reserved: 0x60 - 0xEF
	volatile UInt8 __reserved[0x90] SDIO_RESERVED;  // Do not use, will change

// Common Area: 0xF0 - 0xFF
	volatile UInt8 __reservedCommonArea[0xC] SDIO_RESERVED;  // Do not use, will change
	volatile UInt16 slotInterruptStatus;
	volatile UInt16 hostControllerVersion;
};


#pragma mark -
#pragma mark SDHC Register Bits

enum SDIOTransferModeField
{
	kSDIOTransferEnableDMA        = (1 << 0),
	kSDIOTransferEnableBlockCount = (1 << 1),
	kSDIOTransferEnableAutoCmd12  = (1 << 2),
	
	kSDIOTransferReadFromCard     = (1 << 4),
	kSDIOTransferMultipleBlocks   = (1 << 5),
};
	
	
#pragma mark -
#pragma mark Capabilities

UInt16 sdhc_getMaxBlockLength(const SDHCRegisters_t *sdhc);
	
UInt32 sdhc_getBaseClockFrequencyHz(const SDHCRegisters_t *sdhc);
	
bool sdhc_isSuspendResumeSupported(const  SDHCRegisters_t *sdhc);
	
bool sdhc_isHighSpeedSupported(const  SDHCRegisters_t *sdhc);
	

#pragma mark -
#pragma mark Present State

bool sdhc_isCommandInhibitedOnCmd(const SDHCRegisters_t *sdhc);

bool sdhc_isCommandInhibitedOnData(const SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Clocks

/** @brief Sets the SD Clock rate.
 * @param[in] hcReg
 *	Target SDIO Host Controller.
 * @param[in] targetSDClkRateHz
 *	Desired SD Clock rate, in Hz
 * @param[in] inputClkRateHz
 *	SD Block's input clock rate, in Hz
 * @return
 *	The actual clock rate set, in Hz.
 */
UInt32 sdhc_setClockDividerRate(SDHCRegisters_t *sdhc, UInt32 targetSDClkRateHz, UInt32 inputClkRateHz);

void sdhc_setClockDivider(SDHCRegisters_t *sdhc, UInt16 divider);

UInt16 sdhc_getClockDivider(const SDHCRegisters_t *sdhc);

void sdhc_enableInternalClock(SDHCRegisters_t *sdhc, bool enable);

bool sdhc_isInternalClockStable(const SDHCRegisters_t *sdhc);

void sdhc_enableSDClock(SDHCRegisters_t *sdhc, bool enable);

bool sdhc_isSDClockEnabled(const SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Block Size

void sdhc_setBlockSize(SDHCRegisters_t *sdhc, UInt16 size);


#pragma mark -
#pragma mark Commands

void sdhc_copyCommandResponse(const SDHCRegisters_t *sdhc, struct SDIOCommandResponse *response);


#pragma mark -
#pragma mark Command State

/** @brief Enable/Disable status bits needed for command completion and error checking. */
void sdhc_enableCommandStatus(SDHCRegisters_t *sdhc, bool enable);
	
/** @brief True if the command has completed for any reason. */
bool sdhc_isCommandComplete(const SDHCRegisters_t *sdhc);

/** @brief Gets the command status after completion. */
SDIOReturn_t sdhc_getCommandStatus(const SDHCRegisters_t *sdhc);

/** @brief Clear status bits needed for command completion and error checking. */
void sdhc_clearCommandStatus(SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Transfer State

/** @brief Enable/Disable status bits needed for transfer completion and error checking. */
void sdhc_enableTransferStatus(SDHCRegisters_t *sdhc, bool enable);

/** @brief True if the transfer has completed for any reason. */
bool sdhc_isTransferComplete(const SDHCRegisters_t *sdhc);

/** @brief Gets the current transfer status after completion. */
SDIOReturn_t sdhc_getTransferStatus(const SDHCRegisters_t *sdhc);

/** @brief Clear status bits needed for transfer completion and error checking. */
void sdhc_clearTransferStatus(SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark PIO

bool sdhc_isBufferReadReady(const SDHCRegisters_t *sdhc);

void sdhc_enableBufferReadReady(SDHCRegisters_t *sdhc, bool enable);

void sdhc_clearBufferReadReady(SDHCRegisters_t *sdhc);

bool sdhc_isBufferWriteReady(const SDHCRegisters_t *sdhc);

void sdhc_enableBufferWriteReady(SDHCRegisters_t *sdhc, bool enable);

void sdhc_clearBufferWriteReady(SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Data Timeout

void sdhc_setDataTimeoutCounter(SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Reset

enum SDHCResetFlags
{
	kSDHCResetAll      = (1 << 0),
	kSDHCResetCmdLine  = (1 << 1),
	kSDHCResetDataLine = (1 << 2),
};
typedef UInt8 SDHCResetFlags_t;

void sdhc_reset(SDHCRegisters_t *sdhc, SDHCResetFlags_t flags);

SDHCResetFlags_t sdhc_isResetting(const SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Interrupts

bool sdhc_isSignalingInterrupt(const SDHCRegisters_t *sdhc);

/** @brief Clear all interrupt signals.
 * Only the status fields that are signaling an interrupt are cleared.
 */
void sdhc_clearInterruptSignals(SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Card Interrupt

bool sdhc_isCardInterrupt(const SDHCRegisters_t *sdhc);

void sdhc_enableCardInterrupt(SDHCRegisters_t *sdhc, bool enable);

bool sdhc_isCardInterruptEnabled(const SDHCRegisters_t *sdhc);


#pragma mark -
#pragma mark Host Control

void sdhc_setHighSpeedMode(SDHCRegisters_t *sdhc, bool enabled);

void sdhc_setDataTransferWidth(SDHCRegisters_t *sdhc, UInt8 width);



#ifdef __cplusplus
}
#endif

#endif /* _SDIO_SDHC_H */

