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
#include <lib/libc.h>
#include <platform.h>
#include <platform/int.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>
#include <sys.h>

#include "aes.h"

/* aes driver, copy and customize */
#define AES_MAX_BOUNCE_BUFFER 64

static volatile u_int32_t gAesIrqDoneFlag = 0;
static u_int8_t g_bounce_buffer[AES_MAX_BOUNCE_BUFFER] __attribute__ ((aligned (32)));
static int aes_inited = 0;
static int key_size_to_bytes(enum aes_key_size size)
{
	return 16 + (size * 8);
}

void aes_init(void)
{
	dprintf(DEBUG_SPEW,"aes_init()\n");
	clock_gate(CLK_AES, true);
	
	AES_MODULE_POWER_ON();
	AesInit();
	aes_inited = 1;
}

/*! \fn void AesInit(void)
*  \brief	power on and initialize AES module
*  \param	none
*  \return	none
*/
u_int32_t AesInit(void)
{
	gAesIrqDoneFlag = 0;
/* 	install_int_handler(INT_AES, Isr_AesHandler, 0); */
/* 	unmask_int(INT_AES); */
	rAES_IRQ_MASK = 0x00000000;			/**  Mask all interrupt source */
	
	return 0;	
}

/*! \fn void AesUninit(void)
*  \brief	release AES module
*  \param	none
*  \return	none
*/
u_int32_t AesUninit(void)
{
	gAesIrqDoneFlag = 0;

	AES_MODULE_POWER_OFF();				
	//rAES_IRQ_MASK = 0x00000000;			/**  Mask all interrupt source */

/* 	mask_int(INT_AES); */
//	SysResetInterrupt(INT_AES);
	
	return 0;	
}

/*! \fn void AesOff(void)
*  \brief	Power Off AES module
*  \param	none
*  \return	
*/
void AesOff(void)
{
	AES_MODULE_POWER_OFF();			/** AES module is disabled and preparation for clock down */	
	//while(!(rAES_POWER & 0x02));			/** ready for clock down */
}

/*! \fn void _AesSetKeyType(u_int32_t cipherKeyType) 
*  \brief	power off AES module
*  \param	cipherKeyType cipher key type
*  \return	none
*/
void _AesSetKeyType(enum aes_key_type cipherKeyType)
{
	rAES_CIPHERKEY_SEL = cipherKeyType;
#if AES_VERSION > 0
	rAES_COMPLIMENT = ~rAES_CIPHERKEY_SEL;
#endif
}

/*! \fn void _AesSetKey(u_int8_t *pKeyArr)
*  \brief	set cipher keys into AES module
*  \param[in]	pKeyArr 128-bit Key
*  \return	none
*/
void _AesSetKey(u_int8_t *pKeyArr)
{
	/*<<< joe.kang@H1 061212: EVT 2 new code 061212*/
	u_int32_t index;
	u_int32_t uLength;
	volatile u_int32_t * reg;
	
	if(rAES_CIPHERKEY_SEL == 0) /* 0 is a user defined key */
	{
		switch ((rAES_CFG & 0x30)>>4)
		{

			case 1: // 192 bit
				reg = &rAES_KEY_MM;
				uLength = 6;
				break;
			case 2: // 256 bit
				reg = &rAES_KEY_MX;
				uLength = 8;
				break;
			case 0: // 128 bit
			default:
				reg = &rAES_KEY_X;
				uLength = 4;
				break;
		}

		for(index = 0 ;index < uLength ;index ++)
		{
#if AES_VERSION > 0
			*(reg + index) = *(u_int32_t *)(pKeyArr + index * 4);
#else
			*(reg + index) = swap32(*(u_int32_t *)(pKeyArr + index * 4));
#endif
		}
	}
}
/*! \fn void _AesClearKey(void)
*  \brief	clear cipher keys into AES module
*  \param[in]	void
*  \return	none
*/
void _AesClearKey(void)
{
	/*<<< joe.kang@H1 061212: EVT 2 new code 061212*/
	memset ((void *) &rAES_KEY_MX, 0, 32);
}

/*! \fn void _AesClearIV(void)
*  \brief	clear cipher keys into AES module
*  \param[in]	void
*  \return	none
*/
void _AesClearIV(void)
{
	/*<<< joe.kang@H1 061212: EVT 2 new code 061212*/
	memset ((void *) &rAES_IV_1, 0, 16);
}


