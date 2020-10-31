/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <drivers/pke.h>
#include <platform/clocks.h>
#include <platform/power.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwregbase.h>
#include <string.h>
#include <sys.h>

#include "AppleS5L8900XPKE-hardware.h"


#define Pke_Shim_DEBUG 0

#if Pke_Shim_DEBUG
static void debug_data(uint8_t *data, size_t length)
{
    uint32_t i=0;
    printf("data %p,%d\n" , data, length);
    for (i = 0; i < length; i++) {
		if( !(i%256) )	
			printf("Seg : %p \n", data);
        printf("%02X, ", *(data + i));
        if ((i % 16) == 15)
            printf("\n");
    }
    printf("\n");
}
#define debug(fmt, args...)	printf("pke_new::%s: " fmt "\n", __FUNCTION__ , ##args)
#else
#define debug(fmt, args...)
#define debug_data(data, length)
#endif


//PKE hardware memory map.
static volatile struct pke_regs *registers = (struct pke_regs *)PKE_BASE_ADDR;
static volatile uint8_t *memory = (uint8_t*)(PKE_BASE_ADDR + 0x800);

extern bool rsa_cal_exp(void *dst, uint32_t *len, uint32_t options,
					uint8_t *rsquare, uint32_t *rsquare_length,
					uint8_t * const base, uint32_t base_length, 
					uint8_t * const expn, uint32_t expn_length, 
					uint8_t * const modulus, uint32_t modulus_length, 
					volatile struct pke_regs *registers, volatile uint8_t *memory);

bool pke_do_exp(uint8_t *dst, size_t *len, size_t key_length, 
                                uint8_t * const base, size_t base_length,
                                uint8_t * const expn, size_t expn_length,
                                uint8_t * const mods, size_t mods_length)
{
	bool ret = false;
	uint32_t rsquare[64]; //word aligned 256byte buffer.
	uint32_t rlen = sizeof(rsquare);

	clock_gate(CLK_PKE, true);	
	platform_power_set_gate(PWRBIT_USB, true);
	
#if Pke_Shim_DEBUG	
	utime_t st, et;
	st = system_time();
#endif
	ret = rsa_cal_exp(dst, (uint32_t*)len, 0,
					(uint8_t*)rsquare, &rlen,
					base, (uint32_t)base_length, 
					expn, (uint32_t)expn_length, 
					mods, (uint32_t)mods_length, 
					registers, memory);
#if Pke_Shim_DEBUG	
	et = system_time();
	debug("rsa_cal_exp took %llu micros for keylen: %lu", (et-st), key_length);
	//debug("The result:");
	//debug_data(dst, *len);
#endif

	clock_gate(CLK_PKE, false);	
	return ret;
}
