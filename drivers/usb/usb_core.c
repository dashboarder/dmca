/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

////////////////////////////////////////////////////////////////////
//			USB Core
//
// This file implements USB stack which is responsible for
// constructing descriptors, handling all the setup requests, pass
// interface/class specific requests to respective drivers, handle
// vendor specific requests
///////////////////////////////////////////////////////////////////

#include <debug.h>
#include <platform.h>
#include <string.h>
#include <drivers/usb/usb_chap9.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usb/usb_core.h>
#include <drivers/usb/usb_controller.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <lib/env.h>

#if WITH_HW_POWER
#include <drivers/power.h>
#endif // WITH_HW_POWER

//===================================================================================
//		Local consts and macros
//===================================================================================

#define DEBUG   0
#if DEBUG
#define print(fmt, args...) printf("%s --- " fmt, __FUNCTION__, ##args)
#else
#define print(fmt, args...) (void)0
#endif // DEBUG

#define USB_MAX_STRING_DESCRIPTOR_INDEX		10
#define USB_STRING_DESCRIPTOR_INDEX_LANGID	0
#define USB_STRING_DESCRIPTOR_INDEX_NONCE	1

#define ADDRESS_0			0
#define EP0_TX_BUFFER_LENGTH 		256
#define USB_MAX_INTERFACES_SUPPORTED	2

#define NO_DATA_PHASE_HANDLER           (-2)
#define DEVICE_DATA_PHASE_HANDLER       (-1)

//===================================================================================
//		Global and Local variables
//===================================================================================

u_int32_t usb_configuration_string_desc_index = 0;

static const struct usb_string_descriptor lang_id_string_desc = {
	0x4,
	USB_DT_STRING,
	{0x09, 0x04}
};

static struct usb_string_descriptor* nonce_string_desc = NULL;

static struct usb_device_descriptor usb_core_device_descriptor = {
    USB_DT_DEVICE_SIZE,
    USB_DT_DEVICE,
    USB_BCD_VERSION,
    0x00,
    0x00,
    0x00,
    EP0_MAX_PACKET_SIZE,
    0,
    0,
    0,
    0,
    0,
    0,
    1
};

static struct usb_device_qualifier_descriptor usb_core_device_qualifier_descriptor = {
    USB_DT_DEVICE_QUALIFIER_SIZE,
    USB_DT_DEVICE_QUALIFIER,
    USB_BCD_VERSION,
    0x00,
    0x00,
    0x00,
    EP0_MAX_PACKET_SIZE,
    1,
    0
};

static volatile __aligned(CPU_CACHELINE_SIZE) u_int8_t ep0_tx_buffer[EP0_TX_BUFFER_LENGTH];

static struct usb_interface_instance *registered_interfaces[USB_MAX_INTERFACES_SUPPORTED];
static int registered_interfaces_count = 0;

static u_int8_t *usb_core_hs_configuration_descriptor;
static u_int8_t *usb_core_fs_configuration_descriptor;
static u_int8_t *usb_core_configuration_descriptor;
static u_int8_t *usb_core_other_speed_configuration_descriptor;
static struct usb_string_descriptor *usb_core_string_descriptors[USB_MAX_STRING_DESCRIPTOR_INDEX];

static u_int32_t ep0_data_phase_length;
static u_int32_t ep0_data_phase_rcvd;
static u_int8_t *ep0_data_phase_buffer;
static int ep0_data_phase_if_num; // < -1 = nothing to handle the data, -1 = device is handling, >=0 the index of the interface handling it

static int usb_device_state = DEVICE_ST_INIT;
static struct usb_device_request setup_request;

static u_int32_t usb_active_config = 0;

static u_int8_t test_mode_selector;

#if WITH_HW_POWER
static struct task_event usb_core_event_high_current;
static struct task_event usb_core_event_no_current;
static struct task *task_high_current = NULL; 
static struct task *task_no_current = NULL;
#endif // WITH_HW_POWER

static int usb_core_cable_state;

#if WITH_RECOVERY_MODE
static struct task *task_usb_req = NULL;
static u_int16_t last_command_id; //last command fully executed, so we can return output in a get request
static u_int16_t current_command_id; //command we are in the process of executing, we set last_command_id to this when completed.
static bool command_is_blind; //is the current command blind or not
static struct task_event vendor_request_event; //so we can signal task level to do the command
static bool command_in_progress; //so we don't attempt to receive a command while we're already processing one.

static void handle_vendor_device_completion(int data_received);
#endif // WITH_RECOVERY_MODE

//===================================================================================
//		Local Functions
//===================================================================================

static struct usb_device_io_request *alloc_ep0_device_io_request (volatile u_int8_t *io_buffer, int io_length,
                                                                  void (*callback)(struct usb_device_io_request *))
{
	struct usb_device_io_request *io_request;
    
	if(!io_buffer)
		ASSERT(io_length <= EP0_TX_BUFFER_LENGTH);
    
	io_request = calloc(1, sizeof(struct usb_device_io_request));
    
	io_request->endpoint = EP0_IN;
    if(!io_buffer)
        io_request->io_buffer = ep0_tx_buffer;
    else
        io_request->io_buffer = io_buffer;
	io_request->io_length = io_length;
	io_request->callback = callback;
    
	return io_request;
}

