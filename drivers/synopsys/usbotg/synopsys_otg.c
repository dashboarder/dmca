/*
 * Copyright (C) 2007-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <platform.h>
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/power.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <platform/usbconfig.h>
#include <sys/task.h>
#include <sys.h>
#include <debug.h>
#include <drivers/usbphy.h>
#if WITH_HW_POWER
#include <drivers/power.h>
#endif

#include "synopsys_otg_regs.h"
#include <drivers/usb/usb_chap9.h>
#include <drivers/usb/usb_public.h>
#include <drivers/usb/usb_controller.h>
#include <drivers/usb/usb_core.h>

//===================================================================================
//		typedefs, structs
//===================================================================================

struct usb_endpoint_instance
{
	u_int32_t	endpoint_address;
	u_int32_t 	max_packet_size;
	u_int32_t 	attributes;
	u_int32_t 	bInterval;
	
	u_int32_t 	interrupt_status;
	
	u_int32_t 	transfer_size;
	u_int32_t 	packet_count;
	bool		is_active;
	
	struct usb_endpoint_instance *next_ep;
	
	struct usb_device_io_request *io_head;
	struct usb_device_io_request *io_tail;
    
	struct usb_device_io_request *completion_head;
	struct usb_device_io_request *completion_tail;
    
	int	 tx_fifo_number;
};


//===================================================================================
//		Local macros and consts
//===================================================================================
#define EP_INT_MASK			0x1
#define MAX_EP0_DMA_SIZE	0x40
#define MAX_EP_NEEDED		12 	// MAX_EP_NUMBER * 2 + EP0 IN/OUT
#define MAX_EP_NUMBER		5
#define MAX_EPn_DMA_SIZE	0x7ffff
#define MAX_TX_FIFO_SUPPORTED	15

#define EP_NUM_MASK		(0x7f)
#define EP_DIR_MASK		(0x80)

#define USB_AHBIDLE_TIMEOUT	(5 * 1000 * 1000)
#define USB_SFTRST_TIMEOUT	(5 * 1000 * 1000)

#define DEBUG 0

#define print(fmt, args...) (void)0

#if (DEBUG == 1)
#undef print
#define print(fmt, args...) dprintf(DEBUG_INFO, "%s --- " fmt, __FUNCTION__, ##args)
#endif

#if (DEBUG == 2)
#undef print
#define print(fmt, args...) usb_print_message_var("%s --- " fmt, __FUNCTION__, ##args)
#endif

#define ep_to_epnum(ep) ((ep) & EP_NUM_MASK)
#define ep_to_epdir(ep) ((ep) & EP_DIR_MASK)
#define ep_to_epindex(ep) ((2 * ep_to_epnum((ep))) + (ep_to_epdir((ep)) ? 0 :  1))

#define DTKNQR1_INTKNWPTR_OFFSET        2
#define DTKNQR_INTKNWPTR_WIDTH          4
#define DTKNQR_TOTAL_TOKENS             (32 / DTKNQR_INTKNWPTR_WIDTH)

#define dtknqr_eptkn_get(x, off)        (((x) & (0xF << ((off) * DTKNQR_INTKNWPTR_WIDTH))) >> ((off) * DTKNQR_INTKNWPTR_WIDTH))

#define SHARED_FIFO_NP_TX_FIFO_NUM      0

#define EPDISBLD_TIMEOUT        (1 * 1000)
#define FIFOFLSH_TIMEOUT        (1 * 1000)
#define NAKEFF_TIMEOUT          (1 * 1000)

enum
{
	// Use different bits for each event type
	EVENT_TYPE_NONE = 0,
	EVENT_TYPE_INTERRUPT = 1,
	EVENT_TYPE_EXIT = 2
};

#define VERSION_300a	0x300a

//===================================================================================
//		Global and local variables
//===================================================================================

static u_int8_t __aligned(CPU_CACHELINE_SIZE) ep0_rx_buffer[EP0_MAX_PACKET_SIZE];
static u_int8_t setup_pkt_buffer[SETUP_PACKET_LEN];
static bool setup_pkt_buffer_valid = false;
static struct usb_endpoint_instance synopsys_otg_endpoints[MAX_EP_NEEDED];
static u_int32_t next_tx_fifo_start_addr = 0;
static u_int8_t tx_fifo_list[MAX_TX_FIFO_SUPPORTED];
static u_int32_t max_dma_size;
static u_int32_t max_packet_count;

static u_int32_t core_version;

static struct task * controller_task = NULL;
static struct task_event usb_task_event;
static struct task_event usb_task_terminate_event;
static u_int32_t usb_task_event_status;
static uint32_t synopsys_otg_gintsts;
static uint32_t synopsys_otg_daintsts;


//===================================================================================
//		Local function prototypes
//===================================================================================
static void synopsys_otg_start_ep0_out(bool data_phase);
static void synopsys_otg_print_endpoints(void);
static void synopsys_otg_start_endpoint_in(u_int32_t endpoint);
static void synopsys_otg_start_endpoint_out(u_int32_t endpoint);
static void synopsys_otg_int_handler(void *blah);
static int set_global_in_nak(void);
static void clear_global_in_nak(void);
static int flush_tx_fifo(int fifo);
static int set_global_out_nak(void);
static void clear_global_out_nak(void);
static int synopsys_otg_stop_endpoint_out(int ep_num);
static int synopsys_otg_stop_endpoint_in(int ep_num);
static void synopsys_otg_disable_in_endpoint(u_int32_t endpoint);
static void synopsys_otg_reset_states(void);
static void enqueue_finished_request(struct usb_endpoint_instance *ep, struct usb_device_io_request *io_req);
static void scavenge_endpoint_finished_list(u_int32_t endpoint);
static void synopsys_otg_go_on_bus(void);

//===================================================================================
//		Logging stuff
//===================================================================================

#if (DEBUG == 2)

#define MAX_LOG_LEN 100

static u_int8_t *log_buffer;
static const int log_buffer_len = (32 * 1024);

static u_int8_t *writeptr;
static u_int8_t *readptr;

void usb_print_message_var(char *fmt, ...)
{
	va_list ap;
	char buf[MAX_LOG_LEN];
	char *bp = buf;
	u_int8_t *end = log_buffer + log_buffer_len;
	
	va_start(ap, fmt);
	vsnprintf(buf, MAX_LOG_LEN, fmt, ap);
	va_end(ap);
	
	while(*bp) {
		*writeptr++ = *bp++;
		
		if(writeptr == end) {
			writeptr = log_buffer;
		}
		
		if(writeptr == readptr) {
			if(++readptr == end) {
				readptr = log_buffer;
			}
		}
	}
}

void usb_print_log_buffer()
{
	u_int8_t *end = log_buffer + log_buffer_len;
	
	while(readptr != writeptr) {
		printf("%c", *readptr++);
		if(readptr == end)
			readptr = log_buffer;
	}
}

#endif

//===================================================================================
//			Local functions
//===================================================================================

static u_int32_t read_reg(addr_t address)
{
	return (*(volatile u_int32_t *)(address));
}


static void write_reg(addr_t address, u_int32_t value)
{
	*(volatile u_int32_t *)(address) = value;
}


static void or_reg(addr_t address, u_int32_t value)
{
	*(volatile u_int32_t *)(address) = read_reg(address) | value;
}


static void and_reg(addr_t address, u_int32_t value)
{
	*(volatile u_int32_t *)(address) = read_reg(address) & value;
}


static void set_bits_in_reg(addr_t address, u_int32_t pos, u_int32_t mask, u_int32_t val)
{
	u_int32_t reg_val = *(volatile u_int32_t *)address;
	
	reg_val &= ~(mask);
	reg_val |= (val << pos);
	
	*(volatile u_int32_t *)address = reg_val;
}


static int get_next_tx_fifo_num(void)
{
	int i;
	
	for(i = 0; i < MAX_TX_FIFO_SUPPORTED; i++) {
		if(tx_fifo_list[i] == 0) {
			tx_fifo_list[i] = 1;
			break;
		}
	}
	
	return(i + 1);
}


static u_int32_t get_rx_fifo_size(void)
{
	return((4 * 1 + 6) + 2 * ((HS_BULK_EP_MAX_PACKET_SIZE / 4) + 8) + 1);
}


static u_int32_t get_np_tx_fifo_size(void)
{
	return((2 * (HS_BULK_EP_MAX_PACKET_SIZE / 4)));
}


static u_int32_t get_ep0_tx_fifo_size(void)
{
	return((2 * (HS_BULK_EP_MAX_PACKET_SIZE / 4)));
}


static u_int32_t get_endpoint_fifo_number(int endpoint)
{
	return (synopsys_otg_endpoints[ep_to_epindex(endpoint)].tx_fifo_number);
}


static void synopsys_otg_print_endpoints(void)
{
	u_int32_t num_dev_eps = 0;
	u_int32_t dir = 0;
	u_int32_t hwcfg1 = 0;
	u_int32_t in_eps = 0;
	u_int32_t out_eps = 0;
	u_int32_t i;
	
	num_dev_eps = ((read_reg(DWCUSB_GHWCFG2) >> DWCUSB_GHWCFG2_NUMDEVEPS_SHIFT) & 0xf);
	hwcfg1 = read_reg(DWCUSB_GHWCFG1);
    
	i = 1;
	do {
		dir = (hwcfg1 >> (i * 2)) & 0x3;
		switch(dir)
		{
			case 0 : /* BI */
				in_eps++;
				out_eps++;
				print("EP %d BIDIR \n", i);
				break;
				
			case 1 : /* IN */
				in_eps++;
				print("EP %d IN \n", i);
				break;
				
			case 2 : /* OUT */
				out_eps++;
				print("EP %d OUT \n", i);
				break;
				
			default :
				/* Bad value */
				dprintf(DEBUG_INFO, "synopsys_otg_controller : synopsys_otg_set_max_endpoints --- Bad Value \n");
				return ;
		}
        
		i++;
		if((in_eps + out_eps) >= num_dev_eps) break;
        
	} while (true);
}


