/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <lib/libc.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/clocks.h>
#include <platform/gpio.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwisr.h>
#include <sys.h>
#include <sys/task.h>

#include "spi.h"

#ifndef SPIS_MASK
#define SPIS_MASK		((1 << SPIS_COUNT) - 1)
#endif

#define SPI_DMA 0

#define SPI_AGD_MODE 1

//#define DEBUG_SPI_SPEW DEBUG_SPEW
#ifndef DEBUG_SPI_SPEW
#define DEBUG_SPI_SPEW (0x7FFFFFFF)
#endif

/* previously from libc.h, but it pollutes the namespace there */
#define BITS_SHIFT(x, high, low) (((x) >> (low)) & ((1<<((high)-(low)+1))-1))

/* spi driver, copy and customize */

enum spi_clk {
	PCLK,
	NCLK
};

static struct spi_status_t {
	int bits; // 8, 16, 32
	int clkpol; // clock polarity
	int clkpha; // clock phase
	enum spi_clk clk; // PCLK or NCLK
	int baud;
	bool master;
	bool dma;

	/* interrupt driven stuff */
	const void *tx_buf;
	size_t tx_pos;
	size_t tx_len;
	void *rx_buf;
	size_t rx_pos;
	size_t rx_len;	
	int overrun_errors;

	struct task_event event;

	uint32_t shadow_spcon;

#if SPI_DMA
	/* dma stuff */
	dma_t tx_dma;
	struct iovec tx_iov;
	struct dma_xfer_t tx_xfer;

	dma_t rx_dma;
	struct iovec rx_iov;
	struct dma_xfer_t rx_xfer;
#endif

	/* completion flags */
	volatile int tx_complete;
	volatile int rx_complete;
} spi_status[SPIS_COUNT];

typedef struct {
	volatile uint32_t *spclkcon;
	volatile uint32_t *spcon;
	volatile uint32_t *spsta;
	volatile uint32_t *sppin;
	volatile uint32_t *sptdat;
	volatile uint32_t *sprdat;
	volatile uint32_t *sppre;
	volatile uint32_t *spcnt;
	volatile uint32_t *spidd;
	volatile uint32_t *spirto;
	volatile uint32_t *spihangd;
	volatile uint32_t *spiswrst;
	volatile uint32_t *spiver;
	volatile uint32_t *sptdcnt;
	uint32_t clock;
	uint32_t irq;
} spi_regs_t;

static const spi_regs_t spi_regs[] = {
#if SPIS_COUNT > 0
	{ &rSPCLKCON0, &rSPCON0, &rSPSTA0, &rSPPIN0, &rSPTDAT0, &rSPRDAT0, &rSPPRE0, &rSPCNT0,
	  &rSPIDD0, &rSPIRTO0, &rSPIHANGD0, &rSPISWRST0, &rSPIVER0, &rSPTDCNT0, CLK_SPI0, INT_SPI0 },
#endif
#if SPIS_COUNT > 1
	{ &rSPCLKCON1, &rSPCON1, &rSPSTA1, &rSPPIN1, &rSPTDAT1, &rSPRDAT1, &rSPPRE1, &rSPCNT1,
	  &rSPIDD1, &rSPIRTO1, &rSPIHANGD1, &rSPISWRST1, &rSPIVER1, &rSPTDCNT1, CLK_SPI1, INT_SPI1 },
#endif
#if SPIS_COUNT > 2
	{ &rSPCLKCON2, &rSPCON2, &rSPSTA2, &rSPPIN2, &rSPTDAT2, &rSPRDAT2, &rSPPRE2, &rSPCNT2,
	  &rSPIDD2, &rSPIRTO2, &rSPIHANGD2, &rSPISWRST2, &rSPIVER2, &rSPTDCNT2, CLK_SPI2, INT_SPI2 },
#endif
#if SPIS_COUNT > 3
	{ &rSPCLKCON3, &rSPCON3, &rSPSTA3, &rSPPIN3, &rSPTDAT3, &rSPRDAT3, &rSPPRE3, &rSPCNT3,
	  &rSPIDD3, &rSPIRTO3, &rSPIHANGD3, &rSPISWRST3, &rSPIVER3, &rSPTDCNT3, CLK_SPI3, INT_SPI3 },
#endif
#if SPIS_COUNT > 4
	{ &rSPCLKCON4, &rSPCON4, &rSPSTA4, &rSPPIN4, &rSPTDAT4, &rSPRDAT4, &rSPPRE4, &rSPCNT4,
	  &rSPIDD4, &rSPIRTO4, &rSPIHANGD4, &rSPISWRST4, &rSPIVER4, &rSPTDCNT4, CLK_SPI4, INT_SPI4 },
#endif
};

