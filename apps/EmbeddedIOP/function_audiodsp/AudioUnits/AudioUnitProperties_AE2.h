/*
 File:       AudioUnitProperties_AE2.h
 
 Contains:   Property constants for AudioUnits, stripped down for AE2
 
 Copyright:  (c) 2010 by Apple, Inc., all rights reserved.
 
 */

#ifndef __AUDIOUNITPROPERTIES_AE2__
#define __AUDIOUNITPROPERTIES_AE2__

#include "CoreAudioTypes_AE2.h"
#include "AUComponent_AE2.h"

#pragma mark -
#pragma mark Core Implementation
#pragma mark -
/*!
 @enum           Audio Unit scope types
 @abstract       The scope IDs for audio units define basic roles and contexts for an audio unit's state.
 @discussion		Each scope is a discrete context. Apple reserves scope IDs from 0 through 1024.
 
 @constant		kAudioUnitScope_Global	The context for audio unit characteristics that apply to the audio unit as a 
 whole
 @constant		kAudioUnitScope_Input	The context for audio data coming into an audio unit
 @constant		kAudioUnitScope_Output	The context for audio data leaving an audio unit
 @constant		kAudioUnitScope_Group	A context specific to the control scope of parameters (for instance, 
 MIDI Channels is an example of this scope)
 @constant		kAudioUnitScope_Part	A distinct rendering context. For instance a single timbre in a multi-timbral 
 instrument, a single loop in a multi looping capable looper unit, etc.
 @constant		kAudioUnitScope_Layer	A context which functions as a layer within a part and allows
 grouped control of Cell-scope parameters.
 An example is the percussive attack layer for an electric organ instrument
 @constant		kAudioUnitScope_Note	A scope that can be used to apply changes to an individual note. The 
 elementID used with this scope is the unique note ID returned from
 a started note (see MusicDeviceStartNote)
 @constant		kAudioUnitScope_Cell	A scope which represents the finest level of granularity within an AU.
 The individual sample zones and envelope generators within a synth are an
 example of this.
 */
enum {
	kAudioUnitScope_Global	= 0,
	kAudioUnitScope_Input	= 1,
	kAudioUnitScope_Output	= 2
};

//=====================================================================================================================
#pragma mark - Parameter Definitions

// assume kAudioUnitParameterUnit_Generic if not found in this enum
/*!
 @enum			AudioUnitParameterUnit
 @constant		kAudioUnitParameterUnit_Generic
 untyped value generally between 0.0 and 1.0
 @constant		kAudioUnitParameterUnit_Indexed
 takes an integer value (good for menu selections)
 @constant		kAudioUnitParameterUnit_Boolean
 0.0 means FALSE, non-zero means TRUE
 @constant		kAudioUnitParameterUnit_Percent
 usually from 0 -> 100, sometimes -50 -> +50
 @constant		kAudioUnitParameterUnit_Seconds
 absolute or relative time
 @constant		kAudioUnitParameterUnit_SampleFrames
 one sample frame equals (1.0/sampleRate) seconds
 @constant		kAudioUnitParameterUnit_Phase
 -180 to 180 degrees
 @constant		kAudioUnitParameterUnit_Rate
 rate multiplier, for playback speed, etc. (e.g. 2.0 == twice as fast)
 @constant		kAudioUnitParameterUnit_Hertz
 absolute frequency/pitch in cycles/second
 @constant		kAudioUnitParameterUnit_Cents
 unit of relative pitch
 @constant		kAudioUnitParameterUnit_RelativeSemiTones
 useful for coarse detuning
 @constant		kAudioUnitParameterUnit_MIDINoteNumber
 absolute pitch as defined in the MIDI spec (exact freq may depend on tuning table)
 @constant		kAudioUnitParameterUnit_MIDIController
 a generic MIDI controller value from 0 -> 127
 @constant		kAudioUnitParameterUnit_Decibels
 logarithmic relative gain
 @constant		kAudioUnitParameterUnit_LinearGain
 linear relative gain
 @constant		kAudioUnitParameterUnit_Degrees
 -180 to 180 degrees, similar to phase but more general (good for 3D coord system)
 @constant		kAudioUnitParameterUnit_EqualPowerCrossfade
 0 -> 100, crossfade mix two sources according to sqrt(x) and sqrt(1.0 - x)
 @constant		kAudioUnitParameterUnit_MixerFaderCurve1
 0.0 -> 1.0, pow(x, 3.0) -> linear gain to simulate a reasonable mixer channel fader response
 @constant		kAudioUnitParameterUnit_Pan
 standard left to right mixer pan
 @constant		kAudioUnitParameterUnit_Meters
 distance measured in meters
 @constant		kAudioUnitParameterUnit_AbsoluteCents
 absolute frequency measurement : 
 if f is freq in hertz then absoluteCents = 1200 * log2(f / 440) + 6900
 @constant		kAudioUnitParameterUnit_Octaves
 octaves in relative pitch where a value of 1 is equal to 1200 cents
 @constant		kAudioUnitParameterUnit_BPM
 beats per minute, ie tempo
 @constant		kAudioUnitParameterUnit_Beats
 time relative to tempo, i.e., 1.0 at 120 BPM would equal 1/2 a second
 @constant		kAudioUnitParameterUnit_Milliseconds
 parameter is expressed in milliseconds
 @constant		kAudioUnitParameterUnit_Ratio
 for compression, expansion ratio, etc.
 @constant		kAudioUnitParameterUnit_CustomUnit
 this is the parameter unit type for parameters that present a custom unit name
 */
