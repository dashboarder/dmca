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
#include <drivers/usb/usb_core.h>
#include <drivers/dockfifo/dockfifo.h>
#include <drivers/usb/usb_controller.h>
#include <sys/task.h>
#include <endian_tools.h>
#include <platform.h>

// #undef DEBUG_LEVEL
// #define DEBUG_LEVEL DEBUG_SPEW

/*
 * 3 bytes header:
 *	1byte:	endpoint and flags
 * 		bit 7 is direction:
 *                  0: Host to Device (OUT in USB terms)
 *                  1: Device to Host (IN in USB terms)
 * 		bit 6 is control packet phase:
 *                  0: data or N/A
 *                  1: setup
 *              bit 5 is fragment continuation:
 *                  0: last or only packet in fragment
 *                  1: more packets follow, continuing data
 * 		ep 31: reserved for link control management
 * 		ep 0-30: available
 *
 * Controller management requests: 1 byte + data 
 *	1byte:  command
 * 	nbytes: data
 */

#define EP_DIR_OUT	(0 << 7)
#define EP_DIR_IN	(1 << 7)
#define EP_DIR_MASK	(1 << 7)

#define EP_PHASE_DATA	(0 << 6)
#define EP_PHASE_SETUP	(1 << 6)
#define EP_PHASE_MASK	(1 << 6)

#define EP_FRAG_LAST	(0 << 5)
#define EP_FRAG_MORE	(1 << 5)
#define EP_FRAG_MASK	(1 << 5)

#define EP_NUM_MASK	0x1f
#define EP_LINK_CONTROL	0x1f

#define MAX_EP_NUMBER	5
#define MAX_EP_NEEDED	(MAX_EP_NUMBER * 2 + 2) // 2*EP and EP0 IN/OUT

#define ep_to_epnum(ep)	((ep) & EP_NUM_MASK)
#define ep_to_epdir(ep)	((ep) & EP_DIR_MASK)
#define ep_to_epindex(ep) ((2 * ep_to_epnum((ep))) + (ep_to_epdir((ep)) ? 0 :  1))

enum {
	// Host to Device
	kCommand_LinkReady 		= 0x01,		// 4 bytes: nonce
	kCommand_HostConnect 		= 0x02,		// no data
	kCommand_HostDisconnect 	= 0x03,		// no data
	kCommand_DeviceBusReset 	= 0x04,		// no data
	kCommand_DeviceConnectionSpeed 	= 0x05,		// 1 byte: FS or HS

	// Device to Host
	kCommand_LinkReadyAck 		= 0x81,		// 4 bytes: repeat nonce from LinkReady command
	kCommand_DeviceConnect 		= 0x82,		// no data
	kCommand_DeviceDisconnect 	= 0x83,		// no data
	kCommand_ConfigureEndpoint 	= 0x84,		// 2 bytes: ep# (msb is direction), XXX config data to be defined 
	kCommand_EnableEndpoint 	= 0x85,		// 2 byte: ep#, enable (1) or disable (0)
	kCommand_StallEndpoint 		= 0x86,		// 2 byte: ep#, set (1) or clear (0) stall
	kCommand_Error			= 0xff		// 1+ bytes: code, data
};

enum {
	kError_OK			= 0x00,		// No error
	kError_LinkNotReady		= 0x01,		// Need LinkReady first
};

#define MAX_PAYLOAD_SIZE	(512)

enum {
	kEndpointState_Free 			= 0,
	kEndpointState_Allocated_Idle		= (1 << 0),
	kEndpointState_Allocated_Busy		= (1 << 1),
	kEndpointState_Allocated_AbortRequested	= (1 << 2),
	kEndpointState_Allocated_Stalled	= (1 << 3),
};

struct usb_endpoint_instance
{
	uint32_t			endpoint_address;
	uint32_t 			max_packet_size;
	uint32_t 			attributes;
	uint32_t 			bInterval;

	uint32_t			state;
	struct task_event		idle_state_event;
	uint32_t			remote_available;
	
