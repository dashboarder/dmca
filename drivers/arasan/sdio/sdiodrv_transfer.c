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


#include "sdiodrv_transfer.h"

#include <debug.h>
#include <AssertMacros.h>

#include <sys/task.h>
#include <drivers/dma.h>
#include <platform.h>

#include <platform/soc/hwdmachannels.h>
#include <platform/soc/hwclocks.h>
#include <platform/clocks.h>

#include "sdiodrv_config.h"
#include "sdiodrv_command.h"


#define TRANSFER_COMPLETE_CHECKS (1000)

#define DMA_WORD_SIZE	(4)
#define DMA_BURST_SIZE	(8)

#define DMA_WORKAROUND_PRESENT (1)


static SDIOReturn_t
sdiodrv_prepareTransfer(SDHCRegisters_t *sdhc, const struct SDIOTransfer *transfer)
{
	check(sdhc_getMaxBlockLength(sdhc) < transfer->blockSize);
	
	sdhc_setBlockSize(sdhc, transfer->blockSize);
	
	sdhc->blockCount = transfer->blockCount;

	UInt16 transferMode = kSDIOTransferEnableBlockCount | kSDIOTransferEnableDMA;
	
	if(sdhc->blockCount > 1) {
		transferMode |= kSDIOTransferMultipleBlocks;
	}
	
	if(transfer->direction & kSDIODirectionRead) {
		transferMode |= kSDIOTransferReadFromCard;
	}

	sdhc->transferMode = transferMode;

//	dprintf(DEBUG_INFO, "\n%s: blockSize = %u, blockCount = %u, cmd53ItemCount = %u, cmd53BlkMode = 0x%X, mode = 0x%X\n",  __func__,
//		transfer->blockSize, transfer->blockCount,
//		(0x1FF & transfer->command.argument), ((1 << 27) & transfer->command.argument), transferMode);

	return kSDIOReturnSuccess;
}

static SDIOReturn_t
sdiodrv_completeTransfer(SDHCRegisters_t *sdhc, const struct SDIOTransfer *transfer)
{
	SDIOReturn_t retval = sdhc_getTransferStatus(sdhc);

	if(kSDIOReturnSuccess != retval) {
		dprintf(DEBUG_CRITICAL, "%s: SDIO Transfer Failed (0x%X): cmd = 0x%X, blkSize = %u, blkCount = %u / %u, normalInt = 0x%X, errorInt = 0x%X, state = 0x%X\n",
			__func__, retval,
			sdhc->command,
			transfer->blockSize, sdhc->blockCount, transfer->blockCount,
			sdhc->normalInterruptStatus, sdhc->errorInterruptStatus, sdhc->presentState);
		
		sdiodrv_resetSDHC(sdhc, kSDHCResetDataLine);
	}

	sdhc_clearTransferStatus(sdhc);

	return retval;
}

static void
sdiodrv_logSegments(const struct dma_segment *segment, UInt32 numSegments, UInt32 totalSize)
{
	dprintf(DEBUG_INFO, "%s: numSegments = %u, totalSize = %u\n",  __func__,
		numSegments, totalSize);
	for (unsigned int i = 0; i < numSegments; i++) {
		dprintf(DEBUG_INFO, "%s:    segment[%u] @ %p: paddr = 0x%X, len = %u\n",  __func__,
			i, &segment[i], segment[i].paddr, segment[i].length);
	}
}


/** @brief Adjusts a segment in the list such that the first N segments sum to the block size. 
 *
 * @param[in] segmentList
 *	The segment list
 * @param[in] numSegments
 *	how many segments are in the segment list.
 * @param[in] blockSize
 *	the target blocksize
 * @param[out] origLength
 *	Original length of the returned segment
 * @return
 *	a pointer to the adjusted segment, NULL if no adjustment occured
 */
