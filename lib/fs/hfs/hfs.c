/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
/*
 *  hfs.c - File System Module for HFS and HFS+.
 *
 *  Copyright (c) 1999-2004 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include <debug.h>
#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/libc.h>
#include <string.h>
#include "hfs.h"
#include "hfs_format.h"

#define HFS_MAXPATHLEN (512)
#define HFS_MAXNAMLEN (255)

#define kBlockSize (0x200)
#define kBlockSizeMax	(0x10000)

#define kMDBBaseOffset (2 * kBlockSize)

#define kBTreeCatalog (0)
#define kBTreeExtents (1)
#define kBTreeSize (256)
#define kDirLevelMax (48)

static CICell                  gCurrentIH;
static off_t                   gAllocationOffset;
static bool                    gIsHFSPlus;
static uint32_t                gBlockSize;
static uint8_t                 gBTreeHeaderBuffer[HFS_MAXPATHLEN];
static BTHeaderRec             *gBTHeaders[2];
static uint8_t                 gHFSPlusHeader[kBlockSize];
static HFSPlusVolumeHeader     *gHFSPlus =(HFSPlusVolumeHeader*)gHFSPlusHeader;
static uint64_t                gVolID;
static char                    gTempStr[kBTreeSize];

static int ReadFile(void *file, size_t *length, void *base, off_t offset);
static int GetCatalogEntryInfo(void *entry, uint32_t *flags, time_t *time, off_t *size);
static int ResolvePathToCatalogEntry(const char *filePath, uint32_t *flags, void *entry, uint32_t dirID, off_t *dirIndex, time_t *time, off_t *size);

static int GetCatalogEntry(off_t *dirIndex, char *name, size_t nameSize, uint32_t *flags, time_t *time, off_t *size);
static int ReadCatalogEntry(const char *fileName, uint32_t dirID, void *entry, off_t *dirIndex);
static int ReadExtentsEntry(uint32_t fileID, uint32_t startBlock, void *entry);

static int ReadBTreeEntry(uint32_t btree, void *key, uint8_t *entry, off_t *dirIndex);
static int GetBTreeRecord(uint32_t index, uint8_t *nodeBuffer, uint32_t nodeSize, uint8_t **key, uint8_t **data);

static int ReadExtent(uint8_t *extent, uint32_t extentSize, uint32_t extentFile, off_t offset, uint32_t size, void *buffer, bool cache);

static uint32_t GetExtentStart(void *extents, uint32_t index);
static uint32_t GetExtentSize(void *extents, uint32_t index);

static int CompareHFSPlusCatalogKeys(void *key, void *testKey);
static int CompareHFSPlusExtentsKeys(void *key, void *testKey);

static int32_t BinaryUnicodeCompare (uint16_t * str1, uint32_t length1, uint16_t * str2, uint32_t length2);
static int utf_encodestr(const uint16_t *ucsp, unsigned ucslen, uint8_t *utf8p, size_t bufsize);
static void utf_decodestr(const char *utf8p, uint16_t *ucsp, uint16_t *ucslen, uint32_t bufsize);
static void utf_decodestr255(const char *utf8p, struct HFSUniStr255 *ucsstrp);


#if 0
# define HFSdebug(fmt, args...)	printf("%s: " fmt "\n", __FUNCTION__ , ##args)
static inline void
DPrintUTF(uint16_t *ucsp, int length)
{
	while (length--)
		printf("%c", (*(ucsp++) & 0xff00) >> 8);
}
#else
# define HFSdebug(fmt, args...)	do { } while(0)
# define DPrintUTF(s, l)	do { } while(0)
#endif

static bool
isPowerOf2(uint32_t x)
{
	return ((x & (x-1)) == 0);
}


// Sanity-checks some values in the HFS+ header
static bool
ValidateHFSPlusHeader(void)
{
	bool valid = true;
	uint32_t blockSize = HFSntohl(&gHFSPlus->blockSize);
	
	// check signature field
	if ((HFSntohs(&gHFSPlus->signature) != kHFSPlusSigWord) && (HFSntohs(&gHFSPlus->signature) != kHFSXSigWord)) {
		dprintf(DEBUG_CRITICAL, "Not HFS+ (signature 0x%04x)\n", HFSntohs(&gHFSPlus->signature));
		valid = false;
		goto exit;
	}
	
	// check blockSize field
	if ((blockSize < kBlockSize) || (blockSize > kBlockSizeMax) || (!isPowerOf2(blockSize))) {
		dprintf(DEBUG_CRITICAL, "Bad HFS+ blockSize(0x%08x, 0x%x, 0x%x)\n", blockSize, kBlockSize, kBlockSizeMax);
		valid = false;
		goto exit;
	}
	
exit:
	return valid;
}


static int
SanityCheckBTreeHeader(BTHeaderRec *hdr, off_t fileSize)
{
	uint16_t nodeSize = HFSntohs(&hdr->nodeSize);

	if (!isPowerOf2(nodeSize)) {
		HFSdebug("BTree node size is %d, which is not a power of 2\n", nodeSize);
		return -1;
	}

	if (nodeSize < 512) {
		HFSdebug("BTree node size is %d, minimum is 512\n", nodeSize);
		return -1;
	}
	if ((fileSize / nodeSize) != HFSntohl(&hdr->totalNodes)) {
		HFSdebug("BTree file size indicates %u nodes, header indicates %u\n", (uint32_t)(fileSize / nodeSize), HFSntohl(&hdr->totalNodes));
		return -1;
	}
	return 0;
}

int
HFSInitPartition(CICell ih)
{
	uint32_t extentSize, extentFile, nodeSize, btree;
	uint32_t result;
	void *extent;
  
	if (ih == gCurrentIH)
		return 0;
	dprintf(DEBUG_INFO, "HFSInitPartition: %p\n", ih);
	
	gAllocationOffset = 0;
	gIsHFSPlus = 0;
	gBTHeaders[0] = 0;
	gBTHeaders[1] = 0;
  
	// Look for the HFSPlus Header
	result = HFSBlockRead(ih, gHFSPlusHeader, gAllocationOffset + kMDBBaseOffset, kBlockSize);
	if (result != kBlockSize) {
		HFSdebug("Error reading HFS+ header");
		return -1;
	}
  
	// Verify contents of header before continuing
	if (!ValidateHFSPlusHeader()) {
		gCurrentIH = NULL;
		return -1;
	}

	gIsHFSPlus = 1;
	gBlockSize = HFSntohl(&gHFSPlus->blockSize);
	HFSdebug("allocation block size %x", gBlockSize);
	if (gBlockSize == 0) {
		HFSdebug("Volume block size cannot be zero!");
		return -1;
	}

	if (HFSntohl(&gHFSPlus->totalBlocks) == 0) {
		HFSdebug("Volume totalBlocks cannot be zero!");
		return -1;
	}

	CacheInit(ih, gBlockSize);
	gCurrentIH = ih;
  
	// grab the 64 bit volume ID
	memcpy(&gVolID, &gHFSPlus->finderInfo[24], 8);

	// Get the Catalog BTree header
	btree      = kBTreeCatalog;
	extent     = &gHFSPlus->catalogFile.extents;
	extentSize = HFSntohll(&gHFSPlus->catalogFile.logicalSize);
	if (extentSize < 512) {
		HFSdebug("Catalog btree file is too short, %lld\n", (off_t)extentSize);
		return -1;
	}
	extentFile = kHFSCatalogFileID;
	HFSdebug("reading catalog Btree, extent %p extentSize %x, extentFile %d", extent, extentSize, extentFile);
	if (ReadExtent(extent, extentSize, extentFile, 0, kBTreeSize, gBTreeHeaderBuffer + btree * kBTreeSize, 0) != kBTreeSize) {
		HFSdebug("Cannot read catalog file header node");
		return -1;
	}
	gBTHeaders[btree] = (BTHeaderRec *)(gBTreeHeaderBuffer + btree * kBTreeSize + /*sizeof(BTNodeDescriptor)*/ 14);

	HFSdebug("Catalog BTree header: %p", gBTHeaders[btree]);
	HFSdebug("  maximum height (usually leaf nodes)      %x", HFSntohs(&gBTHeaders[btree]->treeDepth));
	HFSdebug("  node number of root node                 %x", HFSntohl(&gBTHeaders[btree]->rootNode));
	HFSdebug("  number of leaf records in all leaf nodes %x", HFSntohl(&gBTHeaders[btree]->leafRecords));
	HFSdebug("  node number of first leaf node           %x", HFSntohl(&gBTHeaders[btree]->firstLeafNode));
	HFSdebug("  node number of last leaf node            %x", HFSntohl(&gBTHeaders[btree]->lastLeafNode));
	HFSdebug("  size of a node, in bytes                 %x", HFSntohs(&gBTHeaders[btree]->nodeSize));
	HFSdebug("  total number of nodes in tree            %x", HFSntohl(&gBTHeaders[btree]->totalNodes));
	HFSdebug("  number of unused (free) nodes in tree    %x", HFSntohl(&gBTHeaders[btree]->freeNodes));
	HFSdebug("  Key string Comparison Type               %x", gBTHeaders[btree]->keyCompareType);
	HFSdebug("  persistent attributes about the tree     %x", HFSntohl(&gBTHeaders[btree]->attributes));
	if (SanityCheckBTreeHeader(gBTHeaders[btree], extentSize) == -1) {
		return -1;
	}
	// Get the Extents BTree header
	btree      = kBTreeExtents;
	extent     = &gHFSPlus->extentsFile.extents;
	extentSize = HFSntohll(&gHFSPlus->extentsFile.logicalSize);
	if (extentSize < 512) {
		HFSdebug("Extents overflow btree file is too short, %lld\n", (off_t)extentSize);
		return -1;
	}
	extentFile = kHFSExtentsFileID;
	if (ReadExtent(extent, extentSize, extentFile, 0, kBTreeSize, gBTreeHeaderBuffer + btree * kBTreeSize, 0) != kBTreeSize) {
		HFSdebug("Cannot read extents overflow file header node");
		return -1;
	}
	gBTHeaders[btree] = (BTHeaderRec *)(gBTreeHeaderBuffer + btree * kBTreeSize + /*sizeof(BTNodeDescriptor)*/ 14);

	HFSdebug("Extents BTree header:");
	HFSdebug("  maximum height (usually leaf nodes)      %x", HFSntohs(&gBTHeaders[btree]->treeDepth));
	HFSdebug("  node number of root node                 %x", HFSntohl(&gBTHeaders[btree]->rootNode));
	HFSdebug("  number of leaf records in all leaf nodes %x", HFSntohl(&gBTHeaders[btree]->leafRecords));
	HFSdebug("  node number of first leaf node           %x", HFSntohl(&gBTHeaders[btree]->firstLeafNode));
	HFSdebug("  node number of last leaf node            %x", HFSntohl(&gBTHeaders[btree]->lastLeafNode));
	HFSdebug("  size of a node, in bytes                 %x", HFSntohs(&gBTHeaders[btree]->nodeSize));
	HFSdebug("  total number of nodes in tree            %x", HFSntohl(&gBTHeaders[btree]->totalNodes));
	HFSdebug("  number of unused (free) nodes in tree    %x", HFSntohl(&gBTHeaders[btree]->freeNodes));
	HFSdebug("  Key string Comparison Type               %x", gBTHeaders[btree]->keyCompareType);
	HFSdebug("  persistent attributes about the tree     %x", HFSntohl(&gBTHeaders[btree]->attributes));
	if (SanityCheckBTreeHeader(gBTHeaders[btree], extentSize) == -1) {
		return -1;
	}

	// If the BTree node size is larger than the block size, reset the cache.
	nodeSize = HFSntohs(&gBTHeaders[btree]->nodeSize);
	if (nodeSize > gBlockSize)
		CacheInit(ih, nodeSize);

	return 0;
}


