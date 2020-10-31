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
#include <string.h>
#include "AppleS5L8900XPKE-hardware.h"


#define AppleS5L8900XPKE_hardware_DEBUG 0
#undef absolutetime_to_nanoseconds

#if AppleS5L8900XPKE_hardware_DEBUG
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
#define debug(fmt, args...)	printf("AppleS5L8900XPKE-hardware::%s: " fmt "\n", __FUNCTION__ , ##args)
#else
#define debug(fmt, args...)
#define debug_data(data, length)
#endif


#if IN_KERNEL
extern uint64_t iodelay_time[3];
static uint64_t delay_time;
#endif 


uint32_t
PkeSetMode(uint32_t uSegSize)
{
    uint32_t uWriteVal;

    switch (uSegSize) {
        case PKE_SEG_SIZE_256:
            uWriteVal = PKE_REGVAL_SEG_SIZE_SEGSIZE_256;
            break;

        case PKE_SEG_SIZE_128:
            uWriteVal = PKE_REGVAL_SEG_SIZE_SEGSIZE_128;
            break;

        case PKE_SEG_SIZE_64:
            uWriteVal = PKE_REGVAL_SEG_SIZE_SEGSIZE_64;
            break;

        default:
			uWriteVal = 0;
            debug("error setting segment size/mode\n");
    }

    /* ! write SEG_SIZE and MODE_ID */
    return (uWriteVal | PKE_REGMASK_SEG_SIZE_MUSTBEONE);
}

uint32_t
PkeGetSegmentCount(uint32_t uSegSize)
{
    /* return segment total number */
    switch (uSegSize) {
        case PKE_SEG_SIZE_256:
            return PKE_SEG_NUM_CASE256;
        case PKE_SEG_SIZE_128:
            return PKE_SEG_NUM_CASE128;
        case PKE_SEG_SIZE_64:
            return PKE_SEG_NUM_CASE64;
        default:
            return 0;
    }
}

static uint32_t get_precision(uint32_t uBitLen)
{
	uint32_t uPrecision;
	
	if (uBitLen <= PKE_LEN_BIT_512) {
        uPrecision = PKE_SIZE_PREC_0;
    } else if ((uBitLen <= PKE_LEN_BIT_1024) && (uBitLen != PKE_LEN_BIT_672) && (uBitLen != PKE_LEN_BIT_864)) {
        uPrecision = PKE_SIZE_PREC_1;
    } else if ((uBitLen <= PKE_LEN_BIT_1536) && (uBitLen != PKE_LEN_BIT_1280) && (uBitLen != PKE_LEN_BIT_1408)) {
        uPrecision = PKE_SIZE_PREC_2;
    } else {
        uPrecision = PKE_SIZE_PREC_3;
    }
	return uPrecision;
}

static uint32_t get_chunkSize(uint32_t uBitLen, uint32_t uPrecision)
{
	uint32_t uDiv, uChunk;
	
    switch (uPrecision) {
        case PKE_SIZE_PREC_0:
            if (uBitLen < PKE_LEN_BIT_128) {
                uBitLen = PKE_LEN_BIT_128;
            }
            uDiv = uBitLen >> 5;
            uChunk = uDiv;
            if (uBitLen - (uChunk << 5)) {
                uDiv++;
            }
            uChunk = --uDiv;
            break;

        case PKE_SIZE_PREC_1:
            uDiv = uBitLen >> 6;
            uChunk = uDiv;
            if (uBitLen - (uChunk << 6)) {
                uDiv++;
            }
            uChunk = --uDiv;
            break;

        case PKE_SIZE_PREC_2:
            uDiv = (uBitLen / 3) >> 5;
            uChunk = uDiv;
            if (uBitLen - ((uChunk * 3) << 5)) {
                uDiv++;
            }
            uChunk = --uDiv;
            break;

        case PKE_SIZE_PREC_3:
            uDiv = uBitLen >> 7;
            uChunk = uDiv;
            if (uBitLen - (uChunk << 7)) {
                uDiv++;
            }
            uChunk = --uDiv;
            break;
		default:
			debug("Unsupported precision value: %u", uPrecision);
			uChunk = 0;
    }
	 return (((uChunk << 3) + 8) << 2);
}