	struct usb_device_io_request *io_request;
};

static void usb_dockfifo_stall_endpoint(uint32_t endpoint, bool stall);

static bool usb_dockfifo_link_ready;
static uint8_t usb_connection_speed;
static bool usb_ep0_data_phase_expected;
static struct usb_endpoint_instance usb_dockfifo_endpoints[MAX_EP_NEEDED];
static struct task *usb_dockfifo_main_task_handle;
static struct task *usb_dockfifo_writer_task_handle;
static struct task_event usb_dockfifo_writer_task_event;
static uint8_t usb_dockfifo_read_buffer_data[MAX_PAYLOAD_SIZE + 1];
static size_t usb_dockfifo_read_buffer_bytes;

//=======================
// Local funtions
//=======================

static void dump_bytes(uint8_t *buffer, size_t size)
{
	int32_t i;

	dprintf(DEBUG_INFO, "===\n");
	for(i = 0; i < (int32_t)size; i++) {
		dprintf(DEBUG_INFO, "%02x ", buffer[i]);
	}
	dprintf(DEBUG_INFO, "\n===\n");
}

static int32_t usb_dockfifo_read_data()
{
	static int frame_count = 0;
	int32_t ret = dockfifo_bulk_read_frame(
		usb_dockfifo_read_buffer_data,
		sizeof(usb_dockfifo_read_buffer_data),
		&usb_dockfifo_read_buffer_bytes);
	if ((ret != 0) || (usb_dockfifo_read_buffer_bytes < 1)) {
		dprintf(DEBUG_INFO, "failed to read frame: %d\n", ret);
		return ret;
	}
	dprintf(DEBUG_SPEW, "Receive frame %d\n", ++frame_count);
	return 0;
}

static int32_t usb_dockfifo_send_bytes(const uint8_t *data, size_t size)
{
	static int frame_count = 0;
	dprintf(DEBUG_SPEW, "Send frame %d\n", ++frame_count);
	int32_t ret = dockfifo_bulk_write_frame(data, size);
	if (ret != 0) {
		dprintf(DEBUG_INFO, "failed to write frame: %d\n", ret);
		return ret;
	}
	return 0;
}

static void usb_dockfifo_go_on_bus(bool on)
{
	uint8_t data[2];

	data[0] = EP_LINK_CONTROL | EP_DIR_IN;
	if (on)
		data[1] = kCommand_DeviceConnect;
	else
		data[1] = kCommand_DeviceDisconnect;
	usb_dockfifo_send_bytes(data, sizeof(data));
}

static void usb_dockfifo_send_error(uint8_t error_code)
{
	dprintf(DEBUG_INFO, "Responding with error 0x%02x\n", error_code);
	
	uint8_t data[3] = {
		EP_LINK_CONTROL | EP_DIR_IN,
		kCommand_Error,
		error_code
	};
	usb_dockfifo_send_bytes(data, sizeof(data));
}

static void usb_dockfifo_send_available(uint8_t epnum)
{
	dprintf(DEBUG_SPEW, "ep%02xOUT available\n", epnum);
	
	uint8_t data[3] = {
		epnum | EP_DIR_OUT,
		MAX_PAYLOAD_SIZE & 0xff,
		(MAX_PAYLOAD_SIZE >> 8) & 0xff
	};
	usb_dockfifo_send_bytes(data, sizeof(data));
}

static void usb_dockfifo_handle_ready(void)
{
	dprintf(DEBUG_SPEW, "Got READY packet. Replying.\n");

	// Send ReadyAck.
	uint8_t data[8];
	data[0] = EP_LINK_CONTROL | EP_DIR_IN;
	data[1] = kCommand_LinkReadyAck;
	// Ping back with the same nonce value we received.
	memcpy(data + 2, usb_dockfifo_read_buffer_data + 2, 4);
	// Inform the host about the max packet size (not including
	// endpoint byte) we can receive.
	ASSERT(DOCKFIFO_BULK_MRU >= MAX_PAYLOAD_SIZE + 1);
	write_le_16(data, 6, MAX_PAYLOAD_SIZE);

	usb_dockfifo_send_bytes(data, sizeof(data));

	// Accept other packets.
	usb_dockfifo_link_ready = true;
}

