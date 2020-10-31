/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_IIC_H
#define __DRIVERS_IIC_H

#include <sys/types.h>

__BEGIN_DECLS

typedef enum {
	IIC_NORMAL = 1,
	IIC_COMBINED = 2,
} iic_fmt_t;

void iic_init(void);
void iic_set_filter(int iic, uint32_t value);
void iic_set_frequency(int iic, u_int32_t frequency);
int iic_read(int iic, uint8_t address, const void *send_data, size_t send_len, void *data, size_t len, iic_fmt_t fmt);
int iic_write(int iic, uint8_t address, const void *data, size_t len);

bool iic_probe(int iic, uint8_t address);

__END_DECLS

#endif /* __DRIVERS_IIC_H */
