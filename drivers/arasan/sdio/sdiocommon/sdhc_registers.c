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



#include "sdhc_registers.h"


// TODO: Endianness

#pragma mark -
#pragma mark Macros

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif



#pragma mark -
#pragma mark Capabilities

enum SDHCCapabilitiesFieldOffset
{
	kSDHCCapabilityBaseClockRateOffset        = 8,
	kSDHCCapabilityMaxBlockLengthOffset       = 16,
	kSDHCCapabilityHighSpeedSupportOffset     = 21,
	kSDHCCapabilitySuspendResumeSupportOffset = 23
};

enum SDHCCapabilitiesField
{
	kSDHCCapabilityBaseClockRate        = (0x3F << kSDHCCapabilityBaseClockRateOffset),
	kSDHCCapabilityMaxBlockLength       = (0x3 << kSDHCCapabilityMaxBlockLengthOffset),
	kSDHCCapabilityHighSpeedSupport     = (1 << kSDHCCapabilityHighSpeedSupportOffset),
	kSDHCCapabilitySuspendResumeSupport = (1 << kSDHCCapabilitySuspendResumeSupportOffset),
};



UInt16
sdhc_getMaxBlockLength(const SDHCRegisters_t *sdhc)
{
	static UInt16 mapBlkLenCapToLen[] = {512, 1024, 2048, 0};
	
	UInt8 cap = (sdhc->capabilities & kSDHCCapabilityMaxBlockLength) >> kSDHCCapabilityMaxBlockLengthOffset;
	
	return mapBlkLenCapToLen[cap];
}

UInt32
sdhc_getBaseClockFrequencyHz(const SDHCRegisters_t *sdhc)
{
	UInt32 rateMHz = (sdhc->capabilities & kSDHCCapabilityBaseClockRate) >> kSDHCCapabilityBaseClockRateOffset;
	return rateMHz * 1000000;
}

bool
sdhc_isSuspendResumeSupported(const  SDHCRegisters_t *sdhc)
{
	return sdhc->capabilities & kSDHCCapabilitySuspendResumeSupport;
}

bool
sdhc_isHighSpeedSupported(const  SDHCRegisters_t *sdhc)
{
	return sdhc->capabilities & kSDHCCapabilityHighSpeedSupport;
}



#pragma mark -
#pragma mark Present State

enum PresentStateFieldOffsets
{
	kCmdInhibitOffset  = 0,
	kDataInhibitOffset = 1,
};

enum PresentStateField
{
	kCmdInhibit   = (1 << kCmdInhibitOffset),
	kDataInhibit  = (1 << kDataInhibitOffset),
};


bool
sdhc_isCommandInhibitedOnCmd(const SDHCRegisters_t *sdhc)
{
	return sdhc->presentState & kCmdInhibit;
}

bool
sdhc_isCommandInhibitedOnData(const SDHCRegisters_t *sdhc)
{
	return sdhc->presentState & kDataInhibit;
}



#pragma mark -
#pragma mark Clocks


#define kMaxClockDivisor (256)

enum ClkControlFieldOffsets
{
	kInternalClkEnableOffset    = 0,
	kInternalClkStableOffset    = 1,
	kSDClkEnableOffset          = 2,
	kSDClkFrequencySelectOffset = 8
};

enum ClkControlFields
{
	kInternalClkEnable    = (1    << kInternalClkEnableOffset),
	kInternalClkStable    = (1    << kInternalClkStableOffset),
	kSDClkEnable          = (1    << kSDClkEnableOffset),
	kSDClkFrequencySelect = (0xFF << kSDClkFrequencySelectOffset)
};

UInt32
sdhc_setClockDividerRate(SDHCRegisters_t *sdhc, UInt32 targetSDClkRateHz, UInt32 inputClkRateHz)
{
	UInt16 divisor = 1;
	
	while(inputClkRateHz / divisor > targetSDClkRateHz) {
		divisor *= 2;
	}
	
	divisor = min(divisor, kMaxClockDivisor);
	
	sdhc_setClockDivider(sdhc, divisor);

	return inputClkRateHz / divisor;
}

