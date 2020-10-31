/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_SPI_H
#define __DRIVERS_SPI_H

#include <sys/types.h>

__BEGIN_DECLS

void spi_setup(int port, int baud, int width, bool master, int clkpol, int clkpha);
int  spi_write_etc(int port, const void *buf, size_t len, bool wait, bool with_rx);
int  spi_read_etc(int port, void *buf, size_t len, bool wait, bool with_tx);
void spi_select(int port, bool select_state);

void spi_init(void);

u_int32_t spi_gpio_read(int port);
void spi_gpio_write(int port, u_int32_t val);
void spi_gpio_configure(int port, u_int32_t config);

__END_DECLS

#endif /* __DRIVERS_SPI_H */