int
HFSDetect(void)
{

	if (gCurrentIH == NULL) {
		dprintf(DEBUG_CRITICAL, "No current device\n");
		return 1;
	}
	dprintf(DEBUG_INFO, "Filesystem: HFS+\n signature 0x%04x version 0x%04x\n", HFSntohs(&gHFSPlus->signature), HFSntohs(&gHFSPlus->version));
	dprintf(DEBUG_INFO, " allocation block size %d bytes\n total block count %d\n volume size %d Mbytes\n",
	    HFSntohl(&gHFSPlus->blockSize), HFSntohl(&gHFSPlus->totalBlocks),
	    (int)((long long)HFSntohl(&gHFSPlus->blockSize) * HFSntohl(&gHFSPlus->totalBlocks) / (1024 * 1024)));
	dprintf(DEBUG_INFO, " %d files\n %d folders\n", HFSntohl(&gHFSPlus->fileCount), HFSntohl(&gHFSPlus->folderCount));
	dprintf(DEBUG_INFO, " created 0x%08x\n modified 0x%08x\n backed up 0x%08x\n checked 0x%08x\n",
	    HFSntohl(&gHFSPlus->createDate), HFSntohl(&gHFSPlus->modifyDate),
	    HFSntohl(&gHFSPlus->backupDate), HFSntohl(&gHFSPlus->checkedDate));

	dprintf(DEBUG_INFO, " Allocation file\n");
	dprintf(DEBUG_INFO, "  logical size %d  clumpSize %d  totalBlocks %d\n",
	    (int)HFSntohll(&gHFSPlus->allocationFile.logicalSize),
	    HFSntohl(&gHFSPlus->allocationFile.clumpSize),
	    HFSntohl(&gHFSPlus->allocationFile.totalBlocks));
	dprintf(DEBUG_INFO, " Extents file\n");
	dprintf(DEBUG_INFO, "  logical size %d  clumpSize %d  totalBlocks %d\n",
	    (int)HFSntohll(&gHFSPlus->extentsFile.logicalSize),
	    HFSntohl(&gHFSPlus->extentsFile.clumpSize),
	    HFSntohl(&gHFSPlus->extentsFile.totalBlocks));
	dprintf(DEBUG_INFO, " Catalog file\n");
	dprintf(DEBUG_INFO, "  logical size %d  clumpSize %d  totalBlocks %d\n",
	    (int)HFSntohll(&gHFSPlus->catalogFile.logicalSize),
	    HFSntohl(&gHFSPlus->catalogFile.clumpSize),
	    HFSntohl(&gHFSPlus->catalogFile.totalBlocks));
	dprintf(DEBUG_INFO, " Attributes file\n");
	dprintf(DEBUG_INFO, "  logical size %d  clumpSize %d  totalBlocks %d\n",
	    (int)HFSntohll(&gHFSPlus->attributesFile.logicalSize),
	    HFSntohl(&gHFSPlus->attributesFile.clumpSize),
	    HFSntohl(&gHFSPlus->attributesFile.totalBlocks));
	
	return 0;
}


