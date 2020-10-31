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
//			USB DFU
//
// This file implements usb dfu protocol (just download). This is used
// in SecureROM.
////////////////////////////////////////////////////////////////////

#include <debug.h>
#include <sys/task.h>
#include <sys/security.h>
#include <drivers/usb/usb_chap9.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usb/usb_core.h>
#include <platform.h>

#define DEBUG 0
#if DEBUG
#define print(fmt, args...) printf("%s --- " fmt, __FUNCTION__, ##args)
#else
#define print(fmt, args...) (void)0
#endif


struct usb_dfu_run_time_descriptor {
	uint8_t length;
	uint8_t type;
	uint8_t bmAttributes;
	uint16_t wDetachTimeOut;
	uint16_t wTransferSize;
} __attribute__((packed));

struct usb_dfu_status_request
{
	UInt8 bStatus;
	UInt8 bwPollTimeout[3];
	UInt8 bState;
	UInt8 iString;
} __attribute__((packed));

enum {
    DFU_DETACH = 0,
    DFU_DNLOAD,
    DFU_UPLOAD,
    DFU_GETSTATUS,
    DFU_CLR_STATUS,
    DFU_GETSTATE,
    DFU_ABORT
};

enum {
    appIDLE,
    appDETACH,
    dfuIDLE,
    dfuDNLOAD_SYNC,
    dfuDNBUSY,
    dfuDNLOAD_IDLE,
    dfuMANIFEST_SYNC,
    dfuMANIFEST,
    dfuMANIFEST_WAIT_RESET,
    dfuUPLOAD_IDLE,
    dfuERROR
};

enum {
    errOK = 0,
    errADDRESS = 0x08,
    errNOTDONE = 0x09,
	errSTALLEDPKT = 0x0f
};

#define DD_DFU                          (0x21)
#define USB_DFU_RUN_TIME_DT_SIZE        (7)
#define DFU_MAX_TRANSFER_SIZE           (2 * 1024)
#define DETACH_TIMEOUT_MS               (10)
#define DFU_FILE_SUFFIX_LENGTH		(16)

//the polling delay we generally report in GETSTATUS requests
#define STANDARD_DELAY_MS       50

//the delay we report after completing the file download. I don't think it needs to be different
//from the standard delay but this is what m68 h1 rom does.
#define DL_DONE_DELAY_MS        3000

#define DFU_BMATTRIBUTES        (1 << 0) // download only

struct usb_dfu_io_buffer {
	u_int8_t	bytes[DFU_MAX_TRANSFER_SIZE];
};

static const struct usb_interface_descriptor usb_dfu_interface_descriptor = {
	USB_DT_INTERFACE_SIZE,
	USB_DT_INTERFACE,
	APPLE_USB_DFU_INTF,
	0,
	0,
	APPLE_USB_DFU_CLASS,
	APPLE_USB_DFU_SUBCLASS,
	APPLE_USB_DFU_PROTOCOL,
	0
};

static const struct usb_dfu_run_time_descriptor usb_dfu_run_time_desc = {
    USB_DFU_RUN_TIME_DT_SIZE,
    DD_DFU,
    DFU_BMATTRIBUTES,
    DETACH_TIMEOUT_MS,
    DFU_MAX_TRANSFER_SIZE
};

static bool 					usb_dfu_inited;
static struct usb_interface_instance 		usb_dfu_interface_instance;
static struct usb_dfu_status_request 		status_req;
static struct task_event        		dfu_event;
static volatile uint8_t           		dfu_state;
static volatile bool            		dfu_done;
static struct usb_dfu_io_buffer 		*io_buffer;
static int                      		completion_status;
static u_int8_t                 		*image_buffer;
static u_int32_t                		image_buffer_size;
static u_int32_t                		total_received;
static u_int32_t                		expecting;

