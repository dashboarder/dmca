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

#include "iop_au_interface.h"
#include "AUSidetone.h"
#include "AUNull.h"
#include <debug.h>

extern "C" audio_unit_t Create_AUSidetone(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize)
{
#if USE_SIDETONE
	AUBase_AE2 *This = AUSidetone::Create_AUSidetone(sampleRate, numChannels, sampleSize);
	return (audio_unit_t)This;
#else
	return NULL;
#endif
}

extern "C" audio_unit_t Create_AUNull(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize)
{
	AUBase_AE2 *This = AUNull::Create_AUNull(sampleRate, numChannels, sampleSize);
	return (audio_unit_t)This;
}

extern "C" void Destroy_AudioUnit(audio_unit_t au)
{
	AUBase_AE2 *This = (AUBase_AE2*)au;
	delete This;
}

extern "C" int32_t AudioUnit_GetParameter(audio_unit_t au, uint32_t parameter, float *parameterValue)
{
	if (!au)
	{
		return -1;
	}
	AUBase_AE2 *This = (AUBase_AE2*)au;
	*parameterValue = This->GetParameter((AudioUnitParameterID)parameter);
	return 0;
}

extern "C" int32_t AudioUnit_SetParameter(audio_unit_t au, uint32_t parameter, float parameterValue)
{
	if (!au)
	{
		return -1;
	}
	AUBase_AE2 *This = (AUBase_AE2*)au;
	return This->SetParameter((AudioUnitParameterID)parameter, (AudioUnitParameterValue)parameterValue);
}

extern "C" int32_t AudioUnit_GetPropertyInfo(audio_unit_t au, uint32_t propID, uint32_t *outSize, bool *outWritable)
{
	if (!au)
	{
		return -1;
	}
	AUBase_AE2 *This = (AUBase_AE2*)au;
	return This->GetPropertyInfo(static_cast<AudioUnitPropertyID>(propID), *outSize, *outWritable);
}

extern "C" int32_t AudioUnit_GetProperty(audio_unit_t au, uint32_t propID, void *outData, uint32_t *ioSize)
{
	if (!au)
	{
		return -1;
	}
	uint32_t propSize;
	bool propWritable;
	int32_t result = AudioUnit_GetPropertyInfo(au, propID, &propSize, &propWritable);
	if (result)
	{
		return result;
	}
	if (propSize > *ioSize)
	{
		return -1;
	}
	*ioSize = propSize;
	AUBase_AE2 *This = (AUBase_AE2*)au;
	return This->GetProperty(static_cast<AudioUnitPropertyID>(propID), outData);
}

extern "C" int32_t AudioUnit_SetProperty(audio_unit_t au, uint32_t propID, const void *inData, uint32_t inSize)
{
	if (!au)
	{
		return -1;
	}
	uint32_t propSize;
	bool propWritable;
	int32_t result = AudioUnit_GetPropertyInfo(au, propID, &propSize, &propWritable);
	if (result)
	{
		return result;
	}
	if ((!propWritable))
	{
		return -1;
	}
	AUBase_AE2 *This = (AUBase_AE2*)au;
	return This->SetProperty(static_cast<AudioUnitPropertyID>(propID), inData, inSize);
}

extern "C" void AudioUnit_Process(audio_unit_t au, const void *inSourceP, void *inDestP, uint32_t inFramesToProcess)
{
	if (!au)
	{
		return ;
	}
	AUBase_AE2 *This = (AUBase_AE2*)au;
	This->Process(inSourceP, inDestP, inFramesToProcess);
}

