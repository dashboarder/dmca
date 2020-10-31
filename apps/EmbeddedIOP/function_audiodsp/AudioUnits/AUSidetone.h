/*
 *  AUSidetone.h
 *
 *  Created by richardp on 11/28/11.
 *  Copyright 2011 Apple, Inc. All rights reserved.
 *
 *  Does necessary translation between AE2 and CA AU speaker protection
 */

#ifndef __AUSidetone_h__
#define __AUSidetone_h__

#include <stdint.h>

#include "AudioUnitProperties_AE2.h"
#include "AUBase_AE2.h"

struct AE2_IIRFilter;
struct AE2_Volume;
struct BiquadDescriptor;

/**
 * AUSidetone implements the sidetone eq and gain.
 *
 * These values are hard-coded for now.
 */
class AUSidetone : public AUBase_AE2
{
public:
	static AUSidetone* Create_AUSidetone(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize);
	virtual ~AUSidetone();

	virtual void Process(const void *inSourceP,
				void *inDestP,
				UInt32 inFramesToProcess);

        // for parameters
	virtual OSStatus SetParameter(AudioUnitParameterID paramID, AudioUnitParameterValue value);
	virtual AudioUnitParameterValue GetParameter(AudioUnitParameterID paramID);
	// for properties
	virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID, uint32_t& outDataSize, bool& outWritable);
	virtual OSStatus GetProperty(AudioUnitPropertyID inID, void* outData);
	virtual OSStatus SetProperty(AudioUnitPropertyID inID, const void* inData, uint32_t inDataSize);

private:
	AUSidetone();
	bool InitWith(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize);

	// some constants
	typedef AUBase_AE2 super;
	static const size_t kMaxNumBands = 5;
	static const float kDefaultSidetoneGain = -32.0;

	// Parameters for the AUNBandEQ unit
	// Note that the parameter IDs listed correspond to band 0 (zero) of the unit. The parameter IDs for
	// higher bands can be obtained by adding the zero-indexed band number to the corresponding band 0
	// parameter ID up to the number of bands minus one, where the number of bands is described by the
	// AUNBandEQ property kAUNBandEQProperty_NumberOfBands. For example, the parameter ID corresponding
	// to the filter type of band 4 would be kAUNBandEQParam_FilterType + 3.
	enum {
		kAUVolume_InDB										= 500,
			// Global, Boolean, 0 or 1, 1
		kAUNBandEQParam_BypassBand								= 1000,
			// Global, Indexed, 0->kNumAUNBandEQFilterTypes-1, 0
		kAUNBandEQParam_FilterType								= 2000,
			// Global, Hz, 20->(SampleRate/2), 1000
		kAUNBandEQParam_Frequency								= 3000,
			// Global, dB, -96->24, 0
		kAUNBandEQParam_Gain									= 4000,
			// Global, octaves, 0.05->5.0, 0.5
		kAUNBandEQParam_Bandwidth								= 5000
	};

	uint32_t mSampleRate;
	uint32_t mNumChannels;
	AE2_IIRFilter *mSidetoneEQ;
	AE2_Volume *mVolume;
        // to hold the coefficients
        uint32_t mNumberBands;

	BiquadDescriptor *mBiquadValues;

	float mVolumeInDB;
	bool mSidetoneEQBypass;
};

#endif // __AUSidetone_h__