int
HFSGetFileInfo(CICell ih, const char *filePath, uint32_t *flags, time_t *time, off_t *size)
{
	char *entry;
	uint32_t dirID;
	int result;

	HFSdebug("fs %p  path %s", ih, filePath);

	entry = (char *)malloc(HFS_MAXPATHLEN);
	dirID = kHFSRootFolderID;
  
	result = ResolvePathToCatalogEntry(filePath, flags, entry, dirID, 0, time, size);

	free(entry);
	return result;
}

int
HFSReadFile(CICell ih, const char *filePath, void *base, off_t offset, size_t length)
{
	char *entry;
	uint32_t dirID, flags;
	int result;
  
	entry = (char *)malloc(HFS_MAXPATHLEN);

	dirID = kHFSRootFolderID;
  
	result = ResolvePathToCatalogEntry(filePath, &flags, entry, dirID, 0, NULL, NULL);
	if (result != 0) {
		dprintf(DEBUG_CRITICAL, "Could not find '%s'\n", filePath);
		goto exit;
	}
	if ((flags & kFileTypeMask) != kFileTypeFlat) {
		dprintf(DEBUG_CRITICAL, "'%s' is not a file\n", filePath);
		result = -1;
		goto exit;
	}
  
	result = ReadFile(entry, &length, base, offset);
	if (result == 0)
		result = length;

exit:
	HFSdebug("returning %d\n", result);
	free(entry);
	return result;
}

int
HFSGetDirEntry(CICell ih, const char *dirPath, off_t *dirIndex, char *name, size_t nameSize, uint32_t *flags, time_t *time, off_t *size)
{
	char *entry;
	uint32_t dirID, dirFlags;
	int ret = 0;
  
	if (*dirIndex == -1)
		return -1;

	entry = (char *)malloc(HFS_MAXPATHLEN);
	dirID = kHFSRootFolderID;
  
	if (*dirIndex == 0) {
		ret = ResolvePathToCatalogEntry(dirPath, &dirFlags, entry, dirID, dirIndex, NULL, NULL);
		if (ret != 0) {
			ret = -1;
			goto exit;
		}

		if (*dirIndex == 0)
			*dirIndex = -1;
		/* if what we looked up didn't give us the header entry for the directory, bail */
		if ((dirFlags & kFileTypeMask) != kFileTypeUnknown) {
			dprintf(DEBUG_CRITICAL, "not a directory: '%s' flags %x\n", dirPath, dirFlags);
			ret = -1;
			goto exit;
		}
	}
  
	if (GetCatalogEntry(dirIndex, name, nameSize, flags, time, size) != 0) {
		ret = -1;
		goto exit;
	}

	/* end of the catalog or end of the directory */
	if ((*dirIndex == 0) || ((*flags & kFileTypeMask) == kFileTypeUnknown))
		*dirIndex = -1;

exit:
	free(entry);
	return ret;
}

struct HFSPlusVolumeHeader *
HFSGetVolumeHeader(void)
{
	return(gHFSPlus);
}

// Private Functions

static int
ReadFile(void *file, size_t *length_inout, void *base, off_t offset)
{
	size_t             length;
	void               *extents;
	uint32_t           fileID, fileLength;
	HFSPlusCatalogFile *hfsPlusFile = file;
	int rv;

	fileID  = HFSntohl(&hfsPlusFile->fileID);
	fileLength = HFSntohll(&hfsPlusFile->dataFork.logicalSize);
	extents = &hfsPlusFile->dataFork.extents;

	length = *length_inout;

	// nothing to do if we're asked to read 0 bytes
	if (length == 0) {
		return 0;
	}

	// Our parameter is a size_t, but the rest of the library treats
	// lengths as uint32_t
	if (length > UINT32_MAX) {
		dprintf(DEBUG_CRITICAL, "Length is too large.\n");
		return -1;
	}
  
	if (offset > UINT32_MAX || offset > fileLength || offset < 0) {
		dprintf(DEBUG_CRITICAL, "Offset is too large.\n");
		return -1;
	}

	if ((size_t)offset + length < length) {
		dprintf(DEBUG_CRITICAL, "Length wraparound.\n");
		return -1;
	}
  
	if ((offset + length) > fileLength) {
		length = fileLength - offset;
	}
  
	rv = ReadExtent((uint8_t *)extents, fileLength, fileID,
	    offset, length, base, 0);
	if (rv < 0) {
		return -1;
	}
	*length_inout = rv;
  
	return 0;
}

