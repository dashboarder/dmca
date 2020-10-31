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

#include "sdhc_debug.h"

#ifdef PLATFORM_VARIANT_IOP
#include <lib/libc.h>
#else
#include <libkern/libkern.h>
#endif


void
sdhc_dumpRegisterFile(const SDHCRegisters_t *sdhc)
{
	printf("SDHC @ %p, size = %lu\n", sdhc, sizeof(*sdhc));
	printf("  SD Command Generation:\n");
	printf("    sdmaSystemAddr  = 0x%08X\n", (uint32_t)sdhc->sdmaSystemAddr);
	printf("    blockSize       = 0x%04X\n", sdhc->blockSize);
	printf("    blockCount      = 0x%04X\n", sdhc->blockCount);
	printf("    argument        = 0x%08X\n", (uint32_t)sdhc->argument);
	printf("    transferMode    = 0x%04X\n", sdhc->transferMode);
	printf("    command         = 0x%04X\n", sdhc->command);

	printf("  Response:\n");
	for(unsigned i=0; i<4; i++) {
		printf("    response[%u]     = 0x%08X\n", i, (uint32_t)sdhc->response[i]);
	}

	printf("  Buffer Data Port:\n");
	printf("    bufferDataPort  = 0x%08X\n", (uint32_t)sdhc->bufferDataPort);

	printf("  Host Controls:\n");
	printf("    presentState    = 0x%08X\n", (uint32_t)sdhc->presentState);
	printf("    hostControl     = 0x%02X\n", sdhc->hostControl);
	printf("    powerControl    = 0x%02X\n", sdhc->powerControl);
	printf("    blockGapControl = 0x%02X\n", sdhc->blockGapControl);
	printf("    wakeupControl   = 0x%02X\n", sdhc->wakeupControl);
	printf("    clockControl    = 0x%04X\n", sdhc->clockControl);
	printf("    timeoutControl  = 0x%02X\n", sdhc->timeoutControl);
	printf("    softwareReset   = 0x%02X\n", sdhc->softwareReset);

	printf("  Interrupt Controls:\n");
	printf("    normalInterruptStatus       = 0x%04X\n", sdhc->normalInterruptStatus);
	printf("    errorInterruptStatus        = 0x%04X\n", sdhc->errorInterruptStatus);
	printf("    normalInterruptStatusEnable = 0x%04X\n", sdhc->normalInterruptStatusEnable);
	printf("    errorInterruptStatusEnable  = 0x%04X\n", sdhc->errorInterruptStatusEnable);
	printf("    normalInterruptSignalEnable = 0x%04X\n", sdhc->normalInterruptSignalEnable);
	printf("    errorInterruptSignalEnable  = 0x%04X\n", sdhc->errorInterruptSignalEnable);
	printf("    autoCmd12ErrorStatus        = 0x%04X\n", sdhc->autoCmd12ErrorStatus);

	printf("  Capabilities:\n");
	printf("    capabilities                = 0x%016llX\n", sdhc->capabilities);
	printf("    maxCurrentCapabilities      = 0x%016llX\n", sdhc->maxCurrentCapabilities);

	printf("  Force Event:\n");
	printf("    forceEventAutoCmd12ErrorStatus = 0x%04X\n", sdhc->forceEventAutoCmd12ErrorStatus);
	printf("    forceEventErrorInterruptStatus = 0x%04X\n", sdhc->forceEventErrorInterruptStatus);

	printf("  ADMA:\n");
	printf("    admaErrorStatus       = 0x%02X\n", sdhc->admaErrorStatus);
	printf("    admaSystemAddr        = 0x%016llX\n", sdhc->admaSystemAddr);

	printf("  Common:\n");
	printf("    slotInterruptStatus   = 0x%04X\n", sdhc->slotInterruptStatus);
	printf("    hostControllerVersion = 0x%04X\n", sdhc->hostControllerVersion);
}

