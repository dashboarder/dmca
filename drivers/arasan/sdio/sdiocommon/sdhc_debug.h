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


#ifndef _SDHC_DEBUG_H
#define _SDHC_DEBUG_H


#ifdef PLATFORM_VARIANT_IOP
#include <sdiocommon/sdhc_registers.h>
#else
#include <IOKit/sdio/sdhc_registers.h>
#endif


#ifdef __cplusplus
extern "C"  {
#endif


void sdhc_dumpRegisterFile(const SDHCRegisters_t *sdhc);


#ifdef __cplusplus
}
#endif

#endif /* _SDHC_DEBUG_H */