/*! \fn void AesSoftwareReset(void) 
*  \brief	apply software reset to AES module
*  \param	none	
*  \return	none
*/
void AesSoftwareReset(void)
{
	rAES_SWRST = 0x00000001;			/**  Software Reset for AES  */
	rAES_SWRST = 0x00000000;			/**  No Reset */
}

/*! \fn INT32T AesReadyToStart(void);
 * @brief	This function set configurations for ready to start AES Engine.
 * 
 * @param	NONE
 * @return	0 if success. others if failed.
 * @see		AesNotifyComplete
 */
u_int32_t AesReadyToStart(void)
{
	/* Set ready to start */
	gAesIrqDoneFlag 	= 0;

	return 0;
}

void AesSetVector(u_int8_t * pLocalIv)
{
	int index;
	volatile u_int32_t * reg;
	
	if (pLocalIv == 0)
	{
		rAES_IV_1  = 0;
		rAES_IV_2  = 0;
		rAES_IV_3  = 0;
		rAES_IV_4  = 0;
	}
	else
	{
		/*<<< joe.kang@H1 061212: EVT 2 new code 061212*/
		reg = &rAES_IV_1;
		for(index = 0 ;index < 4 ;index ++)
		{
#if AES_VERSION > 0
			*(reg + index) = *(u_int32_t *)(pLocalIv + index * 4);
#else
			*(reg + index) = swap32(*(u_int32_t *)(pLocalIv + index * 4));
#endif
		}
	}		

	return;
}

/*! \fn void AesStartEncryption(u_int8_t *destAddr, u_int8_t *srcAddr, u_int32_t size, u_int32_t keyType, u_int32_t *keyArr)
*  \brief	encrypt data
*  \param	destAddr destination address	
*  \param	srcAddr source address	
*  \param	size number of bytes	
*  \param[in]	keyArr cipher key array	
*  \return	none
*/
void AesStartEncryption(
u_int8_t *destAddr, 
u_int8_t *srcAddr, 
u_int8_t *cryptoAddr, 
u_int32_t size, 
u_int32_t keyType, 
u_int8_t *keyArr,
u_int32_t keySize, 
bool	bChainingMode,
u_int32_t srcSize,
u_int32_t dstSize,
u_int32_t cryptoSize,
u_int8_t *pIv)
{
	/* Ready to start */
	AesReadyToStart();

	AesSoftwareReset();
	AES_MODULE_POWER_ON();				
	//	rAES_IRQ_MASK 		= 0x0000000F;					/**  Unmask all interrupt source */

	_AesSetKeyType(keyType);
	_AesSetKey(keyArr);
	//if(bChainingMode == true)
		//AesSetVector();
	/**  aes key size : 128bit, stay in pause state & wait for CPU's action, Encryption */
 	rAES_CFG  			= (0x3<<1);					
	AesSetCryption(true);
	AesSetKeySize(keySize);
	AesSetChainingMode(bChainingMode);

	AesSetVector(pIv);

	rAES_XFR_NUM 		= size;							/**  Total Transfer Number (number of bytes) */
	rAES_TBUF_START 	= (u_int32_t)destAddr;		/**  Start Address of Target Buffer (byte aligned) */
	rAES_TBUF_SIZE 		= dstSize;							/**  Size of Taget Buffer (byte aligned) */


	rAES_SBUF_START 	= (u_int32_t)srcAddr;		/**  Start Address of Source Buffer (byte aligned) */
	rAES_SBUF_SIZE 		= srcSize;							/**  Size of Source Buffer (byte aligned) */


	rAES_CRYPT_START 	= (u_int32_t)cryptoAddr;		/**  Starting Address of Encryption in Source Buffer (byte aligned) */
	rAES_CRYPT_SIZE 	= cryptoSize;							/**  Size of Encryption (16 byte aligned) */

	gAesIrqDoneFlag = 0;

	_AesStart();
}