static struct dma_segment* 
sdiodrv_adjustSegment(struct dma_segment *segmentList, UInt32 numSegments, UInt32 blockSize, UInt32 *origLength)
{
	UInt32 byteCount=0;

	for(UInt32 i = 0; i < numSegments; i++) {
		// If the # bytes in the segments sums evenly to blockSize, no adjustment needed
		if(byteCount == blockSize) {
//			dprintf(DEBUG_INFO, "%s: no adjustment needed: byteCount = %u, blockSize = %u\n", __func__, byteCount, blockSize);
			return NULL;
		}
		
		byteCount += segmentList[i].length;
		
		// Need to adjust this segment
		if(byteCount > blockSize) {
			*origLength = segmentList[i].length;
			UInt32 overage = byteCount - blockSize;
			segmentList[i].length -= overage;
//			dprintf(DEBUG_INFO, "%s: adjusting: byteCount = %u, blockSize = %u, overage = %u\n", __func__, byteCount, blockSize, overage);
			return &segmentList[i];
		}
	}

	return NULL;
}

/** @brief Readjusts a segment back to its original length. */
static void
sdiodrv_readjustSegment(struct dma_segment *segment, UInt32 origLength)
{
	if(!segment) {
		return;
	}

//	dprintf(DEBUG_INFO, "%s: readjusting: origLength = %u\n", __func__, origLength);
	segment->length = origLength;
}

/** @brief Advance the segment list to the item byteOffset bytes in. 
 *
 * Note: This function makes no effort to enforce aligmnment of the output segments.
 * However, if the input segments are aligned and the byte offset is a multiple of 
 * the aligment, the output segments will be aligned.
 *
 * @param[in] segmentList
 *	The segment list
 * @param[in,out] segmentsRemaining
 *	On input: how many segments are in the segment list.
 *	On output: how many segments are in the returned list
 * @param[in] byteOffset
 *	How many bytes into the segment list the new head should be
 * @return
 *	the new head of the segment list, NULL if there isn't a segment at the requested byte offset
 */
static struct dma_segment*
sdiodrv_advanceSegList(struct dma_segment *segmentList, UInt32 *segmentsRemaining, UInt32 byteOffset)
{
	const UInt32 numSegments = *segmentsRemaining;
	UInt32 byteCount=0;

	for(UInt32 i = 0; i < numSegments; i++) {
		// If the current item is the right offset in
		if(byteCount == byteOffset) {
//			dprintf(DEBUG_INFO, "%s: correct offset: byteCount = %u, byteOffset = %u\n", __func__, byteCount, byteOffset);
			return &segmentList[i];
		}
		
		byteCount += segmentList[i].length;
		
		// If the offset puts us somewhere inside the current segment
		if(byteCount > byteOffset) {
			// Adjust the segment to the right offset and return as the new head
			UInt32 overage = byteCount - byteOffset;
			UInt32 segOffset = segmentList[i].length - overage;
//			dprintf(DEBUG_INFO, "%s: updating: byteCount = %u, byteOffset = %u, overage = %u, segOffset = %u\n", __func__, byteCount, byteOffset, overage, segOffset);
			
			segmentList[i].paddr += segOffset;
			segmentList[i].length -= segOffset;
			
			return &segmentList[i];
		}
		(*segmentsRemaining)--;
	}

	return NULL;
}

static inline void
sdiodrv_performCacheOperation(int operation, void *address, u_int32_t length)
{
	// Adjust the address / length so that it is aligned on a cacheline
	void *adjustedAddr = (void*)((u_int32_t)address & ~(CPU_CACHELINE_SIZE-1));
	u_int32_t adjustedLength = length + (address - adjustedAddr);

	adjustedLength = (adjustedLength + (CPU_CACHELINE_SIZE-1)) & ~(CPU_CACHELINE_SIZE-1);

//	dprintf(DEBUG_INFO, "%s: op = 0x%X, addr = %p, length = %u\n", __func__, operation, adjustedAddr, adjustedLength);

	platform_cache_operation(operation, adjustedAddr, adjustedLength);
}


