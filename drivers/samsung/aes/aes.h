/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */

#ifndef _SAMSUNG_AES_H
#define _SAMSUNG_AES_H

#include <platform/soc/hwregbase.h>
#include <sys/types.h>
#include <drivers/aes.h>

__BEGIN_DECLS

#define	 rAES_POWER		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x00))
#define	 rAES_COMMAND		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x04))
#define	 rAES_SWRST		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x08))
#define	 rAES_IRQ		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x0C))
#define	 rAES_IRQ_MASK		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x10))
#define	 rAES_CFG		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x14))
#define	 rAES_XFR_NUM		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x18))
#define	 rAES_XFR_CNT		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x1C))
#define	 rAES_TBUF_START	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x20))
#define	 rAES_TBUF_SIZE		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x24))
#define	 rAES_SBUF_START	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x28))
#define	 rAES_SBUF_SIZE		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x2C))
#define	 rAES_CRYPT_START	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x30))
#define	 rAES_CRYPT_SIZE	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x34))
#define	 rAES_CADDR_TBUF	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x38))
#define	 rAES_CADDR_SBUF	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x3C))
#define	 rAES_XFR_STATUS	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x40))
#define	 rBUS_FIFO_STATUS	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x44))
#define	 rAES_FIFO_STATUS	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x48))
#define	 rAES_KEY_MX		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x4C))
#define	 rAES_KEY_MH		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x50))
#define	 rAES_KEY_MM		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x54))
#define	 rAES_KEY_ML		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x58))
#define	 rAES_KEY_X		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x5C))
#define	 rAES_KEY_H		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x60))
#define	 rAES_KEY_M		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x64))
#define	 rAES_KEY_L		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x68))
#define	 rAES_CIPHERKEY_SEL	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x6C))
#define	 rAES_ENDIAN		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x70))
#define	 rAES_IV_1		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x74))
#define	 rAES_IV_2		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x78))
#define	 rAES_IV_3		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x7C))
#define	 rAES_IV_4		(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x80))
#define	 rAES_COMPLIMENT	(*(volatile u_int32_t *)(AES_BASE_ADDR + 0x88))

#define CMD_AES_STOP		0	//Command Stop (Pause)
#define CMD_AES_START		1	//Command Start (only available in idle state)
#define CMD_AES_ABORT		2	//Command Abort
#define CMD_AES_CONTINUE	3	//Command Continue (only available in pause state)

#define AES_KEY_TYPE_USER_DEFINE	0
#define AES_KEY_TYPE_GLOBAL_ID		1
#define AES_KEY_TYPE_USER_ID		2

#define AES_IRQ_XFR_DONE_BIT	(1<<0)
#define AES_IRQ_TBUF_FULL_BIT	(1<<1)
#define AES_IRQ_SBUF_EMPTY_BIT	(1<<2)
#define AES_IRQ_ILLEGAL_OP_BIT	(1<<3)

#define AES_MODULE_POWER_ON()			rAES_POWER = 0x00000001
#define AES_MODULE_POWER_OFF()			rAES_POWER = 0x00000000

/**
*	AES KEY SIZE
*/
enum aes_key_size {
	_AES_KEY_SIZE_128 = 0,
	_AES_KEY_SIZE_192,
	_AES_KEY_SIZE_256,

	_AES_KEY_SIZE_MAX
};


#define AES_MODE_ECB		0	/*Electronic Codebook*/
#define AES_MODE_CBC		1	/*Cipher Block Chaining*/

u_int32_t AesInit(void);
u_int32_t AesUninit(void);
u_int32_t AesWaitForComplete(void);

void AesSetKeySize(enum aes_key_size keysize);
void AesSetChainingMode(bool bFlag);
void AesSetCryption(bool bFlag);

void _AesStart(void);

__END_DECLS

#endif /* ! _SAMSUNG_AES_H */
