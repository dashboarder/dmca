/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2009 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOKIT_EDID_H
#define _IOKIT_EDID_H


#define kEDIDEstablishedTimingsCount    24
#define kEDIDEstablishedTimingsIIICount 48

typedef struct __attribute__((packed)) _EDIDDetailedHeader {
	uint8_t             header[2];
}EDIDDetailedHeader;

//__packed
typedef struct __attribute__((packed)) _EDIDDetailedTiming {
	uint8_t             pixelClock[2];
    
	uint8_t             hor_addr_low;
	uint8_t             hor_blank_low;
	uint8_t             hor_addr_blank_high;
	uint8_t             ver_addr_low;
	uint8_t             ver_blank_low;
	uint8_t             ver_addr_blank_high;
	
	uint8_t             hor_fporch;    // #8
	uint8_t             hor_sync;
	uint8_t             ver_fporch_sync;
	
	uint8_t             hor_ver_porch_sync_high;
	uint8_t             inmm[3];    // in milimeter
	uint8_t             hor_border;
	uint8_t             ver_border;
	uint8_t             detailed;
} EDIDDetailedTiming;

typedef struct __attribute__((packed)) _EDIDString{
    uint8_t             flag;
    char                string[13];
} EDIDString;

typedef enum {
    kEDIDDetailedRangeFlagRateOffsetVerticalMin     = (1<<0),
    kEDIDDetailedRangeFlagRateOffsetVerticalMax     = (1<<1),
    kEDIDDetailedRangeFlagRateOffsetHorizontalMin   = (1<<2),
    kEDIDDetailedRangeFlagRateOffsetHorizontalMax   = (1<<3)
} EDIDDetailedRangeFlags;

enum {
    kEDIDDetailedRangeVideoTimingTypeGTF                = 0,
    kEDIDDetailedRangeVideoTimingTypeRangeLimitsOnly    = 1,
    kEDIDDetailedRangeVideoTimingTypeSecondaryGTF       = 2,
    kEDIDDetailedRangeVideoTimingTypeCVT                = 4
};
typedef uint8_t EDIDDetailedRangeVideoTimingType;

typedef enum {
    kEDIDSupportedAspectFlag15_9        = (1<<3),
    kEDIDSupportedAspectFlag5_4         = (1<<4),
    kEDIDSupportedAspectFlag16_10       = (1<<5),
    kEDIDSupportedAspectFlag16_9        = (1<<6),
    kEDIDSupportedAspectFlag4_3         = (1<<7)
} EDIDSupportedAspectFlags;

typedef enum {
    kEDIDPreferredAspectType4_3,
    kEDIDPreferredAspectType16_9,
    kEDIDPreferredAspectType16_10,
    kEDIDPreferredAspectType5_4,
    kEDIDPreferredAspectType15_9,
    kEDIDPreferredAspectTypeShift       = 5,
    kEDIDPreferredAspectTypeMask        = 0x7,
    
} EDIDPreferredAspectType;

typedef enum {
    kEDIDCVTBlankingFlagStandard        = (1<<3),
    kEDIDCVTBlankingFlagReduced         = (1<<4),
} EDIDCVTBlankingFlags;

typedef enum {
    kEDIDDigitalScalingFlagVerticalStretch      = (1<<4),
    kEDIDDigitalScalingFlagVerticalShrink       = (1<<5),
    kEDIDDigitalScalingFlagHorizontalStretch    = (1<<6),
    kEDIDDigitalScalingFlagHorizontalShrink     = (1<<7)
} EDIDDigitalScalingFlags;


typedef struct __attribute__((packed)) _EDIDDetailedRangeGTF {
    uint8_t             reserved;
    uint8_t             breakFrequency; // horizontal freq / 2 (khz)
    uint8_t             c;              // c*2
    uint8_t             m[2];
    uint8_t             k;
    uint8_t             j;              // j*2
} EDIDDetailedRangeGTF;

typedef struct __attribute__((packed)) _EDIDDetailedRangeCVT {
    uint8_t             version;
                                                // Max. Pix Clk = [(range.maxPixelClockRate) × 10] – [(Byte 12: bits 7-2) × 0.25MHz]
    uint8_t             precision_activeMaxMSB; // pixel clock precision * 0.25Mhz (bits 7-2) | max active pixels per line MSB (bits 1-0)
    uint8_t             activeMaxLSB;           // max active pixels per line MSB
    uint8_t             supportedFlags;
    uint8_t             preferredFlags;
    uint8_t             scaling;
    uint8_t             verticalRefreshRate;    //hz
} EDIDDetailedRangeCVT;