static struct usb_device_io_request *alloc_device_io_request (u_int32_t endpoint, volatile u_int8_t *io_buffer,
                                                              int io_length, void (*callback)(struct usb_device_io_request *))
{
	struct usb_device_io_request *io_request;
    
	if(endpoint == EP0_IN)
		return alloc_ep0_device_io_request(io_buffer, io_length, callback);
    
	io_request = calloc(1, sizeof(struct usb_device_io_request));
    
	io_request->endpoint = endpoint;
	io_request->io_buffer = io_buffer;
	io_request->io_length = io_length;
	io_request->callback = callback;
    
	return io_request;
}

static void handle_test_mode_request(struct usb_device_io_request *ioreq)
{
#if WITH_HW_POWER
	// set current draw to 0 for test_j & test_k
    if((test_mode_selector == 1) ||
       (test_mode_selector == 2) ||
       (test_mode_selector == 3)) {
        event_signal(&usb_core_event_no_current);
	}
#endif // WITH_HW_POWER
    
	usb_controller_do_test_mode(test_mode_selector);
}

static void handle_ep0_data_phase (u_int8_t *rx_buffer, u_int32_t data_rcvd, bool *data_phase)
{
	u_int32_t remaining;
    u_int32_t to_copy;
    
    if((ep0_data_phase_rcvd + data_rcvd) > ep0_data_phase_length) {
        usb_controller_stall_endpoint(EP0_IN, true);
        goto done;
    }
    
    remaining = ep0_data_phase_length - ep0_data_phase_rcvd;
    to_copy = (data_rcvd > remaining) ? remaining : data_rcvd;
    
    memcpy(ep0_data_phase_buffer, rx_buffer, to_copy);
    
	ep0_data_phase_buffer += to_copy;
	ep0_data_phase_rcvd += to_copy;
    
	*data_phase = true;
    
	print("data-phase payload %d of %d\n", ep0_data_phase_rcvd, ep0_data_phase_length);
    
	if((ep0_data_phase_rcvd == ep0_data_phase_length) || (data_rcvd != EP0_MAX_PACKET_SIZE))
	{
#if WITH_RECOVERY_MODE
		if(ep0_data_phase_if_num == DEVICE_DATA_PHASE_HANDLER) { //the device is handling this
			handle_vendor_device_completion(ep0_data_phase_rcvd);
		} else
#endif // WITH_RECOVERY_MODE
        {
            if((ep0_data_phase_if_num >= 0) && (ep0_data_phase_if_num < registered_interfaces_count)
               && (registered_interfaces[ep0_data_phase_if_num]->non_setup_data_phase_finished_callback)) {
                registered_interfaces[ep0_data_phase_if_num]->non_setup_data_phase_finished_callback(ep0_data_phase_rcvd);
                usb_core_send_zlp(); //we should check returnval of call and possibly stall
            }
        }
		
		goto done;
	}
	
	return;
	
done:
    ep0_data_phase_rcvd = 0;
    ep0_data_phase_length = 0;
    ep0_data_phase_buffer = NULL;
    ep0_data_phase_if_num = NO_DATA_PHASE_HANDLER;
    *data_phase = false;
}

static struct usb_string_descriptor *get_usb_string_descriptor (int index)
{
	if(index == USB_STRING_DESCRIPTOR_INDEX_LANGID) {
		return ((struct usb_string_descriptor *)&lang_id_string_desc);
	}
	
	if(index == USB_STRING_DESCRIPTOR_INDEX_NONCE) {
		return nonce_string_desc;
	}
    
	if((index >= USB_MAX_STRING_DESCRIPTOR_INDEX) || (!usb_core_string_descriptors[index])) {
		return NULL;
	}
    
	return usb_core_string_descriptors[index];
}

static int handle_get_descriptor (struct usb_device_request *request)
{
	size_t actual_length = 0;
    
	switch(request->wValue >> 8) {
		case USB_DT_DEVICE :
			actual_length = __min(request->wLength, sizeof(struct usb_device_descriptor));
			RELEASE_ASSERT(actual_length <= EP0_TX_BUFFER_LENGTH);
			memcpy((void *)ep0_tx_buffer, (void *)&usb_core_device_descriptor, actual_length);
            break;
            
		case USB_DT_DEVICE_QUALIFIER :
			actual_length = __min(request->wLength, sizeof(struct usb_device_qualifier_descriptor));
			RELEASE_ASSERT(actual_length <= EP0_TX_BUFFER_LENGTH);
			memcpy((void *)ep0_tx_buffer, (void *)&usb_core_device_qualifier_descriptor, actual_length);
            break;
            
		case USB_DT_OTHER_SPEED_CONFIGURATION :
			actual_length = __min(request->wLength, ((struct usb_configuration_descriptor *)(usb_core_other_speed_configuration_descriptor))->wTotalLength);
			RELEASE_ASSERT(actual_length <= EP0_TX_BUFFER_LENGTH);
			memcpy((void *)ep0_tx_buffer, (void *)usb_core_other_speed_configuration_descriptor, actual_length);
			((struct usb_configuration_descriptor *)(ep0_tx_buffer))->bDescriptorType = USB_DT_OTHER_SPEED_CONFIGURATION;
            break;
            
		case USB_DT_CONFIGURATION :
			actual_length = __min(request->wLength, ((struct usb_configuration_descriptor *)(usb_core_configuration_descriptor))->wTotalLength);
			RELEASE_ASSERT(actual_length <= EP0_TX_BUFFER_LENGTH);
			memcpy((void *)ep0_tx_buffer, (void *)usb_core_configuration_descriptor, actual_length);
            break;
            
		case USB_DT_STRING :
		{
			struct usb_string_descriptor *string_desc = NULL;
			
			if((string_desc = get_usb_string_descriptor(request->wValue & 0xff))) {
				actual_length = __min(request->wLength, string_desc->bLength);
				RELEASE_ASSERT(actual_length <= EP0_TX_BUFFER_LENGTH);
				memcpy((void *)ep0_tx_buffer, (void *)string_desc, actual_length);
			}
		}
            break;
            
		default :
			return -1;
	}
    
	return actual_length;
}