static void synopsys_otg_start_controller(void)
{
	utime_t start_time;
	u_int32_t val;
	u_int32_t USBTrdTim;
	int i;
	
	val = (read_reg(DWCUSB_GHWCFG3) & DWCUSB_GHWCFG3_PKTSIZEWIDTH_MASK) >> DWCUSB_GHWCFG3_PKTSIZEWIDTH_SHIFT;
	max_packet_count = (1 << (val + 4)) - 1;
	val = read_reg(DWCUSB_GHWCFG3) & DWCUSB_GHWCFG3_XFERSIZEWIDTH_MASK;
	max_dma_size = (1 << (val + 11)) - 1;
	
	print("MAX_PKT_CNT: %d, MAX_XFERSIZ_CNT: %d\n", max_packet_count, max_dma_size);
	
	/* Wait for core reset and AHB idle */
	write_reg(DWCUSB_GRSTCTL, DWCUSB_GRSTCTL_CSFTRST);
	
	start_time = system_time();
	while (read_reg(DWCUSB_GRSTCTL) & DWCUSB_GRSTCTL_CSFTRST) {
		if(time_has_elapsed(start_time, USB_SFTRST_TIMEOUT)) {
			panic("USB Core Reset timed out \n");
		}
	}
	
	/* Set the soft disconnect bit */	
	or_reg(DWCUSB_DCTL, DWCUSB_DCTL_SFTDISCON);
	
	start_time = system_time();
	while (!(read_reg(DWCUSB_GRSTCTL) & DWCUSB_GRSTCTL_AHBIDLE)) {
		if(time_has_elapsed(start_time, USB_AHBIDLE_TIMEOUT)) {
			panic("USB AHBIDLE timed out \n");
		}
	}
	
	/* Program GAHBCFG */
	write_reg(DWCUSB_GAHBCFG, DWCUSB_GAHBCFG_DMAEN | USBOTG_AHB_DMA_BURST | DWCUSB_GAHBCFG_GLBLINTRMSK);
	
#ifdef USBCFG_TURNAROUND_TIME              
	USBTrdTim = USBCFG_TURNAROUND_TIME;
#else
	USBTrdTim = 5;	// Use the default
#endif 
	
	/* Program GUSBCFG */
	write_reg(DWCUSB_GUSBCFG, DWCUSB_GUSBCFG_PHYSEL_HS |
              DWCUSB_GUSBCFG_UTMI |
              (USBTrdTim << DWCUSB_GUSBCFG_USBTRDTIM_SHIFT) |
              DWCUSB_GUSBCFG_PHYIF_16 |
              (0 << DWCUSB_GUSBCFG_TOUTCAL_SHIFT));
	
	/* Program DCFG */
	write_reg(DWCUSB_DCFG, (0 << DWCUSB_DCFG_DEVADDR_SHIFT) |
              DWCUSB_DCFG_NZSTSOUTHSHK |
              DWCUSB_DCFG_DEVSPD_HS |
              (0 << DWCUSB_DCFG_EPMISCNT_SHIFT));
	
	/* mask all the interrupts */
	write_reg(DWCUSB_GINTMSK, 0);
	write_reg(DWCUSB_DOEPMSK, 0);
	write_reg(DWCUSB_DIEPMSK, 0);
	write_reg(DWCUSB_DAINTMSK, 0);
	
	// clear any pending interrupts
	write_reg(DWCUSB_GINTSTS, 0xffffffff);
	for(i = MAX_EP_NUMBER; i >= 0; i-- ) {
		write_reg(DWCUSB_DIEPINT(i), 0x1f);
		write_reg(DWCUSB_DOEPINT(i), 0xf);
	}
	
	/* Program GINTMSK */
	write_reg(DWCUSB_GINTMSK, DWCUSB_GINT_USBRST |
              DWCUSB_GINT_ENUMDONE);
              
	/* Read the OTG Core version */
	core_version = read_reg(DWCUSB_GSNPSID) & 0xffff;
}

static void synopsys_otg_reset_states(void)
{
	// reset next tx fifo related variables
	next_tx_fifo_start_addr = get_ep0_tx_fifo_size() + get_rx_fifo_size();
	memset(tx_fifo_list, 0, MAX_TX_FIFO_SUPPORTED);
}

static void synopsys_otg_handle_usb_reset(void)
{
	int i;
    
	synopsys_otg_reset_states();
        
	// reset all device related interrupts mask, and clear any pending device related interrupt
	write_reg(DWCUSB_DOEPMSK, 0);
	write_reg(DWCUSB_DIEPMSK, 0);
	write_reg(DWCUSB_DAINTMSK, 0);
	for(i = MAX_EP_NUMBER; i >= 0; i--) {
		write_reg(DWCUSB_DIEPINT(i), 0x1f);
		write_reg(DWCUSB_DOEPINT(i), 0xf);
	}
	
	// If any of the interfaces did not abort their endpoint, we should do it now.
	// EP0_OUT should not be aborted. EP0_IN is already aborted. Abort all other endpoints.
	for(i = MAX_EP_NUMBER; i > 0; i-- ) {
		usb_core_abort_endpoint(USB_DIR_IN | i);
		usb_core_abort_endpoint(i);
	}

	// reset endpoint instance array
	bzero(synopsys_otg_endpoints, sizeof(struct usb_endpoint_instance) * MAX_EP_NEEDED);
    
	u_int32_t rxFifo = get_rx_fifo_size();
	u_int32_t txFifo0 =  get_np_tx_fifo_size();
    
	write_reg(DWCUSB_GRXFSIZ, rxFifo);
	write_reg(DWCUSB_GNPTXFSIZ, (txFifo0 << DWCUSB_GNPTXFSIZ_NPTXFDEP_SHIFT) | rxFifo);
    
	synopsys_otg_endpoints[ep_to_epindex(EP0_IN)].endpoint_address = 0x80;
	synopsys_otg_endpoints[ep_to_epindex(EP0_IN)].max_packet_size = EP0_MAX_PACKET_SIZE;
	synopsys_otg_endpoints[ep_to_epindex(EP0_IN)].tx_fifo_number = 0;
	synopsys_otg_endpoints[ep_to_epindex(EP0_IN)].is_active = true;
	synopsys_otg_endpoints[ep_to_epindex(EP0_OUT)].endpoint_address = 0;
	synopsys_otg_endpoints[ep_to_epindex(EP0_OUT)].max_packet_size = EP0_MAX_PACKET_SIZE;
	synopsys_otg_endpoints[ep_to_epindex(EP0_OUT)].is_active = true;
    
	/// activate EP0 and setup EP0 OUT
	write_reg(DWCUSB_DOEPCTL(0), DWCUSB_DEP0CTL_MPS_64);
	write_reg(DWCUSB_DIEPCTL(0), (DWCUSB_DEP0CTL_MPS_64));
    
	// enable more interrupts
	or_reg(DWCUSB_GINTMSK, DWCUSB_GINT_USBSUSP |
		   DWCUSB_GINT_WKUPINT |
		   DWCUSB_GINT_OEPINT |
		   DWCUSB_GINT_IEPINT);
    
	write_reg(DWCUSB_DOEPMSK, DWCUSB_DOEPINT_SETUP | DWCUSB_DOEPINT_XFERCOMPL | DWCUSB_DOEPINT_AHBERR);
	write_reg(DWCUSB_DIEPMSK, DWCUSB_DIEPINT_XFERCOMPL | DWCUSB_DIEPINT_TIMEOUT | DWCUSB_DIEPINT_AHBERR);
    
	write_reg(DWCUSB_DAINTMSK, 1 | (1 << DWCUSB_DAINT_OEP_SHIFT));
    
	synopsys_otg_start_ep0_out(false);
}


