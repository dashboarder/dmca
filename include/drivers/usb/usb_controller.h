/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef _USB_CONTROLLER_H_
#define _USB_CONTROLLER_H_

#include <sys/types.h>
#include <drivers/usb/usb_core.h>

__BEGIN_DECLS

#define MAX_SUPPORTED_USB_CONTROLLER	(2)

typedef enum {
	USB_CONTROLLER_synopsys_otg,
	USB_CONTROLLER_dbgfifo,
} USB_CONTROLLER_T;

struct usb_controller_functions {
	int (*init)(void);
	void (*free_func)(void);
	int (*start)(void);
	void (*stop)(void);
	void (*set_address)(uint32_t new_address);
	int (*get_connection_speed)(void);
	void (*activate_endpoint)(uint32_t endpoint, int type, int max_packet_size, int interval);
	void (*do_endpoint_io)(struct usb_device_io_request *req);
	void (*stall_endpoint)(uint32_t endpoint, bool stall);
	void (*reset_endpoint_data_toggle)(uint32_t endpoint);
	bool (*is_endpoint_stalled)(uint32_t endpoint);
	void (*do_test_mode)(uint32_t selector);
	void (*abort_endpoint)(uint32_t endpoint);
	void (*deactivate_endpoint)(uint32_t endpoint);
};

int usb_controller_register(const struct usb_controller_functions *controller_functions);
int usb_controller_init(void);
void usb_controller_free(void);
int usb_controller_start(void);
void usb_controller_stop(void);
void usb_controller_set_address(u_int32_t new_address);
int usb_controller_get_connection_speed(void);
void usb_controller_activate_endpoint(u_int32_t endpoint, int type, int max_packet_size, int interval);
void usb_controller_do_endpoint_io(struct usb_device_io_request *req);
void usb_controller_stall_endpoint(u_int32_t endpoint, bool stall);
void usb_controller_reset_endpoint_data_toggle(u_int32_t endpoint);
bool usb_controller_is_endpoint_stalled(u_int32_t endpoint);
void usb_controller_do_test_mode(u_int32_t selector);
void usb_controller_abort_endpoint(u_int32_t endpoint);
void usb_controller_deactivate_endpoint(u_int32_t endpoint);

__END_DECLS

#endif // _USB_CONTROLLER_H_
