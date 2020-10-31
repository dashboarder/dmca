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

#if USE_SIDETONE
#include "AUSidetone.h"
#include "IIRFiltersA5.h"
#include "VolumeA5.h"
#include "string.h"
#include <debug.h>


// a basic fixed 16-bit mono 44.1 description
static AudioStreamBasicDescription kDefaultMonoFormat = { 44100.0, 'lpcm', 0xC, 2, 1, 2, 1, 16, 0 };

static void SetDefaultSidetoneEQ(BiquadDescriptor *biquadValues, size_t numBands, uint32_t sampleRate)
{
	size_t i = 0;
	if (i < numBands)
	{
		biquadValues[i].filterType = kAUNBandEQFilterType_2ndOrderButterworthLowPass;
		biquadValues[i].frequencyInHertz = (sampleRate == 8000) ? 3800.0f : 7400.0f;
		biquadValues[i].gain = 0.f;
		biquadValues[i].bw = 0.05;
		biquadValues[i].bypassBand = 0.f;
		++i;
	}
	if (i < numBands)
	{
		biquadValues[i].filterType = kAUNBandEQFilterType_2ndOrderButterworthHighPass;
		biquadValues[i].frequencyInHertz = (sampleRate == 8000) ? 200.0f : 100.0f;
		biquadValues[i].gain = 0.f;
		biquadValues[i].bw = 0.05;
		biquadValues[i].bypassBand = 0.f;
		++i;
	}

	for (; i < numBands; ++i)
	{
		biquadValues[i].filterType = 0;
		biquadValues[i].frequencyInHertz = 100.0f;
		biquadValues[i].gain = 0.f;
		biquadValues[i].bw = 0.05;
		biquadValues[i].bypassBand = 1.f;
	}
}


AUSidetone *
AUSidetone::Create_AUSidetone(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize)
{
	AUSidetone *This = new AUSidetone;
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


AUSidetone::AUSidetone() :
mSampleRate(44100),
mNumChannels(1),
mSidetoneEQ(NULL),
mVolume(NULL),
mNumberBands(kMaxNumBands),
mBiquadValues(NULL),
mVolumeInDB(kDefaultSidetoneGain),
mSidetoneEQBypass(false)
{
}


bool
AUSidetone::InitWith(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize)
{
	if (sampleSize != 2)
	{
		return false;
	}

	mSampleRate = sampleRate;
	mNumChannels = numChannels;

	AudioStreamBasicDescription thisFormat = kDefaultMonoFormat;
	thisFormat.mSampleRate = mSampleRate;
	thisFormat.mBytesPerPacket *= mNumChannels;
	thisFormat.mBytesPerFrame *= mNumChannels;
	thisFormat.mChannelsPerFrame *= mNumChannels;
	thisFormat.mBitsPerChannel *= mNumChannels;

	mSidetoneEQ = NewAE2IIRFilter(&thisFormat, &thisFormat, kMaxNumBands);
	dprintf(DEBUG_INFO, "sidetone eq %p\n", mSidetoneEQ);
	mBiquadValues = new BiquadDescriptor[mNumberBands];
	if (mBiquadValues)
	{
		SetDefaultSidetoneEQ(mBiquadValues, mNumberBands, mSampleRate);
		if (mSidetoneEQ)
		{
			mVolume = NewVolume(thisFormat);
			dprintf(DEBUG_INFO, "volume %p\n", mVolume);
			if (mVolume)
			{
				if ((SetAE2IIRFilter(mSidetoneEQ, &thisFormat, mNumberBands, mBiquadValues) == noErr) &&
					(SetGain(mVolume, mVolumeInDB) == noErr))
				{
					dprintf(DEBUG_INFO, "it is all good\n");
					return true;
				}
				DeleteVolume(mVolume);
				mVolume = NULL;
			}
			DeleteAE2IIRFilter(mSidetoneEQ);
			mSidetoneEQ = NULL;
		}
		delete [] mBiquadValues;
		mBiquadValues = NULL;
	}
	return false;
}


AUSidetone::~AUSidetone()
{
	DeleteAE2IIRFilter(mSidetoneEQ);
	mSidetoneEQ = NULL;
	DeleteVolume(mVolume);
	mVolume = NULL;
	delete [] mBiquadValues;
	mBiquadValues = NULL;
}


// These conversion routines are for translating from UInt32 of the AP/AE2 bridge
// to the floating point of the internal representation.  Scaling and offsets applied here.
Float32 AudioUnitParameterValue_To_BypassBand(AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t((value == 0) ? 0.f : 1.f));
	return (value == 0) ? 0.f : 1.f;
}


AudioUnitParameterValue BypassBand_To_AudioUnitParameterValue(Float32 value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t((value == 0.f) ? 0 : 1));
	return (value == 0.f) ? 0 : 1;
}