static void synopsys_otg_handle_enum_done(void)
{
	int is_hs = ((read_reg(DWCUSB_DSTS) & DWCUSB_DSTS_ENUMSPD_MASK) == DWCUSB_DSTS_ENUMSPD_HS);
    
	if(is_hs)
		print("High-speed \n");
	else
		print("Full-speed \n");
    
	usb_core_event_handler(USB_ENUM_DONE);
}


static void synopsys_otg_handle_ep0_out(void)
{
	struct usb_endpoint_instance *ep = &synopsys_otg_endpoints[ep_to_epindex(EP0_OUT)];
	u_int32_t depintsts;
	u_int32_t data_rcvd;
	bool data_phase = false;
	bool is_setup_pkt;
	bool is_data_pkt;
	
	enter_critical_section();
	depintsts = ep->interrupt_status;
	ep->interrupt_status = 0;
	exit_critical_section();
    
	print("DWCUSB_DOEPINT(0) %08x \n", depintsts);
	
	// On older OTG cores (version < 0x300a) DOEPINT:SETUP interrupt is asserred for setup packets and DOEPINT:XFERCOMPL for data packets.
	// As per the definition, the DOEPINT:SETUP bit is set only when the host begins the data / status phase. This means,
	// if the host sends back to back setup packets, we will get an interrupt only for the last setup packet. At this point
	// we are ready to start decoding the setup packet and prepare the endpoint to receive data / status packet.
	//
	// In OTG verision 0x300a, XFERCOMPL is asserted whenever it sees either a setup or data packet. We then
	// use DOEPINT:STUPKTRCVD bit to identify if this is a setup or a data packet. This means, if the host sends 
	// back to back setup packets, we will get one XFERCOMPL interrupt for each of them. It is the responsibility of SW
	// to buffer the setup packet and decode the last packet later when we get DOEPINT:SETUP interrupt which comes
	// when the host begins the data / status phase.
	//
	// In either case, we should wait for the DOEPINT:SETUP interrupt before decoding the setup packet and preparing 
	// the endpoint to receive data / status packet. See radar <rdar://problem/13445651> for more details.
	
	is_setup_pkt = (core_version < VERSION_300a) ? (depintsts & DWCUSB_DOEPINT_SETUP) : (depintsts & DWCUSB_DOEPINT_STUPKTRCVD);
	is_data_pkt = (!is_setup_pkt && (depintsts & DWCUSB_DOEPINT_XFERCOMPL));
	
	// If we got a setup packet copy the packet into a temporary buffer. If there are back to back setup packets, this buffer
	// stores the latest setup packet
	if(is_setup_pkt) {
		memcpy(setup_pkt_buffer, ep0_rx_buffer, SETUP_PACKET_LEN);
		setup_pkt_buffer_valid = true;
	}
	
	// If there is a DOEPINT:SETUP interrupt, we should process the setup packet
	if((depintsts & DWCUSB_DOEPINT_SETUP) && setup_pkt_buffer_valid) {
		usb_core_handle_usb_control_receive(setup_pkt_buffer, true, SETUP_PACKET_LEN, &data_phase);
		setup_pkt_buffer_valid = false;
	}
	else if(is_data_pkt) {
		// Handle the data packet
		data_rcvd = EP0_MAX_PACKET_SIZE - (read_reg(DWCUSB_DOEPTSIZ(0)) & DWCUSB_DEP0TSIZ_XFERSIZE_MASK);
		usb_core_handle_usb_control_receive(ep0_rx_buffer, false, data_rcvd, &data_phase);
	}

	synopsys_otg_start_ep0_out(data_phase);
}


static void synopsys_otg_start_ep0_out(bool data_phase)
{
	u_int32_t val;
    
	write_reg(DWCUSB_DOEPTSIZ(0), (1 << DWCUSB_DEPTSIZ_SUPCNT_SHIFT) |
			  (1 << DWCUSB_DEPTSIZ_PKTCNT_SHIFT) |
			  (EP0_MAX_PACKET_SIZE));
    
	platform_cache_operation(CACHE_INVALIDATE | CACHE_CLEAN, (void *)ep0_rx_buffer, EP0_MAX_PACKET_SIZE);
    
	write_reg(DWCUSB_DOEPDMA(0), (uint32_t)ep0_rx_buffer);
    
	val = DWCUSB_DEPCTL_EPENA;
    
	if(data_phase) {
		val |= DWCUSB_DEPCTL_CNAK;
	}
    
	or_reg(DWCUSB_DOEPCTL(0), val);
    
	print("EP0 OUT started \n");
}


static void synopsys_otg_handle_endpoint_out(u_int32_t endpoint)
{
	struct usb_device_io_request *finished_req;
	u_int32_t ep_num = ep_to_epnum(endpoint);
	struct usb_endpoint_instance *ep;
	u_int32_t depintsts;
	
	ep = &synopsys_otg_endpoints[ep_to_epindex(endpoint)];
	depintsts = read_reg(DWCUSB_DOEPINT(ep_num));
	ep->interrupt_status |= depintsts;
	write_reg(DWCUSB_DOEPINT(ep_num), depintsts);
    
	print("ep_num: %d depintsts: %08x \n", ep_num, depintsts);
	
	if(endpoint == EP0_OUT) {
		// Don't do anything. All EP0 requests will be handled in the secondary.
		return;
	}
    
	if(depintsts & DWCUSB_DOEPINT_XFERCOMPL) {
		u_int32_t eptsizval = 0;
		u_int32_t len = 0;
		u_int32_t lenMod;
		
		eptsizval = read_reg(DWCUSB_DOEPTSIZ(ep_num));
		
		len = (ep->transfer_size - (int)(eptsizval & DWCUSB_DEPTSIZ_XFERSIZE_MASK));
		
		ep->io_head->return_count += len;
		print("recvd %d \n", ep->io_head->return_count);
		
		lenMod = len % ep->max_packet_size;
		
		if(((len < ep->transfer_size) && lenMod) \
		   || (len == 0)) {
			ep->transfer_size = 0;
			goto done;
		}
		
		// Check following condition - received mutiple of ep mps worth data + a zlp to complete transfer
		// There is no simple way to check this in existing hw. So... we know how many packets we wanted to
		// receive, and how much we received. If we have received multiple of ep mps and zlp,
		// packet count should be decremented by - (data_received / ep_mps + 1)
		if(lenMod == 0) {
			int lenDiv;
			int pktCnt;
            
			lenDiv = len / ep->max_packet_size;
			pktCnt = ep->packet_count -
            (int)((eptsizval & DWCUSB_DEPTSIZ_PKTCNT_MASK) >> DWCUSB_DEPTSIZ_PKTCNT_SHIFT);
			print("%d %d %d %d\n", ep->packet_count, \
				  ((eptsizval & DWCUSB_DEPTSIZ_PKTCNT_MASK) >> DWCUSB_DEPTSIZ_PKTCNT_SHIFT), lenDiv, pktCnt);
			if(lenDiv == (pktCnt - 1)) {
				ep->packet_count = 0;
				ep->transfer_size = 0;
				goto done;
			}
		}
        
		ep->transfer_size = 0;
		ep->packet_count = 0;
        
		if(ep->io_head->return_count < ep->io_head->io_length) {
			synopsys_otg_start_endpoint_out(ep->endpoint_address);
			goto next_out_int;
		}
		
        done :
		ep->io_head->status = USB_IO_SUCCESS;
		
		finished_req = ep->io_head;
		if(ep->io_head != ep->io_tail) {
			ep->io_head = ep->io_head->next;
			finished_req->next = NULL;
			synopsys_otg_start_endpoint_out(ep->endpoint_address);
		}
		else {
			ep->io_head = ep->io_tail = NULL;
		}
		
		enqueue_finished_request(ep, finished_req);
	}
    
    next_out_int :
    
	if(depintsts & DWCUSB_DOEPINT_AHBERR) {
		panic("AHBERR on endpoint %x at %08x \n", endpoint, read_reg(DWCUSB_DOEPDMA(ep_num)));
	}
}


