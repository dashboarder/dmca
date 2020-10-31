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

#ifndef _IOP_CEATA_PROTOCOL_H_
#define _IOP_CEATA_PROTOCOL_H_

#include <sys/types.h>

/*
 * Command size is (somewhat) tunable.
 *
 * The principal consideration here is the maximum scatter/gather list size
 * this permits.
 */
#define kIOPCEATA_COMMAND_SIZE   (512)


typedef UInt32 IOPCEATA_opcode_t;

#define kIOPCEATA_OPCODE_UNKNOWN            ((IOPCEATA_opcode_t) 0)

#define kIOPCEATA_OPCODE_RESET		((IOPCEATA_opcode_t) 1)
#define kIOPCEATA_OPCODE_IDENTIFY	((IOPCEATA_opcode_t) 2)
#define kIOPCEATA_OPCODE_READ		((IOPCEATA_opcode_t) 3)
#define kIOPCEATA_OPCODE_WRITE		((IOPCEATA_opcode_t) 4)
#define kIOPCEATA_OPCODE_STANDBY	((IOPCEATA_opcode_t) 5)
#define kIOPCEATA_OPCODE_FLUSH		((IOPCEATA_opcode_t) 6)


typedef UInt32 IOPCEATA_status_t;

#define kIOPCEATA_STATUS_UNKNOWN            ((IOPCEATA_status_t) 0)

#define kIOPCEATA_STATUS_SUCCESS            ((IOPCEATA_status_t) 1)

#define kIOPCEATA_STATUS_FAILURE            ((IOPCEATA_status_t) 0x80000000)
#define kIOPCEATA_STATUS_DEVICE_ERROR       ((IOPCEATA_status_t) 0x80000001)
#define kIOPCEATA_STATUS_DEVICE_TIMEOUT     ((IOPCEATA_status_t) 0x80000002)
#define kIOPCEATA_STATUS_DMA_TIMEOUT        ((IOPCEATA_status_t) 0x80000003)
#define kIOPCEATA_STATUS_PARAM_INVALID      ((IOPCEATA_status_t) 0x80000004)
#define kIOPCEATA_STATUS_UNIMPLEMENTED      ((IOPCEATA_status_t) 0x80000005)

/* this must match <drivers/dma.h>::struct dma_segment */
struct _IOPCEATA_dma_segment {
	u_int32_t   paddr;
	u_int32_t   length;
};
typedef struct _IOPCEATA_dma_segment IOPCEATA_dma_segment;


struct _IOPCEATA
{
	IOPCEATA_opcode_t  opcode;
	IOPCEATA_status_t  status;
};
typedef struct _IOPCEATA IOPCEATA;


struct _IOPCEATA_Identify
{
	IOPCEATA  iopceata;

	// relevant properties from the CE-ATA Identify operation
	UInt8	serial_number[20];	// ASCII bytes, not NUL-terminated
	UInt8	firmware_revision[8];
	UInt8	model_number[40];
	
	UInt32	device_size;		// note this is size, not max LBA
	UInt32	lba_size;		// LBA size in bytes
};
typedef struct _IOPCEATA_Identify IOPCEATA_Identify;


struct _IOPCEATA_ReadWrite
{
	IOPCEATA  iopceata;

	UInt32	starting_lba;
	UInt32	lba_count;

	UInt32  segment_count;
	IOPCEATA_dma_segment data_segments[];

};
typedef struct _IOPCEATA_ReadWrite IOPCEATA_ReadWrite;


union _IOPCEATA_Command
{
	IOPCEATA                  iopceata;

	IOPCEATA_Identify	identify;
	IOPCEATA_ReadWrite	read_write;

	UInt8                   _pad[kIOPCEATA_COMMAND_SIZE];
};
typedef union _IOPCEATA_Command IOPCEATA_Command;

union _IOPCEATA_sgl_sizing_tool
{
	IOPCEATA_ReadWrite      io_single_dma;
};

#define kIOPCEATA_MAX_SEGMENTS    ((kIOPCEATA_COMMAND_SIZE - sizeof(union _IOPCEATA_sgl_sizing_tool)) / sizeof(IOPCEATA_dma_segment))


#endif // _IOP_CEATA_PROTOCOL_H_