static void
sdiodrv_handleDMAComplete(void *arg)
{
	struct task_event *dmaCompleteEvent = (struct task_event *)arg;

//	dprintf(DEBUG_INFO, "%s: DMA Complete %p\n",  __func__, dmaCompleteEvent);

	event_signal(dmaCompleteEvent);
}

static SDIOReturn_t
sdiodrv_performDMA(SDHCRegisters_t *sdhc,
				  const struct SDIOTransfer *transfer,
				  const struct SDIOMemorySegments *memory,
				  struct task_event *dmaCompleteEvent)
{
	bool dmaRead = transfer->direction & kSDIODirectionRead;
	u_int32_t dmaCommand = (dmaRead ? DMA_CMD_DIR_RX : DMA_CMD_DIR_TX);
	int dmaErr;

	
#if DMA_WORKAROUND_PRESENT

//	dprintf(DEBUG_INFO, "%s: starting dma for block %u\n", __func__, 0);
	dmaErr = dma_execute_async(dmaCommand, DMA_SDIO,
						(struct dma_segment*)memory->segmentList, (void*)&sdhc->bufferDataPort, memory->dataLength,
						DMA_WORD_SIZE, DMA_BURST_SIZE,
						&sdiodrv_handleDMAComplete, dmaCompleteEvent);
	if(dmaErr) {
		dprintf(DEBUG_CRITICAL, "%s: DMA %s failure (%d), %u / %u blocks of size %u remain (%u seg @ %p)\n",
			__func__, (dmaRead ? "read" : "write"), dmaErr,
			sdhc->blockCount, transfer->blockCount, transfer->blockSize,
			memory->segmentCount, memory->segmentList);
		sdiodrv_logSegments((struct dma_segment *)memory->segmentList, memory->segmentCount, memory->dataLength);
		return kSDIOReturnADMAError;
	}

//	dprintf(DEBUG_INFO, "%s: waiting on block %u\n", __func__, 0);
	bool signaled = event_wait_timeout(dmaCompleteEvent, DMA_SDIO_TIMEOUT_US);
	if(!signaled) {
		dprintf(DEBUG_INFO, "%s: Timeout on DMA %s, %u / %u blocks of size %u remain (%u seg @ %p)\n",
			__func__, (dmaRead ? "read" : "write"),
			sdhc->blockCount, transfer->blockCount, transfer->blockSize,
			memory->segmentCount, memory->segmentList);
		sdiodrv_logSegments((struct dma_segment *)memory->segmentList, memory->segmentCount, memory->dataLength);

		// <rdar://problem/7304920> iBoot IOP SDIO Driver doesn't complete transfer after DMA Timeout
		SDIOReturn_t transferStatus = sdiodrv_completeTransfer(sdhc, transfer);
		if(kSDIOReturnSuccess == transferStatus) {
			transferStatus = kSDIOReturnDMATimeout;
		}
		
		// Save & restore the interrupt config across SDHC reset 
		// Fix "No SDIO Cmd Complete" errors after "SDIO Data CRC Error"
		// <rdar://problem/10646932> 9B139: devices lost ability to use WiFi
		UInt16 normalInterruptStatusEnable = sdhc->normalInterruptStatusEnable;
		UInt16 normalInterruptSignalEnable = sdhc->normalInterruptSignalEnable;
		UInt16 errorInterruptStatusEnable = sdhc->errorInterruptStatusEnable;
		UInt16 errorInterruptSignalEnable = sdhc->errorInterruptSignalEnable;
		

		// <rdar://problem/7311911> H2A/H3 CDMA - DMA Hang after SDIO CMD53 Rx Data CRC Error
		// Hard-reset SDIO block before resetting CDMA
		clock_reset_device(CLK_SDIO);
		
		dma_cancel(DMA_SDIO);
		
		sdhc->normalInterruptStatusEnable = normalInterruptStatusEnable;
		sdhc->normalInterruptSignalEnable = normalInterruptSignalEnable;
		sdhc->errorInterruptStatusEnable  = errorInterruptStatusEnable;
		sdhc->errorInterruptSignalEnable  = errorInterruptSignalEnable;
		
		return transferStatus;
	}
	
//	dprintf(DEBUG_INFO, "%s: END, signaled = %u, err = 0x%X\n",  __func__, signaled, sdhc->errorInterruptStatus);


#else 

