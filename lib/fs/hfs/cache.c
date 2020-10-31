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
 *  cache.c - A simple cache for file systems meta-data.
 *
 *  Copyright (c) 2000 - 2003 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hfs.h"

struct CacheEntry {
  CICell    ih;
  time_t    time;
  off_t     offset;
};
typedef struct CacheEntry CacheEntry;

#define kCacheSize            (kFSCacheSize)
#define kCacheMinBlockSize    (0x200)
#define kCacheMaxBlockSize    (0x4000)
#define kCacheMaxEntries      (kCacheSize / kCacheMinBlockSize)

static CICell     gCacheIH;
static uint32_t   gCacheBlockSize;
static uint32_t   gCacheNumEntries;
static time_t     gCacheTime;
static CacheEntry gCacheEntries[kCacheMaxEntries];
static uint8_t    *gCacheBuffer;

uint32_t gCacheHits;
uint32_t gCacheMisses;
uint32_t gCacheEvicts;

void CacheInit(CICell ih, uint32_t blockSize)
{
  if ((blockSize < kCacheMinBlockSize) ||
      (blockSize >= kCacheMaxBlockSize))
    return;

  if (gCacheBuffer)
    free(gCacheBuffer);
  posix_memalign((void **)&gCacheBuffer, blockSize, kFSCacheSize);
  
  gCacheBlockSize = blockSize;
  gCacheNumEntries = kCacheSize / gCacheBlockSize;
  gCacheTime = 0;
  
  gCacheHits = 0;
  gCacheMisses = 0;
  gCacheEvicts = 0;
  
  bzero(gCacheEntries, sizeof(gCacheEntries));
  
  gCacheIH = ih;
}


uint32_t CacheRead(CICell ih, uint8_t *buffer, off_t offset,
	       uint32_t length, bool cache)
{
  uint32_t   cnt, oldestEntry = 0, loadCache = 0;
  uint32_t   result;
  CacheEntry *entry;
  time_t     oldestTime;
  
  // See if the data can be cached.
  if (cache && (gCacheIH == ih) && (length == gCacheBlockSize)) {
    // Look for the data in the cache.
    for (cnt = 0; cnt < gCacheNumEntries; cnt++) {
      entry = &gCacheEntries[cnt];
      if ((entry->ih == ih) && (entry->offset == offset)) {
	entry->time = ++gCacheTime;
	break;
      }
    }
    
    // If the data was found copy it to the caller.
    if (cnt != gCacheNumEntries) {
      memcpy(buffer, gCacheBuffer + cnt * gCacheBlockSize, gCacheBlockSize);
      gCacheHits++;
      return gCacheBlockSize;
    }
    
    // Could not find the data in the cache.
    loadCache = 1;
  }
  
  // Read the data from the disk.
  // <rdar://problem/12080714> CacheRead (in HFS) should return actual amount read
  result = (uint32_t) HFSBlockRead(ih, (CICell)buffer, offset, length);
  if (cache) gCacheMisses++;
  
  // Put the data from the disk in the cache if needed and if read completed
  // successfully
  if (loadCache && (result == length)) {
    // Find a free entry.
    oldestTime = gCacheTime;
    for (cnt = 0; cnt < gCacheNumEntries; cnt++) {
      entry = &gCacheEntries[cnt];
      
      // Found a free entry.
      if (entry->ih == 0) break;
      
      if (entry->time < oldestTime) {
	oldestTime = entry->time;
	oldestEntry = cnt;
      }
    }
    
    // If no free entry was found, use the oldest.
    if (cnt == gCacheNumEntries) {
      cnt = oldestEntry;
      gCacheEvicts++;
    }
    
    // Copy the data from disk to the new entry.
    entry = &gCacheEntries[cnt];
    entry->ih = ih;
    entry->time = ++gCacheTime;
    entry->offset = offset;
    memcpy(gCacheBuffer + cnt * gCacheBlockSize, buffer, gCacheBlockSize);
  }
  
  return result;
}