static uint32_t 
config_pke_key_len(uint32_t uBitLen)
{
    uint32_t uDiv, uChunk, uPrecision;

	/* 1. Get Precision */
	uPrecision = get_precision(uBitLen);

    /* 2. Get chunck size */
	uChunk = get_chunkSize(uBitLen, uPrecision);

	/* 3. Set precision and chunksize */
    uDiv = ((uChunk >> 2) - 8) | uPrecision;

#if IN_KERNEL
	//Set the IODelay time based on uBitLen.
	if (uBitLen <= PKE_LEN_BIT_512) {
		delay_time = iodelay_time[0];
	}
	else if (uBitLen <= PKE_LEN_BIT_1024) {
		delay_time = iodelay_time[1];		
	}
	else { 
		delay_time = iodelay_time[2];	
	}
#endif

    return uDiv;
}

static inline uint32_t
RSAPkeSetInOut(uint32_t uDest, uint32_t uSrc1, uint32_t uSrc2)
{
    uint32_t uPosition = 0;

    /*! Set segment ID of src and dest */
    uPosition = ((uSrc1 <<  PKE_REGSHIFT_SEG_ID_A_SEG_ID)
            | (uSrc2 << PKE_REGSHIFT_SEG_ID_B_SEG_ID)
            | (uDest << PKE_REGSHIFT_SEG_ID_S_SEG_ID));

    return uPosition;
}

static inline bool
RSAPkeRun(volatile struct pke_regs *registers, uint32_t uCmdVal)
{
    /*! Run PKE */
    registers->pke_start = uCmdVal;
	
#if IN_KERNEL
	IODelay(delay_time);
#endif	

    do {
        if (!(registers->pke_start & PKE_REGMASK_START_EXEC_ON))
            return true;
    } while (1);

    return false;
}


static bool shift_left(uint32_t *data, uint32_t data_size)
{
	int prev_carry = 0, cur_carry = 0;
	uint32_t itr = 0;
	
	for (itr=0; itr < (data_size/sizeof(uint32_t)); itr++) {
		cur_carry = data[itr] >> 31;
		data[itr] = (data[itr] << 1) + prev_carry;
		prev_carry = cur_carry;
	}
	return prev_carry;
}


static int32_t highestSetBit(uint32_t *data1, uint32_t data_size)
{
	int32_t ix = data_size/sizeof(uint32_t) - 1; // Most significant word. 

	while( ix>=0 && data1[ix]==0) {
		ix--;
	}

	if(ix < 0)
		return -1; //mods is zero. 

    uint32_t msNZword = data1[ix];
    uint32_t b = ((ix+1) * sizeof(uint32_t) * DATA_BYTE_TO_BIT) - 1 ;
	// Now find the highest set bit. 
	uint32_t mask = 0x80000000;
    while ((msNZword & mask) == 0) {
        mask >>= 1;
        b--;
    }
	return b;
}


static bool is_greater(uint32_t *data1, uint32_t *data2, uint32_t data_size)
{
	uint32_t itr = data_size/sizeof(uint32_t) - 1; //start at the last word.

	do {
		if(data1[itr] > data2[itr])
			return true;
		else if(data1[itr] < data2[itr])
			return false;
		//check next word;	
	}while(itr--);

	return false;
}

/* Do data1 = data1 - data2 */
static uint32_t sub(uint32_t *data1, uint32_t *data2, uint32_t data_size)
{
	uint32_t borrow = 0;
	uint32_t itr = 0;
	
	for(itr=0; itr < (data_size/sizeof(uint32_t)); itr++) {
		if(borrow) {
			if(data1[itr] != 0)
				borrow = 0; //No need to borrow again.
			data1[itr] -= 1;
		}
			
		if( data1[itr] < data2[itr] )
			borrow = 1;

		data1[itr] -= data2[itr];
	}
	if(borrow)
		return 1;

	return 0;
}

/*Do data1 = data1 + data2 */
static uint32_t add(uint32_t *data1, uint32_t *data2, uint32_t data_size)
{
	uint32_t temp;

	int carry = 0;
	uint32_t itr = 0;

	for(itr=0; itr < (data_size/sizeof(uint32_t)); itr++) {
		temp = data1[itr] + data2[itr] + carry;
		if(temp < data1[itr] || (temp == data1[itr] && carry))
			carry = 1;
		else 
			carry = 0;
		data1[itr] = temp;
	}
	return carry;
}

