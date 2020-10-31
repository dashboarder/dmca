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

#ifndef _IOP_AUDIO_PROTOCOL_H_
#define _IOP_AUDIO_PROTOCOL_H_

#include <sys/types.h>

/*
 * Command size is (somewhat) tunable.
 *
 * The principal consideration here is the maximum scatter/gather list size
 * this permits.
 */
#define kIOPAUDIO_COMMAND_SIZE   (512)


/*
 * IOPAUDIO_opcode_t: identifies the command sent and from the AE2
 */
typedef uint32_t IOPAUDIO_opcode_t;

#define kIOPAUDIO_OPCODE_UNKNOWN            ((IOPAUDIO_opcode_t) 0)

/*
 * Commands for communicating with AudioCodecs library
 */
#define kIOPAUDIO_OPCODE_CREATE             ((IOPAUDIO_opcode_t) 3)
#define kIOPAUDIO_OPCODE_DESTROY            ((IOPAUDIO_opcode_t) 4)
#define kIOPAUDIO_OPCODE_RESET              ((IOPAUDIO_opcode_t) 5)
#define kIOPAUDIO_OPCODE_PROCESSFRAME       ((IOPAUDIO_opcode_t) 6)
#define kIOPAUDIO_OPCODE_GETPROPINFO        ((IOPAUDIO_opcode_t) 7)
#define kIOPAUDIO_OPCODE_GETPROPERTY        ((IOPAUDIO_opcode_t) 8)
#define kIOPAUDIO_OPCODE_SETPROPERTY        ((IOPAUDIO_opcode_t) 9)

/*
 * IOPAUDIO_status_t: result from commands
 */
typedef uint32_t IOPAUDIO_status_t;

#define kIOPAUDIO_STATUS_UNKNOWN            ((IOPAUDIO_status_t) 0)
#define kIOPAUDIO_STATUS_SUCCESS            ((IOPAUDIO_status_t) 1)
#define kIOPAUDIO_STATUS_FAILURE            ((IOPAUDIO_status_t) 0x80000000)
#define kIOPAUDIO_STATUS_DEVICE_ERROR       ((IOPAUDIO_status_t) 0x80000001)
#define kIOPAUDIO_STATUS_DEVICE_TIMEOUT     ((IOPAUDIO_status_t) 0x80000002)
#define kIOPAUDIO_STATUS_DMA_TIMEOUT        ((IOPAUDIO_status_t) 0x80000003)
#define kIOPAUDIO_STATUS_PARAM_INVALID      ((IOPAUDIO_status_t) 0x80000004)
#define kIOPAUDIO_STATUS_UNIMPLEMENTED      ((IOPAUDIO_status_t) 0x80000005)
#define kIOPAUDIO_STATUS_CODECERROR         ((IOPAUDIO_status_t) 0x80000010)


/*
 * IOPAUDIO_token_t: The opaque token to use for audio processing
 */
typedef uint32_t IOPAUDIO_token_t;
typedef uint32_t IOPAUDIO_codecstatus_t;

/*
 * IOPAUDIO_CODEC contains the pair of token and codec status
 */
struct _IOPAUDIO_CODEC
{
    IOPAUDIO_token_t        mIOPToken;
    IOPAUDIO_codecstatus_t  mCodecStatus;
};
typedef struct _IOPAUDIO_CODEC IOPAUDIO_CODEC;

// Token for sending messages to the iop_audio system itself
#define kIOPAUDIO_system_token_t            ((IOPAUDIO_opcode_t) 0xFFFFFFFF)

/*
 * IOPAUDIO_status_t result from commands
 */
struct _IOPAUDIO
{
    IOPAUDIO_opcode_t  mOpcode;
    IOPAUDIO_status_t  mStatus;
    IOPAUDIO_CODEC     Codec;
};
typedef struct _IOPAUDIO IOPAUDIO;

struct _FunctionAudioAudioComponentDescription
{
    uint32_t  mComponentType;
    uint32_t  mComponentSubType;
    uint32_t  mComponentManufacturer;
    uint32_t  mComponentFlags;
    uint32_t  mComponentFlagsMask;
};
typedef struct _FunctionAudioAudioComponentDescription FunctionAudioAudioComponentDescription;

struct _FunctionAudioAudioStreamBasicDescription
{
    uint64_t  mSampleRate;
    uint32_t  mFormatID;
    uint32_t  mFormatFlags;
    uint32_t  mBytesPerPacket;
    uint32_t  mFramesPerPacket;
    uint32_t  mBytesPerFrame;
    uint32_t  mChannelsPerFrame;
    uint32_t  mBitsPerChannel;
    uint32_t  mReserved;
};
typedef struct _FunctionAudioAudioStreamBasicDescription FunctionAudioAudioStreamBasicDescription;

enum MetricsFields
{
    ProcessTimeUSec = 0,
    ProcessCycles,
    PeakStackUsage,
    HeapInUsage,
    PeakHeapUsage,
    ICacheAccesses, // these are measured by A5 performance monitors
    ICacheMisses,
    DCacheAccesses,
    DCacheMisses,
    Loads,
    Stores,
    UnalignedLoadStores,
    InstructionsExecuted,
    CorrectPredictedBranches,
    IncorrectPredictedBranches,
    ACLKCycles, // these are measured by AE2 performance monitors
    ACLKCyclesInWFI,
    SRAMBytesRead,
    SRAMBytesWrite,
    SRAMBytesReadWrite,
    DRAMBytesRead,
    DRAMBytesWrite,
    DRAMBytesReadWrite,
    ReservedStart,
    MetricsFieldsSize = 32
};