static const spi_regs_t *spiregs(int port)
{
	if (port < 0 || port >= SPIS_COUNT || !(SPIS_MASK & (1 << port)))
		panic("Trying to access a nonexisted spi port\n");

	return &spi_regs[port];
}

static void spi_set_port_enable(int port, bool enable)
{
	const spi_regs_t *regs = spiregs(port);

	if (enable)
		*regs->spclkcon = 1;
	else
		*regs->spclkcon = 0;
}

#if SPI_DMA
static void spi_set_port_dma(int port, bool dma)
{
	if (dma == spi_status[port].dma)
		return;
	
	if (spi_status[port].dma) {
		release_dma_channel(spi_status[port].tx_dma);
		release_dma_channel(spi_status[port].rx_dma);
		spi_status[port].tx_dma = spi_status[port].rx_dma = -1;
	}

	if (dma) {
		spi_status[port].tx_dma = acquire_dma_channel(DMA_DEVICE_MEMORY, DMA_DEVICE_SPI_0 + port, 1, 1);
		spi_status[port].rx_dma = acquire_dma_channel(DMA_DEVICE_SPI_0 + port, DMA_DEVICE_MEMORY, 1, 1);
		dprintf(DEBUG_INFO, "spi_set_port_dma: allocated channels, tx %d rx %d\n", spi_status[port].tx_dma, spi_status[port].rx_dma);
	}

	spi_status[port].dma = dma;
}
#endif

static void spi_set_baud(int port, int baud)
{
	const spi_regs_t *regs = spiregs(port);
	uint32_t clkrate;
	int div;
	int actual_baud;

	if (spi_status[port].clk == PCLK)
		clkrate = clock_get_frequency(CLK_PCLK);
	else
		clkrate = clock_get_frequency(CLK_NCLK);

	div = clkrate / baud;
#if SPI_VERSION > 0
	if (div < 1) div = 1;
#else
	if (div < 2) div = 2;
#endif
	actual_baud = clkrate / div;

	dprintf(DEBUG_SPI_SPEW, "spi_set_baud: port %d clk %s (%d Hz) baud %d, divider %d, actual baud %d\n", 
		port, (spi_status[port].clk == PCLK) ? "PCLK" : "NCLK", clkrate, baud, div, actual_baud);

	if (div >= (1<<10))
		dprintf(DEBUG_CRITICAL, "spi_set_baud: warning, div 0x%x is too big to fit in register\n", div);

	*regs->sppre = div;
	spi_status[port].baud = baud;
}

//small routine to get the baudrate for a divider
//helper function to be able todo automated testing
static int spi_baud_from_div(int port, int div)
{
	int clkrate;

	if (spi_status[port].clk == PCLK)
		clkrate = clock_get_frequency(CLK_PCLK);
	else
		clkrate = clock_get_frequency(CLK_NCLK);

	if (div < 2) div = 2;
	return (clkrate / div);
}

void spi_set_interdatadelay(int port,int usecs) 
{
	const spi_regs_t *regs = spiregs(port);
	uint32_t clocks;

	clocks = ((1ULL * clock_get_frequency(CLK_PCLK) * usecs) + 1000000 - 1) / 1000000;

	*regs->spidd = clocks;
	dprintf(DEBUG_SPI_SPEW, "spidd: %d\n",*regs->spidd);
}

void spi_set_clk(int port, enum spi_clk clk)
{
	spi_status[port].clk = clk;
}

