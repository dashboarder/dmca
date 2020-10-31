/*
 *  AudioUnit_AE2.h
 *  CAServices-Aspen
 *
 *  Created by richardp on 11/28/11.
 *  Copyright 2011 Apple, Inc. All rights reserved.
 *
 *  Serves as the base class for AudioUnits on AE2
 */

#ifndef __AudioUnit_AE2_h__
#define __AudioUnit_AE2_h__

#include <stdint.h>

#include "AudioUnitProperties_AE2.h"

/**
 * AUBase_AE2 is the wrapper to go from AE2 land to CoreAudio land.
 * For instance, we assume input channels are 16-bit, whereas CoreAudio
 * AudioUnit may require floats.  These modules do the translation.
 * 
 * This is not intended to be parity with AudioUnits, but a crippled
 * version.
 */

class AUBase_AE2
{
public:
	AUBase_AE2() {}
	virtual ~AUBase_AE2() {}

	virtual OSStatus Initialize() { return noErr; }

	virtual void Process(const void *inSourceP,
				void *inDestP,
				uint32_t inFramesToProcess) {}

	// for parameters
	virtual OSStatus SetParameter(AudioUnitParameterID paramID, AudioUnitParameterValue value) { return kAudioUnitErr_InvalidParameter; }
	virtual AudioUnitParameterValue GetParameter(AudioUnitParameterID paramID) { return 0; }

	// for properties
	virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID, uint32_t& outDataSize, bool& outWritable) { return kAudioUnitErr_InvalidProperty; }
	virtual OSStatus GetProperty(AudioUnitPropertyID inID, void* outData) { return kAudioUnitErr_InvalidProperty; }
	virtual OSStatus SetProperty(AudioUnitPropertyID inID, const void* inData, uint32_t inDataSize) { return kAudioUnitErr_InvalidProperty; }
};

#endif // __AudioUnit_AE2_h__