// The src buffer is assumed to be word aligned at this point. 
// The hardware register is 4 byte aligned.
// The data is in little Endian.
static inline void copy_into_segment(volatile uint8_t *memory, uint32_t segment_id, uint32_t segment_size, uint8_t *data, uint32_t length)
{
	volatile uint8_t *dst = memory + segment_id * segment_size;
	uint32_t ii=0;
	
	//Make sure the buffer is word aligned. 
	//assert((data & 3) == 0); //We don't assert in iBoot. The only interface makes sure we have aligned buffer.
	
	uint32_t newLength = length & ~3 ;
	//Copy words.
	for(ii=0; ii<(newLength/4); ii++) 
		*((uint32_t*)dst + ii) = *((uint32_t*)data + ii);
	
	//See if you have less than a word left to copy.
	if(length & 3) { //have less than 4 bytes left to copy.
		uint8_t wordData[4] = {0,0,0,0};
		uint8_t *bytesLeft = data + newLength;
		for(ii=0; ii<(length & 3); ii++) {
			wordData[ii] = *(bytesLeft + ii); 
		}
		*((uint32_t*)(dst+newLength)) = *((uint32_t*)wordData);
	}
	
	//zero fill the rest of the segment.
	uint32_t bytesCopied = ((length+3) & ~3);
	uint32_t left = segment_size - bytesCopied;
	if(left) {
		uint32_t *dest = (uint32_t *)(dst + bytesCopied);
		for(ii=0; ii<(left/4); ii++)
			*(dest+ii) = 0x00000000;
	}
}

/* Calculate R0 in software. 
	rzero has memory allocated of size modulus_length and is zero filled.
*/
bool rsa_cal_R0(uint32_t * const mods, uint32_t modulus_length, uint32_t *rzero)
{
	debug("called");
	
	uint32_t precision = get_precision(modulus_length*DATA_BYTE_TO_BIT);
	uint32_t chunk_size = get_chunkSize(modulus_length*DATA_BYTE_TO_BIT, precision);
	
	 //precision is 1,2,3,4 instead of 0,1,2,3 
	precision += 1;
	
	/* Compute 2^b  */
	uint32_t power = (chunk_size / 16) + 1;
    if (precision == 3)
        power = power * 3;
    power = power + (precision * (chunk_size + 16));
	
	//compute the most significant bit set in the modulus.
	int32_t b = highestSetBit((uint32_t*)mods, modulus_length);
	if(b == -1)
		return false;

	//store 2^b in rzero.
	bzero(rzero, modulus_length);
	rzero[(b/32)] = 1 << (b % 32); 
	
	/* Calculate (2^(power-b) * 2^b) mod M  */
	uint32_t itr;
	for(itr=0; itr<(power-b); itr++) { 
		bool carry = shift_left(rzero, modulus_length); //Shift left.
		if (carry || is_greater(rzero, mods, modulus_length))
			sub(rzero, (uint32_t*)mods, modulus_length);
	}
	return true;
}