static void synopsys_otg_start_endpoint_out(u_int32_t endpoint)
{
	int ep_num;
	struct usb_endpoint_instance *ep;
	struct usb_device_io_request *req;
	u_int32_t xfer_size;
	u_int32_t pkt_cnt = 0;
	u_int32_t rcvd;
    
	ep = &synopsys_otg_endpoints[ep_to_epindex(endpoint)];
	
	ASSERT(ep->io_head != NULL);
    
	ep_num = ep_to_epnum(endpoint);
	req = ep->io_head;
	xfer_size = req->io_length;
	rcvd = req->return_count;
    
	print("ep_num: %d rcvd: %d xfer_size: %d \n", ep_num, rcvd, xfer_size);
    
	xfer_size = xfer_size - rcvd;
	if(xfer_size > max_dma_size) {
		xfer_size = max_dma_size;
	}
    
	platform_cache_operation(CACHE_INVALIDATE | CACHE_CLEAN, (void *)(req->io_buffer + rcvd), xfer_size);
    
	write_reg(DWCUSB_DOEPDMA(ep_num), (uint32_t)req->io_buffer + rcvd);
    
	/* Program the transfer size and packet count as follows:
	 *
	 *  pktcnt = N
	 *  xfersize = N * maxpacket
	 */
	if (xfer_size == 0) {
		/* Zero Length Packet */
		xfer_size = ep->max_packet_size;
		pkt_cnt = 1;
	}
	else {
		pkt_cnt = (xfer_size + (ep->max_packet_size - 1)) / ep->max_packet_size;
		if(pkt_cnt > max_packet_count) {
			pkt_cnt = max_packet_count;
		}
		xfer_size = pkt_cnt * ep->max_packet_size;
	}
    
	ep->transfer_size = xfer_size;
	ep->packet_count = pkt_cnt;
    
	write_reg(DWCUSB_DOEPTSIZ(ep_num), (pkt_cnt << DWCUSB_DEPTSIZ_PKTCNT_SHIFT) | (xfer_size));
    
	or_reg(DWCUSB_DOEPCTL(ep_num), DWCUSB_DEPCTL_EPENA |
		   DWCUSB_DEPCTL_CNAK);
    
	print("Finish start_endpoint_out(%d) \n", ep_num);
}


static void synopsys_otg_handle_endpoint_in(u_int32_t endpoint)
{
	struct usb_device_io_request *finished_req;
	int ep_num = ep_to_epnum(endpoint);
	struct usb_endpoint_instance *ep;
	uint32_t depintsts;
    
	ep = &synopsys_otg_endpoints[ep_to_epindex(endpoint)];
	depintsts = read_reg(DWCUSB_DIEPINT(ep_num));
	ep->interrupt_status |= depintsts;
	write_reg(DWCUSB_DIEPINT(ep_num), depintsts);
    
	print("DWCUSB_DIEPINT(%d) %08x \n", ep_num, depintsts);
    
	if(depintsts & DWCUSB_DIEPINT_XFERCOMPL) {
		
		ep->io_head->return_count += ep->transfer_size;
		ep->transfer_size = ep->packet_count = 0;
		
		print("return_count %d io_size %d \n", ep->io_head->return_count, ep->io_head->io_length);
		
		if(ep->io_head->return_count < ep->io_head->io_length) {
			synopsys_otg_start_endpoint_in(ep->endpoint_address);
			goto next_in_int;
		}
		
		ep->io_head->status = USB_IO_SUCCESS;
		
		finished_req = ep->io_head;
		if(ep->io_head != ep->io_tail) {
			ep->io_head = ep->io_head->next;
			finished_req->next = NULL;
			synopsys_otg_start_endpoint_in(ep->endpoint_address);
		}
		else {
			ep->io_head = ep->io_tail = NULL;
		}
		
		enqueue_finished_request(ep, finished_req);
	}
    
    next_in_int :
    
	if(depintsts & DWCUSB_DIEPINT_TIMEOUT) {
		dprintf(DEBUG_INFO, "TIME OUT condition detected for EP %x \n", endpoint);		
		synopsys_otg_disable_in_endpoint((u_int32_t)endpoint);		
		synopsys_otg_start_endpoint_in(endpoint);
	}
    
	if(depintsts &  DWCUSB_DIEPINT_AHBERR) {
		panic("AHBERR on endpoint %x at %08x \n", endpoint, read_reg(DWCUSB_DIEPDMA(ep_num)));
	}
}


static void synopsys_otg_start_endpoint_in(u_int32_t endpoint)
{
	u_int32_t ep_num;
	struct usb_endpoint_instance *ep;
	struct usb_device_io_request *req;
	u_int32_t max_dma_size_local;
	u_int32_t xfer_size;
	u_int32_t sent;
	u_int32_t pkt_cnt = 0;
    
	ep = &synopsys_otg_endpoints[ep_to_epindex(endpoint)];
	
	ASSERT(ep->io_head != NULL);
    
	ep_num = ep_to_epnum(endpoint);
	req = ep->io_head;
	xfer_size = req->io_length;
	max_dma_size_local = (ep_num ? max_dma_size : MAX_EP0_DMA_SIZE);
	sent = req->return_count;
	
	print("ep_num: %d sent: %d io_len: %d \n", ep_num, sent, xfer_size);
	
	xfer_size = xfer_size - sent;
	if(xfer_size > max_dma_size_local) {
		xfer_size = max_dma_size_local;
	}
    
	platform_cache_operation(CACHE_INVALIDATE | CACHE_CLEAN, (void *)(req->io_buffer + sent), xfer_size);
    
	write_reg(DWCUSB_DIEPDMA(ep_num), (uint32_t)req->io_buffer + sent);
    
	print("dma: %08x\n", read_reg(DWCUSB_DIEPDMA(ep_num)));
	
	if (xfer_size == 0) {
		/* Zero Length Packet */
		pkt_cnt = 1;
	}
	else {
		pkt_cnt = (xfer_size - 1 + ep->max_packet_size) / ep->max_packet_size;
		if(pkt_cnt > max_packet_count) {
			pkt_cnt = max_packet_count;
			xfer_size = pkt_cnt * ep->max_packet_size;
		}
	}
    
	write_reg(DWCUSB_DIEPTSIZ(ep_num), (1 << DWCUSB_DEPTSIZ_MC_SHIFT) |
			  (pkt_cnt << DWCUSB_DEPTSIZ_PKTCNT_SHIFT) |
			  (xfer_size));
    
	ep->transfer_size = xfer_size;
	ep->packet_count = pkt_cnt;
    
	print("xfer_size:%d, pkt_cnt:%d\n", xfer_size, pkt_cnt);
    
	or_reg(DWCUSB_DIEPCTL(ep_num), DWCUSB_DEPCTL_EPENA |
		   DWCUSB_DEPCTL_CNAK);
    
	print("Finish start_endpoint_in(%d) \n", ep_num);
}


