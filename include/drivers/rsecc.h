/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_RSECC_H
#define __DRIVERS_RSECC_H

#include <sys/types.h>

__BEGIN_DECLS

enum rsecc_mode {
	ECC_4BIT = 0,
	ECC_6BIT,
	ECC_8BIT,
};

int rsecc_encode(enum rsecc_mode mode, int sector_count, const void *datapointer, void *eccbuf);
int rsecc_decode(enum rsecc_mode mode, int sector_count, void *datapointer, const void *eccbuf);

__END_DECLS

#endif /* __DRIVERS_RSECC_H */