enum
{
	kAudioUnitParameterUnit_Generic				= 0,
	kAudioUnitParameterUnit_Indexed				= 1,
	kAudioUnitParameterUnit_Boolean				= 2,
	kAudioUnitParameterUnit_Percent				= 3,
	kAudioUnitParameterUnit_Seconds				= 4,
	kAudioUnitParameterUnit_SampleFrames		= 5,
	kAudioUnitParameterUnit_Phase				= 6,
	kAudioUnitParameterUnit_Rate				= 7,
	kAudioUnitParameterUnit_Hertz				= 8,
	kAudioUnitParameterUnit_Cents				= 9,
	kAudioUnitParameterUnit_RelativeSemiTones	= 10,
	kAudioUnitParameterUnit_MIDINoteNumber		= 11,
	kAudioUnitParameterUnit_MIDIController		= 12,
	kAudioUnitParameterUnit_Decibels			= 13,
	kAudioUnitParameterUnit_LinearGain			= 14,
	kAudioUnitParameterUnit_Degrees				= 15,
	kAudioUnitParameterUnit_EqualPowerCrossfade = 16,
	kAudioUnitParameterUnit_MixerFaderCurve1	= 17,
	kAudioUnitParameterUnit_Pan					= 18,
	kAudioUnitParameterUnit_Meters				= 19,
	kAudioUnitParameterUnit_AbsoluteCents		= 20,
	kAudioUnitParameterUnit_Octaves				= 21,
	kAudioUnitParameterUnit_BPM					= 22,
    kAudioUnitParameterUnit_Beats               = 23,
	kAudioUnitParameterUnit_Milliseconds		= 24,
	kAudioUnitParameterUnit_Ratio				= 25,
	kAudioUnitParameterUnit_CustomUnit			= 26
};
/*!
 @typedef		AudioUnitParameterUnit
 */
typedef UInt32		AudioUnitParameterUnit;

/*!
 @struct			AudioUnitParameterInfo
 @field			name
 UNUSED - set to zero - UTF8 encoded C string (originally). 
 @field			unitName
 only valid if kAudioUnitParameterUnit_CustomUnit is set. If kAudioUnitParameterUnit_CustomUnit
 is set, this field must contain a valid CFString.
 @field			clumpID
 only valid if kAudioUnitParameterFlag_HasClump
 @field			cfNameString
 only valid if kAudioUnitParameterFlag_HasCFNameString
 @field			unit				
 if the "unit" field contains a value not in the enum above, then assume 
 kAudioUnitParameterUnit_Generic
 @field			minValue
 @field			maxValue
 @field			defaultValue
 @field			flags
 Due to some vagaries about the ways in which Parameter's CFNames have been described, it was
 necessary to add a flag: kAudioUnitParameterFlag_CFNameRelease
 In normal usage a parameter name is essentially a static object, but sometimes an audio unit will 
 generate parameter names dynamically.. As these are expected to be CFStrings, in that case
 the host should release those names when it is finished with them, but there was no way
 to communicate this distinction in behavior.
 Thus, if an audio unit will (or could) generate a name dynamically, it should set this flag in 
 the parameter's info. The host should check for this flag, and if present, release the parameter
 name when it is finished with it.
 */
typedef struct AudioUnitParameterInfo
{
	//char						name[52];
	//CFStringRef					unitName;
	UInt32						clumpID;
	//CFStringRef					cfNameString;
	AudioUnitParameterUnit		unit;						
	AudioUnitParameterValue		minValue;			
	AudioUnitParameterValue		maxValue;			
	AudioUnitParameterValue		defaultValue;		
	UInt32						flags;				
} AudioUnitParameterInfo;

