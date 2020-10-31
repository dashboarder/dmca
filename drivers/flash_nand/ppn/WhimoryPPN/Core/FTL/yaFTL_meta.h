/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */


#ifndef __YAFTL_META_H__
#define __YAFTL_META_H__

#include "yaFTL_whoami.h"
#include "WMROAM.h"

#if NAND_PPN
#include "yaFTL_meta_ppn.h"
#else
// Default to raw to avoid compilation errors--meta isn't needed by some of the parsers that include yaFTLTypes.h
#include "yaFTL_meta_raw.h"
#endif

#endif // __YAFTL_META_H__