UInt32 AudioUnitParameterValue_To_FilterType(AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(value));
	return value;
}


AudioUnitParameterValue FilterType_To_AudioUnitParameterValue(UInt32 value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(value));
	return value;
}


Float32 AudioUnitParameterValue_To_Frequency(AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(value));
	return value;
}


AudioUnitParameterValue Frequency_To_AudioUnitParameterValue(Float32 value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(value));
	return value;
}


Float32 AudioUnitParameterValue_To_Gain(AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(-96.0 + (value)));
	return -96.0 + (value);
}


AudioUnitParameterValue Gain_To_AudioUnitParameterValue(Float32 value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t((value) - (-96.0)));
	return (value) - (-96.0);
}


Float32 AudioUnitParameterValue_To_Bandwidth(AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(value / 100.0f));
	return value / 100.0f;
}


AudioUnitParameterValue Bandwidth_To_AudioUnitParameterValue(Float32 value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(value * 100.0f));
	return value * 100.0f;
}


Float32 AudioUnitParameterValue_To_VolumeInDB(AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t(-96.0 + (value)));
	return -96.0 + (value);
}


AudioUnitParameterValue VolumeInDB_To_AudioUnitParameterValue(Float32 value)
{
	dprintf(DEBUG_INFO, "%s %d value %d\n", __FUNCTION__, uint32_t(value), uint32_t((value) - (-96.0)));
	return (value) - (-96.0);
}


OSStatus
AUSidetone::SetParameter(AudioUnitParameterID paramID, AudioUnitParameterValue value)
{
	dprintf(DEBUG_INFO, "SetParameter %d value %d\n", paramID, uint32_t(value));
	if (paramID == kAUVolume_InDB)
	{
		mVolumeInDB = AudioUnitParameterValue_To_VolumeInDB(value);
		return SetGain(mVolume, mVolumeInDB);
	}

	AudioStreamBasicDescription thisFormat = kDefaultMonoFormat;
	thisFormat.mSampleRate = mSampleRate;
	thisFormat.mBytesPerPacket *= mNumChannels;
	thisFormat.mBytesPerFrame *= mNumChannels;
	thisFormat.mChannelsPerFrame *= mNumChannels;
	thisFormat.mBitsPerChannel *= mNumChannels;

	// get the index
	uint32_t which = paramID % 1000;
	uint32_t type = (paramID / 1000)*1000;
	bool success = false;
	if (which < mNumberBands)
	{
		success = true;
		switch (type)
		{
		case kAUNBandEQParam_BypassBand:
			mBiquadValues[which].bypassBand = AudioUnitParameterValue_To_BypassBand(value);
			break;
		case kAUNBandEQParam_FilterType:
			mBiquadValues[which].filterType = AudioUnitParameterValue_To_FilterType(value);
			break;
		case kAUNBandEQParam_Frequency:
			mBiquadValues[which].frequencyInHertz = AudioUnitParameterValue_To_Frequency(value);
			break;
		case kAUNBandEQParam_Gain:
			mBiquadValues[which].gain = AudioUnitParameterValue_To_Gain(value);
			break;
		case kAUNBandEQParam_Bandwidth:
			mBiquadValues[which].bw = AudioUnitParameterValue_To_Bandwidth(value);
			break;
		default:
			success = false;
		}
	}

	OSStatus result = 0;
	if (success)
	{
		result = SetAE2IIRFilter(mSidetoneEQ, &thisFormat, mNumberBands, mBiquadValues);
	}
	else
	{
		result = super::SetParameter(paramID, value);
	}
	dprintf(DEBUG_INFO, "result is %d\n", result);
	return result;
}


AudioUnitParameterValue
AUSidetone::GetParameter(AudioUnitParameterID paramID)
{
	if (paramID == kAUVolume_InDB)
	{
		return VolumeInDB_To_AudioUnitParameterValue(mVolumeInDB);
	}
	// get the index
	uint32_t which = paramID % 1000;
	uint32_t type = (paramID / 1000)*1000;
	if (which < mNumberBands)
	{
		switch (type)
		{
		case kAUNBandEQParam_BypassBand:
			return BypassBand_To_AudioUnitParameterValue(mBiquadValues[which].bypassBand);
		case kAUNBandEQParam_FilterType:
			return FilterType_To_AudioUnitParameterValue(mBiquadValues[which].filterType);
		case kAUNBandEQParam_Frequency:
			return Frequency_To_AudioUnitParameterValue(mBiquadValues[which].frequencyInHertz);
		case kAUNBandEQParam_Gain:
			return Gain_To_AudioUnitParameterValue(mBiquadValues[which].gain);
		case kAUNBandEQParam_Bandwidth:
			return Bandwidth_To_AudioUnitParameterValue(mBiquadValues[which].bw);
		}
	}
	return super::GetParameter(paramID);
}


