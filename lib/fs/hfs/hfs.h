/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* Derived from: */
/*
 *  fs.h - Externs for the File System Modules
 *
 *  Copyright (c) 1999-2004 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#ifndef _BOOTX_FS_H_
#define _BOOTX_FS_H_

// tunable
#define kFSCacheSize	(64 * 1024)

// File Permissions and Types
enum {
	kPermOtherExecute  = 1 << 0,
	kPermOtherWrite    = 1 << 1,
	kPermOtherRead     = 1 << 2,
	kPermGroupExecute  = 1 << 3,
	kPermGroupWrite    = 1 << 4,
	kPermGroupRead     = 1 << 5,
	kPermOwnerExecute  = 1 << 6,
	kPermOwnerWrite    = 1 << 7,
	kPermOwnerRead     = 1 << 8,
	kPermMask          = 0x1FF,
	kOwnerNotRoot      = 1 << 9,
	kFileTypeUnknown   = 0x0 << 16,
	kFileTypeFlat      = 0x1 << 16,
	kFileTypeDirectory = 0x2 << 16,
	kFileTypeLink      = 0x3 << 16,
	kFileTypeMask      = 0x3 << 16
};

typedef void *CICell;

// Externs for cache.c
extern uint32_t gCacheHits;
extern uint32_t gCacheMisses;
extern uint32_t gCacheEvicts;

extern void CacheInit(CICell ih, uint32_t chunkSize);
extern uint32_t CacheRead(CICell ih, uint8_t *buffer, off_t offset,
		      uint32_t length, bool cache);

// Externs for hfs.c
extern int HFSInitPartition(CICell ih);
extern int  HFSDetect(void);
extern int HFSReadFile(CICell ih, const char *filePath, void *base, off_t offset, size_t length);
extern int HFSGetFileInfo(CICell ih, const char *filePath, uint32_t *flags, time_t *time, off_t *size);
extern int HFSGetDirEntry(CICell ih, const char *dirPath, off_t *dirIndex, char *name, size_t nameSize, uint32_t *flags, time_t *time, off_t *size);
extern struct HFSPlusVolumeHeader *HFSGetVolumeHeader(void);

// Externs for hfs_fs.c
extern int HFSBlockRead(CICell *ih, void *buf, off_t offset, uint32_t length);

/*
 * We may need to work with big-endian data that's not
 * aligned.  For ARM at least, this is an issue.  Rather
 * than use swap functions and macrotize, these are explicitly
 * called out to avoid alignment issues even on big-endian systems.
 *
 * No these aren't efficient.  No, it doesn't really matter...
 */
static inline u_int64_t
HFSntohll(const void *ov)
{
	unsigned const char	*ovb = (unsigned const char *)ov;

	return(((u_int64_t)ovb[0] << 56) +
	    ((u_int64_t)ovb[1] << 48) +
	    ((u_int64_t)ovb[2] << 40) +
	    ((u_int64_t)ovb[3] << 32) +
	    ((u_int64_t)ovb[4] << 24) +
	    ((u_int64_t)ovb[5] << 16) +
	    ((u_int64_t)ovb[6] << 8) +
	    (u_int64_t)ovb[7]);
}

static inline u_int32_t
HFSntohl(const void *ov)
{
	unsigned const char	*ovb = (unsigned const char *)ov;

	return(((u_int32_t)ovb[0] << 24) +
	    ((u_int32_t)ovb[1] << 16) +
	    ((u_int32_t)ovb[2] << 8) +
	    (u_int32_t)ovb[3]);
}

static inline u_int16_t
HFSntohs(const void *ov)
{
	unsigned const char	*ovb = (unsigned const char *)ov;

	return(((u_int16_t)ovb[0] << 8) + (u_int16_t)ovb[1]);
}

static inline short
HFShtons(const unsigned short ov)
{
	unsigned short	nv;
	unsigned char	*nvb = (unsigned char *)&nv;

	nvb[0] = (ov >> 8);
	nvb[1] = ov & 0xff;
	
	return(nv);    
}


#endif /* ! _BOOTX_FS_H_ */