typedef struct __attribute__((packed)) _EDIDDetailedRange{
    uint8_t             flag;
    uint8_t             minVerticalRate;
    uint8_t             maxVerticalRate;
    uint8_t             minHorizontalRate;
    uint8_t             maxHorizontalRate;
    uint8_t             maxPixelClockRate;
    
    struct {
        EDIDDetailedRangeVideoTimingType type;
        
        union {
            EDIDDetailedRangeGTF    gtf;
            EDIDDetailedRangeCVT     cvt;
        } data;
    }timing;
    
} EDIDDetailedRange;

typedef struct __attribute__((packed)) _EDIDDetailedWhitePoint {
    uint8_t             index;
	uint8_t             xyLSB;
	uint8_t             x;
	uint8_t             y;
    uint8_t             gamma;
} EDIDDetailedWhitePoint;

typedef struct __attribute__((packed)) _EDIDDetailedColorPoint {
    EDIDDetailedWhitePoint whitePoint[2];
} EDIDDetailedColorPoint;

typedef struct __attribute__((packed)) _EDIDDetailedEstablishedTimingsIII {
    uint8_t data[6];
} EDIDDetailedEstablishedTimingsIII;

typedef struct __attribute__((packed)) _EDIDDetailedDescriptor {
	uint8_t             zero[2];
    uint8_t             flag;
    uint8_t             type;
    
    union {
        EDIDString                          name;
        EDIDDetailedRange                   range;
        EDIDDetailedColorPoint              color;
        uint8_t                             standard[12];
        uint8_t                             cvt[12];
        EDIDDetailedEstablishedTimingsIII   established;
    }data;
    
}EDIDDetailedDescriptor;


typedef struct __attribute__((packed)) EDIDDetailed {

    union {
        EDIDDetailedHeader      header;
        EDIDDetailedTiming      timing;
        EDIDDetailedDescriptor  descriptor;
    } data;
    
} EDIDDetailed;


typedef struct __attribute__((packed)) _EDIDHeader {
	uint8_t             header[8];
} EDIDHeader;

typedef enum {
    kEDIDStandardFeatureFlagGTF             = (1<<0),
    kEDIDStandardFeatureFlagPreferredNative = (1<<1),
    kEDIDStandardFeatureFlagSRGB            = (1<<2),
    kEDIDStandardFeatureFlagColorDepth      = (3<<3),
    kEDIDStandardFeatureFlagActiveOff       = (1<<5),
    kEDIDStandardFeatureFlagSuspend         = (1<<6),
    kEDIDStandardFeatureFlagStandby         = (1<<7)
} EDIDStandardFeatureFlags;

// STANDARD STUFF
typedef struct __attribute__((packed)) _EDIDStandard {
	uint8_t             header[8];
	// Vender / Product 
	uint8_t             vendorID[2];
	uint8_t             productID[2];
	uint8_t             serialNumber[4];
	uint8_t             weekOfManu;
	uint8_t             yearOfManu;
	// EDID structure version / rev.
	uint8_t             version;
	uint8_t             revision;
	// Basic Display Parameters / Features
	uint8_t             videoInputDef;
	uint8_t             maxHorzSize;    // cm
	uint8_t             maxVertSize;    // cm
	uint8_t             dispTransferChr; 
	uint8_t             feature;
	// Color Characteristics
	uint8_t             redGreenLowBits;
	uint8_t             blueWhiteLowBits;
	uint8_t             redX;
	uint8_t             redY;
	uint8_t             greenX;
	uint8_t             greenY;
	uint8_t             blueX;
	uint8_t             blueY;
	uint8_t             whiteX;
	uint8_t             whiteY;
	// Established Timings
	uint8_t             established[3];
	// Standard Timing Identification
	uint8_t             standard[16];
	EDIDDetailed        preferred;
	EDIDDetailed        detailed[3];
	// Extra
	uint8_t             extensionFlag;
	uint8_t             checksum;
} EDIDStandard;


