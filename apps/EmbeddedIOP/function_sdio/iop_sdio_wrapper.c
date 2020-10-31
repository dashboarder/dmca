/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "iop_sdio_wrapper.h"

#include <sys/task.h>
#include <AssertMacros.h>

#include <sdiodrv_config.h>
#include <sdiodrv_command.h>
#include <sdiodrv_transfer.h>
#include <platform.h>
#include <platform/memmap.h>

#include "iop_sdio_protocol.h"
#include <sdiocommon/sdio_cmdfields.h>

#include "debug.h"
#define cache_op_size(buf_size) (((buf_size) + (CPU_CACHELINE_SIZE-1)) & ~(CPU_CACHELINE_SIZE-1))

#define MAGIC_WRITE 0x44555544
#define MAGIC_READ 0x55555555

#define MAX_SDIO_READ_LEN (32*1024) //agreement with AppleBCMWLAN driver that there wont be any read bigger than that

#define BCOM_SDIO_CHANNEL_OFFSET 5
#define BCOM_SDIO_NEXTLEN_OFFSET 6
#define BCOM_SDIO_DATAOFFSET_OFFSET 7

#define BCOM_SDIO_GLOM_CHANNEL 3
#define BCOM_SDIO_GLOM_FRAME_DESCRIPTOR 0x80

#define MAGIC_READ_PACKET_LEN_OFFSET 1
#define MAGIC_READ_ENABLE_OFFSET 2
#define MAGIC_READ_BUFFER_LENGTH_OFFSET 3


#define LOOKAHEAD_READ_PKT_SIZE (lookahead_read_pkt_size)

struct cacheCmd {
    struct IOPSDIOTransferCmd transferCmd;
    struct IOPSDIO_dma_segment segment;
};

struct cacheCmd cmd;
SDIOReturn_t cachedError = kSDIOReturnSuccess;
unsigned int do_lookahead = 0;
unsigned int lookahead_read_pkt_size = 12 * 1024; //default buffer, max 12K per packet


IOPSDIO_status_t
iopsdio_init(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOInitCmd *initCmd)
{
	check(targetSDHC);
	check(initCmd);
	
	SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
	if(!sdhc) {
		return kSDIOReturnNoHostController;
	}

	// Create DMA completion event.
	struct task_event* dmaCompleteEvent = malloc(sizeof(struct task_event));
	if(!dmaCompleteEvent) {
		return kSDIOReturnNoMemory;
	}
	
	event_init(dmaCompleteEvent, EVENT_FLAG_AUTO_UNSIGNAL, false);

	targetSDHC->dmaCompleteEventAddr = (UInt32)dmaCompleteEvent;

#if 0
        /* 7256044 */
	initCmd->sdhcCapabilities = sdhc->capabilities;
	initCmd->sdhcMaxCurrentCapabilities = sdhc->maxCurrentCapabilities;
#else
	initCmd->sdhcCapabilities = 0;
	initCmd->sdhcMaxCurrentCapabilities = 0;
#endif

	return kSDIOReturnSuccess;
}

IOPSDIO_status_t
iopsdio_free(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOFreeCmd *freeCmd)
{
	check(targetSDHC);
	check(freeCmd);
	
	SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
	if(!sdhc) {
		return kSDIOReturnNoHostController;
	}

	// Destroy the DMA completion event
	free((void*)targetSDHC->dmaCompleteEventAddr);
	targetSDHC->dmaCompleteEventAddr = 0;

	return kSDIOReturnSuccess;
}

IOPSDIO_status_t
iopsdio_reset(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOResetCmd *resetCmd)
{
	check(targetSDHC);
	check(resetCmd);
	
	SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
	if(!sdhc) {
		return kSDIOReturnNoHostController;
	}

	SDIOReturn_t retval = sdiodrv_resetSDHC(sdhc, resetCmd->resetFlags);
	if(kSDIOReturnSuccess != retval) {
		return retval;
	}

	// restore interrupt state on reset
	if(resetCmd->resetFlags & kSDHCResetAll) {
		sdhc_enableCommandStatus(sdhc, true);
		sdhc_enableTransferStatus(sdhc, true);
	}

    cachedError = retval;
    
	return retval;
}