static int set_global_in_nak(void)
{
	int ret = 0;
    
	write_reg(DWCUSB_GINTSTS, DWCUSB_GINT_GINNAKEFF);
    
	or_reg(DWCUSB_DCTL, DWCUSB_DCTL_SGNPINNAK);
    
	utime_t start_time = system_time();
	while((read_reg(DWCUSB_GINTSTS) & DWCUSB_GINT_GINNAKEFF) == 0) {
		if(time_has_elapsed(start_time, NAKEFF_TIMEOUT)) {
			dprintf(DEBUG_INFO, "synopsys_otg::set_global_in_nak : GINNAKEFF taking longer than expected \n");
			ret = -1;
			break;
		}
	}
    
	write_reg(DWCUSB_GINTSTS, DWCUSB_GINT_GINNAKEFF);
    
	return ret;
}


static void clear_global_in_nak(void)
{
	or_reg(DWCUSB_DCTL, DWCUSB_DCTL_CGNPINNAK);
}


static int flush_tx_fifo(int fifo)
{
	int ret = 0;
    
	set_bits_in_reg(DWCUSB_GRSTCTL, DWCUSB_GRSTCTL_TXFNUM_SHIFT, DWCUSB_GRSTCTL_TXFNUM_MASK, 0);
	or_reg(DWCUSB_GRSTCTL, (fifo << DWCUSB_GRSTCTL_TXFNUM_SHIFT) | DWCUSB_GRSTCTL_TXFFLSH);
    
	utime_t start_time = system_time();
	while((read_reg(DWCUSB_GRSTCTL) & DWCUSB_GRSTCTL_TXFFLSH) != 0) {
		if(time_has_elapsed(start_time, FIFOFLSH_TIMEOUT)) {
			dprintf(DEBUG_INFO, "synopsys_otg::flush_tx_fifo : Flushing Fifo %d taking longer than expected \n", fifo);
			ret = -1;
			break;
		}
	}
    
	return ret;
}


static void synopsys_otg_disable_in_endpoint(u_int32_t endpoint)
{
	int ep_num = ep_to_epnum(endpoint);
	struct usb_endpoint_instance *ep = &synopsys_otg_endpoints[ep_to_epindex(endpoint)];
	utime_t start_time;
	int ret = 0;
	u_int32_t pkt_cnt;
    
	print("starting ep %x disable \n", endpoint);
    
	if((read_reg(DWCUSB_DIEPCTL(ep_num)) & DWCUSB_DEPCTL_EPENA) == 0) {
		print("endpoint %x already disabled \n", endpoint);
		return;
	}
	
	set_global_in_nak();
	
	or_reg(DWCUSB_DIEPCTL(ep_num), DWCUSB_DEPCTL_SNAK);
	start_time = system_time();
	while((read_reg(DWCUSB_DIEPINT(ep_num)) & DWCUSB_DIEPINT_IEPNAKEFF) == 0) {
		if(time_has_elapsed(start_time, EPDISBLD_TIMEOUT)) {
			dprintf(DEBUG_INFO, "synopsys_otg_disable_in_endpoint: NAKEFF not set in time for endpoint %x \n", endpoint);
			ret = -1;
			break;
		}
	}
	
	or_reg(DWCUSB_DIEPCTL(ep_num), DWCUSB_DEPCTL_EPDIS | DWCUSB_DEPCTL_SNAK);
	start_time = system_time();
	while((read_reg(DWCUSB_DIEPINT(ep_num)) & DWCUSB_DIEPINT_EPDISBLD) == 0) {
        if(time_has_elapsed(start_time, EPDISBLD_TIMEOUT)) {
            dprintf(DEBUG_INFO, "synopsys_otg_disable_in_endpoint: endpoint %x not disabled in time \n", endpoint);
            ret = -1;
            break;
        }
	}
    
	if(ret == 0) {
        pkt_cnt = 0;
        
        if(ep_num == 0) {
            pkt_cnt = ((read_reg(DWCUSB_DIEPTSIZ(0)) & DWCUSB_DEP0TSIZ_PKTCNT_MASK) >> DWCUSB_DEPTSIZ_PKTCNT_SHIFT);
        }
        else {
            pkt_cnt = ((read_reg(DWCUSB_DIEPTSIZ(ep_num)) & DWCUSB_DEPTSIZ_PKTCNT_MASK) >> DWCUSB_DEPTSIZ_PKTCNT_SHIFT);
        }
        
        if(pkt_cnt == 0) {
            ep->io_head->return_count += ep->transfer_size;
        }
        else {
            ep->io_head->return_count += ((ep->packet_count - pkt_cnt) * ep->max_packet_size);
        }
	}
    
	flush_tx_fifo(ep->tx_fifo_number);
    
	clear_global_in_nak();
	
	print("endpoint %x disabled \n", endpoint);
}


static int set_global_out_nak(void)
{
	int ret = 0;
    
	write_reg(DWCUSB_GINTSTS, DWCUSB_GINT_GOUTNAKEFF);
    
	or_reg(DWCUSB_DCTL, DWCUSB_DCTL_SGOUTNAK);
    
	utime_t start_time = system_time();
	while((read_reg(DWCUSB_GINTSTS) & DWCUSB_GINT_GOUTNAKEFF) == 0) {
		if(time_has_elapsed(start_time, 1000)) {
			dprintf(DEBUG_INFO, "%s: OUT NAK EFF not set in time \n", __FUNCTION__);
			ret = -1;
			break;
		}
	}
    
	write_reg(DWCUSB_GINTSTS, DWCUSB_GINT_GOUTNAKEFF);
    
	return ret;
}


static void clear_global_out_nak(void)
{
	or_reg(DWCUSB_DCTL, DWCUSB_DCTL_CGOUTNAK);
}


static void synopsys_otg_disable_out_endpoint(u_int32_t endpoint)
{
	int ep_num = ep_to_epnum(endpoint);
	utime_t start_time;
    
	print("starting ep %x disable \n", endpoint);
    
	if((read_reg(DWCUSB_DOEPCTL(ep_num)) & DWCUSB_DEPCTL_EPENA) == 0) {
		print("endpoint %x already disabled \n", endpoint);
		return;
	}
    
	set_global_out_nak();
	
	or_reg(DWCUSB_DOEPCTL(ep_num), DWCUSB_DEPCTL_EPDIS | DWCUSB_DEPCTL_SNAK);
	start_time = system_time();
	while((read_reg(DWCUSB_DOEPINT(ep_num)) & DWCUSB_DOEPINT_EPDISBLD) == 0) {
		if(time_has_elapsed(start_time, EPDISBLD_TIMEOUT)) {
			dprintf(DEBUG_INFO, "synopsys_otg_disable_out_endpoint: endpoint %x not disabled in time \n", endpoint);
			break;
		}
	}
    
	clear_global_out_nak();
	
	print("endpoint %x disabled \n", endpoint);
}

