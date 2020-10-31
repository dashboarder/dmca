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
#include <string.h>

#include "Buffer.h"
extern "C" {
#include "drivers/sha1.h"
#include "lib/image/image3/Image3.h"
#include "lib/image/image3/Image3Format.h"
#if BOOTX2BIN_WITH_IMAGE4
# include <libDER/asn1Types.h>
# include <libDER/DER_Decode.h>
# include <Img4Decode.h>
#endif
};

extern "C" int image3_load(struct image_info *image_info,
			   u_int32_t type,
			   void **load_addr, size_t *load_len);

void Buffer::alloc(size_t size) {
  free();
  _buf = new char[size];
  _size = size;
  memset(_buf, 0, _size);
}

void Buffer::free() {
  delete[] _buf;
  _buf = NULL;
  _size = 0;
}

void Buffer::swap(Buffer *other) {
  std::swap(_buf, other->_buf);
  std::swap(_size, other->_size);
}

void Buffer::crop(size_t newSize) {
  assert(newSize <= _size);
  _size = newSize;
}

bool Buffer::loadFromFile(const char *file) {
  // Load the contents of a file into a Buffer and return it.
  FILE *str = fopen(file, "rb");
  if (!str) {
    fprintf(stderr, "Couldn't open \"%s\"\n", file);
    return false;
  }
  fseek(str, 0, SEEK_END);
  size_t size = ftell(str);
  fseek(str, 0, SEEK_SET);
  Buffer file_buf;
  file_buf.alloc(size);
  int got = fread(file_buf.buf(), file_buf.size(), 1, str);
  fclose(str);
  if (got != 1) {
    fprintf(stderr, "Error reading \"%s\"\n", file);
    return false;
  }
  file_buf.swap(this);
  return true;
}

#if BOOTX2BIN_WITH_IMAGE4
bool Buffer::loadFromIm4p(const char *file, uint32_t type, bool quiet) {
  // Load a payload of expected 'type' from 'file', which is in im4p
  // format. Returns a Buffer containing the payload.
  Buffer image_buf;
  if (!image_buf.loadFromFile(file)) {
    fprintf(stderr, "Couldn't load file \"%s\"\n", file);
    return false;
  }

  // Decode IM4P DER format.
  DERItem im4p;
  im4p.data = (DERByte *) image_buf.buf();
  im4p.length = image_buf.size();
  DERReturn ret;
  DERImg4Payload decoded;
  memset(&decoded, 0, sizeof(decoded));
  ret = DERImg4DecodePayload(&im4p, &decoded);
  if (ret != DR_Success) {
    // This isn't a .im4p
    if (!quiet)
      fprintf(stderr, "Failed to decode .im4p header: %d\n", (int) ret);
    return false;
  }
  if (decoded.tag.length != 4 || decoded.type.length != 4) {
    fprintf(stderr, "Decoded .im4p header has bad tag lengths\n");
    return false;
  }
  if (memcmp(decoded.tag.data, "IM4P", 4) != 0) {
    fprintf(stderr, "DER file schema matches but not an IM4P?\n");
    return false;
  }
  uint32_t der_type = 0;
  ret = DERParseInteger(&decoded.type, &der_type);
  if (ret != DR_Success) {
    fprintf(stderr, "Failed to decode DER type field: %d\n", (int) ret);
    return false;
  }
  if (der_type != type) {
    fprintf(stderr, "DER type 0x%08x mismatches expected 0x%08x\n",
	    der_type, type);
    return false;
  }

  // Find the payload.
  void *payload_addr = decoded.data.data;
  size_t payload_size = decoded.data.length;

  uint8_t *buf_end = (uint8_t *) image_buf.buf() + image_buf.size();
  uint8_t *data_end = (uint8_t *) payload_addr + payload_size;
  if (data_end > buf_end) {
    fprintf(stderr, "DER data is truncated\n");
    return false;
  }

  // Move the data down to the base of the buffer.
  memmove(image_buf.buf(), payload_addr, payload_size);
  image_buf.crop(payload_size);
  image_buf.swap(this);
  return true;
}
#endif // BOOTX2BIN_WITH_IMAGE4

bool Buffer::loadFromImg3(const char *file, uint32_t type) {
  // Load a payload of expected 'type' from 'file', which is in img3
  // format. Returns a Buffer containing the payload.
  Buffer image_buf;
  if (!image_buf.loadFromFile(file)) {
    fprintf(stderr, "Couldn't load file \"%s\"\n", file);
    return false;
  }

  // Make a handle for the buffer.
  Image3ObjectHandle handle;
  if (image3InstantiateFromBuffer(&handle,
				  image_buf.buf(),
				  image_buf.size(),
				  false /* copy */)) {
    fprintf(stderr, "Image3 instantiation failed\n");
    return false;
  }

  // Find the payload.
  void *payload_addr = NULL;
  size_t payload_size = 0;
  if (image3GetTagStruct(handle,
			 kImage3TagTypeData,
			 &payload_addr,
			 &payload_size,
			 0)) {
    fprintf(stderr, "Couldn't find Image3 payload\n");
    image3Discard(&handle);
    return false;
  }

  // Move the data down to the base of the buffer.
  memmove(image_buf.buf(), payload_addr, payload_size);
  image_buf.crop(payload_size);
  image_buf.swap(this);
  image3Discard(&handle);
  return true;
}

bool Buffer::loadFromAuto(const char *file, uint32_t type) {
  // Try raw - in any case we can check the type from there.
  if (!loadFromFile(file)) {
    fprintf(stderr, "Couldn't load file \"%s\"\n", file);
    return false;
  }
  if (memcmp(buf(), "3gmI", 4)) {
#if BOOTX2BIN_WITH_IMAGE4
    // Try laoding as .im4p - suppress failure message as this is just a test.
    if (loadFromIm4p(file, type, true)) {
      return true;
    }
#endif
    fprintf(stderr, "Treating as raw file: %s\n", file);
    return true;
  } else {
    fprintf(stderr, "Treating as img3 file: %s\n", file);
    return loadFromImg3(file, type);
  }
}

// Externals used by Image3.c
extern "C" int image3AESDecryptUsingLocalKey(void *buffer, size_t length) {
  fprintf(stderr, "Can't personalize\n");
  return -1;
}

extern "C" void
image3SHA1Generate(void *dataBuffer, size_t dataSize, void *hashBuffer) {
  sha1_calculate(dataBuffer, dataSize, hashBuffer);
}

extern "C" int image3PKIVerifyHash(void *hashBuffer,
				   size_t hashSize,
				   void *signedHashBuffer,
				   size_t signedHashSize,
				   void *certBlobBuffer,
				   size_t certBlobSize,
				   void **certCustomData,
				   size_t *certCustomDataSize) {
  fprintf(stderr, "Can't verify hash\n");
  return -1;
}

extern "C" int image3TicketVerifyHash(void *hashBuffer,
				      size_t hashSize,
				      uint32_t imageType,
				      uint32_t expectedType) {
  fprintf(stderr, "Can't verify hash\n");
  return -1;
}

extern "C" void *image3Malloc(size_t size) {
  return calloc(size, 1);
}

extern "C" void image3Free(void *ptr, size_t size __unused) {
  free(ptr);
}