static void usb_dockfifo_handle_link_control(void)
{
	// Handle commands from the host.
	if (usb_dockfifo_read_buffer_bytes < 2) {
		dprintf(DEBUG_INFO,
			"Expected LinkControl bytes >= 2, got %d\n",
			(int) usb_dockfifo_read_buffer_bytes);
		return;
	}
	uint8_t command = usb_dockfifo_read_buffer_data[1];
	if ((!usb_dockfifo_link_ready) && (command != kCommand_LinkReady)) {
		dprintf(DEBUG_INFO, "Require Ready command first, got 0x%02x\n",
			command);
		// Tell host it needs to reset its session.
		usb_dockfifo_send_error(kError_LinkNotReady);
		return;
	}

	switch (command) {
	case kCommand_LinkReady:
		dprintf(DEBUG_INFO, "Got Ready command\n");
		usb_dockfifo_handle_ready();
		break;

	case kCommand_HostConnect:
	{
		dprintf(DEBUG_INFO, "Got HostConnected command\n");
		usb_core_event_handler(CABLE_CONNECTED);
		usb_dockfifo_go_on_bus(true);
		break;
	}

	case kCommand_DeviceBusReset:
		dprintf(DEBUG_INFO, "Got DeviceBusReset command\n");
		usb_core_event_handler(USB_RESET);
		break;

	case kCommand_DeviceConnectionSpeed:
		dprintf(DEBUG_INFO, "Got DeviceConnectionSpeed command\n");
		if (usb_dockfifo_read_buffer_bytes < 3) {
			dprintf(DEBUG_INFO,
				"Expected 3 bytes for DeviceConnectionSpeed\n");
			break;
		}
		usb_connection_speed = usb_dockfifo_read_buffer_data[2] ?
			CONNECTION_SPEED_HIGH : CONNECTION_SPEED_FULL;
		usb_core_event_handler(USB_ENUM_DONE);
		break;

	case kCommand_HostDisconnect:
		dprintf(DEBUG_INFO, "Got HostDisconnected command\n");
		usb_core_event_handler(CABLE_DISCONNECTED);
		usb_dockfifo_go_on_bus(false);
		break;

	default:
		dprintf(DEBUG_INFO, "Unknown command 0x%02x\n", command);
	}
}

static void usb_dockfifo_handle_endpoint_out(uint32_t endpoint, const uint8_t *buffer, size_t bytes)
{
	dprintf(DEBUG_SPEW, "handle epout %02x %d\n", endpoint, (int) bytes);
	// Get endpoint.
	size_t epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	uint32_t ep_num = ep_to_epnum(endpoint);
	ASSERT((ep_num <= EP_NUM_MASK) && (ep_num != EP_LINK_CONTROL));
	struct usb_endpoint_instance *ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	if ((ep->io_request == NULL) || (ep->state & kEndpointState_Allocated_AbortRequested)) {
		dprintf(DEBUG_INFO, "Dropping unexpected data received for endpoint %x, state:%d io_request:%p\n", endpoint, ep->state, ep->io_request);
		return;
	}

	// Get IO request, transfer size.
	struct usb_device_io_request *req = ep->io_request;
	ASSERT(req != NULL);
	uint32_t xfer_size = req->io_length - req->return_count;

	// Make sure we have space left to copy this data
	RELEASE_ASSERT(bytes <= xfer_size);

	// Save data received
	memcpy((void *) req->io_buffer + req->return_count, buffer, bytes);

	// Update IO request.
	req->return_count += bytes;

	dprintf(DEBUG_SPEW, "ep%02xout now %d/%d\n",
		ep_num, (int) req->return_count, (int) req->io_length);

	if ((bytes == ep->max_packet_size) &&
	    (req->return_count < req->io_length) &&
	    ((ep->state & kEndpointState_Allocated_AbortRequested) == 0)) {
		// More data to be received
		dprintf(DEBUG_SPEW, "More to be received\n");
		usb_dockfifo_send_available(ep_to_epnum(req->endpoint));
		return;
	} else {
		// Completed transaction, update status and return
		dprintf(DEBUG_SPEW, "Completed a transaction\n");
		ep->io_request = NULL;
		uint32_t old_state = ep->state;
		ep->state = kEndpointState_Allocated_Idle;
		event_signal(&ep->idle_state_event);
		if (!(old_state & kEndpointState_Allocated_AbortRequested)) {
			req->status = USB_IO_SUCCESS;
	        	usb_core_complete_endpoint_io(req);
		}
	}
}