void spi_setup(int port, int baud, int width, bool master, int clkpol, int clkpha)
{
	const spi_regs_t *regs = spiregs(port);

	if (regs == 0)
		return;

	spi_set_port_enable(port, false);

	switch (width) {
		default:
		case 8:
			spi_status[port].bits = 0;
			break;
		case 16:
			spi_status[port].bits = 1;
			break;
		case 32:
			spi_status[port].bits = 2;
			break;
	}

	spi_status[port].clkpol = clkpol ? 1 : 0;
	spi_status[port].clkpha = clkpha ? 1 : 0;
	spi_set_baud(port, baud);
	spi_status[port].master = master;

	/* set up the control register */
	spi_status[port].shadow_spcon =
			(spi_status[port].clkpha << SPICON_CPHA_SHIFT) |
			(spi_status[port].clkpol << SPICON_CPOL_SHIFT) |
			((spi_status[port].master ? 3 : 0) << SPICON_MASTER_SHIFT) | // if in master mode, enable clock
			((spi_status[port].dma ? 2 : 1) << SPICON_MODE_SHIFT) |
			(0 << SPICON_MSBFT_SHIFT) | // MSB first
			(((spi_status[port].clk == PCLK) ? 0 : 1) << SPICON_CLK_SEL_SHIFT) |
			(spi_status[port].bits << SPICON_BIT_LEN_SHIFT) | // 8/16/32 bit width
			(0 << SPICON_DMA_SIZE_SHIFT); // 4 byte IRQ/DMA
	*regs->spcon = spi_status[port].shadow_spcon;

	*regs->sppin = (1 << 1);

	spi_set_port_enable(port, true); // the port has to be enabled before you write to the FIFO.

	dprintf(DEBUG_SPI_SPEW, "spcon 0x%x\n", *regs->spcon);
}

static void spi_flush_tx_fifo(int port)
{
	const spi_regs_t *regs = spiregs(port);

	*regs->spclkcon |= (1<<2);
}

static void spi_flush_rx_fifo(int port)
{
	const spi_regs_t *regs = spiregs(port);

	*regs->spclkcon |= (1<<3);
}

static void spi_interrupt(void *arg)
{
	int port = (int)arg;
	const spi_regs_t *regs = spiregs(port);
	uint32_t status;
	uint32_t i;

	status = *regs->spsta;
	*regs->spsta = status;

	dprintf(DEBUG_SPI_SPEW, "spi_interrupt(): %d, status 0x%08x / 0x%08x\n", port, status, *regs->spcon);

	if (status & (1 << 3)) { // data collision error
		dprintf(DEBUG_SPI_SPEW, "spi_int, port %d: data collision (lost rx bytes)\n", port);
		spi_status[port].overrun_errors++;
	}
	if (status & (1 << 0)) { // rx done
		dprintf(DEBUG_SPI_SPEW, "spi_int, port %d: rx done, status 0x%x, tx pos %zu, rx pos %zu\n", 
				port, status, spi_status[port].tx_pos, spi_status[port].rx_pos);
		dprintf(DEBUG_SPI_SPEW, "\ttx fifo %d rx fifo %d\n",
			BITS_SHIFT(status, SPSTA_TX_FIFO_BIT_HI, SPSTA_TX_FIFO_BIT_LO),
			BITS_SHIFT(status, SPSTA_RX_FIFO_BIT_HI, SPSTA_RX_FIFO_BIT_LO));
		if (!(status & ((1 << 22) | (1 << 1))))
			goto handle_rx;
	}
	if (status & ((1 << 22) | (1 << 1))) { // transfer done or ready
		/* deal with tx */
handle_tx:
		if (spi_status[port].tx_buf) {
			if (spi_status[port].tx_pos >= spi_status[port].tx_len) {
				/* we're done */
				spi_status[port].tx_complete = 1;
				spi_status[port].tx_buf = 0;
#if SPI_VERSION > 0
				spi_status[port].shadow_spcon &= ~(1 << SPICON_IE_TX_SHIFT);
				*regs->spcon = spi_status[port].shadow_spcon;
#endif
				dprintf(DEBUG_SPI_SPEW, "spi_tx %d: tx done, pos %zu len %zu\n", port, spi_status[port].tx_pos, spi_status[port].tx_len);
			} else {
				uint32_t txfifo_left = SPI_FIFO_SIZE - BITS_SHIFT(status, SPSTA_TX_FIFO_BIT_HI, SPSTA_TX_FIFO_BIT_LO);
				uint32_t to_transfer = spi_status[port].tx_len - spi_status[port].tx_pos;

				to_transfer = __min(txfifo_left, to_transfer);

				dprintf(DEBUG_SPI_SPEW, "spi_tx %d: txfifo_left %d, to_transfer %d, pos %zu, len %zu\n", 
						port, txfifo_left, to_transfer, spi_status[port].tx_pos, spi_status[port].tx_len);

				for (i=0; i < to_transfer; i++)
					*regs->sptdat = ((uint8_t *)spi_status[port].tx_buf)[spi_status[port].tx_pos + i];

				spi_status[port].tx_pos += to_transfer;
			}
		}

		/* look for rx */
handle_rx:
		if (spi_status[port].rx_buf) {
			uint32_t tmp;
			uint32_t rxfifo = BITS_SHIFT(status, SPSTA_RX_FIFO_BIT_HI, SPSTA_RX_FIFO_BIT_LO);
			uint32_t to_transfer = spi_status[port].rx_len - spi_status[port].rx_pos;

			to_transfer = __min(rxfifo, to_transfer);

			dprintf(DEBUG_SPI_SPEW, "spi_rx %d: fifo %d, to_transfer %d, pos %zu, len %zu\n", 
					port, rxfifo, to_transfer, spi_status[port].rx_pos, spi_status[port].rx_len);
			
			for (i=0; i < to_transfer; i++) {
				tmp = *regs->sprdat;
//				dprintf(DEBUG_SPI_SPEW, "sprdat: 0x%x\n",tmp);
				((uint8_t *)spi_status[port].rx_buf)[spi_status[port].rx_pos + i] = tmp; //*regs->sprdat;
			}

			spi_status[port].rx_pos += to_transfer;

			if (spi_status[port].rx_pos >= spi_status[port].rx_len) {
				spi_status[port].rx_complete = 1;
				spi_status[port].rx_buf = 0;
				spi_status[port].shadow_spcon &= ~(1 << SPICON_IE_RX_SHIFT);
				*regs->spcon = spi_status[port].shadow_spcon;
				//Hm, so RX finished, is it possible that TX is finished aswell?
				goto handle_tx;
			}

#if !SPI_AGD_MODE
			if (spi_status[port].tx_buf == 0) {
				uint32_t txfifo_left = SPI_FIFO_SIZE - BITS_SHIFT(status, SPSTA_TX_FIFO_BIT_HI, SPSTA_TX_FIFO_BIT_LO);
				uint32_t cnt = spi_status[port].rx_len - spi_status[port].rx_pos;
				if (cnt > txfifo_left) cnt = txfifo_left;
				while (cnt--) *regs->sptdat = 0xff;
			}
#endif
		}
	}

	if ((spi_status[port].tx_buf == 0) && (spi_status[port].rx_buf == 0)) {
		spi_status[port].shadow_spcon &= ~(1 << SPICON_IE_TR_SHIFT);
		*regs->spcon = spi_status[port].shadow_spcon;
		event_signal(&spi_status[port].event);
	}
}