IOPSDIO_status_t
iopsdio_setBusConfig(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOSetBusParamCmd *busParamCmd)
{
	check(targetSDHC);
	check(busParamCmd);
	
    if (cachedError != kSDIOReturnSuccess)
        return cachedError;
    
	SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
	if(!sdhc) {
		return kSDIOReturnNoHostController;
	}

	SDIOReturn_t retval = kSDIOReturnSuccess;
	
	if(busParamCmd->clockRateHz) {

		retval = sdiodrv_setClockRate(sdhc, &busParamCmd->clockRateHz, busParamCmd->baseClockRateHz);
		if(kSDIOReturnSuccess != retval) {
			return retval;
		}
	}
	
	if(busParamCmd->busWidth) {

		retval = sdiodrv_setBusWidth(sdhc, busParamCmd->busWidth);
		if(kSDIOReturnSuccess != retval) {
			return retval;
		}
	}
	
	if(busParamCmd->busSpeedMode) {

		retval = sdiodrv_setBusSpeedMode(sdhc, busParamCmd->busSpeedMode);
		if(kSDIOReturnSuccess != retval) {
			return retval;
		}
	}
	
	if(busParamCmd->clockMode) {

		retval = sdiodrv_setClockMode(sdhc, busParamCmd->clockMode);
		if(kSDIOReturnSuccess != retval) {
			return retval;
		}
	}
	
	return retval;
}

unsigned int nm = 0;
IOPSDIO_status_t
iopsdio_sendSDIOCmd(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOCommandCmd *commandCmd)
{
	check(targetSDHC);
	check(commandCmd);
	
    if (cachedError  != kSDIOReturnSuccess)
        return cachedError;
    
	SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
	if(!sdhc) {
		return kSDIOReturnNoHostController;
	}

	SDIOReturn_t retval = sdiodrv_sendSDIOCommand(sdhc, &commandCmd->command, &commandCmd->response);

	return retval;
}


/*  
 *  read a complete SDIO packet:
 *  reads are always block aligned
 *
 *  blocks: number of blocks to read, if zero, then issue a one block read in order to get the frame header and 
 *          obtain the packet length from this header, issue a second read in order to to get the remaining blocks
 *  max_blocks: max_blocks allowed
 *  data: virtual address if read buffer
 *
 */
    

