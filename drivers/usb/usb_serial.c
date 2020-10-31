/*
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

////////////////////////////////////////////////////////////////////
//			USB Serial
// This file implements usb serial emulation protocol. We have 'usbterm'
// tool on the Mac to enable this.
////////////////////////////////////////////////////////////////////

#include <debug.h>
#include <sys/types.h>
#include <stdlib.h>
#include <drivers/usb/usb_chap9.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usb/usb_core.h>
#include <platform/memmap.h>

#include <sys.h>
#include <sys/task.h>

#include <lib/cbuf.h>
#include <sys/callout.h>

//==============================================================================================
//				Local conts, macors, structs, typedefs
//==============================================================================================
#define WRITE_CBUF_MAX_CAPACITY		(4096)

#define DEBUG 0

#if DEBUG
#define print(fmt, args...) printf("%s --- " fmt, __FUNCTION__, ##args)
#else
#define print(fmt, args...) (void)0
#endif

struct usb_serial_buffer {
	u_int8_t bytes[HS_BULK_EP_MAX_PACKET_SIZE];
};

//==============================================================================================
//				Global and Local variables
//==============================================================================================

static const struct usb_interface_descriptor usb_serial_interface_descriptors[APPLE_USB_SERIAL_TOTAL_INTFS] =
{
    {
        USB_DT_INTERFACE_SIZE,
        USB_DT_INTERFACE,
        APPLE_USB_SERIAL_INTF,
        0,
        0,
        APPLE_USB_SERIAL_CLASS,
        APPLE_USB_SERIAL_SUBCLASS,
        APPLE_USB_SERIAL_PROTOCOL,
        0
    },
    {
        USB_DT_INTERFACE_SIZE,
        USB_DT_INTERFACE,
        APPLE_USB_SERIAL_INTF,
        1,
        APPLE_USB_SERIAL_TOTAL_EPS,
        APPLE_USB_SERIAL_CLASS,
        APPLE_USB_SERIAL_SUBCLASS,
        APPLE_USB_SERIAL_PROTOCOL,
        0
    }
};

static const struct usb_endpoint_descriptor usb_serial_endpoint_descriptors[APPLE_USB_SERIAL_TOTAL_EPS] = {
	{
		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		APPLE_USB_SERIAL_EP_BULK_IN,
		USB_ENDPOINT_BULK,
		HS_BULK_EP_MAX_PACKET_SIZE,
		0
	},
	{
		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		APPLE_USB_SERIAL_EP_BULK_OUT,
		USB_ENDPOINT_BULK,
		HS_BULK_EP_MAX_PACKET_SIZE,
		0
	}
};

static const char *usb_serial_string_descriptors[APPLE_USB_SERIAL_TOTAL_INTFS] =
{
    NULL,
    "Apple USB Serial Interface"
};

static struct usb_interface_instance usb_serial_interface_instance;

static struct usb_serial_buffer *out_buffer;
static struct usb_serial_buffer *in_buffer;
static struct task_event in_buffer_event;

static struct cbuf *usb_serial_output_cbuf;

static bool usb_serial_inited;
static volatile bool usb_serial_activated;

static int current_alt_setting;

//==============================================================================================
//			Functions Prototypes
//==============================================================================================

static int usb_serial_flush_write_cbuf (bool start_transfer, u_int32_t length);
static void usb_serial_transfer_finished_cb (struct usb_device_io_request *io_request);
static void usb_serial_activate_endpoints (void);
static void usb_serial_deactivate_endpoints (void);
static int usb_serial_get_max_packet_size(void);
static int usb_serial_task_entry(void *arg);

//==============================================================================================
//				Local functions
//==============================================================================================

static void usb_serial_activate_interface (void)
{
	usb_serial_activate_endpoints();
	usb_core_do_transfer(APPLE_USB_SERIAL_EP_BULK_OUT, (u_int8_t *)out_buffer, sizeof(*out_buffer), usb_serial_transfer_finished_cb);
	usb_serial_activated = true;
	task_start(task_create("usb_serial", usb_serial_task_entry, NULL, 0x400));
}


static void usb_serial_deactivate_interface (void)
{
	usb_serial_activated = false;
	usb_serial_deactivate_endpoints();
}

static void usb_serial_activate_endpoints (void)
{
	int i;
	
	for(i = 0; i < APPLE_USB_SERIAL_TOTAL_EPS; i++) {
		usb_core_activate_endpoint(usb_serial_endpoint_descriptors[i].bEndpointAddress,
                                   usb_serial_endpoint_descriptors[i].bmAttributes,
                                   usb_serial_get_max_packet_size(),
                                   usb_serial_endpoint_descriptors[i].bInterval);
	}
}

static void usb_serial_deactivate_endpoints (void)
{
	int i;
	
	for(i = 0; i < APPLE_USB_SERIAL_TOTAL_EPS; i++) {
		usb_core_deactivate_endpoint(usb_serial_endpoint_descriptors[i].bEndpointAddress);
	}
}


static void usb_serial_transfer_finished_cb (struct usb_device_io_request *io_request)
{
	print("callback %x\n", io_request->endpoint);
	
	if(io_request->endpoint == APPLE_USB_SERIAL_EP_BULK_OUT) {
		if (io_request->status == USB_IO_SUCCESS) {
			u_int32_t i = 0;
			u_int32_t count;
			
			count = __min(io_request->return_count, sizeof(*out_buffer));
            
			while(i < count) {
				debug_pushchar(out_buffer->bytes[i++]);
			}
		}
		else {
			dprintf(DEBUG_INFO, "failed to read\n");
		}
		
		usb_core_do_transfer(APPLE_USB_SERIAL_EP_BULK_OUT, (u_int8_t *)out_buffer, sizeof(*out_buffer), usb_serial_transfer_finished_cb);
	}
	else if (io_request->endpoint == APPLE_USB_SERIAL_EP_BULK_IN) {
        event_signal(&in_buffer_event);
        
        if (io_request->status != USB_IO_SUCCESS) {
		    dprintf(DEBUG_INFO, "failed to write\n");
        }
	}
}


static int usb_serial_handle_set_interface (int value)
{
	switch(value) {
		case 0 :
			usb_serial_deactivate_interface();
			current_alt_setting = 0;
			break;
			
		case 1 :
			usb_serial_activate_interface();
			current_alt_setting = 1;
			break;
			
		default :
			return -1;
	}
	
	return 0;
}


static int usb_serial_handle_get_interface (void)
{
	return (current_alt_setting);
}

static int usb_serial_task_entry(void *arg __unused)
{
    while (usb_serial_activated) {
        if(usb_serial_flush_write_cbuf(true, sizeof(*in_buffer)) == 0)
        {
            task_sleep(15 * 1000);
        }
    }
    
    return 0;
}

static int usb_serial_flush_write_cbuf (bool start_transfer, u_int32_t length)
{
	u_int32_t i = 0;
	char c = 0;
	u_int32_t max_len = __min(length, sizeof(*in_buffer));
    
	while(false == event_wait_timeout(&in_buffer_event, 5000 * 1000)) {
        if(usb_serial_activated) {
            usb_core_abort_endpoint(APPLE_USB_SERIAL_EP_BULK_IN);
        }
    }
    
	while(i < max_len) {
		if(cbuf_read_char(usb_serial_output_cbuf, &c) == 0) {
			break;
		}
		in_buffer->bytes[i++] = c;
	}
	
	if(i && start_transfer) {
		usb_core_do_transfer(APPLE_USB_SERIAL_EP_BULK_IN, (u_int8_t *)in_buffer, i, usb_serial_transfer_finished_cb);
	}
    else {
        event_signal(&in_buffer_event);
    }
    
    return i;   // Number of characters written to USB Buffer
}


static int usb_serial_get_max_packet_size(void)
{
	int mps;
    
	if(usb_core_get_connection_speed() == CONNECTION_SPEED_HIGH) {
		mps = HS_BULK_EP_MAX_PACKET_SIZE;
	}
	else {
		mps = FS_EP_MAX_PACKET_SIZE;
	}
    
	return mps;
}

//==============================================================================================
//				Global functions
//==============================================================================================

int usb_serial_early_init (void)
{
	in_buffer = memalign(sizeof(*in_buffer), CPU_CACHELINE_SIZE);
	bzero(in_buffer, sizeof(*in_buffer));
    
	usb_serial_output_cbuf = cbuf_create(WRITE_CBUF_MAX_CAPACITY, NULL);
	if(usb_serial_output_cbuf == NULL) {
		usb_serial_inited = false;
		dprintf(DEBUG_INFO, "usb_serial_output_cbuf create failed \n");
		return -1;
	}
	
	event_init(&in_buffer_event, EVENT_FLAG_AUTO_UNSIGNAL, true);
	usb_serial_inited = true;
    
	return 0;
}

int usb_serial_init (void)
{
	if(!usb_serial_inited) {
		dprintf(DEBUG_INFO, "usb_serial_early_init failed \n");
		return -1;
	}
	
	usb_serial_activated = false;
	current_alt_setting = 0;
	
	out_buffer = memalign(sizeof(*out_buffer), CPU_CACHELINE_SIZE);
	if(out_buffer == NULL) {
		dprintf(DEBUG_INFO, "usb_serial:usb_serial_init --- out_buffer memalign failed \n");
		return -1;
	}
	bzero(out_buffer, sizeof(*out_buffer));
	
	usb_serial_interface_instance.total_interfaces = APPLE_USB_SERIAL_TOTAL_INTFS;
	usb_serial_interface_instance.interface_descs = (struct usb_interface_descriptor *)&usb_serial_interface_descriptors;
	usb_serial_interface_instance.total_endpoints = APPLE_USB_SERIAL_TOTAL_EPS;
	usb_serial_interface_instance.endpoint_descs = (struct usb_endpoint_descriptor *)&usb_serial_endpoint_descriptors;
	usb_serial_interface_instance.total_string_descs = APPLE_USB_SERIAL_TOTAL_INTFS;
	usb_serial_interface_instance.string_descs = (char **)usb_serial_string_descriptors;
	usb_serial_interface_instance.deactivate_interface = usb_serial_deactivate_interface;
	usb_serial_interface_instance.handle_set_interface = usb_serial_handle_set_interface;
	usb_serial_interface_instance.handle_get_interface = usb_serial_handle_get_interface;
	
	usb_core_register_interface(&usb_serial_interface_instance);
	
	return 0;
}

void usb_serial_putchar(int c)
{
	int ret = 0;
	
	if((ret = cbuf_write_char(usb_serial_output_cbuf, c)) == 0) {
		bool start_transfer = false;
		u_int32_t len = 1;
		
		if(usb_serial_activated) {
			len = usb_serial_get_max_packet_size();
			start_transfer = true;
		}
		
		usb_serial_flush_write_cbuf(start_transfer, len);
		
		cbuf_write_char(usb_serial_output_cbuf, c);
	}
}

void usb_serial_exit (void)
{
	if(usb_serial_output_cbuf) {
		cbuf_destroy(usb_serial_output_cbuf);
		usb_serial_output_cbuf = NULL;
	}
	
	if(in_buffer) {
		free(in_buffer);
		in_buffer = NULL;
	}
	
	if(out_buffer) {
		free(out_buffer);
		out_buffer = NULL;
	}
	
	usb_serial_inited = false;
}

int usb_serial_send_cmd_string(u_int8_t *buffer, u_int32_t len)
{
	if(!usb_serial_activated) {
        return -1;
	}
	
    usb_core_do_transfer(APPLE_USB_SERIAL_EP_BULK_IN, buffer, len, NULL);
    
    return 0;
}

bool usb_serial_is_active()
{
    return ((current_alt_setting == 1) ? true : false);
}