typedef struct __attribute__((packed)) _EDIDEXTHeader {
    uint8_t             tag;
    uint8_t             revision;
} EDIDEXTHeader;

typedef struct __attribute__((packed)) _EDIDCVT {
    uint8_t             data[3];
} EDIDCVT;

// CEA STUFF

typedef enum {
    kEDIDCEA3DataBlockTypeReserved,
    kEDIDCEA3DataBlockTypeAudio,
    kEDIDCEA3DataBlockTypeVideo,
    kEDIDCEA3DataBlockTypeVenderSpecific,
    kEDIDCEA3DataBlockTypeSpeakerAllocation,
    kEDIDCEA3DataBlockTypeVESADTC,
    kEDIDCEA3DataBlockTypeReserved1,
    kEDIDCEA3DataBlockTypeExtended
} EDIDCEA3DataBlockType;

typedef struct __attribute__((packed)) _EDIDCEA3DataBlockAudio {
    uint8_t                 descriptor[0];
} EDIDCEA3DataBlockAudio;

typedef struct __attribute__((packed)) _EDIDCEA3DataBlockVideo {
    uint8_t                 descriptor[0];
} EDIDCEA3DataBlockVideo;

typedef struct __attribute__((packed)) _EDIDCEA3DataBlockVendorSpecific {
    uint8_t                 ieeeOui[3];
    uint8_t                 data[0];
} EDIDCEA3DataBlockVendorSpecific;

typedef struct __attribute__((packed)) _EDIDCEA3DataBlockSpeakerAllocation {
    uint8_t                 data[3];
} EDIDCEA3DataBlockSpeakerAllocation;

typedef struct __attribute__((packed)) _EDIDCEA3DataBlockExtended {
    uint8_t                 tag;
    uint8_t                 data[31];
} EDIDCEA3DataBlockExtended;

typedef struct __attribute__((packed)) _EDIDCEA3DataBlock {
    uint8_t flags;
    
    union {
        EDIDCEA3DataBlockAudio               audio;
        EDIDCEA3DataBlockVideo               video;
        EDIDCEA3DataBlockVendorSpecific      vendor;
        EDIDCEA3DataBlockSpeakerAllocation   speaker;
        EDIDCEA3DataBlockExtended            extended;
    } data;
    
} EDIDCEA3DataBlock;

typedef struct __attribute__((packed)) _EDIDCEA_1 {
    struct {
        uint8_t             tag;
        uint8_t             revision;
        uint8_t             dataOffset;
        uint8_t             reserved;
    } header;
    
    uint8_t             data[123];
    uint8_t             checksum;
} EDIDCEA_1;

typedef struct __attribute__((packed)) _EDIDCEA_2 {
    struct {
        uint8_t             tag;
        uint8_t             revision;
        uint8_t             dataOffset;
        uint8_t             detailedFlags;
    } header;
    
    uint8_t             data[123];
    uint8_t             checksum;
} EDIDCEA_2;

typedef struct __attribute__((packed)) _EDIDCEA_3 {
    struct {
        uint8_t             tag;
        uint8_t             revision;
        uint8_t             dataLength;
        uint8_t             detailedFlags;
    } header;
    uint8_t             data[123];
    uint8_t             checksum;
} EDIDCEA_3;

typedef struct __attribute__((packed)) _EDIDCEA {
    union {
        EDIDEXTHeader   header;
        EDIDCEA_1       cea1;
        EDIDCEA_2       cea2;
        EDIDCEA_3       cea3;
    } data;
} EDIDCEA;

typedef struct __attribute__((packed)) _EDIDVTB{
    EDIDEXTHeader   header;
    uint8_t         dtbCount;
    uint8_t         cvtCount;
    uint8_t         stCount;
    uint8_t         data[122];
    uint8_t         checksum;
} EDIDVTB;

typedef enum {
    kEDIDTypeCEA    = 0x02,
    KEDIDTypeVTB    = 0x10,
    kEDIDTypeDI     = 0x40,
} EDIDType;

// __packed 
typedef struct __attribute__((packed)) _EDID{
    union {
        EDIDHeader      header;
        EDIDStandard    standard;
        EDIDCEA         cea;
        EDIDVTB         vtb;
    } data;
    
} EDID;



#endif /* _IOKIT_EDID_H */
