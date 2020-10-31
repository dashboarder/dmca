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
#include <sys/types.h>

#include "sys.h"
#include "Buffer.h"
#include "LoadMachO.h"

#define PACKED			__attribute__((packed))

// mach-o/fat.h

typedef uint32_t cpu_type_t;
typedef uint32_t cpu_subtype_t;

struct  load_command {
  uint32_t cmd;
  uint32_t cmdsize;
};

// mach-o/loader.h

struct mach_header {
  uint32_t        magic;          /* mach magic number identifier */
  cpu_type_t      cputype;        /* cpu specifier */
  cpu_subtype_t   cpusubtype;     /* machine specifier */
  uint32_t        filetype;       /* type of file */
  uint32_t        ncmds;          /* number of load commands */
  uint32_t        sizeofcmds;     /* the size of all the load commands */
  uint32_t        flags;          /* flags */
} PACKED;

struct mach_header_64 {
  uint32_t        magic;          /* mach magic number identifier */
  cpu_type_t      cputype;        /* cpu specifier */
  cpu_subtype_t   cpusubtype;     /* machine specifier */
  uint32_t        filetype;       /* type of file */
  uint32_t        ncmds;          /* number of load commands */
  uint32_t        sizeofcmds;     /* the size of all the load commands */
  uint32_t        flags;          /* flags */
  uint32_t        reserved;       /* reserved */
};

/* Constant for the magic field of the mach_header (32-bit architectures) */
#define MH_MAGIC        0xfeedface      /* the mach magic number */
#define MH_CIGAM        0xcefaedfe      /* NXSwapInt(MH_MAGIC) */
#define MH_MAGIC_64     0xfeedfacf      /* the 64-bit mach magic number */
#define MH_CIGAM_64     0xcffaedfe      /* NXSwapInt(MH_MAGIC_64) */

/* Constants for the cmd field of all load commands, the type */
#define LC_SEGMENT      0x1     /* segment of this file to be mapped */
#define LC_SYMTAB       0x2     /* link-edit stab symbol table info */
#define LC_SYMSEG       0x3     /* link-edit gdb symbol table info (obsolete) */

#define LC_THREAD       0x4     /* thread */
#define LC_UNIXTHREAD   0x5     /* unix thread (includes a stack) */
#define LC_SEGMENT_64   0x19 

typedef int32_t vm_prot_t;

struct segment_command { /* for 32-bit architectures */
  uint32_t        cmd;            /* LC_SEGMENT */
  uint32_t        cmdsize;        /* includes sizeof section structs */
  char            segname[16];    /* segment name */
  uint32_t        vmaddr;         /* memory address of this segment */
  uint32_t        vmsize;         /* memory size of this segment */
  uint32_t        fileoff;        /* file offset of this segment */
  uint32_t        filesize;       /* amount to map from the file */
  vm_prot_t       maxprot;        /* maximum VM protection */
  vm_prot_t       initprot;       /* initial VM protection */
  uint32_t        nsects;         /* number of sections in segment */
  uint32_t        flags;          /* flags */
} PACKED;
 
struct segment_command_64 { /* for 64-bit architectures */
  uint32_t        cmd;            /* LC_SEGMENT_64 */
  uint32_t        cmdsize;        /* includes sizeof section_64 structs */
  char            segname[16];    /* segment name */
  uint64_t        vmaddr;         /* memory address of this segment */
  uint64_t        vmsize;         /* memory size of this segment */
  uint64_t        fileoff;        /* file offset of this segment */
  uint64_t        filesize;       /* amount to map from the file */
  vm_prot_t       maxprot;        /* maximum VM protection */
  vm_prot_t       initprot;       /* initial VM protection */
  uint32_t        nsects;         /* number of sections in segment */
  uint32_t        flags;          /* flags */
};

struct thread_command {
  uint32_t        cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
  uint32_t        cmdsize;        /* total size of this command */
  /* uint32_t flavor                 flavor of thread state */
  /* uint32_t count                  count of longs in thread state */
  /* struct XXX_thread_state state   thread state for this flavor */
  /* ... */
} PACKED;

// arm/_types.h

