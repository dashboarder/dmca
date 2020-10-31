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

#ifndef LOAD_MACH_O_H
#define LOAD_MACH_O_H	1

#include <stdint.h>
#include <stdlib.h>
#include <list>
#include <string>

class Buffer;

class LoadMachO {
 public:
  struct Segment {
    std::string name;
    uint64_t pmaddr;
    uint64_t size;
  };
  typedef std::list<Segment> SegmentList;
  typedef SegmentList::iterator SegmentListIterator;

  LoadMachO(const Buffer &macho, Buffer *ram);
  void OverrideVirtualBase(uint64_t virtual_base);
  bool GetVirtualBase(uint64_t *virtual_base) const;
  bool Load();

  SegmentListIterator SegmentListBegin() { return _segment_list.begin(); }
  SegmentListIterator SegmentListEnd() { return _segment_list.end(); }
  uint64_t LastAddress() const { return _last_address; }
  uint64_t EntryPointPhysOffset() const { return _entry_point_phys_offset; }
  bool Is64Bit() const { return _is_64_bit; } 

 private:
  bool DecodeSegment(const char *cmdBase);
  bool DecodeUnixThread(const char *cmdBase);

  SegmentList _segment_list;
  uint64_t _last_address;
  uint64_t _entry_point_phys_offset;
  const Buffer &_macho;
  uint64_t _virtual_base;
  bool _have_virtual_base;
  bool _is_64_bit;
  Buffer *_ram;
};

#endif  // LOAD_MACH_O_H
