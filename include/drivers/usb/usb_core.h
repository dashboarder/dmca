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

#ifndef _USB_CORE_H_
#define _USB_CORE_H_

#include <sys/types.h>
#include <drivers/usb/usb_chap9.h>

__BEGIN_DECLS

//===================================================================================
//		consts and macros
//===================================================================================

#define APPLE_USB_DFU_INTF		0
#define APPLE_USB_TRNSFR_INTF	        APPLE_USB_DFU_INTF
#define APPLE_USB_SERIAL_INTF		0x1

#define APPLE_USB_SERIAL_CONFIG_STRING_IDX      4
#define APPLE_USB_SERIAL_INTF_STRING_INDEX      5

#define APPLE_USB_SERIAL_EP_BULK_IN     0x81
#define APPLE_USB_SERIAL_EP_BULK_OUT    0x2
#define APPLE_USB_TRNSFR_EP_BULK_OUT	0x4
#define APPLE_USB_TRNSFR_EP_BULK_IN	0x85

#define APPLE_USB_DFU_TOTAL_INTFS	1
#define APPLE_USB_DFU_CLASS		0xfe
#define APPLE_USB_DFU_SUBCLASS		0x1
#define APPLE_USB_DFU_PROTOCOL		0x0

#define APPLE_USB_SERIAL_TOTAL_EPS	2
#define APPLE_USB_SERIAL_TOTAL_INTFS    2
#define APPLE_USB_SERIAL_CLASS		0xff
#define APPLE_USB_SERIAL_SUBCLASS	0xff
#define APPLE_USB_SERIAL_PROTOCOL	0x51

#if WITH_BULK_UPLOAD
#define APPLE_USB_TRNSFR_TOTAL_EPS      2
#else
#define APPLE_USB_TRNSFR_TOTAL_EPS      1
#endif

#define APPLE_USB_TRNSFR_TOTAL_INTFS    1
#define APPLE_USB_DFU_TRNSFR_CLASS      0xfe
#define APPLE_USB_DFU_TRNSFR_SUBCLASS   0x1
#define APPLE_USB_DFU_TRNSFR_PROTOCOL   0x2
#define APPLE_USB_TRNSFR_CLASS		APPLE_USB_DFU_TRNSFR_CLASS
#define APPLE_USB_TRNSFR_SUBCLASS	APPLE_USB_DFU_TRNSFR_SUBCLASS
#define APPLE_USB_TRNSFR_PROTOCOL       APPLE_USB_DFU_TRNSFR_PROTOCOL

//===================================================================================
//		Enums, structs and Typedefs
//===================================================================================

enum
{
	USB_IO_ERROR = -1,
	USB_IO_SUCCESS = 0,
	USB_IO_ABORTED
};

enum
{
	CABLE_CONNECTED = 0,
	CABLE_DISCONNECTED,
	USB_RESET,
	USB_ENUM_DONE
};

enum {
    CONNECTION_SPEED_FULL = 0,
    CONNECTION_SPEED_HIGH
};

#if WITH_RECOVERY_MODE
enum {
	PR_RESET_REQUEST = 0,	// put device into reset state before starting restore with bulk blaster protocol
	PR_VENDOR_REQUEST = 0,
	PR_VENDOR_BLIND_REQUEST // return success on usb before executing the command, primarily for fsboot or reboot
};
#endif

struct usb_device_io_request
{
	u_int32_t                       endpoint;
	volatile u_int8_t               *io_buffer;
	int                             status;
	u_int32_t                       io_length;
	u_int32_t                       return_count;
	void (*callback) (struct usb_device_io_request *io_request);
	struct usb_device_io_request    *next;
};

struct usb_interface_instance
{
    int                                     total_interfaces;
    struct usb_interface_descriptor   *interface_descs;
    int                                     total_other_descs;
    void                              *other_descs;
    int                                     total_endpoints;
    struct usb_endpoint_descriptor    *endpoint_descs;
    int                                     total_string_descs;
    char                              **string_descs;
    
	int (*handle_request) (struct usb_device_request *request, u_int8_t ** out_data);
	void (*non_setup_data_phase_finished_callback) (u_int32_t data_rcvd);
	void (*activate_interface) (void);
	void (*deactivate_interface) (void);
	void (*activate_endpoints) (void);
	void (*deactivate_endpoints) (void);
	void (*handle_bus_reset) (void);
	int (*handle_get_interface) (void);
	int (*handle_set_interface) (int alt_setting);
	int (*handle_vendor_request) (struct usb_device_request *request);
};

//===================================================================================
//			Global APIs
//===================================================================================

int usb_core_init (const char *config_desc_string);
void usb_core_free (void);
int usb_core_start();
void usb_core_stop();
void usb_core_handle_usb_control_receive (u_int8_t *ep0_rx_buffer, bool is_setup, int receive_length, bool *data_phase);
void usb_core_register_interface (struct usb_interface_instance *if_ops);
void usb_core_event_handler (int event);
void usb_core_do_transfer (int endpoint, u_int8_t *buffer, int length, void (*callback)(struct usb_device_io_request *));
void usb_core_send_zlp (void);
void usb_core_complete_endpoint_io (struct usb_device_io_request *req);
void usb_core_activate_endpoint (u_int32_t endpoint, int type, int max_packet_size, int interval);
void usb_core_deactivate_endpoint (u_int32_t endpoint);
void usb_core_abort_endpoint (int endpoint);
bool usb_core_get_connection_speed (void);
bool usb_core_get_cable_state (void);

__END_DECLS

#endif