IOPSDIO_status_t iopsdio_read_packet(unsigned int blocks, unsigned int max_blocks, unsigned int * data, struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOTransferCmd *transferCmd)
{
    IOPSDIO_status_t retval = 0;
    SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
    struct task_event *dmaCompleteEvent = (struct task_event *)targetSDHC->dmaCompleteEventAddr;

    if (blocks == 0) {
        
        transferCmd->transfer.blockCount = 1;
        transferCmd->memory.dataLength = transferCmd->transfer.blockSize;
        transferCmd->segment[0].length = transferCmd->transfer.blockSize;
        transferCmd->transfer.command.argument = (transferCmd->transfer.command.argument& 0xfffffe00) | 1;
        
        data = mem_static_map_cached((uint32_t)data);

        platform_cache_operation(CACHE_INVALIDATE, 
                                 (void *)data, 
                                 (32)); 
        
        if (transferCmd->memory.dataLength > MAX_SDIO_READ_LEN) {
            //paranoia
            dprintf(DEBUG_CRITICAL, "iopsdio_read_packet1: len frame is too large %llu \n", transferCmd->memory.dataLength);
            retval = kSDIOReturnBadArgument;
            return retval;
        }
        
        retval = sdiodrv_transferData(sdhc, &transferCmd->transfer, &transferCmd->memory, &transferCmd->response, dmaCompleteEvent);
        
        if (retval)
            return retval;
        

        
        data = mem_static_map_cached((uint32_t)data);
        
        platform_cache_operation(CACHE_INVALIDATE, 
                                 (void *)data, 
                                 (32));
        
        unsigned short check = data[0]>>16;
        unsigned short fflen = data[0];
        
        if (fflen) {
            
            
            
            if ((fflen + check) != 0xffff) {
                
                dprintf(DEBUG_CRITICAL, "iopsdio_read_packet error %x %d - %x blocks %u %x\n", fflen, fflen, check, blocks, data[1]);
                dprintf(DEBUG_CRITICAL, " =====  %p %x %x %x\n", data, transferCmd->segment[0].paddr, data[0],*(unsigned int*)transferCmd->segment[0].paddr);

                return retval;
            }
            
            unsigned short mask = transferCmd->transfer.blockSize-1;
            unsigned short flen = (fflen+mask) & ~mask; //round up
            
            unsigned int remain_blocks = flen ;
            if (128 == transferCmd->transfer.blockSize) {
                remain_blocks = remain_blocks/128;
            } else {
                remain_blocks = remain_blocks/transferCmd->transfer.blockSize;
            }
            
            if (remain_blocks == 0) {
                dprintf(DEBUG_CRITICAL, "iopsdio_read_packet: error zero blocks \n");
                
                return kSDIOReturnBadArgument;
                
            }
            remain_blocks--;
            
            if (remain_blocks >= max_blocks) {
                dprintf(DEBUG_CRITICAL, "iopsdio_read_packet: not enough blocks %u orig %u flen %x check %x\n", remain_blocks, max_blocks, fflen, check);
                
                return kSDIOReturnBadArgument;
            }
            if (remain_blocks) {
                
                //read remaining blocks
                transferCmd->transfer.blockCount = remain_blocks;
                transferCmd->memory.dataLength = transferCmd->transfer.blockSize * remain_blocks;
                transferCmd->segment[0].length = transferCmd->transfer.blockSize * remain_blocks;
                transferCmd->segment[0].paddr += transferCmd->transfer.blockSize;
                
                transferCmd->transfer.command.argument = (transferCmd->transfer.command.argument& 0xfffffe00) | remain_blocks;
                
                if (transferCmd->memory.dataLength > MAX_SDIO_READ_LEN) {
                    //paranoia, 
                    dprintf(DEBUG_CRITICAL, "iopsdio_read_packet2: len frame is too large %llu remain %u size %hu\n", transferCmd->memory.dataLength, remain_blocks, transferCmd->transfer.blockSize);
                    retval = kSDIOReturnBadArgument;
                    return retval;
                }
                
                retval = sdiodrv_transferData(sdhc, &transferCmd->transfer, &transferCmd->memory, &transferCmd->response, dmaCompleteEvent);
                transferCmd->segment[0].paddr -= transferCmd->transfer.blockSize;
            } 
            
        } 
        
        
    } else {
        
        transferCmd->transfer.blockCount = blocks;
        transferCmd->memory.dataLength = transferCmd->transfer.blockSize * blocks;
        transferCmd->segment[0].length = transferCmd->transfer.blockSize * blocks;
        
        transferCmd->transfer.command.argument = (transferCmd->transfer.command.argument& 0xfffffe00) | blocks;
        platform_cache_operation(CACHE_INVALIDATE, 
                                 (void *)data, 
                                 (32));  
        retval = sdiodrv_transferData(sdhc, &transferCmd->transfer, &transferCmd->memory, &transferCmd->response, dmaCompleteEvent);

        
    }
    

    return retval;
    
}

void iopsdio_cacheTransferSDIOData(struct IOPSDIOTargetSDHC *targetSDHC)
{
    
    SDIOReturn_t retval = 0;
    cachedError = kSDIOReturnSuccess;
    
    //perform access based on the cached command
    struct IOPSDIOTransferCmd *transferCmd = &cmd.transferCmd;

    unsigned int * data = (unsigned int*)transferCmd->segment[0].paddr;
    data = mem_static_map_cached((uint32_t)data);    

    unsigned int max_blocks;
    if (128 == transferCmd->transfer.blockSize)
        max_blocks = LOOKAHEAD_READ_PKT_SIZE/128;
    else 
        max_blocks = LOOKAHEAD_READ_PKT_SIZE/transferCmd->transfer.blockSize;
    
    //kick SDIO transfer
    retval = iopsdio_read_packet(transferCmd->transfer.blockCount, max_blocks, data, targetSDHC, transferCmd);
    
    if (retval) {
        if ((retval < kSDIOReturnDataError) || (retval > kSDIOReturnDataTimeout)) {
            //paranoia, make sure we return a Data error here, which means that the host will trigger a Reset
            cachedError = kSDIOReturnDataError;
        } else {
            
            cachedError = retval;
        }
    } else {
        
        cachedError = kSDIOReturnSuccess;
    }
    
    unsigned int *retp = (unsigned int*)(((unsigned char*)data)+LOOKAHEAD_READ_PKT_SIZE); 
    
    //dprintf(0, "iopsdio_cacheTransferSDIOData done %x %x\n", *retp, retval);

    //make sure we tell the host processor that we're done
    * retp = retval;
    platform_cache_operation(CACHE_CLEAN, 
                             (void *)retp, 
                             (32));

}



