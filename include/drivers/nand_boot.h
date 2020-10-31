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

#ifndef _NAND_BOOT_H
#define _NAND_BOOT_H
#if WITH_NAND_BOOT

#include <sys/types.h>

__BEGIN_DECLS

int nand_boot_init(void);

__END_DECLS

#endif /* WITH_NAND_BOOT */
#endif /* ! _NAND_BOOT_H */