static void synopsys_otg_int_handler(void *blah)
{
	uint32_t gintsts = read_reg(DWCUSB_GINTSTS);
	write_reg(DWCUSB_GINTSTS, gintsts);
	print("GINTSTS %08x \n", gintsts);
	synopsys_otg_gintsts |= gintsts;
    
	if(gintsts & (DWCUSB_GINT_OEPINT | DWCUSB_GINT_IEPINT)) {
		u_int32_t daint = read_reg(DWCUSB_DAINT);
		synopsys_otg_daintsts |= daint;
        
		if(daint) {
			u_int32_t out_daint = (daint & DWCUSB_DAINT_OEP_MASK) >> DWCUSB_DAINT_OEP_SHIFT;
			u_int32_t in_daint = (daint & DWCUSB_DAINT_IEP_MASK);
            
			/* rest of endpoints */
			for(int i = 0; (in_daint || out_daint) && (i <= (MAX_EP_NUMBER)); i++, in_daint >>= 1, out_daint >>= 1) {
				if(in_daint & EP_INT_MASK) {
					synopsys_otg_handle_endpoint_in(USB_DIR_IN | i);
				}
				if(out_daint & EP_INT_MASK) {
					synopsys_otg_handle_endpoint_out(i);
				}
			}
		}
	}
    
	if((gintsts & read_reg(DWCUSB_GINTMSK)) && !(usb_task_event_status & EVENT_TYPE_INTERRUPT))
	{
		usb_task_event_status |= EVENT_TYPE_INTERRUPT;
		event_signal(&usb_task_event);
	}
}

static void handle_interrupt()
{
	u_int32_t gintsts;
	u_int32_t daint;
    
	enter_critical_section();
	gintsts = synopsys_otg_gintsts;
	daint = synopsys_otg_daintsts;
	synopsys_otg_gintsts = 0;
	synopsys_otg_daintsts = 0;
	usb_task_event_status &= ~EVENT_TYPE_INTERRUPT;
	exit_critical_section();
	
	print("GINTSTS %08x \n", gintsts);
	
	if (gintsts & DWCUSB_GINT_WKUPINT) {
		print("WKUPINT \n");
		//handle_usb_resume();
		
	}
	if(gintsts & DWCUSB_GINT_USBSUSP) {
		print("USBSUSP \n");
		//handle_usb_suspend();
		
		// we get USBSUSP interrupt when cable is disconnected.
		// we can use this information to notify stack and PMU driver
		if(usbphy_is_cable_connected() == false) {
			usb_core_event_handler(CABLE_DISCONNECTED);
			synopsys_otg_start_controller();
			synopsys_otg_go_on_bus();
		}
	}
	if(gintsts & DWCUSB_GINT_USBRST) {
		print("USBRST \n");
		usb_core_event_handler(USB_RESET);
		synopsys_otg_handle_usb_reset();
	}
	if(gintsts & DWCUSB_GINT_ENUMDONE) {
		print("USBENUMDONE \n");
		synopsys_otg_handle_enum_done();
	}
	
	if(gintsts & (DWCUSB_GINT_OEPINT | DWCUSB_GINT_IEPINT)) {
		print("DAINT %08x \n", daint);
        
		if(daint) {
			u_int32_t out_daint = (daint & DWCUSB_DAINT_OEP_MASK) >> DWCUSB_DAINT_OEP_SHIFT;
			u_int32_t in_daint = (daint & DWCUSB_DAINT_IEP_MASK);
			int i;
			
			if(in_daint & EP_INT_MASK) {
				scavenge_endpoint_finished_list(EP0_IN);
			}
            
			if(out_daint & EP_INT_MASK) {
				synopsys_otg_handle_ep0_out();
			}
            
			for(i = 1, in_daint >>= 1, out_daint >>= 1; (in_daint || out_daint) && (i <= (MAX_EP_NUMBER)); i++, in_daint >>= 1, out_daint >>= 1) {
				if(in_daint & EP_INT_MASK) {
					scavenge_endpoint_finished_list(USB_DIR_IN | i);
				}
				if(out_daint & EP_INT_MASK) {
					scavenge_endpoint_finished_list(i);
				}
			}
		}
	}
}

static void scavenge_endpoint_finished_list(u_int32_t endpoint)
{
    struct usb_endpoint_instance *ep;
    struct usb_device_io_request *completed_list;
    
    ep = &synopsys_otg_endpoints[ep_to_epindex(endpoint)];
    
    enter_critical_section();
    completed_list = ep->completion_head;
    ep->completion_head = ep->completion_tail = NULL;
    exit_critical_section();
    
    // Call completion on all finished transactions
    while(completed_list) {
        struct usb_device_io_request *completed_req = completed_list;
        completed_list = completed_req->next;
        usb_core_complete_endpoint_io(completed_req);
    }
}

static void enqueue_finished_request(struct usb_endpoint_instance *ep, struct usb_device_io_request *io_req)
{
	ASSERT(ep != NULL);
	ASSERT(io_req != NULL);
	
	io_req->next = NULL;
	if(ep->completion_tail) {
		ep->completion_tail->next = io_req;
		ep->completion_tail = io_req;
	}
	else {
		ep->completion_head = ep->completion_tail = io_req;
	}
}


static int synopsys_otg_task(void *arg)
{
	for(;;) {
        
		event_wait(&usb_task_event);
		
		if(usb_task_event_status & EVENT_TYPE_INTERRUPT)
		{
			handle_interrupt(); // Invoke bottom half.
		}
		if(usb_task_event_status & EVENT_TYPE_EXIT)
		{
			event_signal(&usb_task_terminate_event);
			break;
		}
	}
    
	return 0;
}