static int
GetCatalogEntryInfo(void *entry, uint32_t *flags, time_t *time, off_t *size)
{
	uint32_t tmpTime = 0;
	uint64_t tmpSize = 0;

	HFSdebug("fetching information for file type 0x%04x", HFSntohs((uint16_t *)entry));
	
	// Get information about the file.
	switch (HFSntohs((uint16_t *)entry)) {
	case kHFSFolderRecord           :
		*flags = kFileTypeDirectory;
		tmpTime = HFSntohl((uint32_t *)&((HFSCatalogFolder *)entry)->modifyDate);
		break;
    
	case kHFSPlusFolderRecord       :
		*flags = kFileTypeDirectory |
		    (HFSntohs(&((HFSPlusCatalogFolder *)entry)->bsdInfo.fileMode) & kPermMask);
		tmpTime = HFSntohl(&((HFSPlusCatalogFolder *)entry)->contentModDate);
		break;
    
	case kHFSFileRecord             :
		*flags = kFileTypeFlat;
		tmpSize = HFSntohl(&((HFSCatalogFile *)entry)->dataLogicalSize);
		tmpTime = HFSntohl(&((HFSCatalogFile *)entry)->modifyDate);
		break;
    
	case kHFSPlusFileRecord         :
		*flags = kFileTypeFlat |
		    (HFSntohs(&((HFSPlusCatalogFile *)entry)->bsdInfo.fileMode) & kPermMask);
		tmpSize = HFSntohll(&((HFSPlusCatalogFile *)entry)->dataFork.logicalSize);
		tmpTime = HFSntohl(&((HFSPlusCatalogFile *)entry)->contentModDate);
		HFSdebug("file size is %lld", tmpSize);
		break;
    
	case kHFSFileThreadRecord       :
	case kHFSPlusFileThreadRecord   :
	case kHFSFolderThreadRecord     :
	case kHFSPlusFolderThreadRecord :
		*flags = kFileTypeUnknown;
		tmpTime = 0;
		break;
	default:
		return -1;
	}
  
	if (time != 0) {
		// Convert base time from 1904 to 1970.
		*time = tmpTime - 2082844800;
	}
	if (size != 0)
		*size = tmpSize;
  
	return 0;
}

static int
ResolvePathToCatalogEntry(const char *filePath, uint32_t *flags, void *entry, uint32_t dirID, off_t *dirIndex, time_t *time, off_t *size)
{
	const char	*restPath;
	int             result;
	uint32_t	cnt,  subFolderID = 0;
	static uint32_t	recursionCnt = 0;

	// Copy the file name to gTempStr
	cnt = 0;
	result = -1;

	if (kDirLevelMax <= recursionCnt++) {
		HFSdebug("Too many levels of directories");
		goto exit;
	}

	while ((filePath[cnt] != '/') && (filePath[cnt] != '\0'))
		cnt++;

	if (cnt >= sizeof(gTempStr)) {
		HFSdebug("String too long");
		goto exit;
	}
	strlcpy(gTempStr, filePath, cnt + 1);
  
	// Move restPath to the right place.
	if (filePath[cnt] != '\0')
		cnt++;
	restPath = filePath + cnt;
  
	// gTempStr is a name in the current Dir.
	// restPath is the rest of the path if any.
	HFSdebug("searching for '%s' with remaining path '%s'", gTempStr, restPath);
  
	result = ReadCatalogEntry(gTempStr, dirID, entry, dirIndex);
	if (result < 0) {
		HFSdebug("not found");
		goto exit;
	}
	if (dirIndex != 0)
		HFSdebug("found at index %llx", *dirIndex);
	
	if (GetCatalogEntryInfo(entry, flags, time, size) < 0) {
		HFSdebug("bad catalog entry info");
		result = -1;
		goto exit;
	}

	// recursively look up child
	if ((*flags & kFileTypeMask) == kFileTypeDirectory) {
		subFolderID = HFSntohl(&((HFSPlusCatalogFolder *)entry)->folderID);
		HFSdebug("looking up '%s' in '%s'", restPath, gTempStr);
		result = ResolvePathToCatalogEntry(restPath, flags, entry, subFolderID, dirIndex, time, size);
		if (result != 0) {
			result = -1;
			goto exit;
		}
	} else {
		// if we weren't on the last element of the path, then
		// we should have found a directory
		if (*restPath != '\0') {
			HFSdebug("expected directory");
			result = -1;
			goto exit;
		}
	}

exit: 
	HFSdebug("returning %d", result);
	recursionCnt--;
	return result;
}

static int
VerifyCatalogKeyBounds(HFSPlusCatalogKey *key, uint8_t *bufend)
{
	int ret = 0;

	// Verify key length is contained inside the node 
	if ((uint8_t *)&key->keyLength + sizeof(key->keyLength) > bufend) {
		HFSdebug("keyLength outside of node");
		ret = -1;
		goto exit;
	}
	// Verify end of key doesn't overflow past end of node
	if ((uint8_t *)key + HFSntohs(&key->keyLength) > bufend) {
		HFSdebug("Bad keyLength (ends at %p, bufend %p)", (uint8_t *)key + HFSntohs(&key->keyLength), bufend);
		ret = -1;
		goto exit;
	}
	// There are actually two lengths to check, the second one gives the
	// number of UCS-2 (16-bit) characters in the name
	if ((uint8_t *)&key->nodeName.unicode + HFSntohs(&key->nodeName.length) * 2 > bufend) {
		HFSdebug("Bad unicode length");
		ret = -1;
		goto exit;
	}

exit:
	return ret;
}

