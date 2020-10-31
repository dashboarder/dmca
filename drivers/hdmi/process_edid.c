/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

/*
 * This file is based on a manual C++-to-C translation of:
 * IODisplayPortFamily/IODPService.cpp
 * The scoring system and timing collection is simplified.
 */

#include <debug.h>

#include <drivers/display.h>
#include <drivers/hdmi.h>
#include <drivers/process_edid.h>
#include <platform/memmap.h>
#include <platform/timer.h>
#include <sys/task.h>
#include "edid.h"

// I2C address for the EDID rom.
#define kI2CEdidReadDeviceAddr	0xA1
#define kI2CEdidWriteDeviceAddr	0xA0

// H3 DisplayPipe has issues higher than this.
#define kMaxHorizontalActive	1280

// Some monitors are slow. Insert microseconds of dumb.
#define kEdidRetryTimeout	5000000
// Some monitors are suspiciously single-threaded. Don't retry too
// often or they make no forward progress.
#define kEdidRetryDelay		250000

/////////////////////////////////////////
////////// debug support

#define HDMIDEBUG_MASK (		\
		HDMIDEBUG_EDID |	        \
		HDMIDEBUG_ERROR |	\
		HDMIDEBUG_INFO |         \
		0)

#undef HDMIDEBUG_MASK
#define HDMIDEBUG_MASK HDMIDEBUG_ERROR // (HDMIDEBUG_EDID | HDMIDEBUG_SCORE | HDMIDEBUG_DS | HDMIDEBUG_ERROR | HDMIDEBUG_INFO)

#define HDMIDEBUG_EDID           (1<<16)  // EDID parsing
#define HDMIDEBUG_SCORE          (1<<17)  // Final validation/scoring
#define HDMIDEBUG_DS             (1<<18)  // Downstream type detection
#define HDMIDEBUG_INFO           (1<<19)  // info
#define HDMIDEBUG_ERROR          (1<<20)  // error
#define HDMIDEBUG_ALWAYS         (1<<31)  // unconditional output