void
sdhc_setClockDivider(SDHCRegisters_t *sdhc, UInt16 divisor)
{
	divisor >>= 1;		// divisor is offset, 0x00 == clk / 1
	divisor <<= kSDClkFrequencySelectOffset; // shift to correct offset

	sdhc->clockControl &= ~kSDClkFrequencySelect;

	// Note: must enable internal clock here or SDIO block will lock up
	sdhc->clockControl |= (divisor | kInternalClkEnable);
}

UInt16
sdhc_getClockDivider(const SDHCRegisters_t *sdhc)
{
	UInt16 clkControl = sdhc->clockControl & kSDClkFrequencySelect;
	
	clkControl >>= kSDClkFrequencySelectOffset; // shift to correct offset

	// divisor is offset, 0x00 == clk / 1
	clkControl <<= 1;
	if(0 == clkControl) {
		clkControl = 1;
	}
	
	return clkControl;
}

void
sdhc_enableInternalClock(SDHCRegisters_t *sdhc, bool enable)
{
	UInt16 clkControl = sdhc->clockControl;
	
	if(enable) {
		clkControl |= kInternalClkEnable;
	} else {
		clkControl &= ~kInternalClkEnable;
	}
	
	sdhc->clockControl = clkControl;
}

bool
sdhc_isInternalClockStable(const SDHCRegisters_t *sdhc)
{
	return sdhc->clockControl & kInternalClkStable;
}

void
sdhc_enableSDClock(SDHCRegisters_t *sdhc, bool enable)
{
	UInt16 clkControl = sdhc->clockControl;
	
	if(enable) {
		clkControl |= kSDClkEnable;
	} else {
		clkControl &= ~kSDClkEnable;
	}
	
	sdhc->clockControl = clkControl;
}

bool
sdhc_isSDClockEnabled(const SDHCRegisters_t *sdhc)
{
	return sdhc->clockControl & kSDClkEnable;
}



#pragma mark -
#pragma mark Block Size


enum SDHCBlockSizeOffset
{
	kSDHCTransferSizeOffset       = 0,
	kSDHCHostSDMABufferSizeOffset = 12,
	kSDHCTransferSizeHighOffset   = 15,
};

enum SDHCBlockSizeMask
{
	kSDHCTransferSize       = (0xFFF << kSDHCTransferSizeOffset),
	kSDHCHostSDMABufferSize = (0x7 << kSDHCTransferSizeOffset),
	kSDHCTransferSizeHigh   = (1 << kSDHCTransferSizeHighOffset),
	
};


void
sdhc_setBlockSize(SDHCRegisters_t *sdhc, UInt16 size)
{
	UInt16 blockSize = sdhc->blockSize & ~(kSDHCTransferSizeHigh | kSDHCTransferSize);
	
	
	blockSize |= size & kSDHCTransferSize;
	
	// High bit
	blockSize |= (size & (1 << kSDHCHostSDMABufferSizeOffset)) 
	           << (kSDHCTransferSizeHighOffset - kSDHCHostSDMABufferSizeOffset);
	
	sdhc->blockSize = blockSize;
}



#pragma mark -
#pragma mark Commands

void
sdhc_copyCommandResponse(const SDHCRegisters_t *sdhc, struct SDIOCommandResponse *response)
{
	for(UInt16 i=0; i < 4; ++i) {
		response->response[i] = sdhc->response[i];
	}
}



#pragma mark -
#pragma mark Command State


enum NormalIntFieldOffsets
{
	kCmdCompleteIntOffset      = 0,
	kTransferCompleteIntOffset = 1,
	kBlockGapIntOffset         = 2,
	kDMAIntOffset              = 3, 
	kBufferWriteReadyIntOffset = 4,
	kBufferReadReadyIntOffset  = 5,
	kCardInsertionIntOffset    = 6,
	kCardRemovalIntOffset      = 7,
	kCardIntOffset             = 8,

	kErrorIntOffset            = 15
};