static int
GetCatalogEntry(off_t *dirIndex, char *name, size_t nameSize, uint32_t *flags, time_t *time, off_t *size)
{
	uint32_t          extentSize, nodeSize, curNode, index;
	void              *extent;
	uint8_t           *nodeBuf, *testKey, *entry;
	HFSPlusCatalogKey *catalogKey;
	BTNodeDescriptor  *node;
	int               result;
	int               ret = 0;
  
	extent     = &gHFSPlus->catalogFile.extents;
	extentSize = HFSntohll(&gHFSPlus->catalogFile.logicalSize);
  
	nodeSize = HFSntohs(&gBTHeaders[kBTreeCatalog]->nodeSize);
	nodeBuf = malloc(nodeSize);
	HFSdebug("got nodeBuf %p", nodeBuf);
	node = (BTNodeDescriptor *)nodeBuf;

	HFSdebug("dirIndex %p/%lld nodeS5ze %u", dirIndex, *dirIndex, nodeSize);
	index   = *dirIndex % nodeSize;
	curNode = *dirIndex / nodeSize;
	HFSdebug("working with index %d curNode %d", index, curNode);
  
	// Read the BTree node and get the record for index.
	result = ReadExtent(extent, extentSize, kHFSCatalogFileID, curNode * nodeSize, nodeSize, nodeBuf, 1);
	if (result < 0 || (unsigned)result != nodeSize) {
		ret = -1;
		goto exit;
	}
	if (GetBTreeRecord(index, nodeBuf, nodeSize, &testKey, &entry) != 0) {
		ret = -1;
		goto exit;
	}
	catalogKey = (HFSPlusCatalogKey *)testKey;

	// Check for overflow
	if (VerifyCatalogKeyBounds(catalogKey, nodeBuf + nodeSize) != 0) {
		ret = -1;
		goto exit;
	}

	HFSdebug("got BTnode and index record");
  
	if (GetCatalogEntryInfo(entry, flags, time, size) != 0) {
		ret = -1;
		goto exit;
	}
	HFSdebug("got entry info");

	// Get the file name.
	if (utf_encodestr(catalogKey->nodeName.unicode,
	                  HFSntohs((uint16_t *)&catalogKey->nodeName.length),
	                  (uint8_t *)name, nameSize) <= 0) {
		HFSdebug("failed to UTF encode filename");
		ret = -1;
		goto exit;
	}
	HFSdebug("got file name");
  
	// Update dirIndex.
	index++;
	if (index == HFSntohs(&node->numRecords)) {
		index = 0;
		curNode = HFSntohl(&node->fLink);
	}

	*dirIndex = curNode * nodeSize + index;
	HFSdebug("updated forward sibling index");
	
exit:
	free(nodeBuf);
  
	HFSdebug("returning %d", ret);
	return ret;
}

static int
ReadCatalogEntry(const char *fileName, uint32_t dirID, void *entry, off_t *dirIndex)
{
	uint32_t          length;
	HFSPlusCatalogKey *hfsPlusKey;
	int                ret;

	length = strlen(fileName);
	if (length > HFS_MAXNAMLEN) {
		HFSdebug("length overflowed");
		return -1;
	}
  
	hfsPlusKey = malloc(sizeof(HFSPlusCatalogKey));
	// Make the catalog key.
	hfsPlusKey->parentID = dirID;
	utf_decodestr255((char *)fileName, &hfsPlusKey->nodeName);
	HFSdebug("UTF encoded '%s' from length %d to length %d", fileName, length, (int)hfsPlusKey->nodeName.length * 2);

	ret = ReadBTreeEntry(kBTreeCatalog, (char *)hfsPlusKey, entry, dirIndex);
	free(hfsPlusKey);
	return ret;
}

static int
ReadExtentsEntry(uint32_t fileID, uint32_t startBlock, void *entry)
{
	HFSPlusExtentKey hfsPlusKey;
  
	// Make the extents key.
	hfsPlusKey.forkType   = 0;
	hfsPlusKey.fileID     = fileID;
	hfsPlusKey.startBlock = startBlock;
  
	return ReadBTreeEntry(kBTreeExtents, &hfsPlusKey, entry, 0);
}

