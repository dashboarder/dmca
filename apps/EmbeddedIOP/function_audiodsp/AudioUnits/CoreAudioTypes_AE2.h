/*
 *  CoreAudioTypes_AE2.h
 *  Stripped down version of CoreAudioTypes just for what AE2 needs
 *
 *  Copyright 2010 Apple Computer. All rights reserved.
 *
 */

#ifndef __COREAUDIOTYPES_AE2__
#define __COREAUDIOTYPES_AE2__

#define CA_PREFER_FIXED_POINT 1

#include <stdint.h>

typedef double Float64;
typedef float Float32;

typedef SInt32 OSStatus;
typedef SInt32 ComponentResult;

typedef SInt16 AudioSampleType;
typedef SInt32 AudioUnitSampleType;

//XXX find the place where this is really defined
enum {
	noErr = 0
};

/*!
    @struct         AudioStreamBasicDescription
    @abstract       This structure encapsulates all the information for describing the basic
                    format properties of a stream of audio data.
    @discussion     This structure is sufficient to describe any constant bit rate format that  has
                    channels that are the same size. Extensions are required for variable bit rate
                    data and for constant bit rate data where the channels have unequal sizes.
                    However, where applicable, the appropriate fields will be filled out correctly
                    for these kinds of formats (the extra data is provided via separate properties).
                    In all fields, a value of 0 indicates that the field is either unknown, not
                    applicable or otherwise is inapproprate for the format and should be ignored.
                    Note that 0 is still a valid value for most formats in the mFormatFlags field.

                    In audio data a frame is one sample across all channels. In non-interleaved
                    audio, the per frame fields identify one channel. In interleaved audio, the per
                    frame fields identify the set of n channels. In uncompressed audio, a Packet is
                    one frame, (mFramesPerPacket == 1). In compressed audio, a Packet is an
                    indivisible chunk of compressed data, for example an AAC packet will contain
                    1024 sample frames.
    @field          mSampleRate
                        The number of sample frames per second of the data in the stream.
    @field          mFormatID
                        A four char code indicating the general kind of data in the stream.
    @field          mFormatFlags
                        Flags specific to each format.
    @field          mBytesPerPacket
                        The number of bytes in a packet of data.
    @field          mFramesPerPacket
                        The number of sample frames in each packet of data.
    @field          mBytesPerFrame
                        The number of bytes in a single sample frame of data.
    @field          mChannelsPerFrame
                        The number of channels in each frame of data.
    @field          mBitsPerChannel
                        The number of bits of sample data for each channel in a frame of data.
    @field          mReserved
                        Pads the structure out to force an even 8 byte alignment.
*/
struct AudioStreamBasicDescription
{
    Float64 mSampleRate;
    UInt32  mFormatID;
    UInt32  mFormatFlags;
    UInt32  mBytesPerPacket;
    UInt32  mFramesPerPacket;
    UInt32  mBytesPerFrame;
    UInt32  mChannelsPerFrame;
    UInt32  mBitsPerChannel;
    UInt32  mReserved;
};
typedef struct AudioStreamBasicDescription  AudioStreamBasicDescription;


//  SMPTETime is also defined in the CoreVideo headers.
#if !defined(__SMPTETime__)
#define __SMPTETime__

/*!
 @struct         SMPTETime
 @abstract       A structure for holding a SMPTE time.
 @field          mSubframes
 The number of subframes in the full message.
 @field          mSubframeDivisor
 The number of subframes per frame (typically 80).
 @field          mCounter
 The total number of messages received.
 @field          mType
 The kind of SMPTE time using the SMPTE time type constants.
 @field          mFlags
 A set of flags that indicate the SMPTE state.
 @field          mHours
 The number of hours in the full message.
 @field          mMinutes
 The number of minutes in the full message.
 @field          mSeconds
 The number of seconds in the full message.
 @field          mFrames
 The number of frames in the full message.
 */
struct SMPTETime
{
    SInt16  mSubframes;
    SInt16  mSubframeDivisor;
    UInt32  mCounter;
    UInt32  mType;
    UInt32  mFlags;
    SInt16  mHours;
    SInt16  mMinutes;
    SInt16  mSeconds;
    SInt16  mFrames;
};
typedef struct SMPTETime    SMPTETime;

