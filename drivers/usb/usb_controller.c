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

#include <debug.h>
#include <drivers/usb/usb_controller.h>

static struct usb_controller_functions usb_controller_funcs;
static bool usb_controller_registered;

int usb_controller_register(const struct usb_controller_functions *controller_functions)
{
	dprintf(DEBUG_INFO, "usb_controller_register, %p\n", controller_functions);
	if (controller_functions != NULL) {
		bcopy((const void *)controller_functions, (void *)&usb_controller_funcs, sizeof(struct usb_controller_functions));
		usb_controller_registered = true;
	}
	else {
		bzero((void *)&usb_controller_funcs, sizeof(struct usb_controller_functions));
		usb_controller_registered = false;
	}

	return 0;
}

int usb_controller_init(void)
{
	RELEASE_ASSERT(usb_controller_funcs.init != NULL);
	return usb_controller_funcs.init();
}

void usb_controller_free(void)
{
	RELEASE_ASSERT(usb_controller_funcs.free_func != NULL);
	usb_controller_funcs.free_func();
}

int usb_controller_start(void)
{
	RELEASE_ASSERT(usb_controller_funcs.start != NULL);
	return usb_controller_funcs.start();
}

void usb_controller_stop(void)
{
	RELEASE_ASSERT(usb_controller_funcs.stop != NULL);
	usb_controller_funcs.stop();
}

void usb_controller_set_address(u_int32_t new_address)
{
	RELEASE_ASSERT(usb_controller_funcs.set_address != NULL);
	usb_controller_funcs.set_address(new_address);
}

int usb_controller_get_connection_speed(void)
{
	RELEASE_ASSERT(usb_controller_funcs.get_connection_speed != NULL);
	return usb_controller_funcs.get_connection_speed();
}

void usb_controller_activate_endpoint(u_int32_t endpoint, int type, int max_packet_size, int interval)
{
	RELEASE_ASSERT(usb_controller_funcs.activate_endpoint != NULL);
	usb_controller_funcs.activate_endpoint(endpoint, type, max_packet_size, interval);
}

void usb_controller_do_endpoint_io(struct usb_device_io_request *req)
{
	RELEASE_ASSERT(usb_controller_funcs.do_endpoint_io != NULL);
	usb_controller_funcs.do_endpoint_io(req);
}

void usb_controller_stall_endpoint(u_int32_t endpoint, bool stall)
{
	RELEASE_ASSERT(usb_controller_funcs.stall_endpoint != NULL);
	usb_controller_funcs.stall_endpoint(endpoint, stall);
}

void usb_controller_reset_endpoint_data_toggle(u_int32_t endpoint)
{
	RELEASE_ASSERT(usb_controller_funcs.reset_endpoint_data_toggle != NULL);
	usb_controller_funcs.reset_endpoint_data_toggle(endpoint);
}

bool usb_controller_is_endpoint_stalled(u_int32_t endpoint)
{
	RELEASE_ASSERT(usb_controller_funcs.is_endpoint_stalled != NULL);
	return usb_controller_funcs.is_endpoint_stalled(endpoint);
}

void usb_controller_do_test_mode(u_int32_t selector)
{	
	RELEASE_ASSERT(usb_controller_funcs.do_test_mode != NULL);
	usb_controller_funcs.do_test_mode(selector);
}

void usb_controller_abort_endpoint(u_int32_t endpoint)
{
	RELEASE_ASSERT(usb_controller_funcs.abort_endpoint != NULL);
	usb_controller_funcs.abort_endpoint(endpoint);
}

void usb_controller_deactivate_endpoint(u_int32_t endpoint)
{
	RELEASE_ASSERT(usb_controller_funcs.deactivate_endpoint != NULL);
	usb_controller_funcs.deactivate_endpoint(endpoint);
}