static void usb_dockfifo_start_endpoint_in(uint32_t endpoint)
{
	static uint8_t buf[MAX_PAYLOAD_SIZE + 1];

	// Get endpoint.
	size_t epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	uint32_t ep_num = ep_to_epnum(endpoint);
	ASSERT((ep_num <= EP_NUM_MASK) && (ep_num != EP_LINK_CONTROL));
	struct usb_endpoint_instance *ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	// Get IO request, transfer size.
	struct usb_device_io_request *req = ep->io_request;
	ASSERT(req != NULL);
	uint32_t xfer_size = req->io_length - req->return_count;

	// Fragment the request if it exceeds max buffer size.
	bool fragment = false;
	if (xfer_size > MAX_PAYLOAD_SIZE) {
		xfer_size = MAX_PAYLOAD_SIZE;
		fragment = true;
	}

	// Should be some remote buffer space available before this function
	// was invoked.
	ASSERT(ep->remote_available > 0);
	if (xfer_size > ep->remote_available) {
		xfer_size = ep->remote_available;
		fragment = true;
	}
	ep->remote_available = 0;
	
	// Encode the endpoint and transfer flags.
	uint8_t encoded_ep = ep_num | EP_DIR_IN;
	if (fragment) {
		encoded_ep |= EP_FRAG_MORE;
	}

	dprintf(DEBUG_SPEW, "ep_num:0x%02x sent:%d io_len:%d xfer:%d frag:%d\n",
		ep_num,
		(int) req->return_count,
		(int) req->io_length,
		(int) xfer_size,
		(int) fragment);

	// Send encoded endpoint|flags, followed by payload.
	buf[0] = encoded_ep;
	memcpy(buf + 1, (void *) req->io_buffer + req->return_count, xfer_size);
	usb_dockfifo_send_bytes(buf, xfer_size + 1);
	dprintf(DEBUG_SPEW, "Sent\n");

	// Update IO request.
	req->return_count += xfer_size;

	if (fragment && ((ep->state & kEndpointState_Allocated_AbortRequested) == 0)) {
		// More data needs transmitting for this packet.
		event_signal(&usb_dockfifo_writer_task_event);
	} else {
		// Completed this packet, update status and return
		uint32_t old_state = ep->state;
		ep->state = kEndpointState_Allocated_Idle;
		event_signal(&ep->idle_state_event);
		if (!(old_state & kEndpointState_Allocated_AbortRequested)) {
			req->status = USB_IO_SUCCESS;
			ep->io_request = NULL;
	        	usb_core_complete_endpoint_io(req);
		}
	}
}

static bool usb_dockfifo_handle_ep0_data(uint8_t *buf, size_t bytes,
					bool *data_phase)
{
	// Fragment into EP0_MAX_PACKET_SIZE chunks.
	while (bytes > 0) {
		size_t to_receive = EP0_MAX_PACKET_SIZE;
		if (to_receive > bytes) {
			to_receive = bytes;
		}
		usb_core_handle_usb_control_receive(buf, false, to_receive,
						    data_phase);
		buf += to_receive;
		bytes -= to_receive;
		if (bytes > 0 && !*data_phase) {
			dprintf(DEBUG_INFO, "Began setup phase in fragment\n");
			return false;
		}
	}
	return true;
}

