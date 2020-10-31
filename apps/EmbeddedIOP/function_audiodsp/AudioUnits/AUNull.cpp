/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "AUNull.h"
#include "string.h"
#include <debug.h>

#define GENERATE_SAWTOOTH 0

AUNull *
AUNull::Create_AUNull(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize)
{
	AUNull *This = new AUNull;
	if (This)
	{
		if (This->InitWith(sampleRate, numChannels, sampleSize))
		{
			This->Initialize();
		}
		else
		{
			delete This;
			This = NULL;
		}
	}
	return This;
}

AUNull::AUNull()
{
}


bool
AUNull::InitWith(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize)
{
	mSampleRate = sampleRate;
	mNumChannels = numChannels;
	mSampleSize = sampleSize;
	return true;
}

AUNull::~AUNull()
{
}

#if GENERATE_SAWTOOTH
static int16_t currentValue = 0;
static const uint32_t kFreq = 600;
#endif

void
AUNull::Process(const void *inSourceP,
				void *inDestP,
				UInt32 inFramesToProcess)
{
#if GENERATE_SAWTOOTH
	uint16_t amountToAdd = (0x10000L * kFreq) / mSampleRate;
	int16_t *dst = static_cast<int16_t*>(inDestP);
	size_t i = 0; 
	for (i = 0; i < (inFramesToProcess * mNumChannels); ++i)
	{
		*dst++ = currentValue;
		currentValue += amountToAdd;
	}
#endif
}