enum NormalIntFields
{
	kCmdCompleteInt      = (1 << kCmdCompleteIntOffset),
	kTransferCompleteInt = (1 << kTransferCompleteIntOffset),
	kBlockGapInt         = (1 << kBlockGapIntOffset),
	kDMAInt              = (1 << kDMAIntOffset),
	kBufferWriteReadyInt = (1 << kBufferWriteReadyIntOffset),
	kBufferReadReadyInt  = (1 << kBufferReadReadyIntOffset),
	kCardInsertionInt    = (1 << kCardInsertionIntOffset),
	kCardRemovalInt      = (1 << kCardRemovalIntOffset),
	kCardInt             = (1 << kCardIntOffset),

	kErrorInt            = (1 << kErrorIntOffset),
};

enum ErrorIntFieldOffsets
{
	kCmdTimeoutErrorIntOffset   = 0,
	kCmdCRCErrorIntOffset       = 1,
	kCmdEndBitErrorIntOffset    = 2,
	kCmdIndexErrorIntOffset     = 3,
	kDataTimeoutErrorIntOffset  = 4,
	kDataCRCErrorIntOffset      = 5,
	kDataEndBitErrorIntOffset   = 6,
	kCurrentLimitErrorIntOffset = 7,
	kAutoCmd12ErrorIntOffset    = 8,
	kADMAErrorIntOffset         = 9
};

enum ErrorIntFields
{
	kCmdTimeoutErrorInt   = (1 << kCmdTimeoutErrorIntOffset),
	kCmdCRCErrorInt       = (1 << kCmdCRCErrorIntOffset),
	kCmdEndBitErrorInt    = (1 << kCmdEndBitErrorIntOffset),
	kCmdIndexErrorInt     = (1 << kCmdIndexErrorIntOffset),
	kDataTimeoutErrorInt  = (1 << kDataTimeoutErrorIntOffset),
	kDataCRCErrorInt      = (1 << kDataCRCErrorIntOffset),
	kDataEndBitErrorInt   = (1 << kDataEndBitErrorIntOffset),
	kCurrentLimitErrorInt = (1 << kCurrentLimitErrorIntOffset),
	kAutoCmd12ErrorInt    = (1 << kAutoCmd12ErrorIntOffset),
	kADMAErrorInt         = (1 << kADMAErrorIntOffset)
};


void
sdhc_enableCommandStatus(SDHCRegisters_t *sdhc, bool enable)
{
	if(enable) {
		sdhc->normalInterruptStatusEnable |= kCmdCompleteInt;
		sdhc->errorInterruptStatusEnable  |= (kCmdIndexErrorInt | kCmdEndBitErrorInt | kCmdCRCErrorInt | kCmdTimeoutErrorInt);
	} else {
		sdhc->normalInterruptStatusEnable &= ~kCmdCompleteInt;
		sdhc->errorInterruptStatusEnable  &= ~(kCmdIndexErrorInt | kCmdEndBitErrorInt | kCmdCRCErrorInt | kCmdTimeoutErrorInt);
	}
}

bool
sdhc_isCommandComplete(const SDHCRegisters_t *sdhc)
{
	return (sdhc->normalInterruptStatus & kCmdCompleteInt)
		|| (sdhc->errorInterruptStatus
			   & (kCmdIndexErrorInt | kCmdEndBitErrorInt | kCmdCRCErrorInt | kCmdTimeoutErrorInt));
}

SDIOReturn_t
sdhc_getCommandStatus(const SDHCRegisters_t *sdhc)
{
	const UInt16 state = kCmdTimeoutErrorInt | kCmdCRCErrorInt;
	const UInt16 errorStatus = sdhc->errorInterruptStatus;
	
	if((errorStatus & state) == state) {
		return kSDIOReturnCmdLineConflict;
	} else if(errorStatus & kCmdTimeoutErrorInt) {
		return kSDIOReturnCmdTimeout;
	} else if(errorStatus & kCmdCRCErrorInt) {
		return kSDIOReturnCmdCRCError;
	} else if(errorStatus & kCmdEndBitErrorInt) {
		return kSDIOReturnCmdEndBitError;
	} else if(errorStatus & kCmdIndexErrorInt) {
		return kSDIOReturnCmdIndexError;
	} else {
		return kSDIOReturnSuccess;
	}
}

void
sdhc_clearCommandStatus(SDHCRegisters_t *sdhc)
{
	sdhc->normalInterruptStatus = kCmdCompleteInt;
	sdhc->errorInterruptStatus = (kCmdIndexErrorInt | kCmdEndBitErrorInt | kCmdCRCErrorInt | kCmdTimeoutErrorInt);	
}