static bool usb_dockfifo_dispatch_ep_out(void)
{
	ASSERT(usb_dockfifo_read_buffer_bytes >= 1);

	uint8_t endpoint = usb_dockfifo_read_buffer_data[0];
	ASSERT((endpoint & EP_DIR_MASK) == EP_DIR_OUT);

	switch (endpoint & ~EP_FRAG_MASK) {
	case EP_LINK_CONTROL | EP_DIR_OUT:
		usb_dockfifo_handle_link_control();
		return true;

	case 0 | EP_PHASE_SETUP | EP_DIR_OUT:
		dprintf(DEBUG_SPEW, "Setup packet received\n");
		if (usb_dockfifo_read_buffer_bytes < 1 + SETUP_PACKET_LEN) {
			dprintf(DEBUG_INFO,
				"Setup packet too small: %d\n",
				(int) usb_dockfifo_read_buffer_bytes);
			return false;
		}
		if (usb_dockfifo_read_buffer_bytes > 1 + EP0_MAX_PACKET_SIZE) {
			dprintf(DEBUG_INFO,
				"Setup packet too big: %d\n",
				(int) usb_dockfifo_read_buffer_bytes);
			return false;
		}
		// auto-clear stall if endpoint was previously stalled
		usb_dockfifo_stall_endpoint(0x80, false);
		usb_ep0_data_phase_expected = false;
		usb_core_handle_usb_control_receive(
			usb_dockfifo_read_buffer_data + 1, true,
			SETUP_PACKET_LEN,
			&usb_ep0_data_phase_expected);
		return true;
                               
	case 0 | EP_PHASE_DATA | EP_DIR_OUT:
		dprintf(DEBUG_SPEW, "Data phase for setup packet\n");
		usb_ep0_data_phase_expected = false;
		if (!usb_dockfifo_handle_ep0_data(
			usb_dockfifo_read_buffer_data + 1,
			usb_dockfifo_read_buffer_bytes - 1,
			&usb_ep0_data_phase_expected)) {
			return false;
		}
		return true;

	default:
		usb_dockfifo_handle_endpoint_out(
			(endpoint & ~EP_FRAG_MASK), 
			(usb_dockfifo_read_buffer_data + 1), 
			(usb_dockfifo_read_buffer_bytes - 1));
		return true;
	}
}

static bool usb_dockfifo_dispatch_ep_in(void)
{
	// Update remote_available for this endpoint.

	ASSERT(usb_dockfifo_read_buffer_bytes >= 1);
	uint8_t endpoint = usb_dockfifo_read_buffer_data[0];
	ASSERT((endpoint & EP_DIR_MASK) == EP_DIR_IN);
	uint32_t ep_num = ep_to_epnum(endpoint);
	ASSERT((ep_num <= EP_NUM_MASK) && (ep_num != EP_LINK_CONTROL));
	size_t epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	struct usb_endpoint_instance *ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);
       
	if (usb_dockfifo_read_buffer_bytes < 3) {
		dprintf(DEBUG_INFO, "Runt EPIN packet\n");
		return false;
	}

	uint16_t available = 0;
	memcpy(&available, &usb_dockfifo_read_buffer_data[1],
	       sizeof(available));
	ep->remote_available = available;
	dprintf(DEBUG_SPEW, "ep%02x remote_available %d\n",
		(int) ep_num, (int) available);
	
	if (available > 0) {
		// Writer task may have been waiting to send data.
		event_signal(&usb_dockfifo_writer_task_event);          
	}
	
	return true;
}

