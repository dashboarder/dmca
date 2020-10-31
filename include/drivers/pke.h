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

#ifndef __DRIVERS_PKE_H
#define __DRIVERS_PKE_H

#include <sys/types.h>

__BEGIN_DECLS

bool pke_do_exp(uint8_t *dst, size_t *len,
        size_t  uKeyLen, 
        uint8_t * const base, size_t base_length,
        uint8_t * const expn, size_t expn_length,
        uint8_t * const mods, size_t mods_length);

__END_DECLS

#endif /* __DRIVERS_PKE_H */
