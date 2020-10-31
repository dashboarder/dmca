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


#ifndef _SDIO_CMDPROP_H
#define _SDIO_CMDPROP_H


#ifdef PLATFORM_VARIANT_IOP
#include <sys/types.h>
#else
#include <IOKit/IOTypes.h>
#endif


#ifdef __cplusplus
extern "C"  {
#endif


/** @brief true if the command has an associated data transfer. */
bool sdio_isDataPresent(UInt16 cmdIndex);

/** @brief Generates the value for the command register. */
UInt16 sdio_generateCommand(UInt16 cmdIndex);


	
#ifdef __cplusplus
}
#endif

#endif /* _SDIO_CMDPROP_H */