static void usb_dockfifo_main_task_session(void)
{
	dprintf(DEBUG_INFO, "DockFifo USB session started\n");

	// Reset USB state.
	usb_core_event_handler(USB_RESET);

	// Require a Ready packet first.
	usb_dockfifo_link_ready = false;

	// Wait for DOCK_CONNECT.
	while (platform_get_dock_connect() == false) {
		dprintf(DEBUG_INFO, "Waiting for dock connect\n");
	 	task_sleep(100*1000);
	}

	// Loop while in a good state. Breaking out of the loop breaks
	// the link until a new Ready packet is received.
	for (;;) {
		dprintf(DEBUG_SPEW, "main loop\n");
		// Read a packet.
		usb_dockfifo_read_data();
		if (usb_dockfifo_read_buffer_bytes < 1) {
			dprintf(DEBUG_INFO, "Runt packet\n");
			break;
		}

		// Get endpoint number.
		uint8_t endpoint = usb_dockfifo_read_buffer_data[0];
		dprintf(DEBUG_SPEW, "Got EP%s%02x %s %s %d\n",
			(endpoint & EP_DIR_MASK) == EP_DIR_OUT ? "OUT" : "IN",
			endpoint & EP_NUM_MASK,
			(endpoint & EP_PHASE_MASK) == EP_PHASE_DATA ?
			"data" : "setup",
			(endpoint & EP_FRAG_MASK) == EP_FRAG_MORE ?
			"frag" : "no frag",
			(int) (usb_dockfifo_read_buffer_bytes - 1));

		// If we're not Link-Ready yet, any packet other than
		// LinkReady will be responded with an error, but we
		// don't need to exit the loop.
		if (!usb_dockfifo_link_ready &&
		    endpoint != (EP_LINK_CONTROL | EP_DIR_OUT)) {
			dprintf(DEBUG_INFO, "Non-link packet before Ready\n");
			// Tell host it needs to reset its session.
			usb_dockfifo_send_error(kError_LinkNotReady);
			continue;
		}

		// Dispatch based on endpoint.
		bool ok;
		if ((endpoint & EP_DIR_MASK) == EP_DIR_OUT) {
			ok = usb_dockfifo_dispatch_ep_out();
		} else {
			ok = usb_dockfifo_dispatch_ep_in();
		}
		if (!ok) {
			break;
		}
	}

	dprintf(DEBUG_INFO, "dockfifo USB session ended\n");
}

static int usb_dockfifo_main_task(void *args)
{
	for (;;) {
		// Keep re-trying sessions. If there are protocol
		// errors, restart from scratch.
		usb_dockfifo_main_task_session();
	}
}

static int usb_dockfifo_writer_task(void *args)
{
	for (;;) {
		static int32_t current_ep_in;

		event_wait(&usb_dockfifo_writer_task_event);

		// walk through IN endpoints for any pending io's, if nothing pending wait for more work
		for (current_ep_in = 0; current_ep_in < MAX_EP_NEEDED; current_ep_in += 2) {
			if ((usb_dockfifo_endpoints[current_ep_in].state == kEndpointState_Allocated_Busy) && 
			    (usb_dockfifo_endpoints[current_ep_in].io_request != NULL) &&
			    (usb_dockfifo_endpoints[current_ep_in].remote_available > 0))
				usb_dockfifo_start_endpoint_in(usb_dockfifo_endpoints[current_ep_in].endpoint_address);
		}
	}

	return 0;
}

static int usb_dockfifo_init()
{
	bzero(usb_dockfifo_endpoints, sizeof(struct usb_endpoint_instance) * MAX_EP_NEEDED);

	// main task to do initial handshake and enumeration
	usb_dockfifo_main_task_handle = task_create("usb_dockfifo main", usb_dockfifo_main_task, NULL, 0x1000);

	// writer task
	usb_dockfifo_writer_task_handle = task_create("usb_dockfifo writer", usb_dockfifo_writer_task, NULL, 0x1000);

	event_init(&usb_dockfifo_writer_task_event, EVENT_FLAG_AUTO_UNSIGNAL, false);

	return 0;
}

static void usb_dockfifo_free()
{
//	dockfifo_bulk_quiesce();
}

