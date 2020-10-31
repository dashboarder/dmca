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
#include <debug.h>

#include <target/gpiodef.h>
#include <drivers/spi.h>

static void spi_sample_start(void)
{
	/* Assert the clock to begin bit sample */
	gpio_write(GPIO_SPI_SCK, 0);
}

static void spi_sample_end(void)
{
	/* Deassert the clock to end bit sample */
	gpio_write(GPIO_SPI_SCK, 1);
}

static unsigned char spi_in(unsigned char bit)
{
	/* If the GPIO is set, then this bit is set, and we return the byte value */
	return (gpio_read(GPIO_SPI_MISO) ? 1 : 0) << bit;
}

static void spi_out(unsigned char data, unsigned char bit)
{
	/* Write bit to GPIO */
	gpio_write(GPIO_SPI_MOSI, data & (1 << bit));
}

void spi_init(void)
{
	/* Assert clock and deassert slave select */
	spi_sample_start( );
	spi_end();
}

void spi_start(void)
{
	/* Assert slave select to begin SPI transaction */
	gpio_write(GPIO_SPI_SS, 0);
}

void spi_end(void)
{
	/* Deassert slave select to end SPI transaction */
	gpio_write(GPIO_SPI_SS, 1);
}

void spi_read(unsigned char* data)
{
	signed int i;
	
	/* Start with initial state */
	*data = 0;
    
	/* Loop every bit */
	for (i = 7; i >= 0; i--)
	{
		/* Assert clock */
		spi_sample_start();
        
		/* Dummy write cycle */
		spi_out(0, 0);
        
		/* Read one bit */
		*data |= spi_in(i);
        
		/* Deassert clock */
		spi_sample_end();
	}
}

void spi_write(unsigned char data)
{
	signed int i;
	
	/* Loop every bit */
	for (i = 7; i >= 0; i--)
	{
		/* Assert clock */
		spi_sample_start();
        
		/* Write one bit */
		spi_out(data, i);
        
		/* Dummy read cycle */
		spi_in(0);
        
		/* Deassert clock */
		spi_sample_end();
	}
}