struct arm_thread_state
{
  uint32_t        r[13];          /* General purpose register r0-r12 */
  uint32_t        sp;             /* Stack pointer r13 */
  uint32_t        lr;             /* Link regisster r14 */
  uint32_t        pc;             /* Program counter r15 */
  uint32_t        cpsr;           /* Current program status register */
} PACKED;

struct arm_thread_state_64 {
  uint64_t    x[29];              /* General purpose registers x0-x28 */
  uint64_t    fp;                 /* Frame pointer x29 */
  uint64_t    lr;                 /* Link register x30 */
  uint64_t    sp;                 /* Stack pointer x31 */
  uint64_t    pc;                 /* Program counter */
  uint32_t    cpsr;               /* Current program status register */
} __attribute__((aligned(8)));

LoadMachO::LoadMachO(const Buffer &macho,
		     Buffer *ram)
  : _segment_list(),
    _last_address(0),
    _entry_point_phys_offset(0),
    _macho(macho),
    _virtual_base(0),
    _have_virtual_base(false),
    _ram(ram) {
}

void LoadMachO::OverrideVirtualBase(uint64_t virtual_base) {
  _virtual_base = virtual_base;
  _have_virtual_base = true;
}

bool LoadMachO::GetVirtualBase(uint64_t *virtual_base) const {
  if (_have_virtual_base) {
    *virtual_base = _virtual_base;
    return true;
  } else {
    return false;
  }
}

bool LoadMachO::Load() {
  struct mach_header *mH = (struct mach_header *) _macho.buf();
  uint32_t ncmds = mH->ncmds;
  char *cmdBase;

  _is_64_bit = mH->magic == MH_MAGIC_64 ? true : false;
  if (_is_64_bit) {
    cmdBase = (char *) (((struct mach_header_64*)mH) + 1);
  } else {
    cmdBase = (char *) (mH + 1);
  }

  for (uint32_t cnt = 0; cnt < ncmds; cnt++) {
    uint32_t cmd, cmdsize;
    memcpy(&cmd, cmdBase, 4);
    memcpy(&cmdsize, cmdBase + 4, 4);
    bool ok;

    //fprintf(stdout, "Current command has cmd %x, size %d\n", cmd, cmdsize);

    switch (cmd) {
     case LC_SEGMENT:
       if (_is_64_bit) {
         fprintf(stderr, "Found LC_SEGMENT in 64-bit Mach-O?");
         ok = false;
         break;
       }
       // Copy a segment into ram.
       ok = DecodeSegment(cmdBase);
       break;
     case LC_SEGMENT_64:
       if (!_is_64_bit) {
         fprintf(stderr, "Found LC_SEGMENT64 in 32-bit Mach-O?");
         ok = false;
         break;
       }
       // Copy a segment into ram.
       ok = DecodeSegment(cmdBase);
       break;
     case LC_UNIXTHREAD:
      // Get entry point.
      ok = DecodeUnixThread(cmdBase);
      break;
      
     case LC_SYMTAB:
      // Don't do anything useful with symbol tables.
      ok = true;
      break;
      
     default:
      fprintf(stderr, "Ignoring cmd type %d.\n", (int) cmd);
      ok = true;
    }
    
    if (!ok) return false;
    
    cmdBase += cmdsize;
  }

  return true;
}


static inline void
widen_segment_command(const struct segment_command *scp32,
								    struct segment_command_64 *scp)
{
  scp->cmd = scp32->cmd;
  scp->cmdsize = scp32->cmdsize;
  bcopy(scp32->segname, scp->segname, sizeof(scp->segname));
  scp->vmaddr = scp32->vmaddr;
  scp->vmsize = scp32->vmsize;
  scp->fileoff = scp32->fileoff;
  scp->filesize = scp32->filesize;
  scp->maxprot = scp32->maxprot;
  scp->initprot = scp32->initprot;
  scp->nsects = scp32->nsects;
  scp->flags = scp32->flags;
}