static int synopsys_otg_init()
{
	usb_task_event_status = EVENT_TYPE_NONE;
	event_init(&usb_task_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	event_init(&usb_task_terminate_event, EVENT_FLAG_AUTO_UNSIGNAL, false);
	task_start((controller_task = task_create("usb", &synopsys_otg_task, NULL, 0x1000)));
    
	return 0;
}

static void synopsys_otg_free()
{
}

static int synopsys_otg_start(void)
{
	// power up the phy, this will enable OTG clock
	usbphy_power_up();
	
#if (DEBUG == 2)
	log_buffer = (u_int8_t *)calloc(1, log_buffer_len);
    
	dprintf(DEBUG_INFO, "USB logging buffer 0x%08x\n", log_buffer);
	
	writeptr = readptr = log_buffer;
#endif
    
	synopsys_otg_print_endpoints();
    
	bzero(ep0_rx_buffer, EP0_MAX_PACKET_SIZE);
    
	bzero(synopsys_otg_endpoints, sizeof(struct usb_endpoint_instance) * MAX_EP_NEEDED);
    
	bzero(tx_fifo_list, MAX_TX_FIFO_SUPPORTED);
    
	synopsys_otg_start_controller();
	
#ifdef INT_USB_OTG
	install_int_handler(INT_USB_OTG, synopsys_otg_int_handler, NULL);
	unmask_int(INT_USB_OTG);
#else
	// <rdar://problem/10320827> Add support for multiple USB OTG controllers to iBoot
	if (USBOTG_BASE_ADDR == USBOTG0_BASE_ADDR) {
		install_int_handler(INT_USB_OTG0, synopsys_otg_int_handler, NULL);
		unmask_int(INT_USB_OTG0);
	} else if (USBOTG_BASE_ADDR == USBOTG1_BASE_ADDR) {
		install_int_handler(INT_USB_OTG1, synopsys_otg_int_handler, NULL);
		unmask_int(INT_USB_OTG1);
	} else {
		panic("Invalid USBOTG base address");
	}
#endif

	// Go on bus so that the host can see this
	synopsys_otg_go_on_bus();
	
	return 0;
}

static void synopsys_otg_stop(void)
{
	int i;
	
	// mask our interrupt
#ifdef INT_USB_OTG
	mask_int(INT_USB_OTG);
#else
	// <rdar://problem/10320827> Add support for multiple USB OTG controllers to iBoot
	if (USBOTG_BASE_ADDR == USBOTG0_BASE_ADDR) {
		mask_int(INT_USB_OTG0);
	} else if (USBOTG_BASE_ADDR == USBOTG1_BASE_ADDR) {
		mask_int(INT_USB_OTG1);
	} else {
		panic("Invalid USBOTG base address");
	}
#endif
	
	// Stop the controller task
	enter_critical_section();
	usb_task_event_status |= EVENT_TYPE_EXIT;
	exit_critical_section();
	event_signal(&usb_task_event);
	
	// Wait for the controller task to finish. But first ensure that we are not in the controller task already.
	ASSERT(task_get_current_task() != controller_task);
	event_wait(&usb_task_terminate_event);
	
	// Deactivate all endpoints so that new IOs are not accepted.
	for(i = MAX_EP_NUMBER; i > 0; i-- ) {
		usb_core_deactivate_endpoint(USB_DIR_IN | i);
		usb_core_deactivate_endpoint(i);
	}
	usb_core_deactivate_endpoint(EP0_IN);
	
	// mask all the interrupts
	write_reg(DWCUSB_GINTMSK, 0);
	write_reg(DWCUSB_DOEPMSK, 0);
	write_reg(DWCUSB_DIEPMSK, 0);
	write_reg(DWCUSB_DAINTMSK, 0);
	
	// clear any pending interrupts
	write_reg(DWCUSB_GINTSTS, 0xffffffff);
	for(i = MAX_EP_NUMBER; i >= 0; i-- ) {
		write_reg(DWCUSB_DIEPINT(i), 0x1f);
		write_reg(DWCUSB_DOEPINT(i), 0xf);
	}
	
	// Clear global interrupt status flags
	synopsys_otg_gintsts = 0;
	synopsys_otg_daintsts = 0;
    
	// Set Soft Disconnect bit
	or_reg(DWCUSB_DCTL, DWCUSB_DCTL_SFTDISCON);	

	// power down the phy, this will disable OTG clock
	usbphy_power_down();
    
	// uninstall interrupt handler
    
	// free allocated resources
	task_wait_on(controller_task);
	task_destroy(controller_task);
	controller_task = NULL;
	
#if (DEBUG == 2)
	if(log_buffer) {
		free(log_buffer);
		log_buffer = NULL;
	}
#endif
}

static void synopsys_otg_go_on_bus(void)
{
	/* Enable D+ Pull up. */
	usbphy_enable_pullup();	

	// Reset Soft disconnect. Now the host can see the device.
	and_reg(DWCUSB_DCTL, ~DWCUSB_DCTL_SFTDISCON);	
}

static void synopsys_otg_set_address(u_int32_t new_address)
{
	enter_critical_section();
    
	new_address = (new_address & (DWCUSB_DCFG_DEVADDR_MASK >> DWCUSB_DCFG_DEVADDR_SHIFT));
	set_bits_in_reg(DWCUSB_DCFG, DWCUSB_DCFG_DEVADDR_SHIFT, DWCUSB_DCFG_DEVADDR_MASK, new_address);
    
	exit_critical_section();
    
	print("new_address %d \n", new_address);
}

static int synopsys_otg_get_connection_speed(void)
{
	int is_hs;
    
	enter_critical_section();
	is_hs = ((read_reg(DWCUSB_DSTS) & DWCUSB_DSTS_ENUMSPD_MASK) == DWCUSB_DSTS_ENUMSPD_HS);
	exit_critical_section();
    
	return (is_hs ? CONNECTION_SPEED_HIGH : CONNECTION_SPEED_FULL);
}

static void synopsys_otg_do_test_mode(u_int32_t selector)
{
	enter_critical_section();
	selector = (selector & (DWCUSB_DCTL_TSTCTL_MASK >> DWCUSB_DCTL_TSTCTL_SHIFT));
	set_bits_in_reg(DWCUSB_DCTL, DWCUSB_DCTL_TSTCTL_SHIFT, DWCUSB_DCTL_TSTCTL_MASK, selector);
	exit_critical_section();
}

static void synopsys_otg_activate_endpoint(u_int32_t endpoint, int type, int max_packet_size, int interval)
{
	print("ep %x , type %d, mps %d, interval %d \n", endpoint, type, max_packet_size, interval);
    
	u_int32_t ep_num = ep_to_epnum(endpoint);
	u_int32_t ep_dir = ep_to_epdir(endpoint);
	u_int32_t ep_idx = ep_to_epindex(endpoint);
	addr_t ep_ctl_reg = 0;
	u_int32_t ep_daint_msk_shift = 0;
	u_int32_t val = 0;
	int next_ep = 0;
	struct usb_endpoint_instance *ep;
	
	ASSERT(ep_num <= MAX_EP_NUMBER);
	ASSERT(ep_idx < MAX_EP_NEEDED);
	
	ep = &synopsys_otg_endpoints[ep_idx];
	ASSERT(ep != NULL);
	
	enter_critical_section();
	
	// Check if this endpoint is already active
	if(ep->is_active) {
		exit_critical_section();
		return;
	}
    
	ep->endpoint_address = endpoint;
	ep->max_packet_size = max_packet_size;
	ep->attributes = type;
	ep->bInterval = interval;
	ep->is_active = true;
    
	val = DWCUSB_DEPCTL_SETD0PID | DWCUSB_DEPCTL_SNAK;
    
	switch(type | ep_dir)
	{
		case USB_ENDPOINT_BULK | USB_DIR_IN :
		case USB_ENDPOINT_INTERRUPT | USB_DIR_IN :
			
			ep_ctl_reg = DWCUSB_DIEPCTL(ep_num);
			ep_daint_msk_shift = ep_num;
			
			write_reg(ep_ctl_reg, 0);
			
			u_int32_t txFifo_n =  get_np_tx_fifo_size();
			int next_tx_fifo = get_next_tx_fifo_num();
			
			print("ep_num %x, tx_fifo %d\n", ep_num, next_tx_fifo);
			
			val |= (next_tx_fifo << DWCUSB_DEPCTL_TXFNUM_SHIFT);
			write_reg(DWCUSB_DPTXFSIZ(next_tx_fifo),
                      (txFifo_n << DWCUSB_DIEPTXF_INEPTXFDEP_SHIFT) | next_tx_fifo_start_addr);
			ep->tx_fifo_number = next_tx_fifo;
			next_tx_fifo_start_addr += txFifo_n;
			break;
			
		case USB_ENDPOINT_BULK | USB_DIR_OUT :
		case USB_ENDPOINT_INTERRUPT | USB_DIR_OUT :
			ep_ctl_reg = DWCUSB_DOEPCTL(ep_num);
			ep_daint_msk_shift = (DWCUSB_DAINT_OEP_SHIFT + ep_num);
			write_reg(ep_ctl_reg, 0);
            break;
			
		default:
			dprintf(DEBUG_INFO, "synopsys_otg :: activate_endpoint --- trying to activate un-supported \n");
			goto exit;
	}
    
	write_reg(ep_ctl_reg, val |
              (type << DWCUSB_DEPCTL_EPTYPE_SHIFT) |
              (next_ep << DWCUSB_DEPCTL_NEXTEP_SHIFT) |
              DWCUSB_DEPCTL_USBACTEP |
              max_packet_size);
    
	or_reg(DWCUSB_DAINTMSK, (1 << ep_daint_msk_shift));
    
exit:
	exit_critical_section();
}

static void synopsys_otg_do_endpoint_io(struct usb_device_io_request *req)
{
	struct usb_endpoint_instance *ep;
	u_int32_t epindex;
    
	print("endpoint %x \n", req->endpoint);
    
	enter_critical_section();
    
	ASSERT(ep_to_epnum(req->endpoint) <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(req->endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &synopsys_otg_endpoints[epindex];
	ASSERT(ep != NULL);
    
	req->return_count = 0;
	req->status = USB_IO_ERROR;
	req->next = NULL;
	
	// Check validity of endpoint this IO has been submitted to
	if(!ep->is_active)
	{
		print("endpint %x is invalid\n", req->endpoint);
		usb_core_complete_endpoint_io(req);
		goto exit;
	}
    
	// pending IOs
	if(ep->io_head) {
		ep->io_tail->next = req;
		ep->io_tail = req;
		goto exit;
	}
    
	// no Pending IOs
	ep->io_head = ep->io_tail = req;
    
	// start transmitting/receiving the data
	if(ep_to_epdir(req->endpoint)) {
		synopsys_otg_start_endpoint_in(req->endpoint);
	}
	else {
		synopsys_otg_start_endpoint_out(req->endpoint);
	}
    
exit:
	exit_critical_section();
}

static void synopsys_otg_stall_endpoint(u_int32_t endpoint, bool stall)
{
	int ep_dir = ep_to_epdir(endpoint);
	int ep_num = ep_to_epnum(endpoint);
    
	print("stall: %d ep: %x \n", stall, endpoint);
    
	enter_critical_section();
    
	if(ep_num > MAX_EP_NUMBER) {
		goto exit;
	}
    
	if(stall) {
		or_reg((ep_dir ? DWCUSB_DIEPCTL(ep_num) : DWCUSB_DOEPCTL(ep_num)), DWCUSB_DEPCTL_STALL);
	}
	else {
		if(ep_num == 0) {
			/* SC bit, core will clear this bit once a SETUP packet is rcvd */
			goto exit;
		}
		
		and_reg((ep_dir ? DWCUSB_DIEPCTL(ep_num) : DWCUSB_DOEPCTL(ep_num)), ~(DWCUSB_DEPCTL_STALL));
	}
    
exit:
	exit_critical_section();
}

static void synopsys_otg_reset_endpoint_data_toggle(u_int32_t endpoint)
{
	int ep_dir = ep_to_epdir(endpoint);
	int ep_num = ep_to_epnum(endpoint);
    
	print("ep:%02x\n", ep_num);
    
	enter_critical_section();
    
	if(ep_num > MAX_EP_NUMBER) {
		goto exit;
	}
    
	or_reg((ep_dir ? DWCUSB_DIEPCTL(ep_num) : DWCUSB_DOEPCTL(ep_num)), DWCUSB_DEPCTL_SETD0PID);
    
exit:
	exit_critical_section();
}

static bool synopsys_otg_is_endpoint_stalled(u_int32_t endpoint)
{
	int ep_dir = ep_to_epdir(endpoint);
	int ep_num = ep_to_epnum(endpoint);
	bool ret;
	
	print("ep:%02x\n", ep_num);
    
	enter_critical_section();
    
	if(ep_num > MAX_EP_NUMBER) {
		ret = false;
		goto exit;
	}
    
	ret = ((read_reg((ep_dir ? DWCUSB_DIEPCTL(ep_num) : DWCUSB_DOEPCTL(ep_num))) & DWCUSB_DEPCTL_STALL) == DWCUSB_DEPCTL_STALL);
	
exit:
	exit_critical_section();
    
	return ret;
}

static void synopsys_otg_abort_endpoint(u_int32_t endpoint)
{
	struct usb_device_io_request *aborted_list;
	struct usb_device_io_request *completed_list;
	struct usb_endpoint_instance *ep;
	u_int32_t epindex;
	u_int32_t ep_num;
	
	print("endpoint %x \n", endpoint);
    
	enter_critical_section();
    
	ep_num = ep_to_epnum(endpoint);
	ASSERT(ep_num <= MAX_EP_NUMBER);

	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);

	ep = &synopsys_otg_endpoints[epindex];
	ASSERT(ep != NULL);
	
	// Check if this endpoint is valid
	if(!ep->is_active) {
		exit_critical_section();
		return;
	}
    
	if(ep_to_epdir(endpoint)) {
		synopsys_otg_disable_in_endpoint(endpoint);
		
		// Clear pending interrupts as we are going to abort all IOs
		write_reg(DWCUSB_DIEPINT(ep_num), read_reg(DWCUSB_DIEPINT(ep_num)));
	}
	else {
		synopsys_otg_disable_out_endpoint(endpoint);
		
		// Clear pending interrupts as we are going to abort all IOs
		write_reg(DWCUSB_DOEPINT(ep_num), read_reg(DWCUSB_DOEPINT(ep_num)));
	}
    
	ep->interrupt_status = 0;
	ep->transfer_size = ep->packet_count = 0;
	
	completed_list = ep->completion_head;
	ep->completion_head = ep->completion_tail = NULL;
    
	aborted_list = ep->io_head;
	ep->io_head = ep->io_tail = NULL;
    
	exit_critical_section();
    
	// First return all completed transactions
	while(completed_list) {
		struct usb_device_io_request *completed_req = completed_list;
		completed_list = completed_req->next;
		usb_core_complete_endpoint_io(completed_req);
	}
    
	// Then return the aborted transactions
	while(aborted_list) {
		struct usb_device_io_request *aborted_req = aborted_list;
		aborted_list = aborted_req->next;
		aborted_req->status = USB_IO_ABORTED;
		usb_core_complete_endpoint_io(aborted_req);
	}
}

static void synopsys_otg_deactivate_endpoint(u_int32_t endpoint)
{
	print("ep %x \n", endpoint);
	
	u_int32_t ep_num = ep_to_epnum(endpoint);
	u_int32_t ep_dir = ep_to_epdir(endpoint);
	addr_t ep_ctl_reg = 0;
	u_int32_t ep_daint_msk_shift = 0;
	addr_t ep_siz_reg = 0;
	addr_t ep_int_reg = 0;
	addr_t ep_dma_reg = 0;
	struct usb_endpoint_instance *ep;
	u_int32_t epindex;
    
	ASSERT(ep_num <= MAX_EP_NUMBER);
	epindex = ep_to_epindex(endpoint);
	ASSERT(epindex < MAX_EP_NEEDED);
	ep = &synopsys_otg_endpoints[epindex];
	ASSERT(ep != NULL);
	
	enter_critical_section();
	
	if(!ep->is_active)
	{
		print("endpint %x is invalid\n", endpoint);
		goto exit;
	}
	
	if(ep_dir) {
		ep_ctl_reg = DWCUSB_DIEPCTL(ep_num);
		ep_daint_msk_shift = ep_num;
		ep_siz_reg = DWCUSB_DIEPTSIZ(ep_num);
		ep_int_reg = DWCUSB_DIEPINT(ep_num);
		ep_dma_reg = DWCUSB_DIEPDMA(ep_num);
	}
	else {
		ep_ctl_reg = DWCUSB_DOEPCTL(ep_num);
		ep_daint_msk_shift = (DWCUSB_DAINT_OEP_SHIFT + ep_num);
		ep_siz_reg = DWCUSB_DOEPTSIZ(ep_num);
		ep_int_reg = DWCUSB_DOEPINT(ep_num);
		ep_dma_reg = DWCUSB_DOEPDMA(ep_num);
	}
    
	write_reg(ep_ctl_reg, 0);
	write_reg(ep_siz_reg, 0);
	write_reg(ep_int_reg, 0x1ff);
	write_reg(ep_dma_reg, 0);
	and_reg(DWCUSB_DAINTMSK, ~(1 << ep_daint_msk_shift));
    
	if(ep->tx_fifo_number) {
		tx_fifo_list[ep->tx_fifo_number - 1] = 0;
	}
    
	bzero(&synopsys_otg_endpoints[epindex], sizeof(struct usb_endpoint_instance));
	
exit:   
	exit_critical_section();
}

//===================================================================================
//		Global Functions
//===================================================================================

static const struct usb_controller_functions synopsys_otg_controller_functions = {
	.init = synopsys_otg_init,
	.free_func = synopsys_otg_free,
	.start = synopsys_otg_start,
	.stop = synopsys_otg_stop,
	.set_address = synopsys_otg_set_address,
	.get_connection_speed = synopsys_otg_get_connection_speed,
	.activate_endpoint = synopsys_otg_activate_endpoint,
	.do_endpoint_io = synopsys_otg_do_endpoint_io,
	.stall_endpoint = synopsys_otg_stall_endpoint,
	.reset_endpoint_data_toggle = synopsys_otg_reset_endpoint_data_toggle,
	.is_endpoint_stalled = synopsys_otg_is_endpoint_stalled,
	.do_test_mode = synopsys_otg_do_test_mode,
	.abort_endpoint = synopsys_otg_abort_endpoint,
	.deactivate_endpoint = synopsys_otg_deactivate_endpoint,
};

const struct usb_controller_functions *synopsys_otg_controller_init()
{
	return &synopsys_otg_controller_functions;
}