struct _IOPAUDIO_METRICS
{
    uint32_t  mMetricsItemsValid; // bitmap marking which values are valid
    uint32_t  mMetricsFields[MetricsFieldsSize];
};
typedef struct _IOPAUDIO_METRICS IOPAUDIO_METRICS;

/*
 * Messages.  Message layout is dependent on the opcode.  For instance, if
 * the opcode is kIOPAUDIO_OPCODE_CREATE, then the message will be interpreted
 * as IOPAUDIO_CREATE.
 * It is the senders responsiblity to make sure a message is set correctly.
 */

/*
 * IOPAUDIO_CREATE message
 * Message used to create a new audio process.
 * If message succeeds, iopToken will contain the token used for processing.
 * Fill the input and output with necessary parameters.  Additional parameters
 * passed through the additional_paramters pointer.
 *
 * It is decoder specific how additional data is laid out.
 */
struct _IOPAUDIO_CREATE
{
    IOPAUDIO          mIOPHeader;

    FunctionAudioAudioComponentDescription   mComponentDesc;
    FunctionAudioAudioStreamBasicDescription mInputFormat;
    FunctionAudioAudioStreamBasicDescription mOutputFormat;
    uint32_t          mAdditionalParametersSizeBytes;
    uint8_t           mAdditionalParameters[];
};
typedef struct _IOPAUDIO_CREATE IOPAUDIO_CREATE;

/*
 * IOPAUDIO_DESTROY message
 * Message used to destroy a token.
 * If message succeeds, iopToken is no longer valid.
 */
struct _IOPAUDIO_DESTROY
{
    IOPAUDIO          mIOPHeader;
};
typedef struct _IOPAUDIO_DESTROY IOPAUDIO_DESTROY;

/*
 * IOPAUDIO_RESET message
 * Message used to reset an audio process
 * If message succeeds, iopToken is reset to the starting state.
 */
struct _IOPAUDIO_RESET
{
    IOPAUDIO          mIOPHeader;
};
typedef struct _IOPAUDIO_RESET IOPAUDIO_RESET;

/*
 * IOPAUDIO_GETPROPINFO message
 * Message used to get property information.  mPropertyID is the 4-char code
 * indicating what property to get info on.
 * If message succeeds, size and writablility are set.
 */
struct _IOPAUDIO_GETPROPINFO
{
    IOPAUDIO          mIOPHeader;

    uint32_t          mPropertyID;
    uint32_t          mPropertySize;
    uint32_t          mPropertyWritable;
};
typedef struct _IOPAUDIO_GETPROPINFO IOPAUDIO_GETPROPINFO;

/*
 * IOPAUDIO_GETPROPERTY message
 * Message used to get property.  mPropertyID is the 4-char code indicating what
 * property to get.
 * Addr passed in is the physical address where should be written.  It can
 * point within the message or point to some other buffer that IOP can access.
 * If message succeeds, the property is written to the Addr.
 */
struct _IOPAUDIO_GETPROPERTY
{
    IOPAUDIO          mIOPHeader;

    uint32_t          mPropertyID;
    uint32_t          mPropertySizeBytes;
    uint8_t           mPropertyData[];
};
typedef struct _IOPAUDIO_GETPROPERTY IOPAUDIO_GETPROPERTY;

/*
 * IOPAUDIO_SETPROPERTY message
 * Message used to set property.  mPropertyID is the 4-char code indicating what
 * property to set.
 * Addr passed in is the physical address which has the data to write.  It can
 * point within the message or point to some other buffer that IOP can access.
 * If message succeeds, the property is written to the Addr.
 */
struct _IOPAUDIO_SETPROPERTY
{
    IOPAUDIO          mIOPHeader;

    uint32_t          mPropertyID;
    uint32_t          mPropertySizeBytes;
    uint8_t           mPropertyData[];
};
typedef struct _IOPAUDIO_SETPROPERTY IOPAUDIO_SETPROPERTY;

/*
 * IOPAUDIO_PROCESSFRAME message
 * Process audio from the src_addr to the dst_addr.  On input, src_size is 
 * the size of the source data, dst_size is the size available to write into.
 * On output, src_size is the amount of data consumed, dst_size is the amount
 * of data written.
 * Metrics may or may not be filled in, depending on the build.
 */
struct _IOPAUDIO_PROCESSFRAME
{
    IOPAUDIO          mIOPHeader;

    uint32_t          mSrcAddr;
    uint32_t          mDstAddr;
    uint32_t          mSrcSizeBytes;
    uint32_t          mDstSizeBytes;
    IOPAUDIO_METRICS  mMetrics;
    //add packet information for multple packets
    uint16_t          mNumPackets;
    uint16_t          mPacketSize[];
};
typedef struct _IOPAUDIO_PROCESSFRAME IOPAUDIO_PROCESSFRAME;

union _IOPAUDIO_Command
{
    IOPAUDIO                iopaudio;

    IOPAUDIO_CREATE         create;
    IOPAUDIO_DESTROY        destroy;
    IOPAUDIO_RESET          reset;
    IOPAUDIO_PROCESSFRAME   process_frame;
    IOPAUDIO_GETPROPINFO    get_propinfo;
    IOPAUDIO_GETPROPERTY    get_property;
    IOPAUDIO_SETPROPERTY    set_property;

    uint8_t                 _pad[kIOPAUDIO_COMMAND_SIZE];
};
typedef union _IOPAUDIO_Command IOPAUDIO_Command;


#endif // _IOP_AUDIO_PROTOCOL_H_
