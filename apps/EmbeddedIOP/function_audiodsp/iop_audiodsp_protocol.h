/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _IOP_AUDIODSP_PROTOCOL_H_
#define _IOP_AUDIODSP_PROTOCOL_H_

#include <sys/types.h>

/*
 * Command size is (somewhat) tunable.
 *
 * The principal consideration here is the maximum scatter/gather list size
 * this permits.
 */
#define kIOPAUDIODSP_COMMAND_SIZE   (512)


/*
 * IOPAUDIODSP_opcode_t: identifies the command sent and from the AE2
 */
typedef uint32_t IOPAUDIODSP_opcode_t;

#define kIOPAUDIODSP_OPCODE_UNKNOWN            ((IOPAUDIODSP_opcode_t) 0)

/*
 * Commands for communicating with DSP library(ies)
 */
#define kIOPAUDIODSP_OPCODE_START_LOOPBACK_PROCESSING	((IOPAUDIODSP_opcode_t) 3)
#define kIOPAUDIODSP_OPCODE_STOP_LOOPBACK_PROCESSING		((IOPAUDIODSP_opcode_t) 4)
#define kIOPAUDIODSP_OPCODE_INITTIMESTAMP				((IOPAUDIODSP_opcode_t) 5)
#define kIOPAUDIODSP_OPCODE_TIMESTAMP					((IOPAUDIODSP_opcode_t) 6)
#define kIOPAUDIODSP_OPCODE_SET_PARAMETER				((IOPAUDIODSP_opcode_t) 7)
#define kIOPAUDIODSP_OPCODE_GET_PARAMETER				((IOPAUDIODSP_opcode_t) 8)
#define kIOPAUDIODSP_OPCODE_DO_TRANSFER					((IOPAUDIODSP_opcode_t) 9)
#define kIOPAUDIODSP_OPCODE_SET_PROPERTY				((IOPAUDIODSP_opcode_t) 10)
#define kIOPAUDIODSP_OPCODE_GET_PROPERTY				((IOPAUDIODSP_opcode_t) 11)

/*
 * IOPAUDIODSP_status_t: result from commands
 */
typedef uint32_t IOPAUDIODSP_status_t;

#define kIOPAUDIODSP_STATUS_UNKNOWN            ((IOPAUDIODSP_status_t) 0)
#define kIOPAUDIODSP_STATUS_SUCCESS            ((IOPAUDIODSP_status_t) 1)
#define kIOPAUDIODSP_STATUS_FAILURE            ((IOPAUDIODSP_status_t) 0x80000000)
#define kIOPAUDIODSP_STATUS_DEVICE_ERROR       ((IOPAUDIODSP_status_t) 0x80000001)
#define kIOPAUDIODSP_STATUS_DEVICE_TIMEOUT     ((IOPAUDIODSP_status_t) 0x80000002)
#define kIOPAUDIODSP_STATUS_DMA_TIMEOUT        ((IOPAUDIODSP_status_t) 0x80000003)
#define kIOPAUDIODSP_STATUS_PARAM_INVALID      ((IOPAUDIODSP_status_t) 0x80000004)
#define kIOPAUDIODSP_STATUS_UNIMPLEMENTED      ((IOPAUDIODSP_status_t) 0x80000005)

/*
 * IOPAUDIODSP_module_t: dsp modules
 */
typedef uint32_t IOPAUDIODSP_module_t;

#define kIOPAUDIODSP_MODULE_LOOPBACK_PROCESSING ((IOPAUDIODSP_module_t) 1)

/*
 * IOPAUDIODSP_token_t: The opaque token to use for audio processing
 */
typedef uint32_t IOPAUDIODSP_token_t;

// Token for sending messages to the iop_audiodsp system itself
#define kIOPAUDIODSP_system_token_t            ((IOPAUDIODSP_opcode_t) 0xFFFFFFFF)

/*
 * IOPAUDIODSP_status_t result from commands
 */
struct _IOPAUDIODSP
{
    IOPAUDIODSP_opcode_t  mOpcode;
    IOPAUDIODSP_status_t  mStatus;
};
typedef struct _IOPAUDIODSP IOPAUDIODSP;

/*
 * Messages.  Message layout is dependent on the opcode.  For instance, if
 * the opcode is kIOPAUDIODSP_OPCODE_START, then the message will be interpreted
 * as IOPAUDIODSP_START.
 * It is the senders responsiblity to make sure a message is set correctly.
 */

/*
 * IOPAUDIODSP_START message
 * Message used to start a new audio process.
 * If message succeeds, iopToken will contain the token used for processing.
 * Fill the input and output with necessary parameters.  Additional parameters
 * passed through the additional_paramters pointer.
 */