#pragma mark -
#pragma mark Transfer State

void
sdhc_enableTransferStatus(SDHCRegisters_t *sdhc, bool enable)
{
	if(enable) {
		sdhc->normalInterruptStatusEnable |= kTransferCompleteInt;
		sdhc->errorInterruptStatusEnable  |= (kDataTimeoutErrorInt | kDataCRCErrorInt | kDataEndBitErrorInt);
	} else {
		sdhc->normalInterruptStatusEnable &= ~kTransferCompleteInt;
		sdhc->errorInterruptStatusEnable  &= ~(kDataTimeoutErrorInt | kDataCRCErrorInt | kDataEndBitErrorInt);
	}
}

bool
sdhc_isTransferComplete(const SDHCRegisters_t *sdhc)
{
	return (sdhc->normalInterruptStatus & kTransferCompleteInt)
		|| (sdhc->errorInterruptStatus 
			  & (kDataTimeoutErrorInt | kDataCRCErrorInt | kDataEndBitErrorInt));
}

SDIOReturn_t
sdhc_getTransferStatus(const SDHCRegisters_t *sdhc)
{
	const UInt16 errorStatus = sdhc->errorInterruptStatus;
	
	if((errorStatus & kDataTimeoutErrorInt)
		  && !(sdhc->normalInterruptStatus & kTransferCompleteInt)) {
		return kSDIOReturnDataTimeout;
	} else if(errorStatus & kDataCRCErrorInt) {
		return kSDIOReturnDataCRCError;
	} else if(errorStatus & kDataEndBitErrorInt) {
		return kSDIOReturnDataEndBitError;
	} else {
		return kSDIOReturnSuccess;
	}
}

void
sdhc_clearTransferStatus(SDHCRegisters_t *sdhc)
{
	sdhc->normalInterruptStatus = kTransferCompleteInt;
	sdhc->errorInterruptStatus = (kDataTimeoutErrorInt | kDataCRCErrorInt | kDataEndBitErrorInt);	
}



#pragma mark -
#pragma mark PIO

bool
sdhc_isBufferReadReady(const SDHCRegisters_t *sdhc)
{
	return sdhc->normalInterruptStatus & kBufferReadReadyInt;
}

void
sdhc_enableBufferReadReady(SDHCRegisters_t *sdhc, bool enable)
{
	if(enable) {
		sdhc->normalInterruptStatusEnable |= kBufferReadReadyInt;
	} else {
		sdhc->normalInterruptStatusEnable &= ~kBufferReadReadyInt;
	}
}

void
sdhc_clearBufferReadReady(SDHCRegisters_t *sdhc)
{
	sdhc->normalInterruptStatus = kBufferReadReadyInt;
}

bool
sdhc_isBufferWriteReady(const SDHCRegisters_t *sdhc)
{
	return sdhc->normalInterruptStatus & kBufferWriteReadyInt;
}

void
sdhc_enableBufferWriteReady(SDHCRegisters_t *sdhc, bool enable)
{
	if(enable) {
		sdhc->normalInterruptStatusEnable |= kBufferWriteReadyInt;
	} else {
		sdhc->normalInterruptStatusEnable &= ~kBufferWriteReadyInt;
	}
}

void
sdhc_clearBufferWriteReady(SDHCRegisters_t *sdhc)
{
	sdhc->normalInterruptStatus = kBufferWriteReadyInt;
}



#pragma mark -
#pragma mark Data Timeout


enum SDHCTimeoutControlOffset
{
	kSDHCDataTimeoutCounterOffset = 0,
};

enum SDHCTimeoutControl
{
	kSDHCDataTimeoutCounter = (0xF << kSDHCDataTimeoutCounterOffset),
};