#if SPI_DMA
static void spi_dma_tx_handler(struct dma_xfer_t *xfer, int error)
{
	struct spi_status_t *spi = xfer->user;
	
	dprintf(DEBUG_SPI_SPEW, "spi_dma_tx_handler(): %d\n", error);

	spi->tx_complete = 1;
}

static void spi_dma_rx_handler(struct dma_xfer_t *xfer, int error)
{
	struct spi_status_t *spi = xfer->user;
	
	dprintf(DEBUG_SPI_SPEW, "spi_dma_rx_handler(): %d\n", error);

	spi->rx_complete = 1;
}

static void spi_cancel_dma(int port)
{
	if(spi_status[port].dma) {
		cancel_dma_transfers(spi_status[port].rx_dma);
		cancel_dma_transfers(spi_status[port].tx_dma);
	}
}
#endif

bool spi_tx_complete(int port)
{
	return spi_status[port].tx_complete;
}

bool spi_rx_complete(int port)
{
	return spi_status[port].rx_complete;
}

int spi_write_etc(int port, const void *buf, size_t len, bool wait, bool with_rx)
{
	const spi_regs_t *regs = spiregs(port);
	uint32_t i;
	int err = 0;

	dprintf(DEBUG_SPI_SPEW, "spi_write(): port %d (%p) buf %p len %zu\n", port, &spi_status[port], buf, len);

	if (regs == 0)
		return -1;

	/* clear the tx & rx fifos */
	spi_flush_tx_fifo(port);
	spi_flush_rx_fifo(port);
	
#if SPI_DMA
	if (spi_status[port].dma) {
		spi_status[port].tx_iov.iov_base = (void *)buf;
		spi_status[port].tx_iov.iov_len = len;
		spi_status[port].tx_xfer.dev_addr = (void *)regs->sptdat;
		spi_status[port].tx_xfer.handler = spi_dma_tx_handler;
		spi_status[port].tx_xfer.iov = &spi_status[port].tx_iov;
		spi_status[port].tx_xfer.niov = 1;
		spi_status[port].tx_xfer.user = &spi_status[port];
		spi_status[port].tx_xfer.platform_flags = H1_DMA_FLAG__8BIT_TRANSFER | H1_DMA_FLAG__4_BURST_PERIPHERAL;
		spi_status[port].tx_complete = 0;

		if (!with_rx)
			*regs->spcnt = 0;

		spi_set_port_enable(port, true);

		platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, 0, 0);
		err = queue_dma_transfer(spi_status[port].tx_dma, &spi_status[port].tx_xfer);
		if (err < 0) {
			dprintf(DEBUG_CRITICAL, "error 0x%x queuing tx dma\n", err);
			return -1;
		}

		if (wait) {
			while (!spi_status[port].tx_complete || BITS_SHIFT(regs->spsta, SPSTA_TX_FIFO_BIT_HI, SPSTA_TX_FIFO_BIT_LO)) {
				task_yield();
//				dprintf(DEBUG_SPI_SPEW, "spsta 0x%x\n", *regs->spsta);
			}
			err = len;
		}
	} else {
#endif
		spi_status[port].tx_buf = buf;
		spi_status[port].tx_pos = __min(len, 8u);
		spi_status[port].tx_len = len;
		spi_status[port].tx_complete = 0;
		spi_status[port].rx_buf = 0;
		spi_status[port].rx_pos = 0;
		spi_status[port].rx_len = 0;

		event_unsignal(&spi_status[port].event);

		if (with_rx) {
			spi_status[port].shadow_spcon |= (1 << SPICON_IE_RX_SHIFT);
		} else {
			*regs->spcnt = 0;
		}

		for (i=0; i < spi_status[port].tx_pos; i++)
			*regs->sptdat = ((uint8_t *)buf)[i];

#if SPI_VERSION > 0
		*regs->sptdcnt = len;
		spi_status[port].shadow_spcon |= (1 << SPICON_IE_TX_SHIFT);
#endif

		spi_status[port].shadow_spcon |= (1 << SPICON_IE_TR_SHIFT);
		*regs->spcon = spi_status[port].shadow_spcon;

		spi_set_port_enable(port, true);

		if (wait) {
			while (!spi_status[port].tx_complete) {
				event_wait(&spi_status[port].event);
			}

			while (BITS_SHIFT(*regs->spsta, SPSTA_TX_FIFO_BIT_HI, SPSTA_TX_FIFO_BIT_LO));

			err = len;
		}
#if SPI_DMA
	}