static int
ReadBTreeEntry(uint32_t btree, void *key, uint8_t *entry, off_t *dirIndex)
{
	uint32_t         extentSize;
	void             *extent;
	uint16_t         extentFile;
	BTHeaderRec      *headerRec;
	uint8_t          *nodeBuf;
	BTNodeDescriptor *node;
	uint32_t         nodeSize, entrySize = 0;
	uint32_t         curNode, index = 0;
	int32_t          numRecords, lowerBound, upperBound;
	uint8_t          *testKey, *recordData;
	uint32_t         level;
	int              extentLen;
	int              result = 0;
	int              ret = 0;
	
	// Figure out which tree is being looked at.
	if (btree == kBTreeCatalog) {
		HFSdebug("working with the catalog BTree");
		extent     = &gHFSPlus->catalogFile.extents;
		extentSize = HFSntohll(&gHFSPlus->catalogFile.logicalSize);
		extentFile = kHFSCatalogFileID;
	} else {
		HFSdebug("working with the extents BTree");
		extent     = &gHFSPlus->extentsFile.extents;
		extentSize = HFSntohll(&gHFSPlus->extentsFile.logicalSize);
		extentFile = kHFSExtentsFileID;
	}

	headerRec = gBTHeaders[btree];
	ASSERT(headerRec != NULL);
  
	curNode = HFSntohl(&headerRec->rootNode);
	level = HFSntohs(&headerRec->treeDepth);
	
	nodeSize = HFSntohs(&headerRec->nodeSize);
	nodeBuf = malloc(nodeSize);
	node = (BTNodeDescriptor *)nodeBuf;
	HFSdebug("btree %d headers %p curNode %x nodeSize %d nodeBuf %p node %p",
	    btree, headerRec, curNode, nodeSize, nodeBuf, node);
  
	while (1) {
		HFSdebug("current node %x", curNode);
		
		// Node 0 is the header node, and is never an index or leaf node.
		if (curNode == 0) {
			ret = -1;
			goto exit;
		}

		// Read the current node.
		extentLen = ReadExtent(extent, extentSize, extentFile, curNode * nodeSize, nodeSize, nodeBuf, 1);
		if (extentLen < 0 || (unsigned)extentLen != nodeSize) {
			HFSdebug("short result from ReadExtent");
			ret = -1;
			goto exit;
		}
    
    		// Make sure the node's height and kind are correct, and numRecords is non-zero.
    		if (node->height != level) {
    			HFSdebug("Bad node: height is %d; expected %d", node->height, level);
    			ret = -1;
    			goto exit;
    		}
    		if (node->kind != (level == 1 ? kBTLeafNode : kBTIndexNode)) {
    			HFSdebug("Bad node: kind is %d; expected %d", node->kind,
    				(level == 1 ? kBTLeafNode : kBTIndexNode));
    			ret = -1;
    			goto exit;
    		}

		numRecords = HFSntohs(&node->numRecords);

		if (numRecords <= 0) {
    			HFSdebug("Bad node: numRecords must be positive");
    			ret = -1;
    			goto exit;
		}

		// Find the matching key.
		lowerBound = 0;
		upperBound = numRecords - 1;
		HFSdebug("node has %d records", upperBound + 1);
		while (lowerBound <= upperBound) {
			ASSERT(lowerBound >= 0 && upperBound >= 0);
			index = ((uint32_t)lowerBound + (uint32_t)upperBound) / 2;
			HFSdebug("current record %d (lower %d upper %d)", index, lowerBound, upperBound);
      
			if(GetBTreeRecord(index, nodeBuf, nodeSize, &testKey, &recordData) != 0) {
				HFSdebug("GetBTreeRecord failed");
				ret = -1;
				goto exit;
			}
      
			if (btree == kBTreeCatalog) {
				// catalog keys contain two lengths and GetBTreeRecord only validates
				// the first one, so check both lengths to be sure
				if (VerifyCatalogKeyBounds((HFSPlusCatalogKey *)testKey, nodeBuf + nodeSize) != 0) {
					HFSdebug("VerifyCatalogKeyBounds failed");
					ret = -1;
					goto exit;
				}
				result = CompareHFSPlusCatalogKeys(key, testKey);
			} else {
				// Unlike catalog keys, extent keys just have the one length
				// and GetBTreeRecord validates that length is in the buffer
				result = CompareHFSPlusExtentsKeys(key, testKey);
			}
			HFSdebug("compare result %d", result);
      
			if (result < 0)
				upperBound = index - 1;        // search < trial
			else if (result > 0)
				lowerBound = index + 1;   // search > trial
			else break;                                    // search = trial
		}
		HFSdebug("best match %d", index);
    
		if (result < 0) {
			index = upperBound;
			if(GetBTreeRecord(index, nodeBuf, nodeSize, &testKey, &recordData) != 0) {
				ret = -1;
				goto exit;
			}
		}
    
		// Found the closest key... Recurse on it if this is an index node.
		if (node->kind == kBTIndexNode) {
			HFSdebug("recursing on index node from %p", recordData);
			curNode = HFSntohl((uint32_t *)recordData);
		} else {
			break;
		}
		
		--level;
	}
  
	// Return error if the file was not found.
	if (result != 0) {
		HFSdebug("file not found");
		ret = -1;
		goto exit;
	}
  
	if (btree == kBTreeCatalog) {
		switch (HFSntohs((uint16_t *)recordData)) {
		case kHFSFolderRecord           : entrySize = 70;  break;
		case kHFSFileRecord             : entrySize = 102; break;
		case kHFSFolderThreadRecord     : entrySize = 46;  break;
		case kHFSFileThreadRecord       : entrySize = 46;  break;
		case kHFSPlusFolderRecord       : entrySize = 88;  break;
		case kHFSPlusFileRecord         : entrySize = 248; break;
		case kHFSPlusFolderThreadRecord : entrySize = 264; break;
		case kHFSPlusFileThreadRecord   : entrySize = 264; break;
		default:
			// we don't know how much to copy, so we have to fail
			ret = -1;
			goto exit;
		}
	} else {
		entrySize = sizeof(HFSPlusExtentRecord);
	}
	HFSdebug("entrySize %d", entrySize);

	// Check for overflow out of the node
	if ((uint8_t *)recordData + entrySize > nodeBuf + nodeSize) {
		ret = -1;
		goto exit;
	}
  
	memcpy(entry, recordData, entrySize);
  
	// Update dirIndex.
	if (dirIndex != 0) {
		index++;
		if (index == HFSntohs(&node->numRecords)) {
			index = 0;
			curNode = HFSntohl(&node->fLink);
		}
		*dirIndex = curNode * nodeSize + index;
	}
	
exit:
	free(nodeBuf);
  
	return ret;
}

static int
GetBTreeRecord(uint32_t index, uint8_t *nodeBuffer, uint32_t nodeSize, uint8_t **key, uint8_t **data)
{
	int ret = 0;
	uint32_t keySize;
	uint32_t recordOffset;
	
	// check index range
	if ((2 * index + 2) < index || (2 * index + 2) > nodeSize) {
		HFSdebug("index value (%d) out of range", index);
		ret = -1;
		goto exit;
	}
	
	// List of offsets for each record in the BTree node is at the end of the node,
	// in reverse order (highest index is closest to the start of the node)
	recordOffset = HFSntohs((uint16_t *)(nodeBuffer + (nodeSize - 2 * index - 2)));
	*key = nodeBuffer + recordOffset;
	
	// check key ptr range
	if (*key < nodeBuffer || *key > (nodeBuffer + nodeSize - 1)) {
		HFSdebug("key ptr (%p) out of range", *key);
		ret = -1;
		goto exit;
	}
	
	keySize = HFSntohs((uint16_t *)*key);
	*data = *key + 2 + keySize;
	
	// check data ptr range
	if (*data < nodeBuffer || *data > (nodeBuffer + nodeSize - 1)) {
		HFSdebug("data ptr (%p) out of range", *data);
		ret = -1;
		goto exit;
	}
	
	HFSdebug("record %d at offset %d (%p) keysize %d data at %p",
	    index, recordOffset, *key, keySize, *data);
	
exit:
	return ret;
}