	struct dma_segment *blkSegmentList = (struct dma_segment*)memory->segmentList;
	UInt32 segmentsRemaining = memory->segmentCount;
	struct dma_segment *adjustedSegment = NULL;
	UInt32 origLength = 0;

	// We have to setup a new DMA for each SDIO block. Yuck.
	for(UInt32 b = 0; b < transfer->blockCount && blkSegmentList; b++) {

		sdiodrv_performCacheOperation(CACHE_CLEAN, blkSegmentList, sizeof(blkSegmentList[0]));

		adjustedSegment = sdiodrv_adjustSegment(blkSegmentList, segmentsRemaining, transfer->blockSize, &origLength);
		if(adjustedSegment) {
			sdiodrv_performCacheOperation(CACHE_CLEAN, adjustedSegment, sizeof(*adjustedSegment));
		}
		
//		sdiodrv_logSegments((struct dma_segment *)blkSegmentList, segmentsRemaining, transfer->blockSize);
//		dprintf(DEBUG_INFO, "%s:    block[%u] @ %p: paddr = 0x%X, len = %u\n",  __func__,
//			b, &blkSegmentList[0], blkSegmentList[0].paddr, blkSegmentList[0].length);

		dmaErr = dma_execute_async(dmaCommand, DMA_SDIO,
						blkSegmentList, (void*)&sdhc->bufferDataPort, transfer->blockSize,
						DMA_WORD_SIZE, DMA_BURST_SIZE,
						&sdiodrv_handleDMAComplete, dmaCompleteEvent);

		if(dmaErr) {
			dprintf(DEBUG_CRITICAL, "%s: DMA %s failure (%d) on block %u of size %u (%u segRem @ %p)\n", __func__, 
				(dmaRead ? "read" : "write"), dmaErr, b, transfer->blockSize,
				segmentsRemaining, blkSegmentList);
			sdiodrv_logSegments((struct dma_segment *)blkSegmentList, segmentsRemaining, memory->dataLength);
			return kSDIOReturnADMAError;
		}


//		dprintf(DEBUG_INFO, "%s: waiting on block %u\n", __func__, b);
		bool signaled = event_wait_timeout(dmaCompleteEvent, DMA_SDIO_TIMEOUT_US);
		if(!signaled) {
			dprintf(DEBUG_INFO, "%s: Timeout on DMA %s, block %u of size %u (%u segRem @ %p)\n", __func__, 
				(dmaRead ? "read" : "write"), b, transfer->blockSize,
				segmentsRemaining, blkSegmentList);
			sdiodrv_logSegments((struct dma_segment *)blkSegmentList, segmentsRemaining, memory->dataLength);
			
			// <rdar://problem/7304920> iBoot IOP SDIO Driver doesn't complete transfer after DMA Timeout
			SDIOReturn_t transferStatus = sdiodrv_completeTransfer(sdhc, transfer);
			if(kSDIOReturnSuccess == transferStatus) {
				transferStatus = kSDIOReturnDMATimeout;
			}
			
			// Save & restore the interrupt config across SDHC reset 
			// Fix "No SDIO Cmd Complete" errors after "SDIO Data CRC Error"
			// <rdar://problem/10646932> 9B139: devices lost ability to use WiFi
			UInt16 normalInterruptStatusEnable = sdhc->normalInterruptStatusEnable;
			UInt16 normalInterruptSignalEnable = sdhc->normalInterruptSignalEnable;
			UInt16 errorInterruptStatusEnable = sdhc->errorInterruptStatusEnable;
			UInt16 errorInterruptSignalEnable = sdhc->errorInterruptSignalEnable;
			
			
			// <rdar://problem/7311911> H2A/H3 CDMA - DMA Hang after SDIO CMD53 Rx Data CRC Error
			// Hard-reset SDIO block before resetting CDMA
			clock_reset_device(CLK_SDIO);
			
			dma_cancel(DMA_SDIO);
			
			sdhc->normalInterruptStatusEnable = normalInterruptStatusEnable;
			sdhc->normalInterruptSignalEnable = normalInterruptSignalEnable;
			sdhc->errorInterruptStatusEnable  = errorInterruptStatusEnable;
			sdhc->errorInterruptSignalEnable  = errorInterruptSignalEnable;
			
			return transferStatus;
		}

		sdiodrv_readjustSegment(adjustedSegment, origLength);

		blkSegmentList = sdiodrv_advanceSegList(blkSegmentList, &segmentsRemaining, transfer->blockSize);
	}

#endif

