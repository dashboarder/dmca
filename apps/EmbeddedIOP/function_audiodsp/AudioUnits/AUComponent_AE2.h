/*
 *  AUComponent_AE2.h
 *  Stripped down version of AUComponent just for what AE2 needs
 *
 *  Copyright 2010 Apple Computer. All rights reserved.
 *
 */

#ifndef __AUCOMPONENT_AE2__
#define __AUCOMPONENT_AE2__

/*!
 @enum			Audio unit errors
 @discussion		These are the various errors that can be returned by AudioUnit... API calls
 
 @constant		kAudioUnitErr_InvalidProperty
 The property is not supported
 @constant		kAudioUnitErr_InvalidParameter
 The parameter is not supported
 @constant		kAudioUnitErr_InvalidElement
 The specified element is not valid
 @constant		kAudioUnitErr_NoConnection
 There is no connection (generally an audio unit is asked to render but it has 
 not input from which to gather data)
 @constant		kAudioUnitErr_FailedInitialization
 The audio unit is unable to be initialised
 @constant		kAudioUnitErr_TooManyFramesToProcess
 When an audio unit is initialised it has a value which specifies the max 
 number of frames it will be asked to render at any given time. If an audio 
 unit is asked to render more than this, this error is returned.
 @constant		kAudioUnitErr_InvalidFile
 If an audio unit uses external files as a data source, this error is returned 
 if a file is invalid (Apple's DLS synth returns this error)
 @constant		kAudioUnitErr_FormatNotSupported
 Returned if an input or output format is not supported
 @constant		kAudioUnitErr_Uninitialized
 Returned if an operation requires an audio unit to be initialised and it is 
 not.
 @constant		kAudioUnitErr_InvalidScope
 The specified scope is invalid
 @constant		kAudioUnitErr_PropertyNotWritable
 The property cannot be written
 @constant		kAudioUnitErr_CannotDoInCurrentContext
 Returned when an audio unit is in a state where it can't perform the requested 
 action now - but it could later. Its usually used to guard a render operation 
 when a reconfiguration of its internal state is being performed.
 @constant		kAudioUnitErr_InvalidPropertyValue
 The property is valid, but the value of the property being provided is not
 @constant		kAudioUnitErr_PropertyNotInUse
 Returned when a property is valid, but it hasn't been set to a valid value at 
 this time.	
 @constant		kAudioUnitErr_Initialized
 Indicates the operation cannot be performed because the audio unit is 
 initialized.
 @constant		kAudioUnitErr_InvalidOfflineRender
 Used to indicate that the offline render operation is invalid. For instance,
 when the audio unit needs to be pre-flighted, 
 but it hasn't been.
 @constant		kAudioUnitErr_Unauthorized
 Returned by either Open or Initialise, this error is used to indicate that the 
 audio unit is not authorised, that it cannot be used. A host can then present 
 a UI to notify the user the audio unit is not able to be used in its current 
 state.
 */
enum
{
	kAudioUnitErr_InvalidProperty			= -10879,
	kAudioUnitErr_InvalidParameter			= -10878,
	kAudioUnitErr_InvalidElement			= -10877,
	kAudioUnitErr_NoConnection				= -10876,
	kAudioUnitErr_FailedInitialization		= -10875,
	kAudioUnitErr_TooManyFramesToProcess	= -10874,
	kAudioUnitErr_InvalidFile				= -10871,
	kAudioUnitErr_FormatNotSupported		= -10868,
	kAudioUnitErr_Uninitialized				= -10867,
	kAudioUnitErr_InvalidScope				= -10866,
	kAudioUnitErr_PropertyNotWritable		= -10865,
	kAudioUnitErr_CannotDoInCurrentContext	= -10863,
	kAudioUnitErr_InvalidPropertyValue		= -10851,
	kAudioUnitErr_PropertyNotInUse			= -10850,
	kAudioUnitErr_Initialized				= -10849,
	kAudioUnitErr_InvalidOfflineRender		= -10848,
	kAudioUnitErr_Unauthorized				= -10847
};

/*!
 @typedef			AudioUnitPropertyID
 @discussion			Type used for audio unit properties. 
 Properties are used to describe the state of an audio unit (for instance, 
 the input or output audio format)
 */
typedef UInt32							AudioUnitPropertyID;
/*!
 @typedef			AudioUnitScope
 @discussion			Type used for audio unit scopes. Apple reserves the 0 < 1024 range for 
 audio unit scope identifiers.  
 Scopes are used to delineate a major attribute of an audio unit 
 (for instance, global, input, output)
 */
typedef UInt32							AudioUnitScope;
/*!
 @typedef			AudioUnitElement
 @discussion			Type used for audio unit elements.
 Scopes can have one or more member, and a member of a scope is 
 addressed / described by its element
 For instance, input bus 1 is input scope, element 1
 */
typedef UInt32							AudioUnitElement;
/*!
 @typedef			AudioUnitParameterID
 @discussion			Type used for audio unit parameters. 
 Parameters are typically used to control and set render state 
 (for instance, filter cut-off frequency)
 */
typedef UInt32							AudioUnitParameterID;
/*!
 @typedef			AudioUnitParameterValue
 @discussion			Type used for audio unit parameter values. 
 The value of a given parameter is specified using this type 
 (typically a Float32)
 */
typedef	Float32							AudioUnitParameterValue;


//-----------------------------------------------------------------------------
//	Render flags
//-----------------------------------------------------------------------------

enum
{
	/* these are obsolete, were never implemented: */
	/*	kAudioUnitRenderAction_Accumulate			= (1 << 0) */
	/*	kAudioUnitRenderAction_UseProvidedBuffer	= (1 << 1) */
	kAudioUnitRenderAction_PreRender		= (1 << 2),
	kAudioUnitRenderAction_PostRender		= (1 << 3),
	kAudioUnitRenderAction_OutputIsSilence	= (1 << 4),
	/* Provides hint on return from Render(). if set, the buffer contains all zeroes */
	kAudioOfflineUnitRenderAction_Preflight	= (1 << 5),
	kAudioOfflineUnitRenderAction_Render	= (1 << 6),
	kAudioOfflineUnitRenderAction_Complete	= (1 << 7),
	// this flag is set if on the post-render call an error was returned by the AUs render
	// in this case, the error can be retrieved through the lastRenderError property
	// and the ioData handed to the post-render notification will be invalid.
	kAudioUnitRenderAction_PostRenderError	= (1 << 8)
};

typedef UInt32							AudioUnitRenderActionFlags;

#endif