bool LoadMachO::DecodeSegment(const char *cmdBase) {
  const char *src;
  char *dst;
  uint64_t phys_offset;
  uint64_t vmsize, filesize;
  struct segment_command_64 segment_command, *scp;
  size_t segment_command_size;
  struct load_command *lcp = (struct load_command*)cmdBase;
  
  if (LC_SEGMENT_64 == lcp->cmd) {
    segment_command_size = sizeof(struct segment_command_64);
  } else {
    segment_command_size = sizeof(struct segment_command);
  }    

  if (lcp->cmdsize < segment_command_size)
    return false;

  if (LC_SEGMENT_64 == lcp->cmd) {
    scp = (struct segment_command_64 *)lcp;
  } else {
    scp = &segment_command;
    widen_segment_command((struct segment_command *)cmdBase, scp);
  }    

  if (!_have_virtual_base) {
    // Assume the virtual address base is the top bits of the first
    // segment we encounter, aligned to 2MB
    _have_virtual_base = true;
    _virtual_base = scp->vmaddr;
    _virtual_base -= _virtual_base % (1<<21);
    fprintf(stderr, "inferring virtual base is 0x%016llx\n", _virtual_base);
  }

  phys_offset = scp->vmaddr - _virtual_base;
  vmsize = scp->vmsize;

  fprintf(stdout, "Handling %d-bit segment with vmaddr %llx, vmsize %llx, fileoff %lld, filesize %lld, maxprot %d, nsects %d\n", 
		  lcp->cmd == LC_SEGMENT_64 ? 64 : 32, scp->vmaddr, scp->vmsize, scp->fileoff, scp->filesize, scp->maxprot, scp->nsects);
  fprintf(stdout, "Placing at phys offset %llx\n", phys_offset);

  if (phys_offset + vmsize > _ram->size()) {
    fprintf(stderr, "segment overruns end of memory (0x%016llx vs 0x%08x)\n",
	    (phys_offset + vmsize),
	    (unsigned)_ram->size());
    return false;
  }

  src = (const char *) _macho.buf() + scp->fileoff;
  dst = (char *) _ram->buf() + phys_offset;
  filesize = scp->filesize;
  if (scp->fileoff + filesize > _macho.size()) {
    fprintf(stderr,
	    "segment in Mach-O image past end of file (0x%016llx vs 0x%08x)\n",
	    (scp->fileoff + filesize),
	    (unsigned) _macho.size());
    return false;
  }

  uint64_t cpysize = std::min(filesize, vmsize);
  assert(cpysize == (size_t)cpysize);
  memcpy(dst, src, (size_t)cpysize);

  if (vmsize > filesize) {
    assert((vmsize - filesize) == (size_t)(vmsize - filesize));
    memset(dst + filesize, 0, (size_t)(vmsize - filesize));
  }
  
  // Adjust the last address used by the kernel
  if (phys_offset + vmsize > _last_address) {
    _last_address = phys_offset + vmsize;
  }

  // Record the segment in the list.
  Segment segment;
  segment.name = scp->segname;
  segment.pmaddr = phys_offset;
  segment.size = vmsize;
  _segment_list.push_back(segment);
  return true;
}

bool LoadMachO::DecodeUnixThread(const char *cmdBase) {
  // The ARM Thread State starts after the thread command stuct plus,
  // 2 longs for the flaver an num longs.
  if (!_have_virtual_base) {
    fprintf(stderr,
	    "got unix thread load command but don't know the virtual base\n");
    return false;
  }
  if (_is_64_bit) {
    const struct arm_thread_state_64 *armThreadState =
      (const struct arm_thread_state_64 *)
      (cmdBase + sizeof(struct thread_command) + 8);
    _entry_point_phys_offset = armThreadState->pc - _virtual_base;
    fprintf(stdout, "Setting 64-bit thread entry point (phys offset) to 0x%llx (pc %llx)\n", _entry_point_phys_offset, armThreadState->pc);
  } else {
    const struct arm_thread_state *armThreadState =
      (const struct arm_thread_state *)
      (cmdBase + sizeof(struct thread_command) + 8);
    _entry_point_phys_offset = armThreadState->pc - _virtual_base;
    fprintf(stdout, "Setting 32-bit thread entry point (phys offset) to 0x%x (pc %x)\n", (unsigned)_entry_point_phys_offset, armThreadState->pc);
  }

  return true;
}