	return kSDIOReturnSuccess;
}


SDIOReturn_t
sdiodrv_transferData(SDHCRegisters_t *sdhc,
					 const struct SDIOTransfer *transfer,
					 const struct SDIOMemorySegments *memory,
					 struct SDIOCommandResponse *response,
					 struct task_event *dmaCompleteEvent)
{
	check(sdhc);
	check(transfer);
	check(memory);
	check(response);
	check(dmaCompleteEvent);
	check(transfer->direction);
	check((transfer->direction & (kSDIODirectionRead | kSDIODirectionWrite)) != (kSDIODirectionRead | kSDIODirectionWrite));
	check(transfer->blockCount);
	check(transfer->blockSize);
	check(memory->dataLength);
	check(memory->segmentCount);
	check(memory->segmentList);
	
	
	// Prepare the SDHC for the transfer
	SDIOReturn_t retval = sdiodrv_prepareTransfer(sdhc, transfer);
	if(kSDIOReturnSuccess != retval) {
		return retval;
	}
	
	// Send the SDIO Command
	retval = sdiodrv_sendSDIOCommand(sdhc, &transfer->command, response);
	if(kSDIOReturnSuccess != retval) {
		return retval;
	}

	// TODO: PIO Mode support?
	
	// Kick off the DMA
	retval = sdiodrv_performDMA(sdhc, transfer, memory, dmaCompleteEvent);
	if(kSDIOReturnSuccess != retval) {
		return retval;
	}

	// Wait until the Host Controller says the transfer is complete
	// Should be done by the time the DMA / PIO is done
	for(unsigned int i=0; !sdhc_isTransferComplete(sdhc); i++) {
		if(i >= TRANSFER_COMPLETE_CHECKS) {
			dprintf(DEBUG_CRITICAL, "%s: No (%s) transfer complete after %u checks: cmd = 0x%X, blkSize = %u, blkCount = %u / %u, normalInt = 0x%X, errorInt = 0x%X, state = 0x%X\n",
				__func__, ((transfer->direction & kSDIODirectionRead) ? "read" : "write"), TRANSFER_COMPLETE_CHECKS,
				sdhc->command,
				transfer->blockSize, sdhc->blockCount, transfer->blockCount,
				sdhc->normalInterruptStatus, sdhc->errorInterruptStatus, sdhc->presentState);
			
			sdiodrv_resetSDHC(sdhc, kSDHCResetDataLine);
			
			if(kSDIOReturnSuccess == retval) {
				retval = kSDIOReturnNoTransferComplete;
			}
			
			break;
		}
		
		task_yield();
	}

	// Get the transfer status and cleanup the SDHC
	SDIOReturn_t transferStatus = sdiodrv_completeTransfer(sdhc, transfer);
	
	// Note: on transfer error the SDHC block needs to be reset to workaround:
	//<rdar://problem/6413326> H2: Workaround stuck dmacbreq on SW Reset DAT
	// We'll let the AP-side of the SDIO driver do this for now, since it has
	// the state needed to restore the block to an operational state.
	// TODO: <rdar://problem/7312019> Move hardware workarounds into IOP SDIO driver
	
	return kSDIOReturnSuccess == transferStatus ? retval : transferStatus;
}