void
sdhc_setDataTimeoutCounter(SDHCRegisters_t *sdhc)
{
	UInt16 divisor = sdhc->clockControl & kSDClkFrequencySelect;
	
	// BCOM algorithm
	UInt8 timeout = 7;
	while(timeout && (divisor & 1) == 0) {
		timeout--;
		divisor >>= 1;
	}
	
	bool on = sdhc->errorInterruptStatusEnable & kDataTimeoutErrorInt;
	sdhc->errorInterruptStatusEnable &= ~kDataTimeoutErrorInt;
	
	sdhc->timeoutControl |= (divisor & kSDHCDataTimeoutCounter);
	
	if(on) {
		sdhc->errorInterruptStatusEnable |= kDataTimeoutErrorInt;
	}
}



#pragma mark -
#pragma mark Reset


void
sdhc_reset(SDHCRegisters_t *sdhc, SDHCResetFlags_t flags)
{
	sdhc->softwareReset |= (flags & (kSDHCResetDataLine | kSDHCResetCmdLine | kSDHCResetAll));
}

SDHCResetFlags_t
sdhc_isResetting(const SDHCRegisters_t *sdhc)
{
	return sdhc->softwareReset & (kSDHCResetDataLine | kSDHCResetCmdLine | kSDHCResetAll);
}



#pragma mark -
#pragma mark Interrupts


bool
sdhc_isSignalingInterrupt(const SDHCRegisters_t *sdhc)
{
	return (sdhc->normalInterruptStatus & sdhc->normalInterruptSignalEnable)
		|| (sdhc->errorInterruptStatus  & sdhc->errorInterruptSignalEnable);
}


void
sdhc_clearInterruptSignals(SDHCRegisters_t *sdhc)
{
	UInt16 clear;
	
	sdhc->forceEventAutoCmd12ErrorStatus = 0;
	sdhc->forceEventErrorInterruptStatus = 0;

	clear = sdhc->errorInterruptSignalEnable & sdhc->errorInterruptStatus;

	if(clear & kAutoCmd12ErrorInt) {
		UInt16 cmd12Clear = sdhc->autoCmd12ErrorStatus;
		sdhc->autoCmd12ErrorStatus = cmd12Clear;
	}

	sdhc->errorInterruptStatus = clear;

	clear = sdhc->normalInterruptSignalEnable & sdhc->normalInterruptStatus;
	sdhc->normalInterruptStatus = clear;
}



#pragma mark -
#pragma mark Card Interrupt


bool
sdhc_isCardInterrupt(const SDHCRegisters_t *sdhc)
{
	return sdhc->normalInterruptStatus & kCardInt;
}

void
sdhc_enableCardInterrupt(SDHCRegisters_t *sdhc, bool enable)
{
	if(enable) {
		sdhc->normalInterruptStatusEnable |= kCardInt;
		sdhc->normalInterruptSignalEnable |= kCardInt;
	} else {
		sdhc->normalInterruptStatusEnable &= ~kCardInt;
		sdhc->normalInterruptSignalEnable &= ~kCardInt;
	}
}

bool
sdhc_isCardInterruptEnabled(const SDHCRegisters_t *sdhc)
{
	return sdhc->normalInterruptStatusEnable & kCardInt
		&& sdhc->normalInterruptSignalEnable & kCardInt;
}	



#pragma mark -
#pragma mark Host Control


enum HostControlFieldOffsets
{
	kDataTransferWidthOffset = 1,
	kHighSpeedEnableOffset   = 2,
};

enum HostControlFields
{
	kDataTransferWidth    = (1 << kDataTransferWidthOffset), // 1 == 4 bit, 0 == 1 bit
	kHighSpeedEnable       = (1 << kHighSpeedEnableOffset),
};


void
sdhc_setHighSpeedMode(SDHCRegisters_t *sdhc, bool enabled)
{
	if(enabled) {
		sdhc->hostControl |= kHighSpeedEnable;
	} else {
		sdhc->hostControl &= ~kHighSpeedEnable;
	}
}

void
sdhc_setDataTransferWidth(SDHCRegisters_t *sdhc, UInt8 width)
{
	bool cardIntOn = sdhc_isCardInterruptEnabled(sdhc);

	sdhc_enableCardInterrupt(sdhc, false);

	if(width == 4) {
		sdhc->hostControl |= kDataTransferWidth;
	} else {
		sdhc->hostControl &= ~kDataTransferWidth;
	}

	sdhc_enableCardInterrupt(sdhc, cardIntOn);
}