static void set_status(u_int8_t bStatus, int bwPollTimeout, u_int8_t bState, u_int8_t iString);
static int handle_interface_request(struct usb_device_request *request, uint8_t **out_buffer);
static int handle_dfu_request(struct usb_device_request *setup, uint8_t **buffer);
static void handle_bus_reset(void);
static void data_received(u_int32_t received);


int getDFUImage(void* buffer, int maxlen)
{
	dprintf(DEBUG_INFO,  "Trying to get a file with DFU.\n" );
    
    // save the info provided by our caller
	image_buffer = buffer;
	image_buffer_size = maxlen;
	
	print( "Initializing USB...\n" );
#if WITH_PLATFORM_INIT_USB
	if(platform_init_usb() != 0)
		goto exit;
#else
	if(usb_init() != 0)
		goto exit;
#endif
    
    // wait here till dfu download is finished
	while(!dfu_done) event_wait(&dfu_event);
	dprintf(DEBUG_INFO, "DFU complete, status=%d\n", completion_status);
    
exit:
	print( "Exiting USB...\n" );
	usb_quiesce();
    
	return completion_status;
}

int usb_dfu_init()
{
	if(usb_dfu_inited) return 0;
    
	io_buffer = memalign(sizeof(*io_buffer), CPU_CACHELINE_SIZE);
	bzero((void *)io_buffer, sizeof(*io_buffer));
	
	set_status(errOK, STANDARD_DELAY_MS, dfuIDLE, 0);
	
	completion_status = -1;
	total_received = 0;
	dfu_done = false;
	
	event_init(&dfu_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	
	usb_dfu_interface_instance.total_interfaces = APPLE_USB_DFU_TOTAL_INTFS;
	usb_dfu_interface_instance.interface_descs = (struct usb_interface_descriptor *)&usb_dfu_interface_descriptor;
	usb_dfu_interface_instance.total_other_descs = 1;
	usb_dfu_interface_instance.other_descs = (void *)&usb_dfu_run_time_desc;
	usb_dfu_interface_instance.handle_request = handle_interface_request;
	usb_dfu_interface_instance.non_setup_data_phase_finished_callback = data_received;
	usb_dfu_interface_instance.handle_bus_reset = handle_bus_reset;
    
    usb_core_register_interface(&usb_dfu_interface_instance);
    
	usb_dfu_inited = true;
    
    return 0;
}

void usb_dfu_exit()
{
	if(!usb_dfu_inited) return;
    
	bzero((void*)&usb_dfu_interface_instance, sizeof(struct usb_interface_instance));
    
	if(io_buffer) {
		free(io_buffer);
		io_buffer = NULL;
	}
    
	usb_dfu_inited = false;
}


static void set_status(u_int8_t bStatus, int bwPollTimeout, u_int8_t bState, u_int8_t iString)
{
	status_req.bStatus = bStatus;
	status_req.bwPollTimeout[0]= bwPollTimeout & 0xff;
	status_req.bwPollTimeout[1]= (bwPollTimeout >> 8) & 0xff;
	status_req.bwPollTimeout[2]= (bwPollTimeout >> 16) & 0xff;
	dfu_state = status_req.bState = bState;
	status_req.iString = iString;
}

static int handle_interface_request(struct usb_device_request *request, uint8_t **out_buffer)
{
	u_int8_t bRequest = request->bRequest;
	u_int16_t wLength = request->wLength;
	int ret;
	
    print("type: %d\n", (request->bmRequestType & USB_REQ_DIRECTION_MASK));
	print("request=%02x wValue=%04x index=%d length=%d\n", bRequest,
          request->wValue, request->wIndex, wLength);
    
    ret = -1;
    
	// DFU_GETSTATE and DFU_GETSTATUS request are DEVICE2HOST
	// request. Response for these requests are copied into
	// io_buffer.
	// DFU_DNLOAD request is HOST2DEVICE type request. We pass our
	// io_buffer to the stack to copy the data from the host.
	//
	// io_buffer is gauranteed to be larger than any data needs to
	// be copied into it.
	if((request->bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_HOST2DEVICE)
	{
		if(out_buffer == NULL)
			return -1;
        
		switch(bRequest)
		{
			case DFU_DNLOAD:
			{
				if(wLength == 0) {
					print("File transfer complete. Waiting for status sync.\n");
					set_status(errOK, STANDARD_DELAY_MS, dfuMANIFEST_SYNC, 0);
				}
				else {
					if(wLength > sizeof(*io_buffer)) {
						// cleanest way to recover is to stall here
						set_status(errSTALLEDPKT, STANDARD_DELAY_MS, dfuIDLE, 0);
						return -1;
					}
					
					*out_buffer = (uint8_t *)io_buffer;
				}
				expecting = wLength;
				ret = wLength;
				break;
			}
			case DFU_CLR_STATUS :
			case DFU_ABORT:
			{
				set_status(errOK, STANDARD_DELAY_MS, dfuIDLE, 0);
                
				total_received = 0;
                
				if(!dfu_done) {
					completion_status = -1;
					dfu_done = true;
					event_signal(&dfu_event);
				}
				ret = 0;
				break;
			}
		}
        
		return ret;
	}
	else if((request->bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_DEVICE2HOST)
	{
		switch(bRequest) {
                
            case DFU_GETSTATE:
                print("Returning state = %d\n", dfu_state);
                *(u_int8_t *)io_buffer = dfu_state;
                ret = 1;
                break;
                
            case DFU_GETSTATUS:
                memcpy((u_int8_t *)io_buffer, (void *)&status_req, sizeof(struct usb_dfu_status_request));
                
                if(dfu_state == dfuMANIFEST_SYNC) {
                    set_status(errOK, DL_DONE_DELAY_MS, dfuMANIFEST, 0);
                }
                else if(dfu_state == dfuMANIFEST) {
                    // XXX TODO: validate DFU File Suffix? For now erase them
                    RELEASE_ASSERT(total_received >= DFU_FILE_SUFFIX_LENGTH);
                    RELEASE_ASSERT(image_buffer != NULL);
                    completion_status = total_received - DFU_FILE_SUFFIX_LENGTH;
                    bzero((void *)&image_buffer[completion_status], DFU_FILE_SUFFIX_LENGTH);
                    total_received = 0;
                    
                    // transitioning to this state will break us out of the file wait loop
                    set_status(errOK, STANDARD_DELAY_MS, dfuMANIFEST_WAIT_RESET, 0);
                }
                
                ret = sizeof(struct usb_dfu_status_request);
                break;
		}
        
		if(ret < 0)
			return -1;
		
		usb_core_do_transfer(EP0_IN, (u_int8_t *)io_buffer, ret, NULL);
        
		return 0;
	}
    
	return -1;
}

static void handle_bus_reset(void)
{
	if(dfu_state == dfuMANIFEST_WAIT_RESET) {
		dfu_done = true;
		event_signal(&dfu_event);
	}
}

static void data_received(u_int32_t received)
{
	print("DFU received %d of %d\n", received, expecting);
	
	if(received != expecting) {
		set_status(errNOTDONE, STANDARD_DELAY_MS, dfuERROR, 0);
    }
	else if((total_received + received) > image_buffer_size) {
		set_status(errADDRESS, STANDARD_DELAY_MS, dfuERROR, 0);
    }
	else {
		if(!security_allow_memory(&image_buffer[total_received], received)) {
			dprintf(DEBUG_INFO, "Permission Denied\n");
			completion_status = -1;
			dfu_done = true;
			event_signal(&dfu_event);
			return;
  		}
		
		memcpy(&image_buffer[total_received], (void *)io_buffer, received);
		total_received += received;
		expecting = 0;
		set_status(errOK, STANDARD_DELAY_MS, dfuDNLOAD_IDLE, 0);
	}
}