/* Compute the value of R^2 using the hardware. 
	The value of rzero (R0) is calculated in softare and passed into the function
	The hardware should be reset before calling this function.
	The value of R^2 is returned thru' rsquare (memory for this is allocated before calling this function and of modulus_length size)
*/   
static bool rsa_cal_R2modM(volatile struct pke_regs *registers, volatile uint8_t *memory, uint32_t seg_size, uint32_t * const mods, uint32_t mods_length, uint32_t *rsquare, uint32_t *r2_segid)
{
	debug("called");
	
	if(!rsquare) //Need rzero (which should be in rsquare) to proceed
		return false;
	
	//Set the key length;
	registers->pke_key_len = config_pke_key_len(mods_length*DATA_BYTE_TO_BIT);
	//Set segment size;
	registers->pke_seg_size = PkeSetMode(seg_size);
	registers->pke_seg_sign = 0;
	registers->pke_seg_id = 0;

	/* Use hardware to calculate R1 thru' Rn and finally R^2  */
	//Load modulus always into seg0.
	copy_into_segment(memory, mod_segid, seg_size, (uint8_t*)mods, mods_length);
	
	//based on seg_size compute the seg_ids to store the (2^(power-b) * 2^b) mod M
	// and the final value of R^2 
	uint32_t rzero_segid, rsq_segid;
    switch (seg_size) {
        case PKE_SEG_SIZE_256:
            rzero_segid = PKE_SEG_ID_05;
			rsq_segid = PKE_SEG_ID_06;
			break;
        case PKE_SEG_SIZE_128:
            rzero_segid =  PKE_SEG_ID_12;
			rsq_segid = PKE_SEG_ID_14;
			break;
        case PKE_SEG_SIZE_64:
            rzero_segid =  PKE_SEG_ID_28;
			rsq_segid = PKE_SEG_ID_29;
			break;
        default:
			//Should not happen.
            return false;
    }
	//Load the rzero into the hardware.	
	copy_into_segment(memory, rzero_segid, seg_size, (uint8_t*)rsquare, mods_length);

	uint32_t precision = (registers->pke_key_len&PKE_REGMASK_KEY_LEN_PREC_ID) + 1;
	//Compute the number of hardware iterations based on precision.
    uint32_t square_count;
    if (precision == 1 || precision == 3)
        square_count = 4;
    else if (precision == 2)
        square_count = 5;
    else /* precision == 4 */
        square_count = 6;
	
	//Set pre load modulus on when using the hardware for the first time.
	uint32_t start_mask = PKE_REGMASK_START_PLDM_ON | PKE_REGMASK_START_EXEC_ON;
	uint32_t itr = 1;
	uint32_t temp_segid = rsq_segid;
	while (itr <= square_count) {
		//setup seg_id pointers on the hardware
		registers->pke_seg_id = RSAPkeSetInOut(temp_segid, rzero_segid, rzero_segid);
		//kick the hardware. 
		if(!RSAPkeRun(registers, start_mask))
			return false;
		itr++;
		if(itr > square_count)
			break;
		//swap the segids and do the square again.
		temp_segid += rzero_segid;
		rzero_segid = temp_segid - rzero_segid;
		temp_segid -= rzero_segid;
		start_mask = PKE_REGMASK_START_EXEC_ON; //dont have to laod M again.
	}		

	// Check the sign value of temp_segid and add modulus if negative.
	uint32_t uRead, uMask;
	uMask = (1 << temp_segid);
	uRead = registers->pke_seg_sign & uMask;
	//If  negative then add modulus.
	if(uRead) {
		add( (uint32_t*)(memory+(temp_segid*seg_size)), (uint32_t*)mods, mods_length);
	}
	
	*r2_segid = temp_segid;
	
	//Clean up the temp space used. 
	bzero((uint8_t*) memory+rzero_segid*seg_size, mods_length);
	
	/* Reset the signbits */
	registers->pke_seg_sign = 0;
	memcpy(rsquare, (uint8_t *) memory+rsq_segid*seg_size, mods_length);
	return true;
}