OSStatus
AUSidetone::GetPropertyInfo(AudioUnitPropertyID inID, uint32_t& outDataSize, bool& outWritable)
{
	OSStatus result = -1;
	switch (inID)
	{
		case kAUSongbird_SidetoneState:
			result = GetPropertyInfo(kAUSongbird_SidetoneEQBlockData, outDataSize, outWritable);
			if (noErr == result)
			{
				// leave room for the mVolume field of the AU_SidetoneState;
				outDataSize += offsetof(AU_SidetoneState, mFilterDescription);
			}
			break;
		case kAUSongbird_SidetoneEQBlockData:
			outDataSize = sizeof(AU_BiquadFilterDescription) - sizeof(AU_BiquadFilter) + mNumberBands * sizeof(AU_BiquadFilter);
			outWritable = true;
			result = noErr;
			break;
		default:
			result = super::GetPropertyInfo(inID, outDataSize, outWritable);
	}
	return result;
}


OSStatus
AUSidetone::GetProperty(AudioUnitPropertyID inID, void* outData)
{
	OSStatus result = -1;
	switch (inID)
	{
		case kAUSongbird_SidetoneState:
		{
			AU_SidetoneState * state = static_cast<AU_SidetoneState*>(outData);
			state->mVolume = mVolumeInDB;
			state->mBypass = mSidetoneEQBypass;
			void * biquad_filter_description = &state->mFilterDescription;
			result = GetProperty(kAUSongbird_SidetoneEQBlockData, biquad_filter_description);
		}
			break;
		case kAUSongbird_SidetoneEQBlockData:
		{
			AU_BiquadFilterDescription * description = static_cast<AU_BiquadFilterDescription*>(outData);
			if (description)
			{
				description->mBypass = mSidetoneEQBypass;
				description->mNumberFilters = mNumberBands;
				BiquadCoefficientsDescriptor * filter = static_cast<BiquadCoefficientsDescriptor*>(static_cast<void*>(description->mFilters));
				if (filter)
				{
					result = GetAE2IIRFilterCoefficients(mSidetoneEQ, mNumberBands, filter);
				}
			}
		}
			break;
		default:
			result = super::GetProperty(inID, outData);
	}
	return result;
}


OSStatus
AUSidetone::SetProperty(AudioUnitPropertyID inID, const void* inData, uint32_t inDataSize)
{
	OSStatus result = -1;
	switch (inID)
	{
		case kAUSongbird_SidetoneState:
		{
			// determine if have a valid data structure
			if (inDataSize < (offsetof(AU_SidetoneState, mFilterDescription)))
			{
				break;
			}
			const AU_SidetoneState * state = static_cast<const AU_SidetoneState*>(inData);
			const void * biquad_filter_description = &state->mFilterDescription;
			result = SetProperty(kAUSongbird_SidetoneEQBlockData, biquad_filter_description, inDataSize - offsetof(AU_SidetoneState, mFilterDescription));
			if (noErr == result)
			{
				mSidetoneEQBypass = state->mBypass;
				mVolumeInDB = state->mVolume;
				result = SetGain(mVolume, mVolumeInDB);
			}
		}
			break;
		case kAUSongbird_SidetoneEQBlockData:
		{
			// determine if we are talking to a valid structure
			if (inDataSize < sizeof(AU_BiquadFilterDescription) - sizeof(AU_BiquadFilter))
			{
				break;
			}
			const AU_BiquadFilterDescription * description = static_cast<const AU_BiquadFilterDescription*>(inData);
			if (inDataSize < ((sizeof(AU_BiquadFilterDescription) - sizeof(AU_BiquadFilter) + (sizeof(AU_BiquadFilter) * description->mNumberFilters))))
			{
				break;
			}
			if (description->mNumberFilters > kMaxNumBands)
			{
				break;
			}
			const BiquadCoefficientsDescriptor * filter = static_cast<const BiquadCoefficientsDescriptor*>(static_cast<const void*>(description->mFilters));
			result = SetAE2IIRFilterCoefficients(mSidetoneEQ, description->mNumberFilters, static_cast<const BiquadCoefficientsDescriptor*>(filter));
			if (!result)
			{
				mSidetoneEQBypass = description->mBypass;
				mNumberBands = description->mNumberFilters;
			}
		}
			break;
		default:
			result = super::SetProperty(inID, inData, inDataSize);
	}
	return result;
}


void
AUSidetone::Process(const void *inSourceP,
				void *inDestP,
				UInt32 inFramesToProcess)
{
	if (!mSidetoneEQBypass)
	{
		ProcessAE2IIRFilter(mSidetoneEQ, inFramesToProcess, inSourceP, inDestP);
	}
	ProcessVolumeInplace(mVolume, inFramesToProcess, inDestP);
}


#endif