#endif

	return err;
}

int spi_write(int port, const void *buf, size_t len)
{
	return spi_write_etc(port, buf, len, true, false);
}

int spi_read_etc(int port, void *buf, size_t len, bool wait, bool with_tx)
{
	const spi_regs_t *regs = spiregs(port);
	bool enable_tx_int = false;
	int err = 0;

	dprintf(DEBUG_SPI_SPEW, "spi_read():  port %d (%p) buf %p len %zu\n", port, &spi_status[port], buf, len);

	if (regs == 0)
		return -1;

	/* clear the tx & rx fifos */
	spi_flush_tx_fifo(port);
	spi_flush_rx_fifo(port);
	
#if SPI_DMA
	if (spi_status[port].dma) {
		spi_status[port].rx_iov.iov_base = (void *)buf;
		spi_status[port].rx_iov.iov_len = len;
		spi_status[port].rx_xfer.dev_addr = (void *)regs->sprdat;
		spi_status[port].rx_xfer.handler = spi_dma_rx_handler;
		spi_status[port].rx_xfer.iov = &spi_status[port].rx_iov;
		spi_status[port].rx_xfer.niov = 1;
		spi_status[port].rx_xfer.user = &spi_status[port];
		spi_status[port].rx_xfer.platform_flags = H1_DMA_FLAG__8BIT_TRANSFER | H1_DMA_FLAG__4_BURST_PERIPHERAL;
		spi_status[port].rx_complete = 0;

		*regs->spcnt = len;

		spi_set_port_enable(port, true);

		if (!with_tx) {
			spi_set_port_enable(port, true);
			/* start the transmitter, which should start the receive */
			spi_status[port].shadow_spcon |= (1 << SPICON_AGD_SHIFT); // set auto garbage bit
		}

		platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, 0, 0);
		err = queue_dma_transfer(spi_status[port].rx_dma, &spi_status[port].rx_xfer);
		if (err < 0) {
			dprintf(DEBUG_CRITICAL, "error 0x%x queuing rx dma\n", err);
			return -1;
		}

		if (wait) {
			while (!spi_status[port].rx_complete) {
				task_yield();
//				dprintf(DEBUG_SPI_SPEW, "spsta 0x%x\n", *regs->spsta);
			}
			if (!with_tx) {
				spi_status[port].shadow_spcon &= ~(1 << SPICON_AGD_SHIFT); // clear auto garbage bit
				*regs->spcon = spi_status[port].shadow_spcon;
			}
			err = len;
		}
	} else {
#endif
		spi_status[port].rx_buf = buf;
		spi_status[port].rx_pos = 0;
		spi_status[port].rx_len = len;
		spi_status[port].rx_complete = 0;
		spi_status[port].overrun_errors = 0;
		spi_status[port].tx_buf = 0;
		spi_status[port].tx_pos = 0;
		spi_status[port].tx_len = 0;

		event_unsignal(&spi_status[port].event);

		*regs->spcnt = len;

		spi_set_port_enable(port, true);

		if (with_tx) {
			enable_tx_int = true;
		} else {
			/* start the transmitter, which should start the receive */
#if SPI_AGD_MODE
			spi_status[port].shadow_spcon |= (1 << SPICON_AGD_SHIFT); // set auto garbage bit
#else
			int cnt = spi_status[port].rx_len - spi_status[port].rx_pos;
			if (cnt > SPI_FIFO_SIZE) cnt = SPI_FIFO_SIZE;
			while (cnt--) *regs->sptdat = 0xff;
			enable_tx_int = true;
#endif
		}

#if SPI_VERSION > 0
		if (enable_tx_int) {
			*regs->sptdcnt = len;
			spi_status[port].shadow_spcon |= (1 << SPICON_IE_TX_SHIFT);
		}
#endif

		spi_status[port].shadow_spcon |= (1 << SPICON_IE_RX_SHIFT) | (1 << SPICON_IE_TR_SHIFT);
		*regs->spcon = spi_status[port].shadow_spcon;

		if (wait) {
			while (!spi_status[port].rx_complete) {
				event_wait(&spi_status[port].event);
			}
			if (!with_tx) {
				spi_status[port].shadow_spcon &= ~(1 << SPICON_AGD_SHIFT); // clear auto garbage bit
				*regs->spcon = spi_status[port].shadow_spcon;
			}

			err = len;
		}
#if SPI_DMA
	}