/*! \fn void AesStartDecryption(u_int8_t *destAddr, u_int8_t *srcAddr, u_int32_t size, u_int32_t keyType, u_int32_t *keyArr)
*  \brief	decrypt data
*  \param	destAddr destination address	
*  \param	srcAddr source address	
*  \param	size number of bytes	
*  \param[in]	keyArr cipher key array	
*  \return	none
*/
void AesStartDecryption(
u_int8_t *destAddr, 
u_int8_t *srcAddr, 
u_int8_t *cryptoAddr, 
u_int32_t size, 
u_int32_t keyType, 
u_int8_t *keyArr,
u_int32_t keySize, 
bool	bChainingMode,
u_int32_t srcSize,
u_int32_t dstSize,
u_int32_t cryptoSize,
u_int8_t *pIv)
{
	/* Ready to start */
	AesReadyToStart();

	AesSoftwareReset();
	AES_MODULE_POWER_ON();

	_AesSetKeyType(keyType);

	//	rAES_IRQ_MASK 		= 0x0000000F;			/**  Unmask all interrupt source */

	_AesSetKey(keyArr);
	
	//if(bChainingMode == true)
		//AesSetVector();
	/**  aes key size : 128bit, stay in pause state & wait for CPU's action, Decryption */
 	rAES_CFG = (0x3<<1);					
	AesSetCryption(false);
	AesSetKeySize(keySize);
	AesSetChainingMode(bChainingMode);

	AesSetVector(pIv);

	rAES_XFR_NUM 		= size;
	rAES_TBUF_START 	= (u_int32_t)destAddr;		/**  Start Address of Target Buffer (number of bytes) */
	rAES_TBUF_SIZE 		= dstSize;							/**  Size of Target Buffer (byte aligned) */
	rAES_SBUF_START 	= (u_int32_t)srcAddr;		/**  Start Address of Source Buffer (byte aligned) */
	rAES_SBUF_SIZE 		= srcSize;			/**  Unmask all interrupt source */


	rAES_CRYPT_START 	= (u_int32_t)cryptoAddr;		/**  Starting Address of Decryption in Source Buffer (byte aligned) */
	rAES_CRYPT_SIZE 	= cryptoSize;							/**  Size of Decryotion (16 byte aligned) */

	gAesIrqDoneFlag = 0;

	_AesStart();
}

/*! \fn void _AesStart(void)
*  \brief	start aes
*  \param	none	
*  \return	none
*/
void _AesStart(void)
{
#if AES_SYNCH_START
	rAES_ENDIAN = (1 << 1);		// Set Synch-Start bit
#else
	rAES_COMMAND = CMD_AES_START;	// Set Asynch-Start bit
#endif
}

/*! \fn u_int32_t AesGetIrqStatus(void)
*  \brief Get IRQ status return
*  \param	none	
*  \return	none
*/
u_int32_t 
AesGetIrqStatus(void)
{
	return rAES_IRQ;
}

/*! \fn void AesGetIrqStatus(u_int32_t status)
*  \brief Set IRQ status return
*  \param	IRQ status	
*  \return	none
*/
void AesSetIrqStatus(u_int32_t status)
{
	rAES_IRQ = status;
}

/*! \fn INT32T AesNotifyComplete(void);
 * @remark	This function must be implemented by apple. SAMSUNG just offer
 *          a reference code.
 * 
 * @brief	Nofity AES operation complete.
 * @param	NONE
 * @return	@c 0 if success. @c -1 if failed.
 * @see		AesWaitForComplete, AesReadyToStart
 */
u_int32_t AesNotifyComplete(void)
{
	gAesIrqDoneFlag = 1;

	return 0;
}

/*! \fn INT32T AesWaitForComplete(void);
 * \remark	This function must be implemented by apple. SAMSUNG just offer
 *          a reference code.
 * \brief	Wait until AES operation complete.
 * \param	NONE
 * \return	@c 0 if success. @c -1 if failed.
 * \see		AesNotifyComplete
 */
u_int32_t
AesWaitForComplete(void)
{
	/* polling until AES I/O complete */
	//while(gAesIrqDoneFlag == 0);
	while( 0 == (rAES_IRQ & 0xf) ) ;
#if AES_SYNCH_START
	/* synch zeroes XFR_NUM - must wait to avoid racing */
	while( 0 != rAES_XFR_NUM ) ;
#endif
	/*<<< joe.kang@H1 061212: EVT 2 new code 061212*/
	_AesClearKey();
	_AesClearIV();
	// clear all pending IRQs
	rAES_IRQ = 0xf;

	return 0;
}

/*! \fn void Isr_AesHandler(void)
*  \brief	interrupt service routine
*  \param	none	
*  \return	none
*/
void Isr_AesHandler(void *arg)
{
	u_int32_t	status;

	status = rAES_IRQ;
	
	dprintf(DEBUG_SPEW,"aes_int: status 0x%x\n", status);
#if 0
	/* check the status */
	if(status & AES_IRQ_XFR_DONE_BIT)
	{
		gAesIrqDoneFlag = 1;
	}
	
	if(status & AES_IRQ_TBUF_FULL_BIT)
	{
		g_pRomGlobals->gAesIrqDoneFlag = 1;
	}

	if(status & AES_IRQ_SBUF_EMPTY_BIT)
	{
		;
	}

	if(status & AES_IRQ_ILLEGAL_OP_BIT)
	{
		;
	}
#endif

	/* clear status register */
	rAES_IRQ = status;

	/* Notify to the module pended */
	if (AesNotifyComplete() != 0)
	{
		panic("AesNotifyComplete() return !0");
	}
}

