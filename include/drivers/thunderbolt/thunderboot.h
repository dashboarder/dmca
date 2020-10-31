/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef THUNDERBOOT_H
#define THUNDERBOOT_H

#include <drivers/pci.h>

typedef struct thunderboot thunderboot_t;

thunderboot_t * thunderboot_init(pci_device_t bridge, uint32_t dart_id);
void thunderboot_quiesce_and_free(thunderboot_t *tb);

int thunderboot_get_dfu_image(void *load_address, int load_length);

void thunderboot_putchar(int c);

int thunderboot_serial_send_cmd_string(u_int8_t *buffer, u_int32_t len);
void thunderboot_transfer_prepare(void *load_address, uint32_t length);
int thunderboot_transfer_wait(void);

#endif
