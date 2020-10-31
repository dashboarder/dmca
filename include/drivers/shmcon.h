/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_SHMCON_H
#define __DRIVERS_SHMCON_H

#include <sys/types.h>

/*
 * High-level shared memory console interface.
 * Port argument exists for compatibility with UART functions
 */
int shmcon_init(void);

int shmcon_getc(int port, bool wait);
int shmcon_putc(int port, char c);
int shmcon_puts(int port, const char *s);

int shmcon_set_child(uint64_t phys_address, uint32_t num);

#endif /* __DRIVERS_SHMCON_H */
