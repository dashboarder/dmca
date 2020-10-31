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

#ifndef BUFFER_H
#define BUFFER_H	1

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

/*
 * Encapsulation of a memory buffer. The contained buffer is
 * automatically deallocated when the containing Buffer object is
 * destructed, which simplifies functions with many early-exit
 * cases. To transfer ownership to the caller, swap it with a buffer
 * that was passed by reference, e.g:
 *
 * void my_func(Buffer *ret_buf) {
 *   Buffer local_buf;
 *   ... work on local_buf ...
 *   local_buf.swap(ret_buf);
 * }
 */

class Buffer {
 public:
  Buffer() : _buf(NULL), _size(0) {}
  ~Buffer() { free(); }

  // Allocate a buffer of given 'size'. Deletes any previous contents.
  void alloc(size_t size);

  // Allocate and load from a file.
  bool loadFromFile(const char *file);

#if BOOTX2BIN_WITH_IMAGE4
  // Allocate and load a payload from an im4p format file, of expected 'type'.
  bool loadFromIm4p(const char *file, uint32_t type, bool quiet = false);
#endif

  // Allocate and load a payload from an img3 format file, of expected 'type'.
  bool loadFromImg3(const char *file, uint32_t type);

  // Allocate and load a payload, automatically extracting the payload from
  // an im4p or img3 format file if detected, and otherwise raw. Will
  // return failure if im4p or img3 is detected but type does not match.
  bool loadFromAuto(const char *file, uint32_t type);

  void free();
  void swap(Buffer *other);
  void crop(size_t newSize);

  void *buf() { return _buf; }
  const void *buf() const { return _buf; }
  size_t size() const { return _size; }
  
 private:
  char *_buf;
  size_t _size;
};

#endif  // BUFFER_H