#endif

	return err;
}

int spi_read(int port, void *buf, size_t len)
{
	return spi_read_etc(port, buf, len, true, false);
}

void spi_init(void)
{
	uint32_t port;
	const spi_regs_t *regs;

	dprintf(DEBUG_CRITICAL, "spi_init()\n");

	/* clear out the spi structure */
	bzero(spi_status, sizeof(spi_status));

	for (port = 0; port < SPIS_COUNT; port++) {
		regs = spiregs(port);
		if (!regs)
			continue;

		event_init(&spi_status[port].event, EVENT_FLAG_AUTO_UNSIGNAL, false);

		/* make sure the clock is on for this device */
		clock_gate(regs->clock, true);

		/* default clock source to NCLK */
		spi_status[port].clk = NCLK;

		/* turn off all the spi ports */
		spi_set_port_enable(port, false);

#if SPI_DMA
		/* start off in interrupt driven mode */
		spi_status[port].tx_dma = spi_status[port].rx_dma = -1;
		spi_set_port_dma(port, false);
#endif

		/* register interrupt handlers */
		install_int_handler(regs->irq, &spi_interrupt, (void *)port);
		unmask_int(regs->irq);
	}
}

uint32_t spi_gpio_read(int port)
{
	const spi_regs_t *regs = spiregs(port);

	if (regs == 0)
		return 0;

	return (*regs->sppin >> 1) & 1;
}

void spi_gpio_write(int port, uint32_t val)
{
	const spi_regs_t *regs = spiregs(port);

	if (regs == 0)
		return;

	if (val) *regs->sppin |= (1 << 1);
	else *regs->sppin &= ~(1 << 1);
}

void spi_gpio_configure(int port, uint32_t config)
{
	const spi_regs_t *regs = spiregs(port);

	if (regs == 0)
		return;

	switch (config) {
		case GPIO_CFG_OUT_0 :
			*regs->sppin &= ~(1 << 1);
			break;

		case GPIO_CFG_OUT_1 :
			*regs->sppin |= (1 << 1);
			break;
	}
}