static int usb_dockfifo_start()
{
	event_init(&usb_dockfifo_endpoints[ep_to_epindex(EP0_IN)].idle_state_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	usb_dockfifo_endpoints[ep_to_epindex(EP0_IN)].endpoint_address = 0x80;
	usb_dockfifo_endpoints[ep_to_epindex(EP0_IN)].max_packet_size = EP0_MAX_PACKET_SIZE;
	usb_dockfifo_endpoints[ep_to_epindex(EP0_IN)].state = kEndpointState_Allocated_Idle;

	event_init(&usb_dockfifo_endpoints[ep_to_epindex(EP0_OUT)].idle_state_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	usb_dockfifo_endpoints[ep_to_epindex(EP0_OUT)].endpoint_address = 0;
	usb_dockfifo_endpoints[ep_to_epindex(EP0_OUT)].max_packet_size = EP0_MAX_PACKET_SIZE;
	usb_dockfifo_endpoints[ep_to_epindex(EP0_OUT)].state = kEndpointState_Allocated_Idle;

	dockfifo_bulk_init();

	task_start(usb_dockfifo_main_task_handle);
	task_start(usb_dockfifo_writer_task_handle);

	return 0;
}

static void usb_dockfifo_stop()
{
	usb_dockfifo_go_on_bus(false);
}

static void usb_dockfifo_set_address(uint32_t new_address)
{
}

static int usb_dockfifo_get_connection_speed()
{
	return usb_connection_speed;
}

static void usb_dockfifo_activate_endpoint(uint32_t endpoint, int type, int max_packet_size, int interval)
{
	uint32_t epindex;
	struct usb_endpoint_instance *ep;

	enter_critical_section();

	ASSERT(ep_to_epnum(endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	if(ep->state != kEndpointState_Free)
		goto exit;

	ep->endpoint_address = endpoint;
	ep->attributes = type;
	ep->max_packet_size = max_packet_size;
	ep->bInterval = interval;
	event_init(&ep->idle_state_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	ep->state = kEndpointState_Allocated_Idle;

exit:
	exit_critical_section();
}

static void usb_dockfifo_do_endpoint_io(struct usb_device_io_request *req)
{
	struct usb_endpoint_instance *ep;
	uint32_t epindex;

	dprintf(DEBUG_SPEW, "endpoint %02x len %d\n", req->endpoint,
		(int) req->io_length);

	enter_critical_section();

	ASSERT(ep_to_epnum(req->endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(req->endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	req->return_count = 0;
	req->status = USB_IO_ERROR;
	req->next = NULL;

	// make sure endpoint is idle
	if(ep->state != kEndpointState_Allocated_Idle) {
		dprintf(DEBUG_INFO, "endpoint %x is not idle, state:%x\n", req->endpoint, ep->state);
		event_wait(&ep->idle_state_event);
		dprintf(DEBUG_INFO, "endpoint %x got idle state\n",
			req->endpoint);
	}

	// save IO
	RELEASE_ASSERT(ep->io_request == NULL);
	ep->io_request = req;

	// mark endpoint busy
	ep->state = kEndpointState_Allocated_Busy;
	event_unsignal(&ep->idle_state_event);

	// start transmitting/receiving the data
	if (ep_to_epdir(req->endpoint)) {
		dprintf(DEBUG_SPEW, "EP%02x start write len %d\n",
			ep_to_epnum(req->endpoint), (int) req->io_length);
		event_signal(&usb_dockfifo_writer_task_event);
	} else {
		dprintf(DEBUG_SPEW, "EP%02x advertise available\n",
			ep_to_epnum(req->endpoint));
		usb_dockfifo_send_available(ep_to_epnum(req->endpoint));
	}

	exit_critical_section();
}

static void usb_dockfifo_stall_endpoint(uint32_t endpoint, bool stall)
{
	struct usb_endpoint_instance *ep;
	uint32_t epindex;

	// Nothing to do for EP0_OUT
	if(endpoint == 0)
		return;

	enter_critical_section();

	ASSERT(ep_to_epnum(endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	dprintf(DEBUG_SPEW, "stall endpoint %x, state:%x, stall:%d\n", endpoint, ep->state, stall);

	if (stall) {
		// if already stalled, nothing to do
		if ((ep->state & kEndpointState_Allocated_Stalled) != 0)
			return;

		// wait for endpoint to be idle
		if (ep->state != kEndpointState_Allocated_Idle)
			event_wait(&ep->idle_state_event);

		ep->state |= kEndpointState_Allocated_Stalled;
	}
	else {
		// if not stalled, nothing to do
		if ((ep->state & kEndpointState_Allocated_Stalled) == 0)
			return;
	}

	// Notify Dock
	uint8_t data[] = {
		EP_LINK_CONTROL | EP_DIR_IN,
		kCommand_StallEndpoint,
		endpoint,
		stall
	};
	usb_dockfifo_send_bytes(data, sizeof(data));

	if (!stall) {
		ep->state = kEndpointState_Allocated_Idle;
		event_signal(&ep->idle_state_event);
	}

	exit_critical_section();
}

static void usb_dockfifo_reset_endpoint_data_toggle(uint32_t endpoint)
{
}

static bool usb_dockfifo_is_endpoint_stalled(uint32_t endpoint)
{
	struct usb_endpoint_instance *ep;
	uint32_t epindex;

	enter_critical_section();

	ASSERT(ep_to_epnum(endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	dprintf(DEBUG_SPEW, "endpoint %x, state:%x\n", endpoint, ep->state);

	exit_critical_section();

	return ((ep->state & kEndpointState_Allocated_Stalled) != 0);
}

static void usb_dockfifo_do_test_mode(uint32_t selector)
{
}

static void usb_dockfifo_abort_endpoint(uint32_t endpoint)
{
	uint32_t epindex;
	struct usb_endpoint_instance *ep;

	enter_critical_section();

	ASSERT(ep_to_epnum(endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	dprintf(DEBUG_SPEW, "abort endpoint %x, state:%x\n", endpoint, ep->state);

	if (ep->state != kEndpointState_Allocated_Busy)
		goto exit;

	ep->state |= kEndpointState_Allocated_AbortRequested;

	// wait for endpoint to be idle
	if (ep->state != kEndpointState_Allocated_Idle)
		event_wait(&ep->idle_state_event);

	ep->io_request->status = USB_IO_ABORTED;
	usb_core_complete_endpoint_io(ep->io_request);
	ep->io_request = NULL;

	dprintf(DEBUG_SPEW, "abort endpoint %x finished, state:%x\n", endpoint, ep->state);

exit:
	exit_critical_section();
}

static void usb_dockfifo_deactivate_endpoint(uint32_t endpoint)
{
	uint32_t epindex;
	struct usb_endpoint_instance *ep;

	enter_critical_section();

	ASSERT(ep_to_epnum(endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &usb_dockfifo_endpoints[epindex];
	ASSERT(ep != NULL);

	if(ep->state == kEndpointState_Free)
		goto exit;

	usb_dockfifo_abort_endpoint(endpoint);

	bzero(&usb_dockfifo_endpoints[ep_to_epindex(endpoint)], sizeof(struct usb_endpoint_instance));

exit:
	exit_critical_section();
}

static const struct usb_controller_functions usb_dockfifo_controller_functions = {
	.init = usb_dockfifo_init,
	.free = usb_dockfifo_free,
	.start = usb_dockfifo_start,
	.stop = usb_dockfifo_stop,
	.set_address = usb_dockfifo_set_address,
	.get_connection_speed = usb_dockfifo_get_connection_speed,
	.activate_endpoint = usb_dockfifo_activate_endpoint,
	.do_endpoint_io = usb_dockfifo_do_endpoint_io,
	.stall_endpoint = usb_dockfifo_stall_endpoint,
	.reset_endpoint_data_toggle = usb_dockfifo_reset_endpoint_data_toggle,
	.is_endpoint_stalled = usb_dockfifo_is_endpoint_stalled,
	.do_test_mode = usb_dockfifo_do_test_mode,
	.abort_endpoint = usb_dockfifo_abort_endpoint,
	.deactivate_endpoint = usb_dockfifo_deactivate_endpoint,
};

//=======================
// Global funtions
//=======================

const struct usb_controller_functions *usb_dockfifo_controller_init()
{
	return &usb_dockfifo_controller_functions;
}