/*!
 @enum			Audio Unit Parameter Flags
 @discussion		Bit positions 18, 17, and 16 are set aside for display scales. Bit 19 is reserved.
 @constant		kAudioUnitParameterFlag_CFNameRelease
 @constant		kAudioUnitParameterFlag_MeterReadOnly
 @constant		kAudioUnitParameterFlag_DisplayMask
 @constant		kAudioUnitParameterFlag_DisplaySquareRoot
 @constant		kAudioUnitParameterFlag_DisplaySquared
 @constant		kAudioUnitParameterFlag_DisplayCubed
 @constant		kAudioUnitParameterFlag_DisplayCubeRoot
 @constant		kAudioUnitParameterFlag_DisplayExponential
 @constant		kAudioUnitParameterFlag_HasClump
 @constant		kAudioUnitParameterFlag_ValuesHaveStrings
 @constant		kAudioUnitParameterFlag_DisplayLogarithmic		
 @constant		kAudioUnitParameterFlag_IsHighResolution
 @constant		kAudioUnitParameterFlag_NonRealTime
 @constant		kAudioUnitParameterFlag_CanRamp
 @constant		kAudioUnitParameterFlag_ExpertMode
 @constant		kAudioUnitParameterFlag_HasCFNameString
 @constant		kAudioUnitParameterFlag_IsGlobalMeta
 @constant		kAudioUnitParameterFlag_IsElementMeta
 @constant		kAudioUnitParameterFlag_IsReadable
 @constant		kAudioUnitParameterFlag_IsWritable
 */
enum
{
	kAudioUnitParameterFlag_CFNameRelease		= (1L << 4),
	kAudioUnitParameterFlag_MeterReadOnly		= (1L << 15),
	
	// bit positions 18,17,16 are set aside for display scales. bit 19 is reserved.
	kAudioUnitParameterFlag_DisplayMask			= (7L << 16) | (1L << 22),
	kAudioUnitParameterFlag_DisplaySquareRoot	= (1L << 16),
	kAudioUnitParameterFlag_DisplaySquared		= (2L << 16),
	kAudioUnitParameterFlag_DisplayCubed		= (3L << 16),
	kAudioUnitParameterFlag_DisplayCubeRoot		= (4L << 16),
	kAudioUnitParameterFlag_DisplayExponential	= (5L << 16),
	kAudioUnitParameterFlag_HasClump	 		= (1L << 20),
	kAudioUnitParameterFlag_ValuesHaveStrings	= (1L << 21),
	kAudioUnitParameterFlag_DisplayLogarithmic 	= (1L << 22),		
	kAudioUnitParameterFlag_IsHighResolution 	= (1L << 23),
	kAudioUnitParameterFlag_NonRealTime 		= (1L << 24),
	kAudioUnitParameterFlag_CanRamp 			= (1L << 25),
	kAudioUnitParameterFlag_ExpertMode 			= (1L << 26),
	kAudioUnitParameterFlag_HasCFNameString 	= (1L << 27),
	kAudioUnitParameterFlag_IsGlobalMeta 		= (1L << 28),
	kAudioUnitParameterFlag_IsElementMeta		= (1L << 29),
	kAudioUnitParameterFlag_IsReadable			= (1L << 30),
	kAudioUnitParameterFlag_IsWritable			= (1L << 31)
};

typedef struct AUChannelInfo {
	SInt16		inChannels;
	SInt16		outChannels;
} AUChannelInfo;

//==================================================================================================
 /*
 	@constant		kAUSongbird_SidetoneEQBlockData
 	@discussion		Scope: Global
					Value Type: blob
					Access: read/write
					
					A property to get/set the biquad coefficients of the AE2 SidetoneEQ block as an
					array of AU_BiquadFilterDescription, sized to be the number of stages.
*/

enum {
	kAUSongbird_SidetoneEQBlockData		= 10000,
	kAUSongbird_SidetoneState		= 10001
};


/*!
 @struct		AU_BiquadFilter
 @field			mBypass
 Boolean to indicate if biquad is bypassed;
 @field			b0
 @field			b1
 @field			b2
 @field			a1
 @field			a2
 Fields for biquad coefficients values;
 */

typedef struct AU_BiquadFilter {
	UInt32		mBypass;
	Float64		b0, b1, b2, a1, a2;
} AU_BiquadFilter;

/*!
 @struct		AU_BiquadFilterDescription
 kAUSongbird_SidetoneEQBlockData data structure
 @field			mBypass
 Boolean to indicate if all biquads are bypassed;
 @field			mNumberFilters
 Number of active filters in the sidetone eq;
 @field			mFilters
 variable length sized as mNumberFilters AU_BiquadFilter structures;
 */

typedef struct AU_BiquadFilterDescription {
	UInt32		mBypass;
	UInt32		mNumberFilters;
	AU_BiquadFilter	mFilters[1]; // this is a variable length array of mNumberFilters elements
} AU_BiquadFilterDescription;

/*!
 @struct		AU_SidetoneState
 kAUSongbird_SidetoneState data structure
 @field			mVolume
 Gain for Sidetone;
 @field			mFilterDescription
 Filter description for sidetone;
 */

typedef struct AU_SidetoneState {
	bool		mBypass;
	Float32		mVolume;
	AU_BiquadFilterDescription	mFilterDescription;
} AU_SidetoneState;



#endif
