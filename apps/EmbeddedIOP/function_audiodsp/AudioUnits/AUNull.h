/*
 *  AUNull.h
 *
 *  Created by richardp on 11/28/11.
 *  Copyright 2011 Apple, Inc. All rights reserved.
 *
 *  Does necessary translation between AE2 and CA AU speaker protection
 */

#ifndef __AUNull_h__
#define __AUNull_h__

#include <stdint.h>

#include "AudioUnitProperties_AE2.h"
#include "AUBase_AE2.h"

class IIRFilter;

/**
 * AUNull just passes input to output
 * Assumes audio data is 16-bit stereo
 */
class AUNull : public AUBase_AE2
{
public:
	static AUNull* Create_AUNull(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize);
	virtual ~AUNull();

	virtual void Process(const void *inSourceP,
				void *inDestP,
				UInt32 inFramesToProcess);

private:
	AUNull();
	bool InitWith(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize);

	// some constants
	typedef AUBase_AE2 super;
	size_t mSampleRate;
	size_t mNumChannels;
	size_t mSampleSize;
};

#endif // __AUNull_h__