/* Calculate exponential. 
	The options parameter passed in indicates if R^2 is passed in or needs to be computed.
	Options = 00 => Compute R^2 and don't care about caching. *rsquare is ignored.
	Options = 01 => Compute R^2 and return it for caching. *rsquare has mem allocated and is of size modulus_length
	Options = 10 => Use passed in R^2 value. 
	
	Reset the hardware before calling this function. 
*/
bool rsa_cal_exp(void *dst, uint32_t *len, uint32_t options,
					uint8_t *rsquare, uint32_t *rsquare_length,
					uint8_t * const base, uint32_t base_length, 
					uint8_t * const expn, uint32_t expn_length, 
					uint8_t * const modulus, uint32_t modulus_length, 
					volatile struct pke_regs *registers, volatile uint8_t *memory)
{
	debug("called");
	
	/* Perform validation checks */
	if(modulus_length % 4) 
		return false;

	uint32_t segment_size, num_segments;

	if (modulus_length <= (PKE_SEG_SIZE_64)) {
        segment_size = PKE_SEG_SIZE_64;
    } else if (modulus_length <= (PKE_SEG_SIZE_128)) {
        segment_size = PKE_SEG_SIZE_128;
    } else if (modulus_length <= (PKE_SEG_SIZE_256)) {
        segment_size = PKE_SEG_SIZE_256;
    } else
        return false; /* key too long */

    num_segments = PkeGetSegmentCount(segment_size);
	
	if ( !num_segments			   ||
		base_length > segment_size ||
        expn_length > segment_size ||
        modulus_length > segment_size)
            return false;

	/* Sane input, so proceed */
	
	//rsquare segid must always be the last seg ID.
	uint32_t rsq_segid = num_segments - 1;

	if( (options&kIOPKEAcceleratorComputeRsquareMask) == 0 ) { //Compute rsquare.
		if(!(options&kIOPKEAcceleratorPreProcessingDone)) { //R0 was not cacluated. should only happen in iBoot.
			if(!rsquare){
				//should not happen. The expectation is that memory for rsquare should be allocated.
				return false;
			}
			rsa_cal_R0((uint32_t*)modulus, modulus_length, (uint32_t*)rsquare);
		}
		//Compute R^2
		if(!rsa_cal_R2modM(registers, memory, segment_size, (uint32_t*)modulus, modulus_length, (uint32_t*)rsquare, &rsq_segid))
			return false;
		*rsquare_length = modulus_length;
	}
	else { //This would have been done while calculating R^2.  
		//Set the key length;
		registers->pke_key_len = config_pke_key_len(modulus_length*DATA_BYTE_TO_BIT);
		//Set segment size;
		registers->pke_seg_size = PkeSetMode(segment_size);
		/* Reset the signbits */
		registers->pke_seg_sign = 0;
		registers->pke_seg_id = 0;

		//Load modulus into seg0;
		copy_into_segment(memory, mod_segid, segment_size, modulus, modulus_length);
		//copy the rsquare into the hardware segment.
		memcpy((uint8_t*)memory+(rsq_segid*segment_size), rsquare, modulus_length);
	}

	/* Compute (base * r^2 mod M) using hardware */
	//Copy base into tmp_segid.
	copy_into_segment(memory, tmp_segid, segment_size, base, base_length);
	//Setup the segid pointers.
	registers->pke_seg_id = RSAPkeSetInOut(iter_segid, tmp_segid, rsq_segid);

	//Kick the hardware with pre load modulus bit set.
	if(!RSAPkeRun(registers, (PKE_REGMASK_START_PLDM_ON | PKE_REGMASK_START_EXEC_ON)))
			return false;

	/* compute (1*R^2modM) using hardware. */
	registers->pke_seg_size |= PKE_REGVAL_SEG_SIZE_FUNC_ID_A1; // A*1
	//When PKE_REGVAL_SEG_SIZE_FUNC_ID_A1 is set, the hardware ignores the segid_B pointer
	registers->pke_seg_id = RSAPkeSetInOut(acum_segid, rsq_segid, rsq_segid);
	if(!RSAPkeRun(registers, PKE_REGMASK_START_EXEC_ON))
			return false;
	
	// Clear the PKE_REGVAL_SEG_SIZE_FUNC_ID_A1 bit.
	registers->pke_seg_size &= ~PKE_REGVAL_SEG_SIZE_FUNC_ID_A1;
	
	/* Run the PKE */
	uint32_t *exp_word = (uint32_t*)expn;
	/* word aligned byte length */
    uint32_t uExpLen = ((expn_length + (DATA_WORD_TO_BYTE - 1)) & (~0x3));
	
	uint32_t last_word = exp_word[(uExpLen/DATA_WORD_TO_BYTE) - 1];
	uint32_t uIndex = (DATA_WORD_TO_BYTE*DATA_BYTE_TO_BIT) - (highestSetBit(&last_word, DATA_WORD_TO_BYTE) + 1);
	for (; uIndex < (uExpLen * DATA_BYTE_TO_BIT); uIndex++) {
		unsigned int    word;
        unsigned int    bit;

		uint32_t offset = (uExpLen / DATA_WORD_TO_BYTE) - (uIndex / (DATA_WORD_TO_BYTE * DATA_BYTE_TO_BIT)) - 1;
        word = exp_word[offset];

		registers->pke_seg_id = RSAPkeSetInOut(tmp_segid, acum_segid, acum_segid);
		if(!RSAPkeRun(registers, PKE_REGMASK_START_EXEC_ON))
			return false;

        bit = word & (0x80000000 >> (uIndex % (DATA_WORD_TO_BYTE * DATA_BYTE_TO_BIT)));

        if (bit) {
            //debug("bit %d set", uIndex);
			registers->pke_seg_id = RSAPkeSetInOut(acum_segid, tmp_segid, iter_segid);
			if(!RSAPkeRun(registers, PKE_REGMASK_START_EXEC_ON))
				return false;
        } else {
            memcpy( (uint8_t *) memory + acum_segid * segment_size,
                    (uint8_t *) memory + tmp_segid * segment_size,
                    segment_size);
            
            if (registers->pke_seg_sign & (0x01 << tmp_segid))
                registers->pke_seg_sign |= (0x01 << acum_segid);
            else
                registers->pke_seg_sign &= ~(0x01 << acum_segid);
        }
    }

	/* Factor out R^-1 from the result */
    registers->pke_seg_size |= PKE_REGVAL_SEG_SIZE_FUNC_ID_A1; // A*1
    registers->pke_seg_id = RSAPkeSetInOut(tmp_segid, acum_segid, acum_segid);
    if (!RSAPkeRun(registers, PKE_REGMASK_START_EXEC_ON))
        return false;
    registers->pke_seg_size &= ~PKE_REGVAL_SEG_SIZE_FUNC_ID_A1; // A*B
	
	// Check the sign value of tmp_segid and add modulus if negative.
	uint32_t uRead, uMask;
	uMask = (1 << tmp_segid);
	uRead = registers->pke_seg_sign & uMask;
	//If  negative then add modulus.
	if(uRead) {
		registers->pke_seg_sign &= ~uMask; //clear the sign.
		add( (uint32_t*)(memory+(tmp_segid*segment_size)), (uint32_t*)modulus, modulus_length);
	}
	
	/* Copy data out */
	memcpy(dst, (uint8_t*)memory+(tmp_segid*segment_size), modulus_length);
	*len = modulus_length;
	return true;
}

