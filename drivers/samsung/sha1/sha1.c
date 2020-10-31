/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
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
#include <platform/clocks.h>
#include <platform/int.h>
#include <platform/soc/hwclocks.h>
#include <sys.h>
#include <sys/task.h>

#include "sha1.h"


static const sha1_regs sha1_reg = {
	&rSHA1_ADDR_CONF, &rSHA1_ADDR_SWRESET, &rSHA1_ADDR_INT_SRC, &rSHA1_ADDR_INT_MASK,
	&rSHA1_ADDR_ENDIAN, &rSHA1_ADDR_CODE0, &rSHA1_ADDR_CODE1, &rSHA1_ADDR_CODE2,
	&rSHA1_ADDR_CODE3, &rSHA1_ADDR_CODE4, &rSHA1_ADDR_DATA0, &rSHA1_ADDR_DATA1,
	&rSHA1_ADDR_DATA2, &rSHA1_ADDR_DATA3, &rSHA1_ADDR_DATA4, &rSHA1_ADDR_DATA5,
	&rSHA1_ADDR_DATA6, &rSHA1_ADDR_DATA7, &rSHA1_ADDR_DATA8, &rSHA1_ADDR_DATA9,
	&rSHA1_ADDR_DATA10, &rSHA1_ADDR_DATA11, &rSHA1_ADDR_DATA12, &rSHA1_ADDR_DATA13,
	&rSHA1_ADDR_DATA14, &rSHA1_ADDR_DATA15, &rSHA1_MASTER_MODE, &rSHA1_MS_START_ADDR,
	&rSHA1_MS_SIZE };
static sha1_status sha1Status = { 0, 0, 0 };

static void sha1Run(u_int8_t *buf);
static void Sha1NewInit(void);

void Sha1NewAddEnd(u_int32_t dataLen, u_int32_t offset, u_int32_t startCmd);


static void Sha1NewInit(void)
{
	const sha1_regs *reg;

	reg = &sha1_reg;
	clock_gate(CLK_SHA1, true);

	/* wait for SHA-1 engine idle */
	while(*reg->con & (0x1<<0))
		;
	/* Software reset */
	*reg->swreset = 0x1;
	*reg->swreset = 0x0;

	/* Reset Configuration Register */
	*reg->con = 0x0;

	/* Always use slave mode */
	*reg->mode = 0x0;

	/* Always use twist / little endian */
	*reg->twist = 0x0;

	sha1Status.cmd = 1;
	sha1Status.offset = 0;
	sha1Status.length = 0;
}

/* Fill the frame buffer and send it out
 * the buffer size is always 64 bytes
 * Caller is responsible to set the End bit and 0 padding
 */
static void sha1Run(u_int8_t *buf)
{
	const sha1_regs *reg;

	reg = &sha1_reg;

	/* Fill the buffer */
	for (int i = 0; i < 16; i++) {
		volatile u_int32_t *data;
		data = reg->data0;
#if WITH_UNALIGNED_MEM
		*(data + i) = ((u_int32_t *)buf)[i];
#else
		*(data + i) = (buf[i*4] << 0)|(buf[i*4+1] << 8)|(buf[i*4+2] << 16)|(buf[i*4+3] << 24);
#endif
	}

	/* Run the SHA1 engine */
	if (sha1Status.cmd) {
		/* Start new hash code */
		*reg->con  &= ~(1<<3);
        	sha1Status.cmd = 0;
	} else {
		/* Continue hash code */
		*reg->con |= (1<<3);
	}

	*reg->con |= 1<<1;
	while(*reg->con & (1<<0))
                ;
}


/************************************************************************
 * void sha1_hw_calculate(const void *buf, size_t len, u_int32_t *result)
 *    -- We are using slave mode only so the code would be portable
 *       between H1, H2, M1, M2.
 *    -- Caller's responsibility to make sure buffer can hold 20 bytes
 *       SHA1 code.
 ************************************************************************/
void sha1_hw_calculate(const void *buf, size_t len, u_int32_t *result)
{
	u_int32_t  length, counter;
	u_int8_t   *frame;
	u_int64_t  sha1TotalBits;
	u_int8_t   sha1TotalLenAdd[8];
	const sha1_regs *reg;

	Sha1NewInit();

	reg = &sha1_reg;
	frame = (u_int8_t *)buf;
	length = len;
	counter =0;

	/* Prepare the 64 bits length */
	sha1TotalBits = len * 8;
	sha1TotalLenAdd[0] = ((sha1TotalBits >> 32) & (0xff << 24)) >> 24;
	sha1TotalLenAdd[1] = ((sha1TotalBits >> 32) & (0xff << 16)) >> 16;
	sha1TotalLenAdd[2] = ((sha1TotalBits >> 32) & (0xff << 8)) >> 8;
	sha1TotalLenAdd[3] = ((sha1TotalBits >> 32) & (0xff << 0)) >> 0;
	sha1TotalLenAdd[4] = (sha1TotalBits & (0xff << 24)) >> 24;
	sha1TotalLenAdd[5] = (sha1TotalBits & (0xff << 16)) >> 16;
	sha1TotalLenAdd[6] = (sha1TotalBits & (0xff <<  8)) >>  8;
	sha1TotalLenAdd[7] = (sha1TotalBits & (0xff <<  0)) >>  0;

	while (length > 64) {
		sha1Run(&frame[sha1Status.length]);
		sha1Status.length += 64;
		length -= 64;
		counter += 64;
		if (0x7FFFF < counter) {
			task_yield();
			counter = 0;
		}
	}

	if (length > 55) {
		/* Not enough space for the length bytes */
		u_int8_t frameBuf[64];

		for (u_int32_t i = 0; i < 64; i++) {
			if (length > i) {
				frameBuf[i] = frame[sha1Status.length++];
			} else if (length == i) {
				frameBuf[i] = 0x80;
			} else {
				frameBuf[i] = 0x0;
			}
		}
		sha1Run(frameBuf);

		/* Add End bit 0x80 if not added above */
		if (length == 64) {
			frameBuf[0] = 0x80;
		} else {
			frameBuf[0] = 0x0;
		}

		/* Start a new frame to hold the length */
		for (int j = 1; j < 56; j++) {
			frameBuf[j] = 0x0;
		}

		/* Now store the length */
		for (int k = 0; k < 8; k++) {
			frameBuf[56 + k] = sha1TotalLenAdd[k];
		}
		sha1Run(frameBuf);
	} else {
		/* Add End bit 0x80 and 0 padding */
		u_int8_t frameBuf[64];

		for (u_int32_t i = 0; i < 56; i++) {
			if (length > i) {
				frameBuf[i] = frame[sha1Status.length++];
			} else if (length == i) {
				frameBuf[i] = 0x80;
			} else {
				frameBuf[i] = 0x0;
			}
		}

		/* Now store the length */
		for (int j = 0; j < 8; j++) {
			frameBuf[56 + j] = sha1TotalLenAdd[j];
		}
		sha1Run(frameBuf);
		
	}

	/* Get the result */
	result[0] = *reg->code0;
	result[1] = *reg->code1;
	result[2] = *reg->code2;
	result[3] = *reg->code3;
	result[4] = *reg->code4;

	clock_gate(CLK_SHA1, false);
}
