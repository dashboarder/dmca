#ifndef _SYNOPSYS_OTG_PUBLIC_H_
#define _SYNOPSYS_OTG_PUBLIC_H_

#include <sys/types.h>
#include <drivers/usb/usb_chap9.h>

__BEGIN_DECLS

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

struct usb_device_io_request
{
	u_int32_t endpoint;
	volatile u_int8_t *io_buffer;
	int status;
	int io_length;
	int return_count;
	void (*callback) (struct usb_device_io_request *io_request);
	struct usb_device_io_request *next;
};

struct usb_endpoint_instance
{
	int		endpoint_address;
	int 		max_packet_size;
	int 		attributes;
	int 		bInterval;
	
	int 		transfer_size;
	int 		packet_count;
    
	struct usb_endpoint_instance *next_ep;
    
	struct usb_device_io_request *io_head;
	struct usb_device_io_request *io_tail;
    
	int		tx_fifo_number;
};

struct usb_controller_ops
{
	void (*start) (void);
	void (*stop) (void);
	void (*set_address) (int new_address);
	void (*stall_endpoint) (u_int32_t endpoint, bool stall);
	void (*reset_endpoint_data_toggle) (u_int32_t endpoint);
	bool (*is_endpoint_stalled) (u_int32_t endpoint);
	void (*do_endpoint_io) (struct usb_device_io_request *req);
	void (*activate_endpoint) (u_int32_t endpoint, int type, int max_packet_size, int interval);
	void (*deactivate_endpoint) (u_int32_t endpoint);
	void (*abort_endpoint) (u_int32_t endpoint);
	void (*do_test_mode)(int test_selector);
	int (*get_connection_speed) (void);
};

struct usb_interface_instance
{
	int (*interface_request_handler) (struct usb_device_request *request, u_int8_t ** out_data);
	void (*non_setup_data_phase_finished_callback) (int data_rcvd);
	void (*activate_interface) (void);
	void (*bus_reset_handler) (void);
	int (*get_interface_handler) (void);
	int (*set_interface_handler) (int alt_setting);
};

//===================================================================================
//		Global APIs
//===================================================================================

void usb_init (void);

__END_DECLS

#endif
