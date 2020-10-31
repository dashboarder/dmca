/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
 
/////
//
// Whimory Feature Definitions
//
/////

#include "WMRTypes.h"

#ifndef _WMR_FEATURES_H_
#define _WMR_FEATURES_H_

// Define the platform we're on
#define WMR_BUILDING_IBOOT (1)

// Don't bother collect stats in read-only mode
//#define AND_COLLECT_STATISTICS			(1)

// Defined by platform, but they're all OK with 30 for now
#define FIL_MAX_ECC_CORRECTION              (30)

// =============================================================================
// configurable preprocessor compilation control
// =============================================================================

// Set H2FMI_DEBUG below to 1 if you want to build for debugging (default
// to 0).
#define H2FMI_DEBUG  1

// Set H2FMI_TEST_HOOK below to 1 if you want to insert tests at the end
// of FIL_Init in iBoot (default to 0).
#define H2FMI_TEST_HOOK  0

// Set H2FMI_WAIT_USING_ISR to true if you want operations to wait for
// dma and bus events by hooking an interrupt service routine to the
// FMI interrupt vector; set to false for waiting using register
// polling with yield (default to true).
#define H2FMI_WAIT_USING_ISR 1

#define H2FMI_INSTRUMENT_BUS_1 0

// Change the constant below to 1 if you want to see all FMI
// subsystem register writes; note that doing so is likely to change
// timing and therefore behavior, so it should be a rare case that you
// would ever want to do this.  In cases where you want to see just
// one or two writes, consider calling the 'h2fmi_dbg_wr' or
// 'h2fmi_dbg_wr8' function directly (default to 0).
#define H2FMI_TRACE_REG_WRITES  0

// Change the constant below to 1 if you want to see all FMI
// subsystem register reads; note that doing so is likely to change
// timing and therefore behavior, so it should be a rare case that you
// would ever want to do this.  In cases where you want to see just
// one or two reads, consider calling the 'h2fmi_dbg_rd' or
// 'h2fmi_dbg_rd8' function directly (default to 0).
#define H2FMI_TRACE_REG_READS  0

// =============================================================================
// fixed preprocessor compilation control
// =============================================================================

#define H2FMI_BOOTROM 0
#define H2FMI_IOP 1
#define H2FMI_IBOOT 0
#define H2FMI_EFI 0

#define H2FMI_READONLY 0
#endif /* _WMR_FEATURES_H_ */