static void handle_set_configuration (struct usb_device_request *request)
{
	int i;
    
	usb_active_config = request->wValue;
    
	for(i = 0; i < registered_interfaces_count; i++) {
		if(registered_interfaces[i] && registered_interfaces[i]->activate_interface) {
			registered_interfaces[i]->activate_interface();
		}
	}
}

static int handle_clear_feature (struct usb_device_request *request)
{
	int rc = -1;
    
	switch(request->bmRequestType & USB_REQ_RECIPIENT_MASK) {
		case USB_REQ_RECIPIENT_ENDPOINT :
			if(request->wValue != USB_FEATURE_ENDPOINT_HALT) {
				goto done;
			}
            
			// Reset the data toggle
			usb_controller_reset_endpoint_data_toggle(request->wIndex);
            
			// Clear endpoint stall
			usb_controller_stall_endpoint(request->wIndex, false);
            
			rc = 0;
            break;
            
            /* Remote wake up not supported, and Test mode cannot be cleared */
		case USB_REQ_RECIPIENT_DEVICE :
		default :
			goto done;
	}
    
    done :
	return rc;
}

static int handle_set_feature (struct usb_device_request * request , void (**callback)(struct usb_device_io_request *))
{
	int rc = -1;
	
	*callback = NULL;
    
    switch(request->bmRequestType & USB_REQ_RECIPIENT_MASK) {
        case USB_REQ_RECIPIENT_DEVICE :
			/* Remote wakeup not supported */
			if(request->wValue != USB_FEATURE_TEST_MODE) {
				goto done;
			}
			
			if((request->wIndex & 0xff) != 0) {
				goto done;
			}
            
			*callback = handle_test_mode_request;
			test_mode_selector = (request->wIndex >> 8) & 0xf;
			
			rc = 0;
            break;
            
        case USB_REQ_RECIPIENT_ENDPOINT :
            if(request->wValue != USB_FEATURE_ENDPOINT_HALT) {
                goto done;
            }
            
            usb_controller_stall_endpoint(request->wIndex, true);
            
            rc = 0;
            break;
            
        default :
            goto done;
    }
    
    done :
    return rc;
	
}

static void standard_device_request_cb (struct usb_device_io_request *req)
{
	if((req->io_length > 0) && ((req->io_length % EP0_MAX_PACKET_SIZE) == 0) && (setup_request.wLength > req->io_length)) {
		usb_core_send_zlp();
	}
}

#if WITH_RECOVERY_MODE

#define VDR_SIZE 512
static char *vendor_device_request_rxbuffer;

//perform a command line sent by host.
static void do_vendor_command(bool isBlind)
{
	bool syntax_error = true;
	int rval = -1;
	
	vendor_device_request_rxbuffer[VDR_SIZE - 1] = 0; //ensure host data is 0 terminated
#if WITH_MENU
	syntax_error = process_command_line(vendor_device_request_rxbuffer, &rval);
#endif // WITH_MENU
	command_in_progress = false;
	if(!syntax_error) {
		last_command_id = current_command_id; //save the id of last command executed so we can return results if requested
	}
    
	if(!isBlind) { //if not blind we need to convey success to the host by stalling or sending a zlp
		if(syntax_error || rval < 0) { //command doesn't exist or failed, stall the pipe
			usb_controller_stall_endpoint(EP0_IN, true);
		}
		else {
			usb_core_send_zlp();
		}
	}
}

//completion routine after the zlp has been sent for a blind request from host
static void handle_blind_vendor_zlp(struct usb_device_io_request *io_request)
{
	//we successfully sent the zlp, so now perform the command
	event_signal(&vendor_request_event);
}

//handler invoked once all data has been received for a vendor-specific-device request from host
static void handle_vendor_device_completion(int data_received)
{
	if(command_is_blind) { //don't execute the command until we've sent the zlp
		struct usb_device_io_request *io_request = alloc_ep0_device_io_request(NULL, 0,
                                                                               handle_blind_vendor_zlp);
		if(io_request == NULL) {
			usb_controller_stall_endpoint(EP0_IN, true);
			dprintf(DEBUG_INFO, "usb_core : usb_core_send_zlp --- alloc ep0 failed \n");
			command_in_progress = false;
			return;
		}
		
		usb_controller_do_endpoint_io(io_request);
		return;
	}
	else {
		event_signal(&vendor_request_event); //kick it to the task to handle the command line
	}
}