static int
ReadExtent(uint8_t *extent, uint32_t extentSize, uint32_t extentFile, off_t offset, uint32_t size, void *buffer, bool cache)
{
	uint32_t  lastOffset, blockNumber, countedBlocks = 0;
	uint32_t  nextExtent = 0, sizeRead = 0, readSize;
	uint32_t  nextExtentBlock, currentExtentBlock = 0;
	off_t     readOffset;
	uint32_t  extentDensity, sizeofExtent, currentExtentSize;
	uint8_t   *currentExtent, *extentBuffer = 0, *bufferPos = buffer;
	int       result = 0;

	if (offset < 0) {
		return -1;
	}
  
	if (offset >= extentSize) {
		HFSdebug("offset too large");
		return 0;
	}
  
	extentDensity = kHFSPlusExtentDensity;
	sizeofExtent = sizeof(HFSPlusExtentDescriptor);
  
	lastOffset = offset + size;
	HFSdebug("offset %llx  size %x", offset, size);
	while (offset < lastOffset) {
		blockNumber = offset / gBlockSize;
		HFSdebug("file blockNumber %x", blockNumber);
    
		// Find the extent for the offset.
		for (; ; nextExtent++) {
			HFSdebug("nextExtent %x  countedBlocks %x", nextExtent, countedBlocks);
			if (nextExtent < extentDensity) {
				if ((countedBlocks + GetExtentSize(extent, nextExtent) - 1) < blockNumber) {
					HFSdebug("skipping early extent %x blocks", GetExtentSize(extent, nextExtent));
					countedBlocks += GetExtentSize(extent, nextExtent);
					continue;
				}
	
				currentExtent = extent + nextExtent * sizeofExtent;
				HFSdebug("found offset %llx in extent %x", offset, nextExtent);
				break;
			}
      
			if (extentBuffer == 0) {
				extentBuffer = malloc(sizeofExtent * extentDensity);
				HFSdebug("extent buffer at %p", extentBuffer);
			}
      
			nextExtentBlock = nextExtent / extentDensity;
			if (currentExtentBlock != nextExtentBlock) {
				// Special Case: Extents File cannot have an extents-overflow
				if (extentFile == kHFSExtentsFileID) {
				  panic("Attempt to read overflow extents for the Extents-overflow file");
				}

				// For all other cases: Read the overflow extent entry for this file from the Extents file
				if (ReadExtentsEntry(extentFile, countedBlocks, extentBuffer) != 0) {
					result = -1;
					goto exit;
				}
				currentExtentBlock = nextExtentBlock;
			}
      
			currentExtentSize = GetExtentSize(extentBuffer, nextExtent % extentDensity);
      
			if ((countedBlocks + currentExtentSize - 1) >= blockNumber) {
				currentExtent = extentBuffer + sizeofExtent * (nextExtent % extentDensity);
				break;
			}
      
			countedBlocks += currentExtentSize;
		}
    
		readOffset = ((blockNumber - countedBlocks) * gBlockSize) + (offset % gBlockSize);
    
		readSize = GetExtentSize(currentExtent, 0) * gBlockSize - readOffset;
		if (readSize > (size - sizeRead))
			readSize = size - sizeRead;

		if (readSize == 0)
			break;

		readOffset += (long long)GetExtentStart(currentExtent, 0) * gBlockSize;

		HFSdebug("doing cache read %x/%x", (int)readOffset, readSize);
		if (CacheRead(gCurrentIH, bufferPos, gAllocationOffset + readOffset, readSize, cache) != readSize) {
			result = -1;
			goto exit;
		}
    
		sizeRead += readSize;
		offset += readSize;
		bufferPos += readSize;
	}

exit:
	if (extentBuffer)
		free(extentBuffer);

	if (result == 0)
		return sizeRead;
	else
		return result;
}

static uint32_t
GetExtentStart(void *extents, uint32_t index)
{
	uint32_t                start;
	HFSPlusExtentDescriptor *hfsPlusExtents = extents;
  
	start = HFSntohl(&hfsPlusExtents[index].startBlock);
  
	return start;
}

static uint32_t
GetExtentSize(void *extents, uint32_t index)
{
	uint32_t                size;
	HFSPlusExtentDescriptor *hfsPlusExtents = extents;
  
	size = HFSntohl(&hfsPlusExtents[index].blockCount);
  
	return size;
}

/*
 * key native endian, testKey FS endian
 */
static int
CompareHFSPlusCatalogKeys(void *key, void *testKey)
{
	HFSPlusCatalogKey *searchKey, *trialKey;
	uint32_t          searchParentID, trialParentID;
	int               result;
  
	searchKey = key;
	trialKey  = testKey;
  
	searchParentID = searchKey->parentID;
	trialParentID = HFSntohl(&trialKey->parentID);
	HFSdebug("SearchParentID %x  trialParentID %x", searchParentID, trialParentID);
  
	// parent dirID is unsigned
	if (searchParentID > trialParentID)
		result = 1;
	else if (searchParentID < trialParentID)
		result = -1;
	else {
#if 0
		printf("        search name '");
		DPrintUTF(searchKey->nodeName.unicode, searchKey->nodeName.length);
		printf("' trial name '");
		DPrintUTF(trialKey->nodeName.unicode, HFSntohs(&trialKey->nodeName.length));
		printf("'\n");
#endif
		// parent dirID's are equal, compare names
		if ((searchKey->nodeName.length == 0) || (HFSntohs(&trialKey->nodeName.length) == 0)) {
			HFSdebug("search name length %d  trial name length %d",
			    searchKey->nodeName.length, HFSntohs(&trialKey->nodeName.length));
			result = searchKey->nodeName.length - HFSntohs(&trialKey->nodeName.length);
		} else {
			result = BinaryUnicodeCompare(&searchKey->nodeName.unicode[0],
			    searchKey->nodeName.length,
			    &trialKey->nodeName.unicode[0],
			    HFSntohs(&trialKey->nodeName.length));
		}
	}
  
	return result;
}

/*
 * key native endian, testKey FS endian
 */