#if IN_KERNEL

bool internalTest(volatile struct pke_regs *registers, volatile uint8_t *memory, uint32_t mod_size, uint64_t *time_micro)
{
	uint8_t buff[mod_size];
	uint32_t segment_size;

	if (mod_size <= (PKE_SEG_SIZE_64)) {
        segment_size = PKE_SEG_SIZE_64;
    } else if (mod_size <= (PKE_SEG_SIZE_128)) {
        segment_size = PKE_SEG_SIZE_128;
    } else if (mod_size <= (PKE_SEG_SIZE_256)) {
        segment_size = PKE_SEG_SIZE_256;
    } else
        return false; /* key too long */
	
	uint32_t rsq_segid;
    switch (segment_size) {
        case PKE_SEG_SIZE_256:
			rsq_segid = PKE_SEG_ID_06;
			break;
        case PKE_SEG_SIZE_128:
			rsq_segid = PKE_SEG_ID_14;
			break;
        case PKE_SEG_SIZE_64:
			rsq_segid = PKE_SEG_ID_29;
			break;
        default:
			//Should not happen.
            return false;
    }
		
	registers->pke_key_len = config_pke_key_len(mod_size*DATA_BYTE_TO_BIT);
	//Set segment size;
	registers->pke_seg_size = PkeSetMode(segment_size);
	/* Reset the signbits */
	registers->pke_seg_sign = 0;
	registers->pke_seg_id = 0;

	debug("key: %x, reg_key: %x, modsize:%u, reg_segsize: %u", config_pke_key_len(mod_size*DATA_BYTE_TO_BIT), registers->pke_key_len, mod_size, registers->pke_seg_size);

	//Load modulus into seg0;
	memset(buff, 0xF0, mod_size);
	copy_into_segment(memory, mod_segid, segment_size, buff, mod_size);
	//Set R^2
	memset(buff, 0xE5, mod_size);
	copy_into_segment(memory, rsq_segid, segment_size, buff, mod_size);
	//Set A
	memset(buff, 0xD2, mod_size);
	copy_into_segment(memory, iter_segid, segment_size, buff, mod_size);

	
	//Setup the segid pointers.
	registers->pke_seg_id = RSAPkeSetInOut(iter_segid, tmp_segid, tmp_segid);
	
	uint64_t tasm0, tasm1, tasm2, asm_ns;
	tasm0 = mach_absolute_time();
	tasm1 = mach_absolute_time(); 
	
	//Kick the hardware with pre load modulus bit set.
	if(!RSAPkeRun(registers, (PKE_REGMASK_START_PLDM_ON | PKE_REGMASK_START_EXEC_ON)))
			return false;
	
	tasm2 = mach_absolute_time();
	absolutetime_to_nanoseconds( (tasm2 + tasm0 - tasm1 - tasm1), &asm_ns);
	//debug("Single multiply time: %lluns\n", asm_ns);
	
	*time_micro = (asm_ns/1000) - 5;
	
	//clear the hardware.
	registers->pke_key_len = 0;
	registers->pke_seg_size = 0;
	registers->pke_seg_sign = 0;
	registers->pke_seg_id = 0;
	
	bzero((uint8_t*)memory+(tmp_segid*segment_size), mod_size);
	bzero((uint8_t*)memory+(mod_segid*segment_size), mod_size);
	bzero((uint8_t*)memory+(iter_segid*segment_size), mod_size);
		
	return true;
}

#endif
