/*
 *  Copyright (C) 2009  Apple, Inc. All rights reserved.
 *  
 *  This document is the property of Apple Inc.
 *  It is considered confidential and proprietary.
 *
 *  This document may not be reproduced or transmitted in any form,
 *  in whole or in part, without the express written permission of
 *  Apple Inc.
 */

#include <stdio.h>
#include <unistd.h>

#include "sys.h"
#include "Buffer.h"
#include "Kernelcache.h"
#include "lib/cksum.h"
#include "lib/lzss.h"

/* Constant for the magic field of the mach_header (32-bit architectures) */
#define MH_MAGIC        0xfeedface      /* the mach magic number */
#define MH_MAGIC_64     0xfeedfacf      /* the 64-bit mach magic number */

struct compressed_kernel_header {
  uint32_t signature;
  uint32_t compress_type;
  uint32_t adler32;
  uint32_t uncompressed_size;
  uint32_t compressed_size;
  uint32_t reserved[11];
  uint8_t  platform_name[64];
  uint8_t  root_path[256];
  uint8_t  data[];
} __attribute__((packed));

bool DecompressKernelcache(const Buffer &kernelcache, Buffer *ret_macho) {
  if (kernelcache.size() < sizeof(compressed_kernel_header)) {
    fprintf(stderr, "Runt image (only %u bytes)\n",
	    (unsigned) kernelcache.size());
    return false;
  }
  compressed_kernel_header *header =
      (compressed_kernel_header *) kernelcache.buf();
  if (ntohl(header->signature) != 'comp' ||
      ntohl(header->compress_type) != 'lzss') {
    fprintf(stderr, "File does not contain a compressed kernelcache\n");
    return false;
  }
  fprintf(stderr, "Found compressed kernelcache header\n");
  uint32_t decompressed_size = ntohl(header->uncompressed_size);
  uint32_t compressed_size = ntohl(header->compressed_size);
  if (compressed_size + sizeof(compressed_kernel_header) > kernelcache.size()) {
    fprintf(stderr, "Compressed image size (%u) greater than image (%u)\n",
	    (unsigned) compressed_size,
	    (unsigned) (kernelcache.size() - sizeof(compressed_kernel_header)));
    return false;
  }
  fprintf(stderr, "Decompress %u -> %u\n", compressed_size, decompressed_size);
  Buffer decompressed;
  decompressed.alloc(decompressed_size);
  uint32_t actual_size = decompress_lzss((uint8_t *) decompressed.buf(),
					 decompressed.size(),
					 (uint8_t *) &header->data[0],
					 compressed_size);
  if (actual_size != decompressed.size()) {
    fprintf(stderr, "Decompressed size mismatch, expected %u got %u\n",
	    (unsigned) decompressed.size(), (unsigned) actual_size);
    return false;
  }
  if (adler32((const uint8_t *) decompressed.buf(),
	      decompressed_size) != ntohl(header->adler32)) {
    fprintf(stderr, "Adler32 mismatch\n");
    return false;
  }
  uint32_t macho_got_magic;
  memcpy(&macho_got_magic, decompressed.buf(), 4);
  if (macho_got_magic != MH_MAGIC && macho_got_magic != MH_MAGIC_64) {
    fprintf(stderr, "Decompressed kernelcache not in Mach-O format\n");
    return false;
  }
  decompressed.swap(ret_macho);
  return true;
}