/*!
 @enum           SMPTE Time Types
 @abstract       Constants that describe the type of SMPTE time.
 @constant       kSMPTETimeType24
 24 Frame
 @constant       kSMPTETimeType25
 25 Frame
 @constant       kSMPTETimeType30Drop
 30 Drop Frame
 @constant       kSMPTETimeType30
 30 Frame
 @constant       kSMPTETimeType2997
 29.97 Frame
 @constant       kSMPTETimeType2997Drop
 29.97 Drop Frame
 @constant       kSMPTETimeType60
 60 Frame
 @constant       kSMPTETimeType5994
 59.94 Frame
 @constant       kSMPTETimeType60Drop
 60 Drop Frame
 @constant       kSMPTETimeType5994Drop
 59.94 Drop Frame
 @constant       kSMPTETimeType50
 50 Frame
 @constant       kSMPTETimeType2398
 23.98 Frame
 */
enum
{
    kSMPTETimeType24        = 0,
    kSMPTETimeType25        = 1,
    kSMPTETimeType30Drop    = 2,
    kSMPTETimeType30        = 3,
    kSMPTETimeType2997      = 4,
    kSMPTETimeType2997Drop  = 5,
    kSMPTETimeType60        = 6,
    kSMPTETimeType5994      = 7,
    kSMPTETimeType60Drop    = 8,
    kSMPTETimeType5994Drop  = 9,
    kSMPTETimeType50        = 10,
    kSMPTETimeType2398      = 11
};

/*!
 @enum           SMPTE State Flags
 @abstract       Flags that describe the SMPTE time state.
 @constant       kSMPTETimeValid
 The full time is valid.
 @constant       kSMPTETimeRunning
 Time is running.
 */
enum
{
    kSMPTETimeValid     = (1 << 0),
    kSMPTETimeRunning   = (1 << 1)
};

#endif

/*!
 @struct         AudioTimeStamp
 @abstract       A structure that holds different representations of the same point in time.
 @field          mSampleTime
 The absolute sample frame time.
 @field          mHostTime
 The host machine's time base, mach_absolute_time.
 @field          mRateScalar
 The ratio of actual host ticks per sample frame to the nominal host ticks
 per sample frame.
 @field          mWordClockTime
 The word clock time.
 @field          mSMPTETime
 The SMPTE time.
 @field          mFlags
 A set of flags indicating which representations of the time are valid.
 @field          mReserved
 Pads the structure out to force an even 8 byte alignment.
 */
struct AudioTimeStamp
{
    Float64         mSampleTime;
    UInt64          mHostTime;
    Float64         mRateScalar;
    UInt64          mWordClockTime;
    SMPTETime       mSMPTETime;
    UInt32          mFlags;
    UInt32          mReserved;
};
typedef struct AudioTimeStamp   AudioTimeStamp;

/*!
 @enum           AudioTimeStamp Flags
 @abstract       The flags that indicate which fields in an AudioTimeStamp structure are valid.
 @constant       kAudioTimeStampSampleTimeValid
 The sample frame time is valid.
 @constant       kAudioTimeStampHostTimeValid
 The host time is valid.
 @constant       kAudioTimeStampRateScalarValid
 The rate scalar is valid.
 @constant       kAudioTimeStampWordClockTimeValid
 The word clock time is valid.
 @constant       kAudioTimeStampSMPTETimeValid
 The SMPTE time is valid.
 */
enum
{
    kAudioTimeStampSampleTimeValid      = (1 << 0),
    kAudioTimeStampHostTimeValid        = (1 << 1),
    kAudioTimeStampRateScalarValid      = (1 << 2),
    kAudioTimeStampWordClockTimeValid   = (1 << 3),
    kAudioTimeStampSMPTETimeValid       = (1 << 4)
};

/*!
 @enum           Commonly Used Combinations of AudioTimeStamp Flags
 @abstract       Some commonly used combinations of AudioTimeStamp flags.
 @constant       kAudioTimeStampSampleHostTimeValid
 The sample frame time and the host time are valid.
 */
enum
{
    kAudioTimeStampSampleHostTimeValid  = (kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid)
};

#endif
