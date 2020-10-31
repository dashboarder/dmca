/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef _USB_CHAP9_H_
#define _USB_CHAP9_H_

#include <sys/types.h>

//====================================================================
// USB Descriptor Types
//====================================================================
#define USB_DT_DEVICE				1
#define USB_DT_CONFIGURATION                    2
#define USB_DT_STRING                           3
#define USB_DT_INTERFACE                        4
#define USB_DT_ENDPOINT                         5
#define USB_DT_DEVICE_QUALIFIER                 6
#define USB_DT_OTHER_SPEED_CONFIGURATION	7

//====================================================================
// USB Endpoints Types
//====================================================================
#define USB_ENDPOINT_CONTROL            0x00
#define USB_ENDPOINT_ISOCHRONOUS        0x01
#define USB_ENDPOINT_BULK               0x02
#define USB_ENDPOINT_INTERRUPT          0x03

//====================================================================
// USB Standard Request Types
//====================================================================
#define USB_REQ_GET_STATUS              0x00
#define USB_REQ_CLEAR_FEATURE           0x01
#define USB_REQ_SET_FEATURE             0x03
#define USB_REQ_SET_ADDRESS             0x05
#define USB_REQ_GET_DESCRIPTOR          0x06
#define USB_REQ_SET_DESCRIPTOR          0x07
#define USB_REQ_GET_CONFIGURATION       0x08
#define USB_REQ_SET_CONFIGURATION       0x09
#define USB_REQ_GET_INTERFACE           0x0A
#define USB_REQ_SET_INTERFACE           0x0B

//====================================================================
// USB Feature Types
//====================================================================
#define USB_FEATURE_ENDPOINT_HALT		0x00
#define USB_FEATURE_REMOTE_WAKEUP		0x01
#define USB_FEATURE_TEST_MODE			0x02

//====================================================================
// USB Device Request Types
//====================================================================
#define USB_REQ_TYPE_STANDARD           0x00
#define USB_REQ_TYPE_CLASS              0x20
#define USB_REQ_TYPE_VENDOR             0x40

//====================================================================
// USB Class Codes
//====================================================================
#define USB_CLASS_INTERFACE_SPECIFIC         0
#define USB_CLASS_VENDOR_SPECIFIC            0xff

//====================================================================
// 		USB Chap9 specific consts
//====================================================================
#define USB_DT_DEVICE_SIZE		18
#define USB_DT_CONFIGURATION_SIZE      	9
#define USB_DT_INTERFACE_SIZE   	9
#define USB_DT_ENDPOINT_SIZE    	7
#define USB_DT_DEVICE_QUALIFIER_SIZE    10
#define USB_DT_STRING_SIZE		6

#define USB_DIR_OUT                     0
#define USB_DIR_IN                      0x80

#define USB_ENDPOINT_MASK               0x03
#define USB_ENDPOINT_NUMBER_MASK        0x0f
#define USB_ENDPOINT_DIR_MASK           0x80

#define USB_BCD_VERSION			0x0200

#define USB_REQ_DIRECTION_MASK          0x80
#define USB_REQ_TYPE_MASK               0x60
#define USB_REQ_RECIPIENT_MASK          0x1f

#define USB_REQ_DEVICE2HOST             0x80
#define USB_REQ_HOST2DEVICE             0x00

#define USB_REQ_RECIPIENT_DEVICE        0x00
#define USB_REQ_RECIPIENT_INTERFACE     0x01
#define USB_REQ_RECIPIENT_ENDPOINT      0x02
#define USB_REQ_RECIPIENT_OTHER         0x03

#define SETUP_PACKET_LEN	0x8

#define EP0_IN		0x80
#define EP0_OUT		0x0
//====================================================================
// 		Enums, structs and typedefs
//====================================================================
struct usb_device_request
{
    u_int8_t bmRequestType;
    u_int8_t bRequest;
    u_int16_t wValue;
    u_int16_t wIndex;
    u_int16_t wLength;
} __attribute__((packed));

struct usb_device_descriptor
{
    u_int8_t bLength;
    u_int8_t bDescriptorType;
    u_int16_t bcdUSB;
    u_int8_t bDeviceClass;
    u_int8_t bDeviceSubClass;
    u_int8_t bDeviceProtocol;
    u_int8_t bMaxPacketSize0;
    u_int16_t idVendor;
    u_int16_t idProduct;
    u_int16_t bcdDevice;
    u_int8_t iManufacturer;
    u_int8_t iProduct;
    u_int8_t iSerialNumber;
    u_int8_t bNumConfigurations;
} __attribute__((packed));

struct usb_configuration_descriptor
{
    u_int8_t bLength;
    u_int8_t bDescriptorType;
    u_int16_t wTotalLength;
    u_int8_t bNumInterfaces;
    u_int8_t bConfigurationValue;
    u_int8_t iConfiguration;
    u_int8_t bmAttributes;
    u_int8_t bMaxPower;
} __attribute__((packed));

struct usb_interface_descriptor
{
    u_int8_t bLength;
    u_int8_t bDescriptorType;
    u_int8_t bInterfaceNumber;
    u_int8_t bAlternateSetting;
    u_int8_t bNumEndpoints;
    u_int8_t bInterfaceClass;
    u_int8_t bInterfaceSubClass;
    u_int8_t bInterfaceProtocol;
    u_int8_t iInterface;
} __attribute__((packed));

struct usb_endpoint_descriptor
{
    u_int8_t bLength;
    u_int8_t bDescriptorType;
    u_int8_t bEndpointAddress;
    u_int8_t bmAttributes;
    u_int16_t wMaxPacketSize;
    u_int8_t bInterval;
} __attribute__((packed));

struct usb_string_descriptor
{
    u_int8_t bLength;
    u_int8_t bDescriptorType;
    u_int8_t wData[2];
} __attribute__((packed));

struct usb_device_qualifier_descriptor
{
    u_int8_t bLength;
    u_int8_t bDescriptorType;
    u_int16_t bcdUSB;
    u_int8_t bDeviceClass;
    u_int8_t bDeviceSubClass;
    u_int8_t bDeviceProtocol;
    u_int8_t bMaxPacketSize0;
    u_int8_t bNumConfigurations;
    u_int8_t bReserved;
} __attribute__((packed));

enum
{
	DEVICE_ST_UNKNOWN = -1,
	DEVICE_ST_INIT = 0,
    DEVICE_ST_ATTACHED,
    DEVICE_ST_POWERED,
    DEVICE_ST_DEFAULT,
    DEVICE_ST_ADDRESSED,
    DEVICE_ST_CONFIGURED,
    DEVICE_ST_SUSPENDED
};

//====================================================================
// 		USB driver specific consts
//====================================================================
#define EP0_MAX_PACKET_SIZE	0x40
#define HS_BULK_EP_MAX_PACKET_SIZE	0x200
#define HS_INT_EP_MAX_PACKET_SIZE	0x40
#define FS_EP_MAX_PACKET_SIZE		0x40

#endif // _USB_CHAP9_H_


