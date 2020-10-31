// *****************************************************************************
//
// File: H2fmi_dma.h
//
// *****************************************************************************
//
// Notes:
//
//
// *****************************************************************************
//
// Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "WMRFeatures.h"

#ifndef _H2FMI_DMA_TYPES_H_
#define _H2FMI_DMA_TYPES_H_

#if H2FMI_EFI

#include "SoC.h"

// These should match iBoot include/drivers/aes.h
#define AES_CMD_ENC       (0x00000000)
#define AES_CMD_DEC       (0x00000001)
#define AES_CMD_DIR_MASK  (0x0000000F)
#define AES_CMD_ECB	      (0x00000000)
#define AES_CMD_CBC	      (0x00000010)
#define AES_CMD_MODE_MASK (0x000000F0)

#define AES_KEY_TYPE_USER (0x00000000)
#define AES_KEY_TYPE_UID0 (0x00000100)
#define AES_KEY_TYPE_GID0 (0x00000200)
#define AES_KEY_TYPE_GID1 (0x00000201)
#define AES_KEY_TYPE_MASK (0x00000FFF)

#define AES_KEY_SIZE_128  (0x00000000)
#define AES_KEY_SIZE_192  (0x10000000)
#define AES_KEY_SIZE_256  (0x20000000)
#define AES_KEY_SIZE_MASK (0xF0000000)

// This should match iBoot include/drivers/dma.h
struct dma_aes_config {
	/* AES configuration */
    UInt32  command;	/* AES_CMD bits from <drivers/aes.h> */
    UInt32  keying;		/* AES_KEY bits from <drivers/aes.h> */
    void   *key;		/* AES key for AES_KEY_TYPE_USER */

	/* IV generation */
    UInt32  chunk_size;	/* AES chunk size */
    void    (* iv_func)(void *arg, UInt32 chunk_index, void *iv_buffer);
    void   *iv_func_arg;
};


struct dma_segment {
    UInt32	paddr;
    UInt32	length;
};
typedef void	(* dma_completion_function)(void *);

#else //!H2FMI_EFI

#include <drivers/aes.h>
#include <drivers/dma.h>
#include <platform/soc/hwdmachannels.h>

#endif //!H2FMI_EFI

#endif //_H2FMI_DMA_TYPES_H_

// ********************************** EOF **************************************