struct _IOPAUDIODSP_START
{
    IOPAUDIODSP   mIOPHeader;
    uint32_t      mAdditionalParametersSizeBytes;
    uint8_t       mAdditionalParameters[];
};
typedef struct _IOPAUDIODSP_START IOPAUDIODSP_START;

/*
 * IOPAUDIODSP_STOP message
 * Message used to destroy a token.
 * If message succeeds, iopToken is no longer valid.
 */
struct _IOPAUDIODSP_STOP
{
    IOPAUDIODSP   mIOPHeader;
};
typedef struct _IOPAUDIODSP_STOP IOPAUDIODSP_STOP;

/*
 * IOPAUDIODSP_INITTIMESTAMP message
 */
struct _IOPAUDIODSP_INITTIMESTAMP
{
    IOPAUDIODSP   mIOPHeader;
    uint32_t      mTimeStamperBufferAddr;
};
typedef struct _IOPAUDIODSP_INITTIMESTAMP IOPAUDIODSP_INITTIMESTAMP;

/*
 * IOPAUDIODSP_TIMESTAMP message
 */
struct _IOPAUDIODSP_TIMESTAMP
{
    IOPAUDIODSP   mIOPHeader;
    uint64_t      mSampleCount;
    uint64_t      mTimeStamp;
};
typedef struct _IOPAUDIODSP_TIMESTAMP IOPAUDIODSP_TIMESTAMP;

/*
 * IOPAUDIODSP_DO_TRANSFER message
 */
struct _IOPAUDIODSP_DO_TRANSFER
{
    IOPAUDIODSP   mIOPHeader;
    uint32_t      mIndex;
    uint32_t      mDirection;
    uint32_t      mDoTransfer;
    // send the address range of the buffer to work with, as well as where processing should start
    uint32_t      mBufferBegin;
    uint32_t      mBufferEnd;
    uint32_t      mBufferStart;
	
    // this are necessary if we do psuedo transfer by timer
    uint32_t      mSampleRate;
    // this is necessary to report samples transferred
    uint32_t      mBytesPerFrame;
};
typedef struct _IOPAUDIODSP_DO_TRANSFER IOPAUDIODSP_DO_TRANSFER;

/*
 * Format of Parameters is (IOPAUDIODSP_PARAMETER_SUBCOMMAND) ...
 *    {
 *    parameter (4-bytes)
 *    parameter_value (float)
 *    }
 *
 */
struct _IOPAUDIODSP_PARAMETER_SUBCOMMAND
{
    uint32_t	mParameterID;
    float	mParameterValue;
};
typedef struct _IOPAUDIODSP_PARAMETER_SUBCOMMAND IOPAUDIODSP_PARAMETER_SUBCOMMAND;

/*
 * Format of Properties is (IOPAUDIODSP_PROPERTY_SUBCOMMAND) ...
 *    {
 *    Property (4-bytes)
 *    Property_size (4-byte)
 *    Property (void*)
 *    }
 *
 */
struct _IOPAUDIODSP_PROPERTY_SUBCOMMAND
{
    uint32_t      mPropertyID;
    uint32_t      mPropertySizeBytes;
    uint8_t       mPropertyData[];
};
typedef struct _IOPAUDIODSP_PROPERTY_SUBCOMMAND IOPAUDIODSP_PROPERTY_SUBCOMMAND;

struct _IOPAUDIODSP_MODULE_COMMAND
{
    IOPAUDIODSP   mIOPHeader;
    uint32_t      mModule;
    union
    {
        IOPAUDIODSP_PROPERTY_SUBCOMMAND mProperty;
        IOPAUDIODSP_PARAMETER_SUBCOMMAND mParameter;
    };
};
typedef struct _IOPAUDIODSP_MODULE_COMMAND IOPAUDIODSP_MODULE_COMMAND;

union _IOPAUDIODSP_Command
{
    IOPAUDIODSP					iopaudiodsp;
	
    IOPAUDIODSP_START			start;
    IOPAUDIODSP_STOP			stop;
    IOPAUDIODSP_INITTIMESTAMP	init_timestamp;
    IOPAUDIODSP_DO_TRANSFER		do_transfer;
    IOPAUDIODSP_TIMESTAMP		timestamp;
    IOPAUDIODSP_MODULE_COMMAND	module_command;
    UInt8						_pad[kIOPAUDIODSP_COMMAND_SIZE];
};
typedef union _IOPAUDIODSP_Command IOPAUDIODSP_Command;


#endif // _IOP_AUDIODSP_PROTOCOL_H_