/*! \fn void AesSetKeySize(void) 
*  \brief	set key size
*  \param	none	
*  \return	none
*/
void AesSetKeySize(enum aes_key_size keysize)
{
	u_int32_t	rdata;
	u_int32_t	wmask;
	u_int32_t	wdata;
	
	wmask = (3<<4);
	rdata = rAES_CFG & (~wmask);
	
	wdata = rdata | (keysize<<4);
	rAES_CFG = wdata;
}

/*! \fn void AesSetChainingMode(void) 
*  \brief	select chaining Mode
*  \param	none	
*  \return	none
*/
void AesSetChainingMode(bool bFlag)
{
	u_int32_t	rdata;
	u_int32_t	wmask;
	u_int32_t	wdata;
	
	wmask = (1<<3);
	rdata = rAES_CFG & (~wmask);
	
	wdata = rdata | (bFlag<<3);
	rAES_CFG = wdata;
}
/*! \fn void AesSetCryption(void) 
*  \brief	select cryption mode
*  \param	none	
*  \return	none
*/
void AesSetCryption(bool bFlag)
{
	/**
	* bFlag 0: decryption / 1: encryption
	*/
	u_int32_t	rdata;
	u_int32_t	wmask;
	u_int32_t	wdata;
	
	wmask = (1<<0);
	rdata = rAES_CFG & (~wmask);
	
	wdata = rdata | (bFlag<<0);
	rAES_CFG = wdata;
}

bool 
AesGetCryption(void)
{
	bool result;
	
	result = (bool)(rAES_CFG & 0x01);
	
	return result;
}

/*! 
 */
void
AesSetCommand(u_int32_t cmd)
{
	rAES_COMMAND = cmd;
}

/*!
 */
void 
AesSetSourceConfig(u_int32_t addr, u_int32_t size)
{
	rAES_SBUF_START = addr;

	if(size != 0xffffffff)
	{
		rAES_SBUF_SIZE = size;
	}
}
/*!
 */
void 
AesSetCryptConfig(u_int32_t addr, u_int32_t size)
{
	rAES_CRYPT_START = addr;
	
	if(size != 0xffffffff)
	{
		rAES_CRYPT_SIZE = size;	
	}
}

int
AES_CBC_DecryptInPlace(u_int8_t * pAddr, u_int32_t u32Len, u_int32_t keyType, u_int8_t * pKey, u_int8_t * pIv)
{
	u_int8_t *addr = pAddr;

	aes_init();

	if ( AES_MAX_BOUNCE_BUFFER >= u32Len ) {
		memcpy( g_bounce_buffer, pAddr, u32Len );
		addr = g_bounce_buffer;
	}

	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, 0, 0);
	AesStartDecryption((u_int8_t *)addr, (u_int8_t *)addr, (u_int8_t *)addr, u32Len, keyType, pKey, 0, 1, u32Len, u32Len, u32Len, pIv);
	AesWaitForComplete();

	if ( AES_MAX_BOUNCE_BUFFER >= u32Len ) {
		memcpy( pAddr, addr, u32Len );
	}

	return 0;
}


bool
AES_CBC_EncryptInPlace(u_int8_t * pAddr, u_int32_t u32Len, u_int32_t keyType, u_int8_t * pKey, u_int8_t * pIv)
{
	u_int8_t *addr = pAddr;

	aes_init();

	if ( AES_MAX_BOUNCE_BUFFER >= u32Len ) {
		memcpy( g_bounce_buffer, pAddr, u32Len );
		addr = g_bounce_buffer;
	}

	platform_cache_operation(CACHE_CLEAN | CACHE_INVALIDATE, 0, 0);
	AesStartEncryption((u_int8_t *)addr, (u_int8_t *)addr, (u_int8_t *)addr, u32Len, keyType, pKey, 0, 1, u32Len, u32Len, u32Len, pIv);
	AesWaitForComplete();

	if ( AES_MAX_BOUNCE_BUFFER >= u32Len ) {
		memcpy( pAddr, addr, u32Len );
	}

	return 0;
}