//determine if we handle a given vendor-specific-device request from host
static void handle_vendor_device_request (struct usb_device_request *request)
{
	switch (request->bRequest) {
		case PR_VENDOR_REQUEST:
		case PR_VENDOR_BLIND_REQUEST:
			if(!command_in_progress && ((request->bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_HOST2DEVICE)) {
				if(request->wLength && request->wLength <= VDR_SIZE) {
					if(!vendor_device_request_rxbuffer) {
						vendor_device_request_rxbuffer = malloc(VDR_SIZE);
					}
					if(vendor_device_request_rxbuffer) {
						ep0_data_phase_buffer = (u_int8_t *)vendor_device_request_rxbuffer;
						ep0_data_phase_length = request->wLength;
						ep0_data_phase_if_num = DEVICE_DATA_PHASE_HANDLER;
						command_is_blind = (request->bRequest == PR_VENDOR_BLIND_REQUEST);
						current_command_id = request->wValue;
						command_in_progress = true;
						return;
					}
				}
			}
			else if(((request->bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_DEVICE2HOST) &&
                    request->wLength && (request->wValue == last_command_id)) {
				int outputlen;
				const char *data;
				if((data = env_get("cmd-results"))) {
					struct usb_device_io_request* ioreq;
					outputlen = strlen(data) + 1;
					if(outputlen <=  request->wLength) {
						ioreq = alloc_device_io_request(EP0_IN, (unsigned char *)data, outputlen,
                                                        standard_device_request_cb);
						if(ioreq) {
							usb_controller_do_endpoint_io(ioreq);
							return;
						}
					}
				}
			}
            break;
	}
    
	usb_controller_stall_endpoint(EP0_IN, true);
	//usb_controller_stall_endpoint(EP0_OUT, true);
}

static int vendor_req_task(void *arg)
{
    for(;;) {
		// make sure there's something to do
		event_wait(&vendor_request_event);
        
		// make sure menu-task is up to process commands
		event_wait(&gMenuTaskReadyEvent);
        
		// process commands
		do_vendor_command(command_is_blind);
    }
    return 0;
}

#endif // WITH_RECOVERY_MODE

static void handle_standard_device_request (struct usb_device_request *request)
{
	int buffer_length = -1;
	struct usb_device_io_request *io_request;
	void (*callback)(struct usb_device_io_request *) = standard_device_request_cb;
    
	// handle one that returns data
	if ((request->bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_DEVICE2HOST) {
		switch(request->bRequest) {
			case USB_REQ_GET_CONFIGURATION :
				ep0_tx_buffer[0] = usb_active_config;
				buffer_length = 1;
                break;
                
			case USB_REQ_GET_DESCRIPTOR :
				buffer_length = handle_get_descriptor(request);
                break;
                
			case USB_REQ_GET_INTERFACE :
				buffer_length = 1;
				ep0_tx_buffer[0] = 0;
				/* XXX should I stall if intf_num is bad or just return 0 */
				if((request->wIndex < registered_interfaces_count) && (registered_interfaces[request->wIndex]->handle_get_interface)) {
					ep0_tx_buffer[0] = registered_interfaces[request->wIndex]->handle_get_interface();
				}
                break;
                
			case USB_REQ_GET_STATUS :
				buffer_length = 2;
				ep0_tx_buffer[0] = ep0_tx_buffer[1] = 0;
				if((request->bmRequestType & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT) {
					ep0_tx_buffer[0] = usb_controller_is_endpoint_stalled(request->wIndex);
				}
                break;
		}
	}
	// handle ones that do not return data
	else {
		switch(request->bRequest) {
			case USB_REQ_CLEAR_FEATURE :
				buffer_length = handle_clear_feature(request);
                break;
                
			case USB_REQ_SET_FEATURE:
				buffer_length = handle_set_feature(request, &callback);
                break;
                
			case USB_REQ_SET_ADDRESS :
				usb_controller_set_address(request->wValue);
				buffer_length = 0;
				usb_device_state = DEVICE_ST_ADDRESSED;
                break;
                
			case USB_REQ_SET_CONFIGURATION :
				buffer_length = 0;
				if(usb_active_config != request->wValue) {
					handle_set_configuration(request);
				}
				if(usb_active_config != 0) {
					usb_device_state = DEVICE_ST_CONFIGURED;
#if (WITH_HW_POWER && !TARGET_USB_DEVICE_SELF_POWERED)
					event_signal(&usb_core_event_high_current);
#endif // WITH_HW_POWER && !TARGET_USB_DEVICE_SELF_POWERED
				}
                break;
                
			case USB_REQ_SET_INTERFACE :
				buffer_length = -1;
				if((request->wIndex < registered_interfaces_count) && (registered_interfaces[request->wIndex]->handle_set_interface)) {
					buffer_length = registered_interfaces[request->wIndex]->handle_set_interface(request->wValue);
				}
                break;
		}
	}
	
	if(buffer_length < 0) {
        usb_controller_stall_endpoint(EP0_IN, true);
        //usb_controller_stall_endpoint(EP0_OUT, true);
        print("usb_core : handle_standard_device_request --- Setup Request not handled \n");
        return;
	}
	else {
		if((io_request = alloc_ep0_device_io_request(NULL, buffer_length, callback)) == NULL) {
			usb_controller_stall_endpoint(EP0_IN, true);
			dprintf(DEBUG_INFO, "usb_core : handle_standard_device_request --- alloc_ep0 failed \n");
			return;
		}
	}
	
	usb_controller_do_endpoint_io(io_request);
}

static struct usb_string_descriptor* usb_alloc_string_descriptor (const char *str)
{
	struct usb_string_descriptor *string_desc;
	u_int32_t string_desc_len;
	
	// substracting 2 for wData[2]
	string_desc_len = sizeof(struct usb_string_descriptor) - 2 + 2 * strlen(str);
    
	if(string_desc_len > 255)
		panic("usb_alloc_string_descriptor: string_desc_len > 255\n");
    
	string_desc = malloc(string_desc_len);
	
	string_desc->bLength = string_desc_len;
	string_desc->bDescriptorType = USB_DT_STRING;
	
	u_int16_t *wData = (u_int16_t *)string_desc->wData;
	while(*str) {
		*wData++ = (u_int16_t)*str++;
	}
    
	return string_desc;
}

static int usb_create_string_descriptor (const char *str)
{
	int i;
	// String desc at index 0 is used for Lang ID
	// String desc at index 1 is used for Nonce
	for(i = 2; i < USB_MAX_STRING_DESCRIPTOR_INDEX; i++) {
		if(usb_core_string_descriptors[i] == NULL) {
			usb_core_string_descriptors[i] = usb_alloc_string_descriptor(str);
			break;
		}
	}
    
	return i;
}

static void usb_free_string_descriptors (void)
{
	int i;
    
	for(i = 0; i < USB_MAX_STRING_DESCRIPTOR_INDEX; i++) {
		if(usb_core_string_descriptors[i]) {
			free(usb_core_string_descriptors[i]);
			usb_core_string_descriptors[i] = NULL;
		}
	}
}

#if WITH_HW_POWER
static int usb_high_current_task(void *arg)
{
    for(;;) {
        event_wait(&usb_core_event_high_current);

        power_set_usb_state(true, false);
    }
    
    return 0;
}

static int usb_no_current_task(void *arg)
{
    for(;;) {
        event_wait(&usb_core_event_no_current);
	
        power_set_usb_state(false, false);
    }
    
    return 0;
}
#endif // WITH_HW_POWER

//===================================================================================
//		Global Functions
//===================================================================================
int usb_core_init (const char *configuration_string_desc)
{
	nonce_string_desc = usb_alloc_string_descriptor(platform_get_usb_more_other_string(true));
	
	usb_core_device_descriptor.idVendor	 = platform_get_usb_vendor_id();
	usb_core_device_descriptor.idProduct	 = platform_get_usb_product_id();
	usb_core_device_descriptor.bcdDevice	 = platform_get_usb_device_version();
	usb_core_device_descriptor.iManufacturer = usb_create_string_descriptor(platform_get_usb_manufacturer_string());
	usb_core_device_descriptor.iProduct	 = usb_create_string_descriptor(platform_get_usb_product_string());
	usb_core_device_descriptor.iSerialNumber = usb_create_string_descriptor(platform_get_usb_serial_number_string(true));
    
	usb_configuration_string_desc_index = usb_create_string_descriptor(configuration_string_desc);
	
	bzero((void *)ep0_tx_buffer, EP0_TX_BUFFER_LENGTH);
    
	registered_interfaces_count = 0;
    
#if WITH_RECOVERY_MODE
	//prepare the task-context requets handler
	if (task_usb_req == NULL) {
		event_init(&vendor_request_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
#if defined(USB_VENDOR_TASK_SIZE)
		task_usb_req = task_create("usb req", &vendor_req_task, NULL, USB_VENDOR_TASK_SIZE);	
		task_start(task_usb_req);
#else
		task_usb_req = task_create("usb req", &vendor_req_task, NULL, 0x1C00);
		task_start(task_usb_req);
#endif
	}
#endif // WITH_RECOVERY_MODE
    
#if WITH_HW_POWER
	if (task_high_current == NULL) {
		event_init(&usb_core_event_high_current, EVENT_FLAG_AUTO_UNSIGNAL, false);
		task_high_current = task_create("usb-high-current", &usb_high_current_task, NULL, 0x600);
		task_start(task_high_current);
	}
    	
	if (task_no_current == NULL) {
		event_init(&usb_core_event_no_current, EVENT_FLAG_AUTO_UNSIGNAL, false);
		task_no_current = task_create("usb-no-current", &usb_no_current_task, NULL, 0x400);
    		task_start(task_no_current);
	}
#endif // WITH_HW_POWER
	
	usb_core_cable_state = CABLE_DISCONNECTED;
	
	return usb_controller_init();
}

int usb_core_start()
{
    int i, j, k;
    int total_len;
    u_int8_t *hs_config_desc;
    u_int8_t *fs_config_desc;
    u_int8_t *p;
    struct usb_configuration_descriptor desc = {
        USB_DT_CONFIGURATION_SIZE,
        USB_DT_CONFIGURATION,
        0,
        0,
        1,
        usb_configuration_string_desc_index,
#if TARGET_USB_DEVICE_SELF_POWERED
        (1 << 7) | (1 << 6),
        0
#else
        (1 << 7),
        500/2
#endif
    };
    int other_descs_len;
    
    if(registered_interfaces_count == 0) {
        dprintf(DEBUG_INFO, "usb_core has no registered interfaces\n");
        return -1;
    }

#if WITH_HW_POWER
    power_set_usb_enabled(true);
#endif
    
	desc.bNumInterfaces = registered_interfaces_count;
	
    // create configuration descriptors
    
	// Chap-9 compliance tool and some USBHost requires a
	// device to provide "other-speed" descriptor. So we need to
	// build 2 sets of configuration descriptors here, one for
	// high-speed config and other for full-speed config.
	//
	// First, we will calculate total-length for our configuration
	// descriptor. Current implementation requires every function
	// driver (like usb-serial, usb-dfu) to provide their list of
	// interface descriptors, endpoint descriptors, and other
	// functional descriptors. For functional descriptors, we
	// assume (as per USB spec), 1st byte store the length of the
	// descriptor.
	//
	// Next, we have our total length, we update this value for
	// final configuraiton descriptor. Also, now we know how much
	// memory we need to allocate for our config descriptor.
	//
	// Next - We now start copying various descriptors
	// into the memory allocated for the final config
	// decriptor. We start with configuration descriptor
	// first. Then we will start copying interface descriptors and
	// their corresponding endpoint descriptors (trusting the list
	// of descriptors supplied by our function drivers). Every
	// interface could have an alternate setting, and endpoints
	// associated with these alternate settings (>= 0). We are
	// using num_of_endpoints information supplied in the
	// interface descriptor to figure out if we need to copy
	// endpoint descriptors for a particular interface.
	//
	// So far, this seems straightforward. Two things which
	// introduces complexities -
	// 1. functional descriptors - these are associated with a
	// interface. Currently, we are just copying them after the
	// first interface we found. This is not a generic
	// solution. This needs to be addressed.
	// 2. other-speed descriptor - currently, we are building
	// both high and full-speed descriptor together. For FS
	// descriptor we need to change max-packet-size for the
	// endpoints. Again we are trusting contents provided by our
	// function drivers, and just updating them with correct value.
    
	// Note about string descriptors - currently every interface
	// driver will pass list of strings it has, and the
	// count. Some interface driver might not have any strings
	// associated with them. We are assuming, if an interface
	// driver passes us a list with n number of strings, n is
	// always same has total # of interface associated with that
	// interface driver.
    
    other_descs_len = 0;
    total_len = sizeof(struct usb_configuration_descriptor);
    
    for(i = 0; i < registered_interfaces_count; i++) {
        for(j = 0; j < registered_interfaces[i]->total_interfaces; j++)
            total_len += sizeof(struct usb_interface_descriptor);
        for(j = 0; j < registered_interfaces[i]->total_other_descs; j++) {
            p = (u_int8_t *)(registered_interfaces[i]->other_descs) + other_descs_len;
            other_descs_len += p[0];
            total_len += p[0];
        }
        for(j = 0; j < registered_interfaces[i]->total_endpoints; j++)
            total_len += sizeof(struct usb_endpoint_descriptor);
    }
    
    usb_core_hs_configuration_descriptor = (u_int8_t *)malloc(total_len);
    
    usb_core_fs_configuration_descriptor = (u_int8_t *)malloc(total_len);
    
    desc.wTotalLength = total_len;
    
    memcpy(usb_core_hs_configuration_descriptor, (void *)&desc, sizeof(struct usb_configuration_descriptor));
    memcpy(usb_core_fs_configuration_descriptor, (void *)&desc, sizeof(struct usb_configuration_descriptor));
    
    hs_config_desc = usb_core_hs_configuration_descriptor + sizeof(struct usb_configuration_descriptor);
    fs_config_desc = usb_core_fs_configuration_descriptor + sizeof(struct usb_configuration_descriptor);
    
    for(i = 0; i < registered_interfaces_count; i++) {
        for(j = 0; j < registered_interfaces[i]->total_interfaces; j++) {
            
			ASSERT((hs_config_desc + sizeof(struct usb_interface_descriptor)) <= (hs_config_desc + total_len));
			ASSERT((fs_config_desc + sizeof(struct usb_interface_descriptor)) <= (fs_config_desc + total_len));
            
            memcpy(hs_config_desc,
                   (u_int8_t *)(registered_interfaces[i]->interface_descs) + (j * sizeof(struct usb_interface_descriptor)),
                   sizeof(struct usb_interface_descriptor));
            
            memcpy(fs_config_desc,
                   (u_int8_t *)(registered_interfaces[i]->interface_descs) + (j * sizeof(struct usb_interface_descriptor)),
                   sizeof(struct usb_interface_descriptor));
            
            if(registered_interfaces[i]->string_descs && registered_interfaces[i]->string_descs[j]) {
                ((struct usb_interface_descriptor *)hs_config_desc)->iInterface = usb_create_string_descriptor(registered_interfaces[i]->string_descs[j]);
                ((struct usb_interface_descriptor *)fs_config_desc)->iInterface = usb_create_string_descriptor(registered_interfaces[i]->string_descs[j]);
            }
            
            p = hs_config_desc;
            
            hs_config_desc += sizeof(struct usb_interface_descriptor);
            fs_config_desc += sizeof(struct usb_interface_descriptor);
            
			ASSERT((hs_config_desc + other_descs_len) <= (hs_config_desc + total_len));
			ASSERT((fs_config_desc + other_descs_len) <= (fs_config_desc + total_len));
            
            if(registered_interfaces[i]->total_other_descs) {
                memcpy(hs_config_desc, registered_interfaces[i]->other_descs, other_descs_len);
                memcpy(fs_config_desc, registered_interfaces[i]->other_descs, other_descs_len);
                
                hs_config_desc += other_descs_len;
                fs_config_desc += other_descs_len;
            }
            
            if(((struct usb_interface_descriptor *)p)->bNumEndpoints == 0)
                continue;
            
            for(k = 0; k < registered_interfaces[i]->total_endpoints; k++) {
                
				ASSERT((hs_config_desc + (k * sizeof(struct usb_endpoint_descriptor)) + sizeof(struct usb_endpoint_descriptor))
				       <= (hs_config_desc + total_len));
				ASSERT((fs_config_desc + (k * sizeof(struct usb_endpoint_descriptor)) + sizeof(struct usb_endpoint_descriptor))
				       <= (fs_config_desc + total_len));
                
                memcpy(hs_config_desc + (k * sizeof(struct usb_endpoint_descriptor)),
                       (u_int8_t *)(registered_interfaces[i]->endpoint_descs) + (k * sizeof(struct usb_endpoint_descriptor)),
                       sizeof(struct usb_endpoint_descriptor));
                
                memcpy(fs_config_desc + (k * USB_DT_ENDPOINT_SIZE),
                       (u_int8_t *)(registered_interfaces[i]->endpoint_descs) + (k * sizeof(struct usb_endpoint_descriptor)),
                       sizeof(struct usb_endpoint_descriptor));
                p = fs_config_desc + (k * USB_DT_ENDPOINT_SIZE);
                if((((struct usb_endpoint_descriptor *)p)->bDescriptorType == USB_DT_ENDPOINT) &&
				   (((struct usb_endpoint_descriptor *)p)->bmAttributes == USB_ENDPOINT_BULK)) {
                    ((struct usb_endpoint_descriptor *)p)->wMaxPacketSize = FS_EP_MAX_PACKET_SIZE;
                }
            }
            
            hs_config_desc += (registered_interfaces[i]->total_endpoints * sizeof(struct usb_endpoint_descriptor));
            fs_config_desc += (registered_interfaces[i]->total_endpoints * sizeof(struct usb_endpoint_descriptor));
        }
    }
    
	return usb_controller_start();
}

void usb_core_register_interface (struct usb_interface_instance *intf)
{
	if(registered_interfaces_count >= USB_MAX_INTERFACES_SUPPORTED) {
		dprintf(DEBUG_INFO, "registered interfaces exceed max \n");
		return;
	}
    
	registered_interfaces[registered_interfaces_count] = intf;
	registered_interfaces_count++;
    
}

void usb_core_handle_usb_control_receive (u_int8_t *ep0_rx_buffer, bool is_setup, int receive_length, bool *data_phase)
{
	ASSERT(data_phase != NULL);
    
    *data_phase = false;
    
	if(is_setup == false) {
		if (receive_length == 0)
			return;
		
		ASSERT(ep0_data_phase_buffer != NULL);
		handle_ep0_data_phase(ep0_rx_buffer, receive_length, data_phase);
		return;
	}
    
	print("process usb setup data \n");
	
	memcpy(&setup_request, ep0_rx_buffer, sizeof(setup_request));
	
	switch(setup_request.bmRequestType & USB_REQ_TYPE_MASK) {
		case USB_REQ_TYPE_STANDARD :
			handle_standard_device_request(&setup_request);
			goto success;
            
#if WITH_RECOVERY_MODE
		case USB_REQ_TYPE_VENDOR :
			switch(setup_request.bmRequestType & USB_REQ_RECIPIENT_MASK) {
				case USB_REQ_RECIPIENT_DEVICE :
					handle_vendor_device_request(&setup_request);
					goto success;
					
				case USB_REQ_RECIPIENT_INTERFACE :
				{
					int intf_num = setup_request.wIndex;
					
					if((intf_num >= 0) && (intf_num < registered_interfaces_count)
					   && (registered_interfaces[intf_num]->handle_vendor_request)) {
						int ret = registered_interfaces[intf_num]->handle_vendor_request(&setup_request);
						
						if((setup_request.bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_HOST2DEVICE) {
							if (ret > 0) {
								ep0_data_phase_length = ret;
								ep0_data_phase_if_num = intf_num;
								goto success;
							}
							else if(ret == 0) {
								usb_core_send_zlp();
								goto success;
							}
						}
						else if((setup_request.bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_DEVICE2HOST) {
							goto success;
						}
					}
					goto error;
				}
			}
#endif // WITH_RECOVERY_MODE
            
		case USB_REQ_TYPE_CLASS :
			if((setup_request.bmRequestType & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE) {
				int intf_num = setup_request.wIndex;
                
				if((intf_num >= 0) && (intf_num < registered_interfaces_count)
                   && (registered_interfaces[intf_num]->handle_request)) {
					int ret = registered_interfaces[intf_num]->handle_request(&setup_request,
                                                                              &ep0_data_phase_buffer);
                    
					if((setup_request.bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_HOST2DEVICE) {
						if (ret > 0) {
							ASSERT(ep0_data_phase_buffer != NULL);
							ep0_data_phase_length = ret;
							ep0_data_phase_if_num = intf_num;
							goto success;
						}
						else if(ret == 0) {
							usb_core_send_zlp();
							goto success;
						}
					}
					else if((setup_request.bmRequestType & USB_REQ_DIRECTION_MASK) == USB_REQ_DEVICE2HOST) {
						goto success;
					}
				}
			}
            
#if WITH_RECOVERY_MODE
        error:
#endif // WITH_RECOVERY_MODE
            //else fall through
		default :
			//usb_controller_stall_endpoint(EP0_OUT, true);
			usb_controller_stall_endpoint(EP0_IN, true);
			*data_phase = false;
			dprintf(DEBUG_INFO, "usb_core : usb_core_handle_usb_control_receive --- Request %02x %02x %04x %04x not handled \n",
                    setup_request.bmRequestType, setup_request.bRequest, setup_request.wIndex, setup_request.wLength);
			return;
    }
    
success:
	switch(setup_request.bmRequestType & USB_REQ_DIRECTION_MASK) {
		case USB_REQ_DEVICE2HOST :
			*data_phase = true;
			break;
		case USB_REQ_HOST2DEVICE :
			*data_phase = (setup_request.wLength > 0);
        	break;
	}
}

void usb_core_event_handler (int event)
{
	int i;
    
	switch(event) {
		case CABLE_CONNECTED :
            break;
            
		case CABLE_DISCONNECTED :
		{
            usb_core_cable_state = CABLE_DISCONNECTED;

			for(i = 0; i < registered_interfaces_count; i++) {
				if(registered_interfaces[i] && registered_interfaces[i]->deactivate_interface) {
					registered_interfaces[i]->deactivate_interface();
				}
			}
			
#if WITH_HW_POWER
            // set current to 0, when cable is gone
			event_signal(&usb_core_event_no_current);
#endif // WITH_HW_POWER
		}
            break;
            
		case USB_RESET :
			print("USB_RESET \n");
			usb_controller_abort_endpoint(EP0_IN);
			usb_controller_set_address(ADDRESS_0);
			usb_device_state = DEVICE_ST_DEFAULT;
			usb_active_config = 0;
			ep0_data_phase_if_num = NO_DATA_PHASE_HANDLER;
			
			// Call reset handler on all interfaces
			for(i = 0; i < registered_interfaces_count; i++) {
				if(registered_interfaces[i] && registered_interfaces[i]->handle_bus_reset) {
					registered_interfaces[i]->handle_bus_reset();
				}
			}
            
			// An USB reset should also deactivate all the interfaces
			for(i = 0; i < registered_interfaces_count; i++) {
				if(registered_interfaces[i] && registered_interfaces[i]->deactivate_interface) {
					registered_interfaces[i]->deactivate_interface();
				}
			}
			
            break;
            
		case USB_ENUM_DONE :
			print("ENUM_DONE event \n");
            
            usb_core_cable_state = CABLE_CONNECTED;
            
            usb_core_configuration_descriptor = usb_core_hs_configuration_descriptor;
            usb_core_other_speed_configuration_descriptor = usb_core_fs_configuration_descriptor;
            if(usb_controller_get_connection_speed() == CONNECTION_SPEED_FULL) {
                usb_core_configuration_descriptor = usb_core_fs_configuration_descriptor;
                usb_core_other_speed_configuration_descriptor = usb_core_hs_configuration_descriptor;
            }
            break;
	}
}

bool usb_core_get_connection_speed (void)
{
    return(usb_controller_get_connection_speed());
}

bool usb_core_get_cable_state (void)
{
    return(usb_core_cable_state);
}

void usb_core_complete_endpoint_io (struct usb_device_io_request *io_req)
{
	if(io_req->callback) {
		io_req->callback(io_req);
	}
    
	free(io_req);
	io_req = NULL;
}

void usb_core_do_transfer (int endpoint, u_int8_t *buffer, int length, void (*callback)(struct usb_device_io_request *))
{
	struct usb_device_io_request *io_request;
	
	if((io_request = alloc_device_io_request(endpoint, buffer, length, callback)) == NULL)
	{
		dprintf(DEBUG_INFO, "usb_core_write_data --- alloc device io request failed \n");
		return;
	}
	
	print("starting transfer on %x of size %d \n", endpoint, length);
	usb_controller_do_endpoint_io(io_request);
}

void usb_core_send_zlp (void)
{
	struct usb_device_io_request *io_request = alloc_ep0_device_io_request(NULL, 0, NULL);
    
	if(io_request == NULL) {
		usb_controller_stall_endpoint(EP0_IN, true);
		dprintf(DEBUG_INFO, "usb_core : usb_core_send_zlp --- alloc ep0 failed \n");
		return;
	}
    
	usb_controller_do_endpoint_io(io_request);
}

void usb_core_activate_endpoint (u_int32_t endpoint, int type, int max_packet_size, int interval)
{
	usb_controller_activate_endpoint(endpoint, type, max_packet_size, interval);
}

void usb_core_abort_endpoint (int endpoint)
{
	usb_controller_abort_endpoint(endpoint);
}

void usb_core_deactivate_endpoint (u_int32_t endpoint)
{
	usb_controller_abort_endpoint(endpoint);
	usb_controller_deactivate_endpoint(endpoint);
}

void usb_core_stop (void)
{
    usb_controller_stop();
	usb_core_free();	
	
#if WITH_HW_POWER
    power_set_usb_enabled(false);
#endif // WITH_HW_POWER
}

void usb_core_free (void)
{
    usb_controller_free();
    
	if(usb_core_hs_configuration_descriptor) {
		free(usb_core_hs_configuration_descriptor);
		usb_core_hs_configuration_descriptor = NULL;
	}
    
	if(usb_core_fs_configuration_descriptor) {
		free(usb_core_fs_configuration_descriptor);
		usb_core_fs_configuration_descriptor = NULL;
	}
    
#if WITH_RECOVERY_MODE
	if(vendor_device_request_rxbuffer) {
		free(vendor_device_request_rxbuffer);
		vendor_device_request_rxbuffer = NULL;
	}
#endif // WITH_RECOVERY_MODE
    
	if(nonce_string_desc) {
	    free(nonce_string_desc);
	    nonce_string_desc = NULL;
	}
    
	usb_free_string_descriptors();
	registered_interfaces_count = 0;
}
