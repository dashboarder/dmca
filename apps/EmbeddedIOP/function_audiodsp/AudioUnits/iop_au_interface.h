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

#ifndef _IOP_AU_INTERFACE_H_
#define _IOP_AU_INTERFACE_H_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>

typedef void * audio_unit_t;

audio_unit_t Create_AUSidetone(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize);
audio_unit_t Create_AUNull(uint32_t sampleRate, uint32_t numChannels, uint32_t sampleSize);

void Destroy_AudioUnit(audio_unit_t au);

// parameters are single values that can be adjusted
int32_t AudioUnit_GetParameter(audio_unit_t au, uint32_t parameter, float *parameterValue);
int32_t AudioUnit_SetParameter(audio_unit_t au, uint32_t parameter, float parameterValue);

// properties are blocks of data that can be adjusted, usually a private setting between user and unit
int32_t AudioUnit_GetPropertyInfo(audio_unit_t au, uint32_t propID, uint32_t *outSize, bool *outWritable);
int32_t AudioUnit_GetProperty(audio_unit_t au, uint32_t propID, void *outData, uint32_t *ioSize);
int32_t AudioUnit_SetProperty(audio_unit_t au, uint32_t propID, const void *inData, uint32_t inSize);

void    AudioUnit_Process(audio_unit_t au, const void *inSourceP, void *inDestP, uint32_t inFramesToProcess);


#ifdef __cplusplus
}
#endif
	
#endif // _IOP_AU_INTERFACE_H_
