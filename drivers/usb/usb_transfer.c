/*
 * Copyright (C) 2010-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

////////////////////////////////////////////////////////////////////
//	      USB Bulk Upload + File transfer
//
// This file implements USB Bulk protocol for file uploads;
// and it also implements vendor specific file transfer
// protocol using a bulk pipe. 
///////////////////////////////////////////////////////////////////

#include <debug.h>
#include <sys/task.h>
#include <lib/profile.h>
#include <lib/env.h>
#include <sys/security.h>
#include <platform/memmap.h>
#include <drivers/usb/usb_chap9.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usb/usb_core.h>

#define DEBUG 0

#if DEBUG
#define print(fmt, args...) printf("%s - " fmt, __FUNCTION__, ##args)
#else
#define print(fmt, args...) (void)0
#endif

#ifdef SUPPORT_FPGA
#define TIMEOUT_SCALER	(10)
#else
#define TIMEOUT_SCALER	(1)
#endif

#define BULK_HS_TRNSFR_TIMEOUT    (15 * 1000 * 1000 * TIMEOUT_SCALER)
#define BULK_FS_TRNSFR_TIMEOUT    (30 * 1000 * 1000 * TIMEOUT_SCALER)

#define ERR_PACKET_LEN      (4)
#define ERR_PKT_START       (0xfb)
#define ERR_PKT_END         (0xfa)

// File transfer error codes
#define ERR_FILE_SUCCESS			0
#define ERR_FILE_UNKNOWN_ERROR		1
#define ERR_FILE_UNEXPECTED_ERROR	2
#define ERR_FILE_NOT_FOUND			3
#define ERR_FILE_READ_ERROR			4
#define ERR_FILE_WRITE_ERROR		5

// Personalization related file transfer error codes
#define ERR_PFILE_NOT_IM4P			11
#define ERR_PFILE_CREATE_FAILED		12
#define ERR_PFILE_READ_ERROR		13

static const struct usb_interface_descriptor usb_trnsfr_interface_descriptor = {
	USB_DT_INTERFACE_SIZE,
	USB_DT_INTERFACE,
	APPLE_USB_TRNSFR_INTF,
	0,
	APPLE_USB_TRNSFR_TOTAL_EPS,
	APPLE_USB_TRNSFR_CLASS,
	APPLE_USB_TRNSFR_SUBCLASS,
	APPLE_USB_TRNSFR_PROTOCOL,
	0
};

static const struct usb_endpoint_descriptor usb_trnsfr_endpoint_descriptors [] = {
	{
		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		APPLE_USB_TRNSFR_EP_BULK_OUT,
		USB_ENDPOINT_BULK,
		HS_BULK_EP_MAX_PACKET_SIZE,
		0
	}
#if WITH_BULK_UPLOAD
	,
	{
		USB_DT_ENDPOINT_SIZE,
		USB_DT_ENDPOINT,
		APPLE_USB_TRNSFR_EP_BULK_IN,
		USB_ENDPOINT_BULK,
		HS_BULK_EP_MAX_PACKET_SIZE,
		0
	}
#endif
};

static struct usb_interface_instance 		usb_trnsfr_interface_instance;
static struct task_event        		trnsfr_event;
static volatile bool            		trnsfr_done;
static int                      		completion_status;
static u_int8_t                 		*image_buffer;
static u_int32_t                		image_buffer_size;
static struct callout           		trnsfr_callout;
static u_int32_t                		max_transfer_len;

#if WITH_BULK_UPLOAD
static u_int8_t __aligned(CPU_CACHELINE_SIZE) 	bulk_in_zlp_buffer[sizeof(u_int32_t)];
static u_int8_t __aligned(CPU_CACHELINE_SIZE) 	bulk_upload_status_buffer[ERR_PACKET_LEN];
static void bulk_in_callback(struct usb_device_io_request *io_request);
static void bulk_upload_status_callback(struct usb_device_io_request *io_request);
#endif // WITH_BULK_UPLOAD

static void activate_interface(void);
static void deactivate_interface(void);
static void bulk_transfer_callback(struct usb_device_io_request *io_request);
static void handle_trnsfr_callout(struct callout *co, void *args);
static int handle_vendor_request(struct usb_device_request *request);
static void transfer_failed();

int usb_transfer_init()
{	
	completion_status = -1;
	image_buffer = NULL;
	image_buffer_size = 0;
	trnsfr_done = false;
	
	event_init(&trnsfr_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
    
	usb_trnsfr_interface_instance.total_interfaces = APPLE_USB_TRNSFR_TOTAL_INTFS;
	usb_trnsfr_interface_instance.interface_descs = (struct usb_interface_descriptor *)&usb_trnsfr_interface_descriptor;
	usb_trnsfr_interface_instance.total_endpoints = APPLE_USB_TRNSFR_TOTAL_EPS;
	usb_trnsfr_interface_instance.endpoint_descs = (struct usb_endpoint_descriptor *)&usb_trnsfr_endpoint_descriptors;
	usb_trnsfr_interface_instance.deactivate_interface = deactivate_interface;
	usb_trnsfr_interface_instance.activate_interface = activate_interface;
#if WITH_RECOVERY_MODE
	usb_trnsfr_interface_instance.handle_vendor_request = handle_vendor_request;
#endif	// WITH_RECOVERY_MODE
    
    usb_core_register_interface(&usb_trnsfr_interface_instance);
    
    return 0;
}

void usb_transfer_exit()
{
}

void usb_transfer_prepare(int type, void *address, u_int32_t length)
{
    u_int32_t timeout;
    
    print("\n");
    
    timeout = 0;
    
	completion_status = -1;
	image_buffer = address;
	image_buffer_size = length;
	trnsfr_done = false;
    
#if WITH_ENV
	env_set_uint("filesize", 0, 0);
#endif // WITH_ENV
    
    if(type == USB_TRANSFER_TYPE_BULK_GET) {
        
        max_transfer_len = DEFAULT_RAMDISK_SIZE;
        
        if(!security_allow_memory((u_int8_t *)image_buffer, max_transfer_len)) {
            dprintf(DEBUG_INFO, "Permission denied\n");
            return;
        }
        
		if(usb_core_get_connection_speed() == CONNECTION_SPEED_HIGH)
            timeout = BULK_HS_TRNSFR_TIMEOUT;
		else
			timeout = BULK_FS_TRNSFR_TIMEOUT;
        
        usb_core_abort_endpoint(APPLE_USB_TRNSFR_EP_BULK_OUT);
        
        usb_core_do_transfer(APPLE_USB_TRNSFR_EP_BULK_OUT, image_buffer, max_transfer_len,
                             bulk_transfer_callback);
    }
#if WITH_BULK_UPLOAD
	else if(type == USB_TRANSFER_TYPE_BULK_PUT) {

		if(!security_allow_memory((u_int8_t *)image_buffer, image_buffer_size)) {
			dprintf(DEBUG_INFO, "Permission denied\n");
			return;
		}
        		
		timeout = (usb_core_get_connection_speed() == CONNECTION_SPEED_HIGH) ? BULK_HS_TRNSFR_TIMEOUT : BULK_FS_TRNSFR_TIMEOUT;
		usb_core_abort_endpoint(APPLE_USB_TRNSFR_EP_BULK_OUT);
		usb_core_abort_endpoint(APPLE_USB_TRNSFR_EP_BULK_IN);
		
		// Queue BULK IN request to upload data to host
		usb_core_do_transfer(APPLE_USB_TRNSFR_EP_BULK_IN, image_buffer, image_buffer_size, bulk_in_callback);
		
		// Keep an active BULK OUT request so that host can communcate failure at any time or communicate success at the end.
		// There is no timeout for this request
		usb_core_do_transfer(APPLE_USB_TRNSFR_EP_BULK_OUT, bulk_upload_status_buffer, ERR_PACKET_LEN, bulk_upload_status_callback);
	}
#endif // WITH_BULK_UPLOAD
    
    callout_enqueue(&trnsfr_callout, timeout, handle_trnsfr_callout, NULL);
}

u_int32_t usb_transfer_start()
{
    print("starting\n");
    
	while(!trnsfr_done) event_wait(&trnsfr_event);
#if DEBUG_BUILD
	dprintf(DEBUG_SPEW, "transfer complete, status=%d\n", completion_status);
#else
	dprintf(DEBUG_INFO, "transfer complete, status=%d\n", completion_status);
#endif

	return completion_status;
}

static void deactivate_interface(void)
{
    usb_core_deactivate_endpoint(APPLE_USB_TRNSFR_EP_BULK_OUT);
#if WITH_BULK_UPLOAD
    usb_core_deactivate_endpoint(APPLE_USB_TRNSFR_EP_BULK_IN);
#endif
}

static void activate_interface(void)
{
    int mps;
    
    mps = (usb_core_get_connection_speed() == CONNECTION_SPEED_HIGH) ? HS_BULK_EP_MAX_PACKET_SIZE : FS_EP_MAX_PACKET_SIZE;
    usb_core_activate_endpoint(APPLE_USB_TRNSFR_EP_BULK_OUT, USB_ENDPOINT_BULK, mps, 0);
    
#if WITH_BULK_UPLOAD
    usb_core_activate_endpoint(APPLE_USB_TRNSFR_EP_BULK_IN, USB_ENDPOINT_BULK, mps, 0);
#endif
}

static void handle_trnsfr_callout(struct callout *co, void *args)
{
    print("\n");
    
	transfer_failed();
}

#if WITH_RECOVERY_MODE
static int handle_vendor_request(struct usb_device_request *request)
{
    int ret;
    
    ret = -1;
    
    print("req: %d\n", request->wValue);
    
	if(request->wValue == PR_RESET_REQUEST) {
		PROFILE_1('UHVR');
		usb_transfer_prepare(USB_TRANSFER_TYPE_BULK_GET, (void *)DEFAULT_LOAD_ADDRESS, DEFAULT_RAMDISK_SIZE);
		ret = 0;
	}
	
	return ret;
}
#endif // WITH_RECOVERY_MODE

static void transfer_failed()
{
    print("\n");
    
	callout_dequeue(&trnsfr_callout);
	completion_status = -1;
#if WITH_ENV
	env_set_uint("filesize", 0, 0);
#endif // WITH_ENV
	trnsfr_done = true;
	event_signal(&trnsfr_event);
}

static void bulk_transfer_callback(struct usb_device_io_request *io_request)
{
    print("status: %d, count: %d\n", io_request->status, io_request->return_count);
    
	if(io_request->status != USB_IO_SUCCESS) {
		transfer_failed();
		return;
	}
    
	// special case, Error packet handling.
	// this length and pattern should not appear in
	// normal case, if someone will hit this often
	// will make a change.
	if(io_request->return_count  == ERR_PACKET_LEN) {
		u_int8_t *buf = (u_int8_t *)image_buffer;
		
		if((buf[0] == ERR_PKT_START) && (buf[3] == ERR_PKT_END)) {
			switch(buf[2])
			{
				case ERR_FILE_UNEXPECTED_ERROR:
					dprintf(DEBUG_CRITICAL, "Unexpected error, exiting\n");
					break;
				case ERR_FILE_NOT_FOUND:
					dprintf(DEBUG_CRITICAL, "File not found or could not be accessed, exiting\n");
					break;
				case ERR_FILE_READ_ERROR:
					dprintf(DEBUG_CRITICAL, "File read error, exiting\n");
					break;
				case ERR_PFILE_NOT_IM4P:
					dprintf(DEBUG_CRITICAL, "Not an im4p file, exiting\n");
					break;
				case ERR_PFILE_CREATE_FAILED:
					dprintf(DEBUG_CRITICAL, "Could not personalize file, exiting\n");
					break;
				case ERR_PFILE_READ_ERROR:
					dprintf(DEBUG_CRITICAL, "Error reading personalized file, exiting\n");
					break;
				case ERR_FILE_UNKNOWN_ERROR:
				default:
					dprintf(DEBUG_CRITICAL, "Unknown error - %d, exiting\n", buf[2]);
					break;
			}
            
            transfer_failed();
            
            return;
	  	}
	}
	
#if WITH_ENV
	env_set_uint("filesize", io_request->return_count, 0);
#endif // WITH_ENV
	completion_status = io_request->return_count;
	callout_dequeue(&trnsfr_callout);
	trnsfr_done = true;
	event_signal(&trnsfr_event);
}

#ifdef WITH_BULK_UPLOAD
static void bulk_in_callback(struct usb_device_io_request *io_request)
{
	int mps;
	int amount;
	
	amount = io_request->return_count;
	print("status: %d, count: %d\n", io_request->status, amount);
	
	if(io_request->status == USB_IO_SUCCESS) {
		completion_status = amount;
		
		mps = (usb_core_get_connection_speed() == CONNECTION_SPEED_HIGH) ? HS_BULK_EP_MAX_PACKET_SIZE : FS_EP_MAX_PACKET_SIZE;
		if((amount > 0) && ((amount & (mps - 1)) == 0))
		{
			// Send an empty packet to indicate end of transfer
			usb_core_do_transfer(APPLE_USB_TRNSFR_EP_BULK_IN, bulk_in_zlp_buffer, 0, NULL);
		}
        
		// Restart the timer waiting for the status packet
		callout_reset(&trnsfr_callout, 0);
	}
	else {
		completion_status = -1;
	}
}
#endif // WITH_BULK_UPLOAD

#ifdef WITH_BULK_UPLOAD
static void bulk_upload_status_callback(struct usb_device_io_request *io_request)
{
	bool upload_success = false;
	
	print("status: %d, count: %d\n", io_request->status, io_request->return_count);
	if(io_request->status == USB_IO_SUCCESS) {
		if(io_request->return_count == ERR_PACKET_LEN) {
			if((bulk_upload_status_buffer[0] == ERR_PKT_START) && (bulk_upload_status_buffer[3] == ERR_PKT_END)) {
				switch(bulk_upload_status_buffer[2]) {
					case ERR_FILE_SUCCESS:
						upload_success = true;
						break;
					case ERR_FILE_UNEXPECTED_ERROR:
						printf("Unexpected error, exiting\n");
						break;
					case ERR_FILE_NOT_FOUND:
						printf("File could not be opened, exiting\n");
						break;
					case ERR_FILE_WRITE_ERROR:
						printf("File write error, exiting\n");
						break;
					case ERR_FILE_UNKNOWN_ERROR:
					default:
						printf("Unknown error - %d, exiting\n", bulk_upload_status_buffer[2]);
						break;
				}
			}
			else {
				dprintf(DEBUG_INFO, "Invalid format for status packet\n");
			}
		}
		else {
			dprintf(DEBUG_INFO, "Invalid status packet length - %d\n", io_request->return_count);
		}
	}
	else {
		dprintf(DEBUG_INFO, "Status packet failed %x\n", io_request->status);
	}
	
	if(!upload_success) {
		usb_core_abort_endpoint(APPLE_USB_TRNSFR_EP_BULK_IN);
	}
	
	trnsfr_done = true;
	callout_dequeue(&trnsfr_callout);
	event_signal(&trnsfr_event);
}
#endif // WITH_BULK_UPLOAD