#define debug(_fac, _fmt, _args...)								\
	do {											\
		if ((HDMIDEBUG_ ## _fac) & (HDMIDEBUG_MASK | HDMIDEBUG_ALWAYS))			\
			dprintf(DEBUG_INFO, "EDID: %s, %d: " _fmt, __FUNCTION__, __LINE__, ##_args);	\
	} while(0)

typedef struct {
	// Slightly compacted vs IODPDisplayTimingElement.cpp
	uint16_t    horizontal;
	uint16_t    vertical;
	uint32_t    rate;
} __IODPTEEstablished;

typedef enum {
        kSupportFlagNone    = (0<<0),
        kSupportFlagGTF     = (1<<0),
        kSupportFlagCVT     = (1<<1)
} SupportFlags;

static int process_edid(EDID *edid);
static int verify_edid(EDID *edid, uint8_t *p_checksum);
static int get_edid(uint8_t offset, EDID *edid);
static int process_edid_vtb(EDIDVTB *vtb);
static int process_edid_cvt_timings(uint8_t *data, uint32_t length);
static int process_edid_cea(EDIDCEA *cea);
static int process_edid_cea1(EDIDCEA_1 *cea1);
static int process_edid_cea2(EDIDCEA_2 *cea2);
static int process_edid_cea3(EDIDCEA_3 *cea3);
static int process_edid_cea3_video(EDIDCEA3DataBlockVideo *video, uint32_t length);
static int process_edid_cea3_vendor_specific(EDIDCEA3DataBlockVendorSpecific *vendor, uint32_t length);
static int process_edid_standard(EDIDStandard *standard);
static int process_edid_standard_feature(EDIDStandard *standard);
static int process_edid_standard_timings(uint8_t *data, uint32_t length);
static int process_edid_standard_established_timings(EDIDStandard *standard);
static int process_edid_detailed(EDIDDetailed *detailed);
static int process_edid_detailed_descriptor(EDIDDetailedDescriptor *descriptor);
static int process_edid_detailed_established_timings_iii(EDIDDetailedEstablishedTimingsIII *established);
static int try_standard_timing_id(uint16_t timing_id);
static int try_established_timing_id(uint8_t index);
static int try_established_timing_iii_id(uint8_t establishedID);
static int try_detailed_timing(EDIDDetailedTiming *detailed);
static int try_cvt_timing_id(EDIDCVT *cvt);
static int try_cea_short_id(uint32_t shortVideoID);
static int get_timing_data_for_cea_short_id(uint32_t shortVideoID, struct video_timing_data *timingData);
static int use_dimensions(uint32_t horizontal, uint32_t vertical, uint32_t rate);
static int use_dimensions_dmt(uint32_t horizontal, uint32_t vertical, uint32_t rate);
static int use_dimensions_cvt(uint32_t horizontal, uint32_t vertical, uint32_t rate);
static int use_dimensions_gtf(uint32_t horizontal, uint32_t vertical, uint32_t rate);
static int use_dimensions_table(const struct video_timing_data * list, uint32_t count, uint32_t horizontal, uint32_t vertical, uint32_t rate);
static int use_timings(struct video_timing_data *data);
static uint16_t read_le16(const void *data, uint32_t offset);
static uint32_t read_le32(const void *data, uint32_t offset);

static bool s_have_timings;
static bool s_abort_edid;
static struct video_timing_data s_best_timings;
static SupportFlags s_timing_support;
static uint32_t s_restrict_h_active;
static uint32_t s_restrict_v_active;
static int s_ds_type = -1;


int obtain_edid(void)
{
	int ret = -1;
	EDID edid;
	uint8_t ext_block_count;
	uint8_t index = 0;

	s_have_timings = false;
	bzero(&s_best_timings, sizeof(s_best_timings));
	s_timing_support = kSupportFlagNone;
	s_ds_type = kHDMI_tx_mode_DVI;

	ret = get_edid(0, &edid);
	debug(EDID, "Obtaining base EDID result=%d\n", ret);
	if (ret) goto exit;
	
	ret = process_edid(&edid);
	debug(EDID, "Processing EDID result=%d\n", ret);
	if (ret) goto exit;
	
	ext_block_count = edid.data.standard.extensionFlag;
	debug(EDID, "EDID contains %d extensions\n", ext_block_count);

	for (index = 1; index <= ext_block_count; ++index) {
		ret = get_edid(index * sizeof(EDID), &edid);
		debug(EDID, "Obtaining EDID extension %d result=%d\n", index, ret);
		if (ret) goto exit;

		ret = process_edid(&edid);
		debug(EDID, "Processing EDID extension %d result=%d\n", index, ret);
		if (ret) goto exit;
	}
	debug(DS, "tx mode %d\n", s_ds_type);

 exit:
	return ret;
}

void abort_edid(void)
{
	s_abort_edid = true;
}

void restrict_edid(uint32_t h_active, uint32_t v_active)
{
	s_restrict_h_active = h_active;
	s_restrict_v_active = v_active;
}

int get_edid_timings(struct video_timing_data *data)
{
	if (!s_have_timings) return -1;
	bcopy(&s_best_timings, data, sizeof(s_best_timings));
	debug(ALWAYS, "EDID: %ux%u%c/%u\n",
	      s_best_timings.axis[kDisplayAxisTypeHorizontal].active,
	      s_best_timings.axis[kDisplayAxisTypeVertical].active,
	      s_best_timings.interlaced ? 'i' : 'p',
	      s_best_timings.axis[kDisplayAxisTypeVertical].sync_rate >> 16);
	return 0;
}

int get_edid_downstream_type(void)
{
	return s_ds_type;
}

static int process_edid(EDID *edid)
{
	static const uint8_t edid_standard_header[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	
	debug(EDID, "Processing EDID\n");
	
	if (!memcmp(edid->data.header.header, edid_standard_header, sizeof(edid_standard_header))) {
		process_edid_standard(&edid->data.standard);
	} else {
		switch (edid->data.header.header[0]) {
		case kEDIDTypeCEA:
			process_edid_cea(&edid->data.cea);
			break;
		case KEDIDTypeVTB:
			process_edid_vtb(&edid->data.vtb);
			break;
		default:
			debug(EDID, "Unknown EDID type %d\n", edid->data.header.header[0]);
			break;
		}
	}
	return 0;
}

static int process_edid_vtb(EDIDVTB *vtb)
{
	uint32_t index, length;
	uint32_t dtbCount, cvtCount, stCount;

	debug(EDID, "Processing EDID VTB\n");
	
	if (1 != vtb->header.revision) goto exit;
    
	if (!(vtb->dtbCount && vtb->cvtCount && vtb->stCount)) goto exit;
    
	length      = sizeof(vtb->data);
	index       = 0;
	dtbCount    = vtb->dtbCount;
	cvtCount    = vtb->cvtCount;
	stCount     = vtb->stCount;
	while ( index<length ) {
		uint32_t remaining = length-index;
        
		if ( dtbCount ) {
			if (sizeof(EDIDDetailed)>remaining)
				break;
            
			process_edid_detailed((EDIDDetailed*)&vtb->data[index]);
			
			index += sizeof(EDIDDetailed);
			dtbCount--;
		}
		else if ( cvtCount ) {
			uint32_t cvtLength = cvtCount * sizeof(EDIDCVT);
			if (cvtLength>remaining)
				break;
			
			process_edid_cvt_timings(&vtb->data[index], cvtLength);
			
			index += cvtLength;
			cvtCount = 0;
		}
		else if ( stCount ) {
			uint32_t stLength = stCount * sizeof(uint16_t);
			if (stLength>remaining)
				break;
			
			process_edid_standard_timings(&vtb->data[index], stLength);
			
			index += stLength;
			stCount = 0;
		}
		else {
			break;
		}
	}
 exit:
	return 0;
}

static int process_edid_cvt_timings(uint8_t *data, uint32_t length)
{
	uint32_t index;
	debug(EDID, "Processing EDID CVT timings\n");
	for( index = 0; index < length; index+=sizeof(EDIDCVT) ) {		
		try_cvt_timing_id((EDIDCVT*)&data[index]);
	}
	return 0;
}

static int process_edid_cea(EDIDCEA *cea)
{
	debug(EDID, "Processing CEA EDID\n");

	switch (cea->data.header.revision) {
        case 1:
		process_edid_cea1(&cea->data.cea1);
		break;
        case 2:
		process_edid_cea2(&cea->data.cea2);
		break;
        case 3:
		process_edid_cea3(&cea->data.cea3);
		break;
        default:
		break;
	}
	
	return 0;
}

static int process_edid_cea1(EDIDCEA_1 * cea1)
{
	uint32_t relativeDataOffset;
	uint32_t index = 0;
	
	debug(EDID, "Processing CEA v1 EDID\n");
	if (cea1->header.dataOffset<=sizeof(cea1->header)) return -1;
	
	relativeDataOffset = cea1->header.dataOffset-sizeof(cea1->header);
	while ( relativeDataOffset < sizeof(cea1->data) && (sizeof(cea1->data)-relativeDataOffset) >= sizeof(EDIDDetailed)) {
		debug(EDID, "Processing Detailed Timing index %d\n", index++);
		process_edid_detailed((EDIDDetailed*)(cea1->data + relativeDataOffset));
		relativeDataOffset += sizeof(EDIDDetailed);
	}
	
	return 0;
}

static int process_edid_cea2(EDIDCEA_2 * cea2)
{
	debug(EDID, "Processing CEA v2 EDID\n");
	// Don't care about audio/color/overscan. Just pass it along.
	return process_edid_cea1((EDIDCEA_1*)cea2);
}

static int process_edid_cea3(EDIDCEA_3 * cea3)
{
	uint32_t startOffset    = 0;
	uint32_t length         = 0;
	uint32_t index          = 0;
	
	debug(EDID, "Processing CEA v3 EDID\n");
    
	if (cea3->header.dataLength <= sizeof(cea3->header) || cea3->header.dataLength>sizeof(EDIDCEA_3)) return -1;
    
	length = cea3->header.dataLength-sizeof(cea3->header);
	while ( startOffset < length ) {
		EDIDCEA3DataBlock *     block = (EDIDCEA3DataBlock*)(cea3->data + startOffset);
		EDIDCEA3DataBlockType   blockType;
		uint32_t                blockLength;
		uint32_t		sectionLength;
		
		debug(EDID, "Processing CEA v3 EDID Data Block index %d\n", index++);
		
		// Check for padding
		if ( !block->flags ) {
			debug(EDID, "Encountered zero padding.\n");
			break;
		}
		
		blockType   = (EDIDCEA3DataBlockType)((block->flags>>5) & 0x7);
		blockLength = (block->flags) & 0x1f;

		sectionLength = blockLength+sizeof(block->flags);
                if ( sectionLength>(length-startOffset) ) return -1;
		
		debug(EDID, "Processing CEA v3 EDID Data Block type %d of length %d\n",
		      blockType, blockLength);
        
		switch ( blockType ) {
		case kEDIDCEA3DataBlockTypeAudio:
			// Don't care.
			// process_edid_cea3_audio((EDIDCEA3DataBlockAudio*)&block->data, blockLength);
			break;
		case kEDIDCEA3DataBlockTypeVideo:
			process_edid_cea3_video((EDIDCEA3DataBlockVideo*)&block->data, blockLength);
			break;
		case kEDIDCEA3DataBlockTypeSpeakerAllocation:
			// Don't care.
			// process_edid_cea3_speaker_allocation((EDIDCEA3DataBlockSpeakerAllocation*)&block->data, blockLength);
			break;
		case kEDIDCEA3DataBlockTypeExtended:
			// Don't care.
			// process_edid_cea3_extended((EDIDCEA3DataBlockExtended*)&block->data, blockLength);
			break;
		case kEDIDCEA3DataBlockTypeVenderSpecific:
			process_edid_cea3_vendor_specific((EDIDCEA3DataBlockVendorSpecific*)&block->data, blockLength);
			break;
		default:
			break;
		}
		startOffset += sectionLength;
	}
	
	return process_edid_cea2((EDIDCEA_2*)cea3);
}

static int process_edid_cea3_video(EDIDCEA3DataBlockVideo * video, uint32_t length)
{
	uint32_t        index;
	
	debug(EDID, "Processing CEA v3 EDID Video\n");
    
	for ( index=0; index<length; index++ ) {
		uint32_t id = video->descriptor[index];
		debug(EDID, "Processing CEA ShortID %d\n", id);
		try_cea_short_id(video->descriptor[index]);
	}
    
	return 0;
}

static int process_edid_cea3_vendor_specific(EDIDCEA3DataBlockVendorSpecific *vendor, uint32_t length)
{
	// Only care that this might be a signal we're HDMI.
	static const uint8_t hdmi_out[] = { 0x03, 0x0c, 0x00 };
	if (!memcmp(hdmi_out, vendor->ieeeOui, sizeof(hdmi_out))) {
		debug(DS, "EDID says oui, we're HDMI\n");
		s_ds_type = kHDMI_tx_mode_HDMI;
	}
	return 0;
}

static int process_edid_standard(EDIDStandard *standard)
{
	debug(EDID, "Processing Standard EDID\n");
	
	// Don't care about vendor/product IDs, color.
	process_edid_standard_feature(standard);
	process_edid_standard_established_timings(standard);
	process_edid_standard_timings(standard->standard, sizeof(standard->standard));
	process_edid_detailed(&standard->preferred);
	process_edid_detailed(&standard->detailed[0]);
	process_edid_detailed(&standard->detailed[1]);
	process_edid_detailed(&standard->detailed[2]);

	return 0;
}

static int process_edid_standard_feature(EDIDStandard *standard)
{
	// Don't care about color, just HDMI vs DVI and timings.
	if ((s_ds_type != kHDMI_tx_mode_None) && (standard->videoInputDef & (1<<7)) ) {
		if ( standard->version == 1 && standard->revision >= 4 ) {
			switch (standard->videoInputDef & 0x0F) {
			case 0:
			case 1:
				debug(DS, "EDID says downstream DVI\n");
				s_ds_type = kHDMI_tx_mode_DVI;
				break;
			case 5:
				debug(DS, "EDID says downstream DP\n");
				s_ds_type = kHDMI_tx_mode_HDMI;
				break;
			default:
				debug(DS, "EDID says downstream HDMI\n");
				s_ds_type = kHDMI_tx_mode_HDMI;
				break;
			}
		}
	}

	if ( standard->feature & kEDIDStandardFeatureFlagGTF ) {
		if ( standard->version == 1 && standard->revision >= 4 )
			s_timing_support = kSupportFlagCVT;
		else 
			s_timing_support = kSupportFlagGTF;
	}
	return 0;
}

static int process_edid_standard_timings(uint8_t *standard, uint32_t length)
{
	uint32_t index;
	for (index = 0; index < length; index += 2) {
		uint16_t timing_id = read_le16(standard, index);
		try_standard_timing_id(timing_id);
	}
	return 0;
}

static int process_edid_standard_established_timings(EDIDStandard *standard)
{
	uint32_t index;

	debug(EDID, "established=0x%02x established1=0x%02x established2=0x%02x\n",
	      standard->established[0],
	      standard->established[1],
	      standard->established[2]);
	
	for (index = 0; index < kEDIDEstablishedTimingsCount; ++index) {
		if (!(standard->established[index>>3] & (1 << (index % 8))))
			continue;
		debug(EDID, "index=%d\n", index);
		try_established_timing_id(index);
	}
	return 0;
}

static int process_edid_detailed(EDIDDetailed *detailed)
{
	if (detailed->data.header.header[0] == 0 && detailed->data.header.header[1] == 0) {
		return process_edid_detailed_descriptor(&(detailed->data.descriptor));
	} else {
		return try_detailed_timing(&(detailed->data.timing));
	}
}

static int process_edid_detailed_descriptor(EDIDDetailedDescriptor *descriptor)
{
	if (!descriptor->flag) return 0;
	
	switch (descriptor->type) {
        case 0xff: //Serial number
		// Don't care.
		break;
        case 0xfd: //Range limits
		// IODPService.cpp doesn't use this.
		// process_edid_detailed_range(&descriptor->data.range);
		break;
        case 0xfc: //Monitor name
		// Don't care.
		break;
        case 0xfb: // color point data
		// Don't care.
		// process_edid_detailed_color_point(&descriptor->data.color);
		break;
        case 0xfa: // standard timing
		process_edid_standard_timings(descriptor->data.standard, sizeof(descriptor->data.standard));
		break;
        case 0xf8: // CVT
		process_edid_cvt_timings(descriptor->data.cvt, sizeof(descriptor->data.cvt));
		break;
        case 0xf7: // established timings III
		process_edid_detailed_established_timings_iii(&descriptor->data.established);
        default: // DCM
		break;		
	}

	return 0;
}

static int process_edid_detailed_established_timings_iii(EDIDDetailedEstablishedTimingsIII *established)
{
	uint32_t index;
    
	debug(EDID, "established=%02x established1=%02x established2=%02x established3=%02x established4=%02x established5=%02x\n", established->data[0], established->data[1], established->data[2], established->data[3], established->data[4], established->data[5]);
    
	for (index = 0; index < kEDIDEstablishedTimingsCount; ++index) {
		debug(EDID, "index=%d mask=%08x\n", index, (1 << (index % 8)));
        
		if ( !(established->data[index>>3] & (1 << (index % 8))) )
			continue;
		
		try_established_timing_iii_id(index);
	}
	
	return 0;
}

static int try_standard_timing_id(uint16_t timing_id)
{
	uint32_t horizontal;
	uint32_t vertical = 0;
	uint32_t rate;
	
	debug(EDID, "timing_id 0x%04x\n", timing_id);

	if (timing_id == 0x0101)
		return -1;

	horizontal = ((timing_id & 0xff) + 31) * 8;
	switch ((timing_id >> 14) & 0x3) {
        case 0:
		vertical = horizontal * 10 / 16;
		break;
        case 1:
		vertical = horizontal * 3 /43;
		break;
        case 2:
		vertical = horizontal * 4 / 5;
		break;
        case 3:
		vertical = horizontal * 9 / 16;
		break;
	}
	
	rate = ((timing_id >> 8) & 0x3f) + 60;
	
	return use_dimensions(horizontal, vertical, rate << 16);
}

static int try_established_timing_id(uint8_t established_id)
{
	static const __IODPTEEstablished established_info[] = {
		{800, 600, 60<<16},
		{800, 600, 56<<16},
		{640, 480, 75<<16},
		{640, 480, 72<<16},
		{640, 480, 67<<16},
		{640, 480, 60<<16},
		{720, 400, 88<<16},
		{720, 400, 70<<16},
		
		{1280, 1024, 75<<16},
		{1024, 768, 75<<16},
		{1024, 768, 70<<16},
		{1024, 768, 60<<16},
		{0,0,0}, //interlaced {1024, 768, 87<<16},
		{832, 624, 75<<16},
		{800, 600, 75<<16},
		{800, 600, 72<<16},
		
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{1152, 870, 75<<16}
	};
	
	if (established_id >= (sizeof(established_info) / sizeof(__IODPTEEstablished)))
		return 0;
        
	return use_dimensions(established_info[established_id].horizontal,
			      established_info[established_id].vertical,
			      established_info[established_id].rate);
}

static int try_established_timing_iii_id(uint8_t establishedID)
{
	static const __IODPTEEstablished sEstablishedInfo[] = {
		{1152, 864, 75<<16},
		{1024, 768, 85<<16},
		{800, 600, 85<<16},
		{848, 480, 60<<16},
		{640, 480, 85<<16},
		{720, 400, 85<<16},
		{640, 400, 85<<16},
		{640, 350, 85<<16},
		
		{1280, 1024, 85<<16},
		{1280, 1024, 60<<16},
		{1280, 960, 85<<16},
		{1280, 960, 60<<16},
		{1280, 768, 85<<16},
		{1280, 768, 75<<16},
		{1280, 768, 60<<16},
		{0,0,0},            // Reduced Blanking {1280, 768, 60<<16},
		
		{1400, 1050, 75<<16},
		{1400, 1050, 60<<16},
		{0,0,0},            // Reduced Blanking {1400, 1050, 60<<16},
		{1440, 900, 85<<16},
		{1440, 900, 75<<16},
		{1440, 900, 60<<16},
		{0,0,0},            // Reduced Blanking {1440, 900, 60<<16},
		{1360, 768, 60<<16},
		
		{1600, 1200, 70<<16},
		{1600, 1200, 65<<16},
		{1600, 1200, 60<<16},
		{1680, 1050, 85<<16},
		{1680, 1050, 75<<16},
		{1680, 1050, 60<<16},
		{0,0,0},            // Reduced Blanking {1680, 1050, 60<<16},
		{1400, 1050, 85<<16},
		
		{1920, 1200, 60<<16},
		{0,0,0},            // Reduced Blanking {1920, 1200, 60<<16},
		{1856, 1392, 75<<16},
		{1856, 1392, 60<<16},
		{1792, 1344, 75<<16},
		{1792, 1344, 60<<16},
		{1600, 1200, 85<<16},
		{1600, 1200, 75<<16},
        
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{0,0,0},
		{1920, 1440, 75<<16},
		{1920, 1440, 60<<16},
		{1920, 1200, 85<<16},
		{1920, 1200, 75<<16},
	};
	
	if (establishedID >= (sizeof(sEstablishedInfo) / sizeof(__IODPTEEstablished)))
		return -1;
        
	return use_dimensions(sEstablishedInfo[establishedID].horizontal, 
			      sEstablishedInfo[establishedID].vertical, 
			      sEstablishedInfo[establishedID].rate);
}

static int try_detailed_timing(EDIDDetailedTiming *detailed)
{
	struct video_timing_data _data;
	uint32_t value;
	uint16_t pixelClock;
	int64_t rate, area;

	bzero(&_data, sizeof(_data));

	value = detailed->hor_addr_low;
	value |= (detailed->hor_addr_blank_high & 0xF0) << 4;
	_data.axis[kDisplayAxisTypeHorizontal].active = value;
    
	value = detailed->hor_fporch;
	value |= (detailed->hor_ver_porch_sync_high & (0x3<<6))<<2;
	_data.axis[kDisplayAxisTypeHorizontal].front_porch = value;
    
	value = detailed->hor_sync;
	value |= (detailed->hor_ver_porch_sync_high & (0x3<<4))<<4;
	_data.axis[kDisplayAxisTypeHorizontal].sync_width = value;
    
	value = detailed->hor_blank_low;
	value |= (detailed->hor_addr_blank_high & 0xF) << 8;
	if ( value < detailed->hor_border )
		return -1;
	value -= detailed->hor_border;
    
	_data.axis[kDisplayAxisTypeHorizontal].total = value + _data.axis[kDisplayAxisTypeHorizontal].active;
    
	if ( value < (_data.axis[kDisplayAxisTypeHorizontal].front_porch + _data.axis[kDisplayAxisTypeHorizontal].sync_width) ) {
		debug(EDID, "Invaid horizontal blanking\n");
		return -1;
	}
    
	_data.axis[kDisplayAxisTypeHorizontal].back_porch = value - _data.axis[kDisplayAxisTypeHorizontal].front_porch - _data.axis[kDisplayAxisTypeHorizontal].sync_width;
    
	value = detailed->ver_addr_low;
	value |= (detailed->ver_addr_blank_high & 0xF0) << 4;
	_data.axis[kDisplayAxisTypeVertical].active = value;
    
	value = (detailed->ver_fporch_sync & 0xF0) >> 4;
	value |= (detailed->hor_ver_porch_sync_high & (0x3<<2))<<2;
	_data.axis[kDisplayAxisTypeVertical].front_porch = value;
    
	value = detailed->ver_fporch_sync & 0x0F;
	value |= (detailed->hor_ver_porch_sync_high & (0x3<<0))<<4;
	_data.axis[kDisplayAxisTypeVertical].sync_width = value;
    
	value = detailed->ver_blank_low;
	value |= (detailed->ver_addr_blank_high & 0xF) << 8;
	if ( value < detailed->ver_border ) {
		debug(EDID, "Vertical blank < vertical border\n");
		return -1;
	}
        
	value -= detailed->ver_border;
    
	_data.axis[kDisplayAxisTypeVertical].total = value + _data.axis[kDisplayAxisTypeVertical].active;
	if ( value < (_data.axis[kDisplayAxisTypeVertical].front_porch + _data.axis[kDisplayAxisTypeVertical].sync_width) )
		return -1;
	_data.axis[kDisplayAxisTypeVertical].back_porch = value - _data.axis[kDisplayAxisTypeVertical].front_porch - _data.axis[kDisplayAxisTypeVertical].sync_width;
	
	pixelClock = (detailed->pixelClock[1]<<8) | detailed->pixelClock[0]; 
	area = _data.axis[kDisplayAxisTypeHorizontal].total * _data.axis[kDisplayAxisTypeVertical].total;

	if ( !area )
		return -1;

	rate = ((((uint64_t)pixelClock) * 10000) << 16) / area;
    
	// round up
	rate += 0x8000;
	rate &= 0xffff0000;
    
	_data.axis[kDisplayAxisTypeVertical].sync_rate = rate;
	_data.interlaced = ( detailed->detailed & (1<<7) ) != 0;
    
	// digital
	if ( ( detailed->detailed & (1<<4) ) != 0 ) {
    
		_data.axis[kDisplayAxisTypeHorizontal].sync_polarity = ( detailed->detailed & (1<<1) ) != 0;

		if ( ( detailed->detailed & (1<<3) ) != 0 ) {
			_data.axis[kDisplayAxisTypeVertical].sync_polarity = ( detailed->detailed & (1<<2) ) != 0;
		} else {
			_data.axis[kDisplayAxisTypeVertical].sync_polarity = _data.axis[kDisplayAxisTypeHorizontal].sync_polarity;

		}

	}

    
	if ( !_data.axis[kDisplayAxisTypeVertical].active || !_data.axis[kDisplayAxisTypeHorizontal].active || !_data.axis[kDisplayAxisTypeVertical].sync_rate)     
		return -1;

	return use_timings(&_data);
}

static int try_cvt_timing_id(EDIDCVT *cvt)
{   
	uint32_t horizontal = 0;
	uint32_t vertical;
	uint32_t rate;
	uint32_t index;
	
	vertical = (((uint32_t)(cvt->data[1]&0xf0))<<4) | cvt->data[0];
	vertical = ( vertical + 1 ) * 2;
	switch ((cvt->data[1]>>2) & 0x3) {
        case 0:
		horizontal = vertical * 4 / 3;
		break;
        case 1:
		horizontal = vertical * 16 / 9;
		break;
        case 2:
		horizontal = vertical * 16 / 10;
		break;
        default:
		return -1;
	}
	
	index = (cvt->data[2]>>5) & 0x3;
	if ( cvt->data[2] & (1<<(4-index)) ) {
		static const uint32_t sRates[] = { 50, 60, 75, 80 };
		rate = sRates[index];
	} else {
		return -1;
	}
	
	return use_dimensions(horizontal, vertical, rate<<16);
}

static int try_cea_short_id(uint32_t shortVideoID)
{
	struct video_timing_data data;
	debug(EDID, "cea_short_id 0x%08x\n", shortVideoID);

	if (get_timing_data_for_cea_short_id(shortVideoID, &data) != 0)
		return -1;
	
	return use_timings(&data);
}

static int get_timing_data_for_cea_short_id(uint32_t shortVideoID, struct video_timing_data *timingData)
{
    bzero(timingData, sizeof(struct video_timing_data));
    
    switch ( shortVideoID ) {
            // 1 640x480p @ 60
        case 1:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 800;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 640;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 96;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 48;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 16;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 525;
            timingData->axis[kDisplayAxisTypeVertical].active          = 480;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 2;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 33;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 10;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;
            break;        
            // 2,3 720x480p @ 60
        case 2:
        case 3:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 858;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 720;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 62;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 60;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 16;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 525;
            timingData->axis[kDisplayAxisTypeVertical].active          = 480;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 6;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 30;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 9;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;
            break; 
            // 4 1280x720p @ 60
        case 4:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1650;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1280;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 40;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 220;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 110;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 750;
            timingData->axis[kDisplayAxisTypeVertical].active          = 720;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 20;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;
            break;        
            // 7, 8 720(1440)x240p @ 60
        case 8:
        case 9:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1716;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1440;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 124;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 114;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 38;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 262;
            timingData->axis[kDisplayAxisTypeVertical].active          = 240;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 3;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 15;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            
            break;        
            // 12, 13 (2880)x240p @ 60
        case 12:
        case 13:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 3432;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 2880;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 248;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 228;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 76;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 262;
            timingData->axis[kDisplayAxisTypeVertical].active          = 240;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 3;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 15;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;
            break;
            // 6,7 720(1440)x480i @ 60
        case 6:
        case 7:
            timingData->interlaced = true;        
            // 14,15 720(1440)x480p @ 60
        case 14:
        case 15:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1716;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1440;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 124;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 120;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 32;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 525;
            timingData->axis[kDisplayAxisTypeVertical].active          = 480;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 6;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 30;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 9;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            
            break;        
            // 5 1920x1080i @ 60
        case 5:
            timingData->interlaced = true;
            // 16 1920x1080p @ 60
        case 16:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2200;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 44;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 148;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 88;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 1125;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 36;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;            
            break;     
            // 21, 22 720(1440)x576i @ 50
        case 21:
        case 22:
            timingData->interlaced = true;
            // 17 18 720x576p @ 50
        case 17:
        case 18:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 864;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 720;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 64;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 68;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 12;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 625;
            timingData->axis[kDisplayAxisTypeVertical].active          = 576;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 39;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;    
            break;
            // 19 1280x720p @ 50
        case 19:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1980;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1280;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 40;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 220;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 440;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 750;
            timingData->axis[kDisplayAxisTypeVertical].active          = 720;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 20;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;
            break;        
            // 23, 24 720(1440)x288p @ 50
        case 23:
        case 24:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1728;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1440;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 126;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 138;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 24;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 312;
            timingData->axis[kDisplayAxisTypeVertical].active          = 288;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 3;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 19;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 2;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;                        
            break;  
            // 27, 28 (2880)x288p @ 50
        case 27:
        case 28:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 3456;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 2880;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 252;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 276;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 48;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 312;
            timingData->axis[kDisplayAxisTypeVertical].active          = 288;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 3;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 19;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 2;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            
            break;        
            // 29, 30 1440x576p @ 50
        case 29:
        case 30:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1728;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1440;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 128;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 136;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 24;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 625;
            timingData->axis[kDisplayAxisTypeVertical].active          = 576;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 39;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            
            break;        
            // 20 1920x1080i @ 50
        case 20:
            timingData->interlaced = true;
            // 31 1920x1080p @ 50
        case 31:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2640;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 44;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 148;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 528;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 1125;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 36;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;            
            break;
            // 32 1920x1080p @ 24
        case 32:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2750;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 44;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 148;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 638;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 1125;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 36;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 24<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;            
            break;
            // 33 1920x1080p @ 25
        case 33:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2640;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 44;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 148;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 528;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 1125;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 36;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 25<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;            
            break;
            // 34 1920x1080p @ 30
        case 34:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2200;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 44;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 148;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 88;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 1125;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 36;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 4;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 30<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;            
            break;
            // 10, 11 (2880)x480i @ 60
        case 10:
        case 11:
            timingData->interlaced = true;
            // 35, 36 2880x480p @ 60
        case 35:
        case 36:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 3432;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 248;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 240;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 64;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 525;
            timingData->axis[kDisplayAxisTypeVertical].active          = 480;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 6;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 30;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 9;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 60<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            
            break;
            // 25, 26 (2880)x576i @ 50
        case 25:
        case 26:
            timingData->interlaced = true;
            // 37, 38 2880x576p @ 50
        case 37:
        case 38:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 3456;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 2880;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 256;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 272;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 48;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 625;
            timingData->axis[kDisplayAxisTypeVertical].active          = 576;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 39;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            
            break;
            // 39 1920x1080i @ 50
        case 39:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2304;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 168;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 184;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 32;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 1250;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 120;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 45;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;            

            timingData->interlaced = true;
            break;
            // 40 1920x1080i @ 100
        case 40:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 2640;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1920;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 44;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 148;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 528;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        

            timingData->axis[kDisplayAxisTypeVertical].total           = 1250;
            timingData->axis[kDisplayAxisTypeVertical].active          = 1080;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 120;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 45;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 50<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;            
            
            timingData->interlaced = true;
            break;
            // 41 1280x720p @ 100
        case 41:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1980;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1280;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 40;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 220;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 440;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 750;
            timingData->axis[kDisplayAxisTypeVertical].active          = 720;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 20;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 100<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;
            break;        
            // 44, 45 720(1440)x576i @ 100  
        case 44:
        case 45:
            timingData->interlaced = true;
            // 42, 43 720x576p @ 100
        case 42:
        case 43:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 864;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 720;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 64;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 68;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 12;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 625;
            timingData->axis[kDisplayAxisTypeVertical].active          = 576;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 39;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 100<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;    
            break;
            // 46 1920x1280i @ 120  
        case 46:
            return -1;
            // 47 1280x720p @ 120
        case 47:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 1650;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 1280;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 40;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 220;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 110;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 1;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 750;
            timingData->axis[kDisplayAxisTypeVertical].active          = 720;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 20;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 120<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 1;
            break;        
            // 48, 49 720x480p @ 120
        case 48:
        case 49:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 858;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 720;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 62;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 60;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 16;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 525;
            timingData->axis[kDisplayAxisTypeVertical].active          = 480;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 6;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 30;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 9;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 120<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;
            break; 
            // 50, 51 720(1440)x480i @ 120
        case 50:
        case 51:
            return -1;
            // 42, 43 720x576p @ 100
        case 52:
        case 53:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 864;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 720;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 64;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 68;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 12;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 625;
            timingData->axis[kDisplayAxisTypeVertical].active          = 576;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 5;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 39;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 5;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 200<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;    
            break;
            // 54, 55 720(1440)x576i @ 200
        case 54:
        case 55:
            return -1;
            // 56, 57 720x480p @ 240
        case 56:
        case 57:
            timingData->axis[kDisplayAxisTypeHorizontal].total         = 858;
            timingData->axis[kDisplayAxisTypeHorizontal].active        = 720;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_width     = 62;
            timingData->axis[kDisplayAxisTypeHorizontal].back_porch     = 60;
            timingData->axis[kDisplayAxisTypeHorizontal].front_porch    = 16;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_rate      = 0;
            timingData->axis[kDisplayAxisTypeHorizontal].sync_polarity  = 0;        
            
            timingData->axis[kDisplayAxisTypeVertical].total           = 525;
            timingData->axis[kDisplayAxisTypeVertical].active          = 480;
            timingData->axis[kDisplayAxisTypeVertical].sync_width       = 6;
            timingData->axis[kDisplayAxisTypeVertical].back_porch       = 30;
            timingData->axis[kDisplayAxisTypeVertical].front_porch      = 9;
            timingData->axis[kDisplayAxisTypeVertical].sync_rate        = 240<<16;
            timingData->axis[kDisplayAxisTypeVertical].sync_polarity    = 0;
            break; 
            // 58, 59 720(1440)x480i @ 240
        case 58:
        case 59:
            return -1;
            // more to come...too lazy right now
        default:
            return -1;
            break;
    }
        
    return 0;
}

static int use_dimensions(uint32_t horizontal, uint32_t vertical, uint32_t rate)
{
	int ret = -1;
	debug(EDID, "Use dimensions %ux%u %uHz\n", horizontal, vertical, rate >> 16);
	if (s_timing_support & kSupportFlagCVT) {
		ret = use_dimensions_cvt(horizontal, vertical, rate);
	} else if (s_timing_support & kSupportFlagGTF) {
		ret = use_dimensions_gtf(horizontal, vertical, rate);
	}
	if (ret != 0) {
		ret = use_dimensions_dmt(horizontal, vertical, rate);
	}
	return ret;
}

static int use_dimensions_dmt(uint32_t horizontal, uint32_t vertical, uint32_t rate)
{
    static const struct video_timing_data sDMTTimingInfo[] = { 
        {false, {{832,  640,    64,     96,     32,     0,  1}, {445,   350,    3,  60,     32, 85<<16,     0}} },
        {false, {{832,  640,    64,     96,     32,     0,  0}, {445,   400,    3,  41,     1,  85<<16,     1}} },
        {false, {{936,  720,    72,     108,    36,     0,  0}, {446,   400,    3,  42,     1,  85<<16,     1}} },
        {false, {{800,  640,    96,     48,     16,     0,  0}, {525,   480,    2,  33,     10, 60<<16,     0}} },
        {false, {{832,  640,    40,     128,    24,     0,  0}, {520,   480,    3,  28,     9,  72<<16,     0}} },
        {false, {{840,  640,    64,     120,    16,     0,  0}, {500,   480,    3,  16,     1,  75<<16,     0}} },
        {false, {{832,  640,    56,     80,     56,     0,  0}, {509,   480,    3,  25,     1,  85<<16,     0}} },
        {false, {{1024, 800,    72,     128,    24,     0,  1}, {625,   600,    2,  22,     1,  56<<16,     1}} },
        {false, {{1056, 800,    128,    88,     40,     0,  1}, {628,   600,    4,  23,     1,  60<<16,     1}} },
        {false, {{1040, 800,    120,    64,     56,     0,  1}, {666,   600,    6,  23,     37, 72<<16,     1}} },
        {false, {{1056, 800,    80,     160,    16,     0,  1}, {625,   600,    3,  21,     1,  75<<16,     1}} },
        {false, {{1048, 800,    64,     152,    32,     0,  1}, {631,   600,    3,  27,     1,  85<<16,     1}} },
        {false, {{960,  800,    32,     80,     48,     0,  1}, {636,   600,    4,  29,     3,  120<<16,    0}} },
        {false, {{1088, 848,    112,    112,    16,     0,  1}, {517,   480,    8,  23,     6,  60<<16,     1}} },
        {false, {{1344, 1024,   136,    160,    24,     0,  0}, {806,   768,    6,  29,     3,  60<<16,     0}} },
        {false, {{1328, 1024,   136,    144,    24,     0,  0}, {806,   768,    6,  29,     3,  70<<16,     0}} },
        {false, {{1312, 1024,   96,     176,    16,     0,  1}, {800,   768,    3,  28,     1,  75<<16,     1}} },
        {false, {{1376, 1024,   96,     208,    48,     0,  1}, {808,   768,    3,  36,     1,  85<<16,     1}} },
        {false, {{1600, 1152,   128,    256,    64,     0,  1}, {900,   864,    3,  32,     1,  75<<16,     1}} },
        {false, {{1650, 1280,   40,     220,    110,    0,  1}, {750,   720,    5,  20,     5,  60<<16,     1}} },
        {false, {{1664, 1280,   128,    192,    64,     0,  0}, {798,   768,    7,  20,     3,  60<<16,     1}} },
        {false, {{1696, 1280,   128,    208,    80,     0,  0}, {805,   768,    7,  27,     3,  75<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {809,   768,    7,  31,     3,  85<<16,     1}} },
        {false, {{1680, 1280,   128,    200,    72,     0,  0}, {831,   800,    6,  22,     3,  60<<16,     1}} },
        {false, {{1696, 1280,   128,    208,    80,     0,  0}, {838,   800,    6,  29,     3,  75<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {843,   800,    6,  34,     3,  85<<16,     1}} },
        {false, {{1800, 1280,   112,    312,    96,     0,  1}, {1000,  960,    3,  36,     1,  60<<16,     1}} },
        {false, {{1728, 1280,   160,    224,    64,     0,  1}, {1011,  960,    3,  47,     1,  85<<16,     1}} },
        {false, {{1688, 1280,   112,    248,    48,     0,  1}, {1066,  1024,   3,  38,     1,  60<<16,     1}} },
        {false, {{1688, 1280,   144,    248,    16,     0,  1}, {1066,  1024,   3,  38,     1,  75<<16,     1}} },
        {false, {{1728, 1280,   160,    224,    64,     0,  1}, {1072,  1024,   3,  44,     1,  85<<16,     1}} },
        {false, {{1792, 1360,   112,    256,    64,     0,  1}, {795,   768,    6,  18,     3,  60<<16,     1}} },
        {false, {{1792, 1366,   143,    213,    70,     0,  1}, {798,   768,    3,  24,     3,  60<<16,     1}} },
        {false, {{1864, 1400,   144,    232,    88,     0,  0}, {1089,  1050,   4,  32,     3,  60<<16,     1}} },
        {false, {{1896, 1400,   144,    248,    104,    0,  0}, {1099,  1050,   4,  42,     3,  75<<16,     1}} },
        {false, {{1912, 1400,   152,    256,    104,    0,  0}, {1105,  1050,   4,  48,     3,  85<<16,     1}} },
        {false, {{1904, 1440,   152,    232,    80,     0,  0}, {934,   900,    6,  25,     3,  60<<16,     1}} },
        {false, {{1936, 1440,   152,    248,    96,     0,  0}, {942,   900,    6,  33,     3,  75<<16,     1}} },
        {false, {{1952, 1440,   152,    256,    104,    0,  0}, {948,   900,    6,  39,     3,  85<<16,     1}} },
        {false, {{2160, 1600,   192,    304,    64,     0,  1}, {1250,  1200,   3,  46,     1,  60<<16,     1}} },
        {false, {{2160, 1600,   192,    304,    64,     0,  1}, {1250,  1200,   3,  46,     1,  65<<16,     1}} },
        {false, {{2160, 1600,   192,    304,    64,     0,  1}, {1250,  1200,   3,  46,     1,  70<<16,     1}} },
        {false, {{2160, 1600,   192,    304,    64,     0,  1}, {1250,  1200,   3,  46,     1,  75<<16,     1}} },
        {false, {{2160, 1600,   192,    304,    64,     0,  1}, {1250,  1200,   3,  46,     1,  85<<16,     1}} },
        {false, {{2240, 1680,   176,    280,    104,    0,  0}, {1089,  1050,   6,  30,     3,  60<<16,     1}} },
        {false, {{2272, 1680,   176,    296,    120,    0,  0}, {1099,  1050,   6,  40,     3,  75<<16,     1}} },
        {false, {{2288, 1680,   176,    304,    128,    0,  0}, {1105,  1050,   6,  46,     3,  85<<16,     1}} },
        {false, {{2448, 1792,   200,    328,    128,    0,  0}, {1394,  1344,   3,  46,     1,  60<<16,     1}} },
        {false, {{2456, 1792,   216,    352,    96,     0,  0}, {1417,  1344,   3,  69,     1,  75<<16,     1}} },
        {false, {{2528, 1856,   224,    352,    96,     0,  0}, {1439,  1392,   3,  43,     1,  60<<16,     1}} },
        {false, {{2560, 1856,   224,    352,    128,    0,  0}, {1500,  1392,   3,  104,    1,  75<<16,     1}} },
        {false, {{2200, 1920,   44,     148,    88,     0,  1}, {1125,  1080,   5,  36,     4,  60<<16,     1}} },
        {false, {{2592, 1920,   200,    336,    136,    0,  0}, {1245,  1200,   6,  36,     3,  60<<16,     1}} },
        {false, {{2608, 1920,   208,    344,    136,    0,  0}, {1255,  1200,   6,  46,     3,  75<<16,     1}} },
        {false, {{2624, 1920,   208,    352,    144,    0,  0}, {1262,  1200,   6,  53,     3,  85<<16,     1}} },
        {false, {{2600, 1920,   208,    344,    128,    0,  0}, {1500,  1440,   3,  56,     1,  60<<16,     1}} },
        {false, {{2640, 1920,   224,    352,    144,    0,  0}, {1500,  1440,   3,  56,     1,  75<<16,     1}} },
        {false, {{3504, 2560,   280,    472,    192,    0,  0}, {1658,  1600,   6,  49,     3,  60<<16,     1}} },
        {false, {{3536, 2560,   280,    488,    208,    0,  0}, {1672,  1600,   6,  63,     3,  75<<16,     1}} },
        {false, {{3536, 2560,   280,    488,    208,    0,  0}, {1682,  1600,   6,  73,     3,  85<<16,     1}} }
    };

    return use_dimensions_table(sDMTTimingInfo, (sizeof(sDMTTimingInfo) / sizeof(struct video_timing_data)), horizontal, vertical, rate);
}

static int use_dimensions_cvt(uint32_t horizontal, uint32_t vertical, uint32_t rate)
{
    static const struct video_timing_data sCVTTimingInfo[] = { 
        {false, {{816,  640,    64,     88,     24,     0,  0}, {423,   400,    6,  14,     3,  85<<16,     1}} },
        {false, {{800,  640,    64,     80,     16,     0,  0}, {500,   480,    4,  13,     3,  60<<16,     1}} },
        {false, {{816,  640,    64,     88,     24,     0,  0}, {504,   480,    4,  17,     3,  75<<16,     1}} },
        {false, {{816,  640,    64,     88,     24,     0,  0}, {507,   480,    4,  20,     3,  85<<16,     1}} },
        {false, {{1024, 800,    80,     112,    32,     0,  0}, {624,   600,    4,  17,     3,  60<<16,     1}} },
        {false, {{1040, 800,    80,     120,    40,     0,  0}, {625,   600,    4,  22,     3,  75<<16,     1}} },
        {false, {{1056, 800,    80,     128,    48,     0,  0}, {633,   600,    4,  26,     3,  85<<16,     1}} },
        {false, {{1328, 1024,   104,    152,    48,     0,  0}, {798,   768,    4,  23,     3,  60<<16,     1}} },
        {false, {{1360, 1024,   104,    168,    64,     0,  0}, {805,   768,    4,  30,     3,  75<<16,     1}} },
        {false, {{1376, 1024,   104,    176,    72,     0,  0}, {809,   768,    4,  34,     3,  85<<16,     1}} },
        {false, {{1536, 1152,   120,    192,    72,     0,  0}, {905,   864,    4,  34,     3,  75<<16,     1}} },
        {false, {{1664, 1280,   128,    192,    64,     0,  0}, {748,   720,    5,  20,     3,  60<<16,     1}} },
        {false, {{1664, 1280,   128,    192,    64,     0,  0}, {798,   768,    7,  20,     3,  60<<16,     1}} },
        {false, {{1696, 1280,   128,    208,    80,     0,  0}, {805,   768,    7,  27,     3,  75<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {809,   768,    7,  31,     3,  85<<16,     1}} },
        {false, {{1680, 1280,   128,    200,    72,     0,  0}, {831,   800,    6,  22,     3,  60<<16,     1}} },
        {false, {{1696, 1280,   128,    208,    80,     0,  0}, {838,   800,    6,  29,     3,  75<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {843,   800,    6,  34,     3,  85<<16,     1}} },
        {false, {{1696, 1280,   128,    208,    80,     0,  0}, {996,   960,    4,  29,     3,  60<<16,     1}} },
        {false, {{1728, 1280,   136,    224,    88,     0,  0}, {1011,  960,    4,  44,     3,  85<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {1063,  1024,   7,  29,     3,  60<<16,     1}} },
        {false, {{1728, 1280,   136,    224,    88,     0,  0}, {1072,  1024,   7,  38,     3,  75<<16,     1}} },
        {false, {{1744, 1280,   136,    232,    96,     0,  0}, {1078,  1024,   7,  44,     3,  85<<16,     1}} },
        {false, {{1776, 1360,   136,    208,    72,     0,  0}, {798,   768,    5,  22,     3,  60<<16,     1}} },
        {false, {{1864, 1400,   144,    232,    88,     0,  0}, {1089,  1050,   4,  32,     3,  60<<16,     1}} },
        {false, {{1896, 1400,   144,    248,    104,    0,  0}, {1099,  1050,   4,  42,     3,  75<<16,     1}} },
        {false, {{1912, 1400,   152,    256,    104,    0,  0}, {1105,  1050,   4,  48,     3,  85<<16,     1}} },
        {false, {{1904, 1440,   152,    232,    80,     0,  0}, {934,   900,    6,  25,     3,  60<<16,     1}} },
        {false, {{1936, 1440,   152,    248,    96,     0,  0}, {942,   900,    6,  33,     3,  75<<16,     1}} },
        {false, {{1952, 1440,   152,    256,    104,    0,  0}, {948,   900,    6,  39,     3,  85<<16,     1}} },
        {false, {{2160, 1600,   168,    280,    112,    0,  0}, {1245,  1200,   4,  38,     3,  60<<16,     1}} },
        {false, {{2160, 1600,   168,    288,    120,    0,  0}, {1255,  1200,   4,  48,     3,  75<<16,     1}} },
        {false, {{2192, 1600,   168,    296,    128,    0,  0}, {1262,  1200,   4,  55,     3,  85<<16,     1}} },
        {false, {{2240, 1680,   176,    280,    104,    0,  0}, {1089,  1050,   6,  30,     3,  60<<16,     1}} },
        {false, {{2272, 1680,   176,    296,    120,    0,  0}, {1099,  1050,   6,  40,     3,  75<<16,     1}} },
        {false, {{2288, 1680,   176,    304,    128,    0,  0}, {1105,  1050,   6,  46,     3,  85<<16,     1}} },
        {false, {{2432, 1792,   192,    320,    128,    0,  0}, {1393,  1344,   4,  42,     3,  60<<16,     1}} },
        {false, {{2448, 1792,   192,    328,    136,    0,  0}, {1405,  1344,   4,  54,     3,  75<<16,     1}} },
        {false, {{2512, 1856,   200,    328,    128,    0,  0}, {1443,  1392,   4,  44,     3,  60<<16,     1}} },
        {false, {{2544, 1856,   200,    344,    144,    0,  0}, {1456,  1392,   4,  57,     3,  75<<16,     1}} },
        {false, {{2576, 1920,   200,    328,    128,    0,  0}, {1120,  1080,   5,  32,     3,  60<<16,     1}} },
        {false, {{2592, 1920,   200,    336,    136,    0,  0}, {1245,  1200,   6,  36,     3,  60<<16,     1}} },
        {false, {{2608, 1920,   208,    344,    136,    0,  0}, {1255,  1200,   6,  46,     3,  75<<16,     1}} },
        {false, {{2624, 1920,   208,    352,    144,    0,  0}, {1262,  1200,   6,  53,     3,  85<<16,     1}} },
        {false, {{2608, 1920,   208,    344,    136,    0,  0}, {1493,  1440,   4,  46,     3,  60<<16,     1}} },
        {false, {{2640, 1920,   208,    360,    152,    0,  0}, {1506,  1440,   4,  59,     3,  75<<16,     1}} },
        {false, {{3504, 2560,   280,    472,    192,    0,  0}, {1658,  1600,   6,  49,     3,  60<<16,     1}} },
        {false, {{3536, 2560,   280,    488,    208,    0,  0}, {1672,  1600,   6,  63,     3,  75<<16,     1}} },
        {false, {{3536, 2560,   280,    488,    208,    0,  0}, {1682,  1600,   6,  73,     3,  85<<16,     1}} }
    };

    return use_dimensions_table(sCVTTimingInfo, (sizeof(sCVTTimingInfo) / sizeof(struct video_timing_data)), horizontal, vertical, rate);
}

static int use_dimensions_gtf(uint32_t horizontal, uint32_t vertical, uint32_t rate)
{
    static const struct video_timing_data sGTFTimingInfo[] = { 
        {false, {{816,  640,    64,      88,    24,     0,  0}, {421,   400,    3,  17,     1,  85<<16,     1}} },
        {false, {{800,  640,    64,      80,    16,     0,  0}, {497,   480,    3,  13,     1,  60<<16,     1}} },
        {false, {{816,  640,    64,      88,    24,     0,  0}, {502,   480,    3,  18,     1,  75<<16,     1}} },
        {false, {{832,  640,    64,      96,    32,     0,  0}, {505,   480,    3,  21,     1,  85<<16,     1}} },
        {false, {{1024, 800,    80,     112,    32,     0,  0}, {622,   600,    3,  18,     1,  60<<16,     1}} },
        {false, {{1040, 800,    80,     120,    40,     0,  0}, {627,   600,    3,  23,     1,  75<<16,     1}} },
        {false, {{1056, 800,    88,     128,    40,     0,  0}, {630,   600,    3,  26,     1,  85<<16,     1}} },
        {false, {{1344, 1024,   104,    160,    56,     0,  0}, {795,   768,    3,  23,     1,  60<<16,     1}} },
        {false, {{1360, 1024,   112,    168,    56,     0,  0}, {802,   768,    3,  30,     1,  75<<16,     1}} },
        {false, {{1376, 1024,   112,    176,    64,     0,  0}, {807,   768,    3,  35,     1,  85<<16,     1}} },
        {false, {{1552, 1152,   128,    200,    72,     0,  0}, {902,   864,    3,  34,     1,  75<<16,     1}} },
        {false, {{1664, 1280,   136,    192,    56,     0,  0}, {746,   720,    3,  22,     1,  60<<16,     1}} },
        {false, {{1680, 1280,   136,    200,    64,     0,  0}, {795,   768,    3,  23,     1,  60<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {802,   768,    3,  30,     1,  75<<16,     1}} },
        {false, {{1728, 1280,   136,    224,    88,     0,  0}, {807,   768,    3,  35,     1,  85<<16,     1}} },
        {false, {{1680, 1280,   136,    200,    64,     0,  0}, {828,   800,    3,  24,     1,  60<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {835,   800,    3,  31,     1,  75<<16,     1}} },
        {false, {{1728, 1280,   136,    224,    88,     0,  0}, {840,   800,    3,  36,     1,  85<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {994,   960,    3,  30,     1,  60<<16,     1}} },
        {false, {{1744, 1280,   136,    232,    96,     0,  0}, {1008,  960,    3,  44,     1,  85<<16,     1}} },
        {false, {{1712, 1280,   136,    216,    80,     0,  0}, {1060,  1024,   3,  32,     1,  60<<16,     1}} },
        {false, {{1728, 1280,   136,    224,    88,     0,  0}, {1069,  1024,   3,  44,     1,  75<<16,     1}} },
        {false, {{1744, 1280,   136,    232,    96,     0,  0}, {1075,  1024,   3,  47,     1,  85<<16,     1}} },
        {false, {{1776, 1360,   144,    208,    64,     0,  0}, {795,   768,    3,  23,     1,  60<<16,     1}} },
        {false, {{1800, 1366,   144,    216,    72,     0,  0}, {795,   768,    3,  23,     1,  60<<16,     1}} },
        {false, {{1880, 1400,   152,    240,    88,     0,  0}, {1087,  1050,   3,  33,     1,  60<<16,     1}} },
        {false, {{1896, 1400,   152,    248,    96,     0,  0}, {1096,  1050,   3,  42,     1,  75<<16,     1}} },
        {false, {{1912, 1400,   152,    256,    104,    0,  0}, {1103,  1050,   3,  49,     1,  85<<16,     1}} },
        {false, {{1904, 1440,   152,    232,    80,     0,  0}, {932,   900,    3,  28,     1,  60<<16,     1}} },
        {false, {{1936, 1440,   152,    248,    96,     0,  0}, {940,   900,    3,  36,     1,  75<<16,     1}} },
        {false, {{1952, 1440,   160,    256,    96,     0,  0}, {945,   900,    3,  41,     1,  85<<16,     1}} },
        {false, {{2160, 1600,   176,    280,    104,    0,  0}, {1242,  1200,   3,  38,     1,  60<<16,     1}} },
        {false, {{2192, 1600,   176,    296,    120,    0,  0}, {1253,  1200,   3,  49,     1,  75<<16,     1}} },
        {false, {{2192, 1600,   176,    296,    120,    0,  0}, {1260,  1200,   3,  56,     1,  85<<16,     1}} },
        {false, {{2256, 1680,   184,    288,    104,    0,  0}, {1087,  1050,   3,  33,     1,  60<<16,     1}} },
        {false, {{2288, 1680,   184,    304,    120,    0,  0}, {1096,  1050,   3,  42,     1,  75<<16,     1}} },
        {false, {{2288, 1680,   184,    304,    120,    0,  0}, {1103,  1050,   3,  49,     1,  85<<16,     1}} },
        {false, {{2432, 1792,   192,    320,    128,    0,  0}, {1391,  1344,   3,  43,     1,  60<<16,     1}} },
        {false, {{2464, 1792,   200,    336,    136,    0,  0}, {1403,  1344,   3,  55,     1,  75<<16,     1}} },
        {false, {{2528, 1856,   200,    336,    136,    0,  0}, {1441,  1392,   3,  45,     1,  60<<16,     1}} },
        {false, {{2544, 1856,   200,    344,    144,    0,  0}, {1453,  1392,   3,  57,     1,  75<<16,     1}} },
        {false, {{2576, 1920,   208,    328,    120,    0,  0}, {1118,  1080,   3,  34,     1,  60<<16,     1}} },
        {false, {{2592, 1920,   208,    336,    128,    0,  0}, {1242,  1200,   3,  38,     1,  60<<16,     1}} },
        {false, {{2624, 1920,   208,    352,    144,    0,  0}, {1253,  1200,   3,  49,     1,  75<<16,     1}} },
        {false, {{2640, 1920,   208,    360,    152,    0,  0}, {1260,  1200,   3,  56,     1,  85<<16,     1}} },
        {false, {{2624, 1920,   208,    352,    144,    0,  0}, {1490,  1440,   3,  46,     1,  60<<16,     1}} },
        {false, {{2640, 1920,   208,    360,    152,    0,  0}, {1503,  1440,   3,  59,     1,  75<<16,     1}} },
        {false, {{3504, 2560,   280,    472,    192,    0,  0}, {1656,  1600,   3,  52,     1,  60<<16,     1}} },
        {false, {{3536, 2560,   280,    488,    208,    0,  0}, {1670,  1600,   3,  66,     1,  75<<16,     1}} },
        {false, {{3552, 2560,   288,    496,    208,    0,  0}, {1680,  1600,   3,  76,     1,  85<<16,     1}} }
    };

    return use_dimensions_table(sGTFTimingInfo, (sizeof(sGTFTimingInfo) / sizeof(struct video_timing_data)), horizontal, vertical, rate);
}

static int use_dimensions_table(const struct video_timing_data * list, uint32_t count, uint32_t horizontal, uint32_t vertical, uint32_t rate)
{
	const struct video_timing_data * matching    = NULL;
	uint32_t                    roundRate   = (rate + 0x8000) & 0xffff0000;
	uint32_t                    index;
	struct video_timing_data data;
	
	if ( !list || !count || !horizontal || !vertical || !rate )
		return -1;
    
	for (index=0; index<count; index++) {
		const struct video_timing_data * timing = &list[index];
		
		if ( timing->axis[kDisplayAxisTypeHorizontal].active != horizontal )
			continue;
		
		if ( timing->axis[kDisplayAxisTypeVertical].active != vertical )
			continue;
		
		if ( (uint32_t) timing->axis[kDisplayAxisTypeVertical].sync_rate != roundRate )
			continue;
		
		matching = timing;
		break;
	}
    
	if ( !matching )
		return -1;
    
	bcopy(matching, &data, sizeof(data));
	data.axis[kDisplayAxisTypeVertical].sync_rate = rate;
	return use_timings(&data);
}

static int use_timings(struct video_timing_data *data)
{
	bool winner = false;
	uint32_t width, height, rate, size, old_rate;
	width = data->axis[kDisplayAxisTypeHorizontal].active;
	height = data->axis[kDisplayAxisTypeVertical].active;
	rate = data->axis[kDisplayAxisTypeVertical].sync_rate;
	size = width * height * 4;  // Assume RGBx8888

	debug(SCORE, "Try timings %ux%u%c/%u\n",
	      width, height, data->interlaced ? 'i' : 'p', rate >> 16);

	// Reject timings we can't support or don't like.
	if (hdmi_controller_validate_video(data) != 0) {
		debug(SCORE, "Rejecting: bad timings\n");
		return -1;
	} else if (s_restrict_h_active && s_restrict_v_active &&
		   (width != s_restrict_h_active || height != s_restrict_v_active)) {
		// Restricting to specific dimensions.
		debug(SCORE, "Rejecting dimensions not matching %ux%u\n",
		      s_restrict_h_active, s_restrict_v_active);
		return -1;
	} else if (width > kMaxHorizontalActive) {
		debug(SCORE, "Rejecting: horizontal active %u greater than %u\n",
		      width, kMaxHorizontalActive);
		return -1;
	} else if (size > DISPLAY_SIZE) {
		debug(SCORE, "Rejecting: total size %u too big for buffer %u bytes\n",
		      size, (uint32_t)DISPLAY_SIZE);
		return -1;
	}

	debug(SCORE, "Accepted\n");

	// Score these new timings vs the best we have so far. Note than wider/taller
	// scoring will only occur if restrict_edid() was not used.
	if (s_have_timings) {
		old_rate = s_best_timings.axis[kDisplayAxisTypeVertical].sync_rate;
	} else {
		old_rate = 0;
	}

	if (!s_have_timings) {
		winner = true;
		debug(SCORE, "First candidate\n");
	} else if ((rate >= (59 << 16) && rate <= (61 << 16)) &&
		   (old_rate < (59 << 16) || old_rate > (61 << 16))) {
		winner = true;
		debug(SCORE, "Preferring 60Hz to non-60Hz\n");
	} else if (width > s_best_timings.axis[kDisplayAxisTypeHorizontal].active) {
		winner = true;
		debug(SCORE, "Preferring wider\n");
	} else if (height > s_best_timings.axis[kDisplayAxisTypeVertical].active) {
		winner = true;
		debug(SCORE, "Preferring taller\n");
	}

	if (winner) {
		s_have_timings = true;
		bcopy(data, &s_best_timings, sizeof(s_best_timings));
	}

	return 0;
}

static int verify_edid(EDID *edid, uint8_t *p_checksum)
{
	uint32_t index;
	uint8_t sum;

	debug(EDID, "Validating EDID\n");

	for (index = 0, sum = 0; index < sizeof(EDID); ++index) {
		sum += ((uint8_t *) edid)[index];
	}

	debug(EDID, "EDID checksum of 0x%02x\n", sum);

	if (p_checksum)
		*p_checksum = sum;

	return sum == 0 ? 0 : -1;
}

static int get_edid(uint8_t offset, EDID *edid)
{
	int ret;
	uint32_t tries = 0;
	uint64_t timer_start;

	timer_start = system_time();

	do {
		uint8_t checksum;
		++tries;
		ret = hdmi_controller_read_bytes_i2c(kI2CEdidReadDeviceAddr, offset,
						   (uint8_t *) edid, sizeof(EDID));
		if (ret) {
			debug(EDID, "Failed to read EDID offset=0x%08x ret=0x%08x\n", offset, ret);
			if (s_abort_edid) {
				// Display is trying to shut down - don't delay it.
				debug(ERROR, "EDID retry loop aborted\n");
				break;
			}
			task_sleep(kEdidRetryDelay);
		} else {
			ret = verify_edid(edid, &checksum);
			debug(EDID, "Verify EDID offset=0x%08x ret=0x%08x checksum=0x%02x\n", offset, ret, checksum);
		}
	} while (ret != 0 && !time_has_elapsed(timer_start, kEdidRetryTimeout));

	if (ret == 0) {
		debug(EDID, "Got EDID after %d tries, %llu usecs\n", tries, system_time() - timer_start);
	} else {
		debug(ERROR, "Failed to get EDID after %llu usecs\n", system_time() - timer_start);
	}

	return ret;
}

static uint16_t read_le16(const void *data, uint32_t offset)
{
	const uint8_t *p = (const uint8_t *) data;
	uint16_t result = p[offset];
	result |= ((uint16_t) p[offset + 1]) << 8;
	return result;
}

static uint32_t read_le32(const void *data, uint32_t offset)
{
	const uint8_t *p = (const uint8_t *) data;
	uint32_t result = p[offset];
	result |= ((uint32_t) p[offset + 1]) << 8;
	result |= ((uint32_t) p[offset + 2]) << 16;
	result |= ((uint32_t) p[offset + 3]) << 24;
	return result;
}
