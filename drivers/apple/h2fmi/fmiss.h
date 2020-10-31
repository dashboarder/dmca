// *****************************************************************************
//
// File: fmiss.h
//
// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Inc.
//
// *****************************************************************************
#ifndef _FMISS_H_
#define _FMISS_H_

#include "H2fmi_private.h"

#if FMISS_ENABLED

#if H2FMI_IOP
#define FMISS_DUMP_ENABLED (1)
#endif // H2FMI_IOP


// Common

#if FMISS_DUMP_ENABLED
void fmiss_dump(h2fmi_t *fmi);
#endif // FMISS_DUMP_ENABLED

// Raw

UInt32 *fmiss_raw_macros(UInt32 *count);

// PPN
//
UInt32 *fmiss_ppn_macros(UInt32 *count);

#endif // FMISS_ENABLED
#endif // _FMISS_H_
