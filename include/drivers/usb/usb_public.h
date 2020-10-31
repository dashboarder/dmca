/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef _USB_PUBLIC_H_
#define _USB_PUBLIC_H_

#include <drivers/usb/usb_controller.h>

__BEGIN_DECLS

enum {
    USB_TRANSFER_TYPE_DFU_GET,
    USB_TRANSFER_TYPE_DFU_PUT,
    USB_TRANSFER_TYPE_BULK_GET,
    USB_TRANSFER_TYPE_BULK_PUT
};

//===================================================================================
//		Global APIs
//===================================================================================

int usb_early_init();
int usb_init_with_controller(struct usb_controller_functions *controller_funcs);
int usb_init();
void usb_quiesce(void);

int getDFUImage(void* buffer, int len);
int usb_dfu_init(void);
void usb_dfu_exit(void);

int usb_transfer_init(void);
void usb_transfer_exit(void);
void usb_transfer_prepare(int type, void *address, u_int32_t maxlen);
u_int32_t usb_transfer_start();

int usb_serial_early_init(void);
int usb_serial_init(void);
void usb_serial_exit(void);
void usb_serial_putchar(int);
int usb_serial_send_cmd_string(u_int8_t *buffer, u_int32_t len);
bool usb_serial_is_active();

__END_DECLS

#endif
