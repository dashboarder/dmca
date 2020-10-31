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

#ifndef _SDIODRV_TRANSFER_H
#define _SDIODRV_TRANSFER_H


#include <sys/types.h>
#include <sdiocommon/sdhc_registers.h>

struct SDIOTransfer;
struct SDIOMemorySegments;
struct SDIOCommandResponse;
struct task_event;


/** @brief Transfers Data across the SDIO bus.
 * @param[in] sdhc
 *	Target SDIO Host Controller.
 * @param[in] transfer
 *	Information describing the transfer to perform
 * @param[in] memory
 *	Information describing the memory that will source/sink the transfer.
 *	(Includes the segment list)
 * @param[out] response
 *	The response to the SDIO command associated with the data transfer
 * @param[in] dmaCompleteEvent
 *	The DMA event used to synchronize the DMA transfer
 * @return
 *	kSDIOReturnSuccess            - on success
 *	kSDIOReturnADMAError          - Error executing the DMA
 *	kSDIOReturnDMATimeout         - DMA failed to complete
 *	kSDIOReturnNoTransferComplete - Transfer complete status never set in SDHC
 *	kSDIOReturnDataTimeout        - SDHC timed out waiting for data
 *	kSDIOReturnDataCRCError       - SDHC detected a CRC error in data
 *	kSDIOReturnDataEndBitError    - SDHC detected an end-bit error in data
 */
SDIOReturn_t sdiodrv_transferData(SDHCRegisters_t *sdhc,
								  const struct SDIOTransfer *transfer,
								  const struct SDIOMemorySegments *memory,
								  struct SDIOCommandResponse *response,
								  struct task_event *dmaCompleteEvent);


#endif /* _SDIODRV_TRANSFER_H */