IOPSDIO_status_t
iopsdio_transferSDIOData(struct IOPSDIOTargetSDHC *targetSDHC, struct IOPSDIOTransferCmd *transferCmd)
{
	check(targetSDHC);
	check(transferCmd);
    unsigned int orig_bcount = 0;
	unsigned int nlen = 0;
    
    if (cachedError != kSDIOReturnSuccess)
        return cachedError;

    
	SDHCRegisters_t *sdhc = (SDHCRegisters_t*)targetSDHC->basePhysicalAddr;
	if(!sdhc) {
		return kSDIOReturnNoHostController;
	}

	struct task_event *dmaCompleteEvent = (struct task_event *)targetSDHC->dmaCompleteEventAddr;
	
	transferCmd->memory.segmentList = transferCmd->segment;
    
    unsigned int orig_data = transferCmd->segment[0].paddr;
    

    unsigned int * data = mem_static_map_cached((uint32_t)orig_data);
    platform_cache_operation(CACHE_INVALIDATE, 
                             (void *)data, 
                             (32));
	SDIOReturn_t retval = 0;
    if (( transferCmd->transfer.direction == kSDIODirectionWrite) && (*data == MAGIC_WRITE) && ((transferCmd->transfer.command.argument & kSDIOCmd53BlockMode) == kSDIOCmd53BlockMode)) {
        unsigned int base_addr = transferCmd->segment[0].paddr + transferCmd->transfer.blockSize;
        unsigned int * l = data + 1;
        unsigned int cnt = 0;
        unsigned int bcount = *l ;
       // dprintf(0, "iopsdio_transferSDIOData magic wwwrite got %x %x [%x %x] %u %u %u\n",l[0],l[1], transferCmd->transfer.command.argument,transferCmd->transfer.command.index, transferCmd->transfer.blockCount,transferCmd->transfer.blockSize, transferCmd->memory.segmentCount);
      
        //first sdio block = MAGIC_WRITE/block size of pkt1/block size of pkt 2/block size of pkt3 etc...
        //second sdio blocks and so on... = packets data

        if (transferCmd->transfer.blockCount == 1) {
            
            dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData magic write error!! %p %x %x %x\n",data, l[0],l[1],l[2]);
            
            return kSDIOReturnBadArgument;
        }
        
        while (bcount!=0) {
            unsigned int transfer_len = bcount * transferCmd->transfer.blockSize;
            
            transferCmd->memory.dataLength = transfer_len;
            
            transferCmd->transfer.command.argument = (transferCmd->transfer.command.argument& 0xfffffe00) | bcount;
            
            transferCmd->transfer.blockCount = bcount;
            
            transferCmd->segment[0].length = transfer_len;
            transferCmd->segment[0].paddr = base_addr;
            unsigned int *p = (unsigned int*)transferCmd->segment[0].paddr;
            
            
            p = mem_static_map_cached((uint32_t)p);

            platform_cache_operation(CACHE_INVALIDATE, 
                                     (void *)p, 
                                     (32));
            
            unsigned short check = p[0]>>16;
            unsigned short fflen = p[0];
            if ((check + fflen) != 0xffff) {
                //internal error, the sender tries to send a corrupted packet
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData detected error ---> %p : %x %x %x %x \n", p, p[0], p[1], p[2], p[3]);
                retval = kSDIOReturnBadArgument;
            } else {
            
                retval = sdiodrv_transferData(sdhc, &transferCmd->transfer, &transferCmd->memory, &transferCmd->response, dmaCompleteEvent);
            }
            if (retval) {
                
                *data = cnt; //tell the sender how many packets we did send successfully
                
                platform_cache_operation(CACHE_CLEAN, 
                                         (void *)data, 
                                         (32));
                
                unsigned int *p = (unsigned int *)base_addr;
                
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData ---> %x %x %x %x \n", p[0], p[1], p[2], p[3]);
                dprintf(DEBUG_CRITICAL, "                         -->  cnt %u bc %u  addr %p arg %x retval %x\n", cnt,  transferCmd->transfer.blockCount, (void *)transferCmd->segment[0].paddr, transferCmd->transfer.command.argument, retval);
                return retval;
            }
            base_addr += transfer_len;//transferCmd->transfer.blockSize * bcount;
            cnt++;
            l++;
            bcount = *l ;
            
        }
        
    } else if ((transferCmd->transfer.direction == kSDIODirectionRead) && (*data == MAGIC_READ) && ((transferCmd->transfer.command.argument & kSDIOCmd53BlockMode) == kSDIOCmd53BlockMode)) {
        
        
        //MAGIC READ:
        //buffer1 of size LOOKAHEAD_READ_PKT_SIZE + buffer2 of size LOOKAHEAD_READ_PKT_SIZE + one 
        //additional block used as end of transfer marker
        
       orig_bcount = transferCmd->transfer.blockCount;
      //    dprintf(0, "iopsdio_transferSDIOData read got %u %u %u \n",l[0],l[1],l[2]);
        unsigned int blocks = *(data+MAGIC_READ_PACKET_LEN_OFFSET);
        unsigned int lk = *(data+MAGIC_READ_ENABLE_OFFSET);
        if (*(data+MAGIC_READ_BUFFER_LENGTH_OFFSET)) {
            lookahead_read_pkt_size = *(data+MAGIC_READ_BUFFER_LENGTH_OFFSET);
        }
        retval = iopsdio_read_packet(blocks, orig_bcount, data, targetSDHC, transferCmd);

        if (lk == 0) {
            //Host processor doesn't want the look ahead
            return retval;
        }
        if (retval) {
            goto return_clear;
        }
    
        
        unsigned char nextlen = *((unsigned char*)data + BCOM_SDIO_NEXTLEN_OFFSET);
        unsigned char channel = *((unsigned char*)data + BCOM_SDIO_CHANNEL_OFFSET);
        unsigned char offset = *((unsigned char*)data + BCOM_SDIO_DATAOFFSET_OFFSET);
       // dprintf(0, "iopsdio_transferSDIOData read got %x %x %x - %u channel %u offset %u\n",data[0],data[1],data[2], nextlen,channel, offset);

        
        unsigned short check = data[0]>>16;
        unsigned short fflen = data[0];
        
        if (fflen) {
            
            
            
            if ((fflen + check) != 0xffff) {
                
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData cksum error %x %d - %x \n", fflen, fflen, check);
                retval = kSDIOReturnDataError;
                goto return_clear;
            }
        } else {
            if (check==0) {
                //fflen and check are null, hence no more data pending
                goto return_clear;
            }
        }
        
        
        nlen = 16 * nextlen;
        

        if (nlen == 0 && channel == (BCOM_SDIO_GLOM_FRAME_DESCRIPTOR|BCOM_SDIO_GLOM_CHANNEL)) {
            //reading a Glom descriptor:  calculate the length of next frame
            //max number of GLom frames is tunable and typically 7, a glom frame descriptor should not be larger than 64 Bytes
#define SANE_MAX_GLOM_FRAME_LEN 64
            if (fflen > SANE_MAX_GLOM_FRAME_LEN) {
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData glom descriptor is too large %hu \n", fflen);
                goto return_clear;
            }
            
            if (fflen > 32) {
                //uncache the rest of the frame
                platform_cache_operation(CACHE_INVALIDATE, 
                                         (void *)data, 
                                         cache_op_size(fflen));
            }
            
            unsigned short * p = (unsigned short *)((unsigned char*)data + offset);
            unsigned short * end = (unsigned short *)((unsigned char*)data + fflen);

            //calculate nlen
            while (p < end){
                nlen += *p++;
            }
            

            if (nlen > LOOKAHEAD_READ_PKT_SIZE) {
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData calculated glom frame is too large %u \n", nlen);
                retval = kSDIOReturnDataError;
                goto return_clear;
            }
        } 

       
        
        //save parameters for the next transfer
        nlen += transferCmd->transfer.blockSize-1;
        nlen = nlen & ~(transferCmd->transfer.blockSize-1);
        
        if (nlen > MAX_SDIO_READ_LEN) {
            //paranoia
            dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData magic read calculating a bad len frame large %u \n", nlen);
            retval = kSDIOReturnBadArgument;
            goto return_clear;
        }
        
        cmd.transferCmd.transfer.command.argument = transferCmd->transfer.command.argument;
        cmd.transferCmd.transfer.command.index = transferCmd->transfer.command.index;
        
        
        if (128 == transferCmd->transfer.blockSize)
            cmd.transferCmd.transfer.blockCount = nlen/128;
        else 
            cmd.transferCmd.transfer.blockCount = nlen/transferCmd->transfer.blockSize;
        
        cmd.transferCmd.transfer.blockSize = transferCmd->transfer.blockSize;
        cmd.transferCmd.transfer.direction = transferCmd->transfer.direction;
     
        cmd.segment.length = nlen;
        cmd.segment.paddr = orig_data+LOOKAHEAD_READ_PKT_SIZE;
        
        cmd.transferCmd.memory.segmentList = &cmd.segment;
        cmd.transferCmd.memory.dataLength = nlen;
        cmd.transferCmd.memory.segmentCount = 1;

        if (lk > 1) {
            //experimental mode, read all at once while the host processor wait - used only for debug 14July2011
            iopsdio_cacheTransferSDIOData(targetSDHC);
        } else {
            //set lookahead to tell the sdio task handler in iop_sdio.c to generate the cached loakahead read
            do_lookahead = lk;
        }

    } else {
        
        if (transferCmd->transfer.direction == kSDIODirectionRead) {
            //if transfer is cmd53 block read, check that the requested length matches the number of blocks requested
            //and check that the total length is not overly large
            nlen = transferCmd->transfer.blockCount * transferCmd->transfer.blockSize;
            
            if (nlen != transferCmd->memory.dataLength) {
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData len mismatch %u %llu direction %hhu\n", nlen, transferCmd->memory.dataLength, transferCmd->transfer.direction);
                return kSDIOReturnBadArgument;
            }
            
            if (nlen > MAX_SDIO_READ_LEN) {
                dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData data transfer is too large %u %llu direction %hhu\n", nlen, transferCmd->memory.dataLength, transferCmd->transfer.direction);
                return kSDIOReturnBadArgument;
            }
        }
        
        retval = sdiodrv_transferData(sdhc, &transferCmd->transfer, &transferCmd->memory, &transferCmd->response, dmaCompleteEvent);
        if (retval) {
            dprintf(DEBUG_CRITICAL, "iopsdio_transferSDIOData error %x\n",retval);
        }
        
    }
	return retval;
    
return_clear:
    {

        unsigned int last_block;
        if (128 == transferCmd->transfer.blockSize)
            last_block = (LOOKAHEAD_READ_PKT_SIZE*2)/128;
        else 
            last_block = (LOOKAHEAD_READ_PKT_SIZE*2)/transferCmd->transfer.blockSize;
        
        //paranoia: check if we have enough space in the buffer, then mark it as done
        if (orig_bcount >= last_block) {
            unsigned int *retp = (unsigned int*)(((unsigned char*)data)+LOOKAHEAD_READ_PKT_SIZE*2); 

            * retp = retval;
            platform_cache_operation(CACHE_CLEAN, 
                                 (void *)retp, 
                                 (32));
        
        }
    }

    return retval;
    
}