static int
CompareHFSPlusExtentsKeys(void *key, void *testKey)
{
	HFSPlusExtentKey *searchKey, *trialKey;
	int              result;
  
	searchKey = key;
	trialKey  = testKey;
  
	// assume searchKey < trialKey
	result = -1;            
  
	if (searchKey->fileID == HFSntohl(&trialKey->fileID)) {
		// FileNum's are equal; compare fork types
		if (searchKey->forkType == trialKey->forkType) {
			// Fork types are equal; compare allocation block number
			if (searchKey->startBlock == HFSntohl(&trialKey->startBlock)) {
				// Everything is equal
				result = 0;
			} else {
				// Allocation block numbers differ; determine sign
				if (searchKey->startBlock > HFSntohl(&trialKey->startBlock))
					result = 1;
			}
		} else {
			// Fork types differ; determine sign
			if (searchKey->forkType > trialKey->forkType)
				result = 1;
		}
	} else {
		// FileNums differ; determine sign
		if (searchKey->fileID > HFSntohl(&trialKey->fileID))
			result = 1;
	}
  
	return result;
}

//
//  BinaryUnicodeCompare - Compare two Unicode strings; produce a relative ordering
//  Compared using a 16-bit binary comparison (no case folding)
//
static int32_t
BinaryUnicodeCompare (uint16_t * str1, uint32_t length1, uint16_t * str2, uint32_t length2)
{
	register uint16_t c1, c2;
	int32_t bestGuess;
	uint32_t length;

	bestGuess = 0;

	if (length1 < length2) {
		length = length1;
		--bestGuess;
	} else if (length1 > length2) {
		length = length2;
		++bestGuess;
	} else {
		length = length1;
	}

	while (length--) {
		c1 = HFSntohs(str1++);
		c2 = HFSntohs(str2++);

		if (c1 > c2)
			return (1);
		if (c1 < c2)
			return (-1);
	}

	return (bestGuess);
}

/*
 * UTF-8 (UCS Transformation Format)
 *
 * The following subset of UTF-8 is used to encode UCS-2 filenames. It
 * requires a maximum of three 3 bytes per UCS-2 character.  Only the
 * shortest encoding required to represent the significant UCS-2 bits
 * is legal.
 * 
 * UTF-8 Multibyte Codes
 *
 * Bytes   Bits   UCS-2 Min   UCS-2 Max   UTF-8 Byte Sequence (binary)
 * -------------------------------------------------------------------
 *   1       7     0x0000      0x007F      0xxxxxxx
 *   2      11     0x0080      0x07FF      110xxxxx 10xxxxxx
 *   3      16     0x0800      0xFFFF      1110xxxx 10xxxxxx 10xxxxxx
 * -------------------------------------------------------------------
 */

/*
 * utf_encodestr - Encodes the UCS-2 (Unicode) string at ucsp into a
 * null terminated UTF-8 string at utf8p.
 *
 * inbufsize is the input buffer length in bytes
 * ucslen is the number of UCS-2 input characters (not bytes)
 * outbufsize is the size of the output buffer in bytes
 * 
 */
static int
utf_encodestr(const uint16_t *ucsp, unsigned ucslen, uint8_t *utf8p, size_t outbufsize)
{
	uint8_t *bufstart;
	uint8_t *bufend;
	uint16_t ucs_ch;

	if (outbufsize <= 0)
		return -1;
	
	bufstart = utf8p;
	bufend = utf8p + outbufsize - 1;

	while (ucslen-- > 0) {
		ucs_ch = HFSntohs(ucsp++);

		if (ucs_ch < 0x0080) {
			if (utf8p >= bufend) {
				HFSdebug("1 byte character past end of buffer");
				return -1;
			}
			if (ucs_ch == '\0')
				continue;	/* skip over embedded NULLs */
			*utf8p++ = ucs_ch;

		} else if (ucs_ch < 0x800) {
			if ((utf8p + 1) >= bufend) {
				HFSdebug("2 byte character past end of buffer");
				return -1;
			}
			*utf8p++ = (ucs_ch >> 6)   | 0xc0;
			*utf8p++ = (ucs_ch & 0x3f) | 0x80;

		} else {
			if ((utf8p + 2) >= bufend) {
				HFSdebug("3 byte character past end of buffer");
				return -1;
			}
			*utf8p++ = (ucs_ch >> 12)         | 0xe0;
			*utf8p++ = ((ucs_ch >> 6) & 0x3f) | 0x80;
			*utf8p++ = ((ucs_ch) & 0x3f)      | 0x80;
		}
		
	}
	
	*utf8p = '\0';

	return utf8p - bufstart;
}

static void
utf_decodestr255(const char *utf8p, struct HFSUniStr255 *ucsstrp)
{
	utf_decodestr(utf8p, ucsstrp->unicode, &ucsstrp->length, sizeof(ucsstrp->unicode));
}

/*
 * utf_decodestr - Decodes the null terminated UTF-8 string at
 * utf8p into a UCS-2 (Unicode) string at ucsp.
 *
 * ucslen is the number of UCS-2 output characters (not bytes)
 * bufsize is the size of the output buffer in bytes
 */
static void
utf_decodestr(const char *utf8p, uint16_t *ucsp, uint16_t *ucslen, uint32_t bufsize)
{
	uint16_t *bufstart;
	uint16_t *bufend;
	uint16_t ucs_ch;
	uint8_t  byte;

	bufstart = ucsp;
	bufend = (uint16_t *)((uint8_t *)ucsp + bufsize);

	while ((byte = *utf8p++) != '\0') {
		if (ucsp >= bufend)
			break;

		/* check for ascii */
		if (byte < 0x80) {
			ucs_ch = byte;
			
			*ucsp++ = HFShtons(ucs_ch);
			continue;
		}

		switch (byte & 0xf0) {
		/*  2 byte sequence*/
		case 0xc0:
		case 0xd0:
			/* extract bits 6 - 10 from first byte */
			ucs_ch = (byte & 0x1F) << 6;  
			break;
		/* 3 byte sequence*/
		case 0xe0:
			/* extract bits 12 - 15 from first byte */
			ucs_ch = (byte & 0x0F) << 6;

			/* extract bits 6 - 11 from second byte */
			if (((byte = *utf8p++) & 0xc0) != 0x80)
				goto stop;

			ucs_ch += (byte & 0x3F);
			ucs_ch <<= 6;
			break;
		default:
			goto stop;
		}

		/* extract bits 0 - 5 from final byte */
		if (((byte = *utf8p++) & 0xc0) != 0x80)
			goto stop;
		ucs_ch += (byte & 0x3F);  

		*ucsp++ = HFShtons(ucs_ch);
	}
stop:
	*ucslen = ucsp - bufstart;
}
