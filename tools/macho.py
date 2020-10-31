#!/usr/bin/python
#
# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
# 

#
# A module for manipulating statically linked mach-o files.
#
# Load information from a mach-o file:
#
#   f = macho.File(pathname)
#
# Search for non-debug global text symbols starting with 'a' :
#
#   m = { "name": "^_a.*", "section": "__TEXT,.*", n_type": (N_STAB | N_EXT, N_EXT, N_STAB)}
#   l = f.symtab.matching(m)
#
# Read a structure pointed to by a symbol
#
#   d = l.unpack("IIQ")
#
# Write changes back to the file
#
#   f.update
#
# Specific things this module does not support:
#  - moving anything
#  - adding, resizing or removing sections or segments
#
#

import io
import struct
import re

# section types/attributes
SECTION_TYPE                    = 0x000000ff # 256 section types 
SECTION_ATTRIBUTES              = 0xffffff00 #  24 section attributes 

S_REGULAR                             = 0x00 # regular section 
S_ZEROFILL                            = 0x01 # zero fill on demand section 
S_CSTRING_LITERALS                    = 0x02 # section with only literal C strings
S_4BYTE_LITERALS                      = 0x03 # section with only 4 byte literals 
S_8BYTE_LITERALS                      = 0x04 # section with only 8 byte literals 
S_LITERAL_POINTERS                    = 0x05 # section with only pointers to 
S_NON_LAZY_SYMBOL_POINTERS            = 0x06 # section with only non-lazy symbol pointers
S_LAZY_SYMBOL_POINTERS                = 0x07 # section with only lazy symbol pointers
S_SYMBOL_STUBS                        = 0x08 # section with only symbol stubs, byte size of stub in the reserved2 field
S_MOD_INIT_FUNC_POINTERS              = 0x09 # section with only function pointers for initializatio
S_MOD_TERM_FUNC_POINTERS              = 0x0a # section with only function pointers for termination
S_COALESCED                           = 0x0b # section contains symbols that are to be coalesced
S_GB_ZEROFILL                         = 0x0c # zero fill on demand section (that can be larger than 4 gigabytes)
S_INTERPOSING                         = 0x0d # section with only pairs of function pointers for interposing
S_16BYTE_LITERALS                     = 0x0e # section with only 16 byte literals
S_DTRACE_DOF                          = 0x0f # section contains DTrace Object Format
S_THREAD_LOCAL_REGULAR                = 0x11 # template of initial values for TLVs
S_THREAD_LOCAL_ZEROFILL               = 0x12 # template of initial values for TLVs
S_THREAD_LOCAL_VARIABLES              = 0x13 # TLV descriptors
S_THREAD_LOCAL_VARIABLE_POINTERS      = 0x14 # pointers to TLV descriptors
S_THREAD_LOCAL_INIT_FUNCTION_POINTERS = 0x15 # functions to call to initialize TLV values

SECTION_ATTRIBUTES_USR          = 0xff000000 # User setable attributes
S_ATTR_PURE_INSTRUCTIONS        = 0x80000000 # section contains only true machine instructions
S_ATTR_NO_TOC                   = 0x40000000 # section contains coalesced symbols that are not to be in a ranlib table of contents
S_ATTR_STRIP_STATIC_SYMS        = 0x20000000 # ok to strip static symbols in this section in files with the MH_DYLDLINK flag
S_ATTR_NO_DEAD_STRIP            = 0x10000000 # no dead stripping
S_ATTR_LIVE_SUPPORT             = 0x08000000 # blocks are live if they reference live blocks
S_ATTR_SELF_MODIFYING_CODE      = 0x04000000 # Used with i386 code stubs written on by dyld
S_ATTR_DEBUG                    = 0x02000000 # a debug section
SECTION_ATTRIBUTES_SYS          = 0x00ffff00 # system setable attributes
S_ATTR_SOME_INSTRUCTIONS        = 0x00000400 # section contains some machine instructions
S_ATTR_EXT_RELOC                = 0x00000200 # section has external relocation entries
S_ATTR_LOC_RELOC                = 0x00000100 # section has local relocation entries

SEG_PAGEZERO                  = "__PAGEZERO" # the pagezero segment which has no protections and catches NULL references
SEG_TEXT                      = "__TEXT"     # the tradition UNIX text segment */
SECT_TEXT                     = "__text"     # the real text part of the text section no headers, and no padding */
SEG_DATA                      = "__DATA"     # the tradition UNIX data segment */
SECT_DATA                     = "__data"     # the real initialized data section, no padding, no bss overlap */
SECT_BSS                      = "__bss"      # the real uninitialized data section, no padding */
SECT_COMMON                   = "__common"   # the section common symbols are allocated in by the link editor */

# not-a-section value for n_sect
NO_SECT                            = 0x00    # section numbers start at 1

# nlist n_type values
N_STAB                             = 0xe0    # mask for debugger symbols
N_TYPE                             = 0x0e    # mask for symbol type

N_PEXT                             = 0x10    # symbol is private-extern
N_EXT                              = 0x01    # symbol is external

N_UNDF                             = 0x00    # symbol is undefined
N_ABS                              = 0x02    # absolute (n_sect is NO_SECT)
#N_TEXT                             = 0x04    # text symbol
N_DATA                             = 0x06    # data symbol
#N_BSS                              = 0x08    # bss symbol
N_INDR                             = 0x0a    # symbol is indirect (n_value is string table index)
N_SECT                             = 0x0e    # symbol is in n_sect
#N_COMM                             = 0x12    # common symbol
#N_FN                               = 0x1e    # filename

# nlist n_desc values
N_WEAK_REF                         = 0x40    # weak reference (value may be zero)
N_WEAK_DEF                         = 0x80    # weak definition
N_ARM_THUMB_DEF	                   = 0x0008  # Thumb function

# selected STAB symbol types
# fields listed here are n_name, n_sect, n_desc, n_value
N_GSYM                             = 0x20    # global symbol:            name,NO_SECT,type,0
N_FUN                              = 0x24    # procedure:                name,n_sect,linenumber,address
N_STSYM                            = 0x26    # static symbol:            name,n_sect,type,address
N_LCSYM                            = 0x28    # .lcomm symbol:            name,n_sect,type,address
N_BNSYM                            = 0x2e    # begin nsect sym:          0,n_sect,0,address
N_OPT                              = 0x3c    # emitted with gcc2_compiled and in gcc source
N_RSYM                             = 0x40    # register sym:             name,NO_SECT,type,register
N_SLINE                            = 0x44    # src line:                 0,n_sect,linenumber,address
N_ENSYM                            = 0x4e    # end nsect sym:            0,n_sect,0,address
N_SSYM                             = 0x60    # structure elt:            name,NO_SECT,type,struct_offset
N_SO                               = 0x64    # source file name:         name,n_sect,0,address
N_OSO                              = 0x66    # object file name:         name,0,0,st_mtime
N_LSYM                             = 0x80    # local sym:                name,NO_SECT,type,offset
N_BINCL                            = 0x82    # include file beginning:   name,NO_SECT,0,sum
N_SOL                              = 0x84    # #included file name:      name,n_sect,0,address
N_PARAMS                           = 0x86    # compiler parameters:      name,NO_SECT,0,0
N_VERSION                          = 0x88    # compiler version:         name,NO_SECT,0,0
N_OLEVEL                           = 0x8A    # compiler -O level:        name,NO_SECT,0,0
N_PSYM                             = 0xa0    # parameter:                name,NO_SECT,type,offset
N_EINCL                            = 0xa2    # include file end:         name,NO_SECT,0,0
N_ENTRY                            = 0xa4    # alternate entry:          name,n_sect,linenumber,address
N_LBRAC                            = 0xc0    # left bracket:             0,NO_SECT,nesting level,address
N_EXCL                             = 0xc2    # deleted include file:     name,NO_SECT,0,sum
N_RBRAC                            = 0xe0    # right bracket:            0,NO_SECT,nesting level,address
N_BCOMM                            = 0xe2    # begin common:             name,NO_SECT,0,0
N_ECOMM                            = 0xe4    # end common:               name,n_sect,0,0
N_ECOML                            = 0xe8    # end common (local name):  0,n_sect,0,address
N_LENG                             = 0xfe    # second stab entry with length information


def dict_from_struct(with_bytes, using_struct, to_names):
    result = {}

    # unpack
    values = using_struct.unpack(str(with_bytes))

    # walk the unpacked values
    keys = to_names.split(" ")
    for i in range(0, len(keys)):
        result[keys[i]] = values[i]
    return result


class File(object):
    """toplevel object representing a mach-o file"""
    # struct mach_header
    # {
    #     uint32_t magic;
    #     cpu_type_t cputype;
    #     cpu_subtype_t cpusubtype;
    #     uint32_t filetype;
    #     uint32_t ncmds;
    #     uint32_t sizeofcmds;
    #     uint32_t flags;
    # };
    # struct mach_header_64
    # {
    #     uint32_t magic;
    #     cpu_type_t cputype;
    #     cpu_subtype_t cpusubtype;
    #     uint32_t filetype;
    #     uint32_t ncmds;
    #     uint32_t sizeofcmds;
    #     uint32_t flags;
    #     uint32_t reserved;
    # };
    mach_header_magic32 = 0xfeedface
    mach_header_magic64 = 0xfeedfacf
    mach_header_f       = "IiiIIII"
    mach_header_n       = "magic cputype cpusubtype filetype ncmds sizeofcmds flags"

    def __init__(self, file, debug=False):
        self._symtab            = None
        self._stringtab         = None
        self._segments          = {}
        self._sections          = {}
        self._section_index     = 1
        self._uuid              = None
        self._debug             = debug

        # stash the file and the header offset
        self._file = file

        # read the file header
        self._properties = self.unpack(self.mach_header_f, self.mach_header_n)

        # skip the reserved field in a 64-bit header
        if self['magic'] == self.mach_header_magic64:
            self._file.seek(4, io.SEEK_CUR)
        elif self['magic'] != self.mach_header_magic32:
            raise RuntimeError("bad magic number {:#010x} in mach-o header".format(self['magic']))

        # read load commands
        for i in range(1, self['ncmds']):
            cmd = LoadCommand.from_file(self)

            # reset the file pointer to the next load command
#            print "seek to load command at {} + {}".format(cmd['__offset'], cmd['cmdsize'])
            self._file.seek(cmd['__offset'] + cmd['cmdsize'])

    def debug(self, str):
        if self._debug is not False:
            print str

    # access to the UUID
    @property
    def uuid(self):
        return self._uuid

    @uuid.setter
    def uuid(self, value):
        self._uuid = value

    # access to the symbol table
    @property
    def symtab(self):
        return self._symtab

    @symtab.setter
    def symtab(self, value):
        self._symtab = value

    # access to the string table
    @property
    def stringtab(self):
        return self._stringtab

    @stringtab.setter
    def stringtab(self, value):
        self._stringtab = value

    # access to the segment list
    @property
    def segments(self):
        return self._segments

    # segments call this to self-register
    def add_segment(self, segment):
        self.debug("segment " + segment['segname'])
        self._segments[segment['segname']] = segment

    # access to the section list
    @property
    def sections(self):
        return self._sections

    def sections_inorder(self):
        '''iterate sections in file order'''
        for i in range(1, self._section_index - 1):
            yield self._sections[i]

    # sections call this to self-register
    def add_section(self, section):
        sectionname = "{},{}".format(section['segname'], section['sectname'])
        self.debug("section {}:{} @ {}".format(self._section_index, sectionname, section['offset']))
        self._sections[self._section_index] = section
        self._section_index += 1
        self._sections[sectionname] = section

    # get the object file flavor (32/64)
    @property
    def flavor(self):
        if self['magic'] == self.mach_header_magic32:
            return 32
        return 64

    # allow properties to be fetched by indexing us
    def __getitem__(self, index):
        if index in self._properties:
            return self._properties[index]
        else:
            return None

    # unpack a structure from the file
    def unpack(self, format, names, offset=-1, rewind=False):

        # if we are told to seek, do that now
        if offset != -1:
            self._file.seek(offset)

        # remember where we read from
        pre_offset = self._file.tell()

        # fetch the data and unpack it
        s = struct.Struct(format)
        bytes = self.readbytes(s.size, offset, rewind)
        result = dict_from_struct(bytes, s, names)

        # default result dict entries
        result['__offset'] = pre_offset
        result['__dirty'] = False

        # if we were asked to rewind
        if rewind is True:
            self._file.seek(pre_offset)

        return result

    # repack a structure into the file
    def repack(self, format, names, props):

        # make a list of the property arguments for the format
        values = [props[key] for key in names.split()]
        s = struct.Struct(format)
        bytes = s.pack(*values)

        self.writebytes(bytes, props['__offset'])

    # read bytes from the file
    def readbytes(self, size, offset=-1, rewind=False):
        o_offset = self._file.tell()
        if offset != -1:
            self._file.seek(offset)
        bytes = self._file.read(size)
        if rewind is True:
            self._file.seek(o_offset)
        return bytearray(bytes)

    # write bytes to the file
    def writebytes(self, bytes, offset=-1):
        if offset != -1:
            self._file.seek(offset)
        count = len(bytes)
        wrote = self._file.write(bytes)
        if wrote != count:
            raise RuntimeError("tried to write {} bytes, only wrote {} bytes".format(count, wrote))

    # push any updates out to the file
    def update(self):
        updated = False
        for i in range(1, self._section_index):
            if self._sections[i].update():
                updated = True
        if updated:
            self._file.flush()
        return updated

    # locate a segment by name
    def segment_for_name(self, name):
        if name in self._segments:
            return self._segments[name]
        return None

    # locate the section that contains data for a given address range
    def section_for_address(self, addr):
        for sect in self._sections.itervalues():
            saddr = sect['addr']
            send = saddr + sect['size']
            if (saddr <= addr) and (send > addr):
                return sect
        return None

    # locate a section by name
    def section_for_name(self, name):
        if name in self._sections:
            return self._sections[name]
        return None

    # locate symbols in a section
    def symbols_in_section(self, name):
        matching = {'section': name}
        return self._symtab.matching(matching)

    # locate symbols referencing an address
    def symbols_for_address(self, addr):
        matching = {'n_value': addr, 'n_type': N_ABS}
        return self._symtab.matching(matching)


class LoadCommand(object):
    """base class for load commands"""

    # struct load_command
    # {
    #     uint32_t cmd;
    #     uint32_t cmdsize;
    # };
    lc_names = {
        0x00000001: "LC_SEGMENT",
        0x00000002: "LC_SYMTAB",
        0x00000019: "LC_SEGMENT_64",
        0x0000001b: "LC_UUID",
        0x00000026: "LC_FUNCTION_STARTS",
        0x00000029: "LC_DATA_IN_CODE",
        0x0000002A: "LC_SOURCE_VERSION",
    }
    load_command_f = "II"
    load_command_n = "cmd cmdsize"

    @classmethod
    def from_file(cls, parent):

        # read the header and reset the file pointer so that a subclass
        # will get the whole thing
        properties = parent.unpack(cls.load_command_f, cls.load_command_n, rewind=True)

        # hand off to the first subclass that is interested in it
        obj = None
        for subcls in cls.__subclasses__():
            obj = subcls.for_load_command(properties['cmd'], parent)
            if obj is not None:
                break

        # if we didn't find a subclass, return a dummy so that it can be skipped
        if obj is None:
#            print "ignoring unhandled section {}".format(properties['cmd'])
            obj = LoadCommand(parent)

        return obj

    def __init__(self, parent):
        self._properties = parent.unpack(self.load_command_f, self.load_command_n)

    # allow properties to be fetched by indexing us
    def __getitem__(self, index):
        if index in self._properties:
            return self._properties[index]
        else:
            return None


class Segment(LoadCommand):
    """handles segment commands"""

    # struct segment_command
    # {
    #     uint32_t cmd;
    #     uint32_t cmdsize;
    #     char segname[16];
    #     uint32_t vmaddr;
    #     uint32_t vmsize;
    #     uint32_t fileoff;
    #     uint32_t filesize;
    #     vm_prot_t maxprot;
    #     vm_prot_t initprot;
    #     uint32_t nsects;
    #     uint32_t flags;
    # };
    # struct segment_command_64
    # {
    #     uint32_t cmd;
    #     uint32_t cmdsize;
    #     char segname[16];
    #     uint64_t vmaddr;
    #     uint64_t vmsize;
    #     uint64_t fileoff;
    #     uint64_t filesize;
    #     vm_prot_t maxprot;
    #     vm_prot_t initprot;
    #     uint32_t nsects;
    #     uint32_t flags;
    # };
    segment_command32_f = "II16sIIIIiiII"
    segment_command64_f = "II16sQQQQiiII"
    segment_command_n   = "cmd cmdsize segname vmaddr vmsize fileoff filesize maxprot initprot nsects flags"

    @classmethod
    def for_load_command(cls, cmd, parent):
        if cmd in cls.lc_names:
            if cls.lc_names[cmd] == "LC_SEGMENT":
                return cls(cmd, parent, 32)
            if cls.lc_names[cmd] == "LC_SEGMENT_64":
                return cls(cmd, parent, 64)
        return None

    def __init__(self, cmd, parent, flavor):
        if flavor == 32:
            self._properties = parent.unpack(self.segment_command32_f, self.segment_command_n)
        else:
            self._properties = parent.unpack(self.segment_command64_f, self.segment_command_n)

        # strip NULs from the segment name
        self._properties['segname'] = self._properties['segname'].rstrip("\x00")

        # advertise to the parent
        parent.add_segment(self)

        # iterate sections
        for i in range(0, self['nsects']):
            Section(parent, flavor)


class Section(object):
    """base class for text/data sections"""

    # struct section
    # {
    #     char sectname[16];
    #     char segname[16];
    #     uint32_t addr;
    #     uint32_t size;
    #     uint32_t offset;
    #     uint32_t align;
    #     uint32_t reloff;
    #     uint32_t nreloc;
    #     uint32_t flags;
    #     uint32_t reserved1; /* offset or index */
    #     uint32_t reserved2; /* count or sizeof */
    # };
    # struct section_64
    # {
    #     char sectname[16];
    #     char segname[16];
    #     uint64_t addr;
    #     uint64_t size;
    #     uint32_t offset;
    #     uint32_t align;
    #     uint32_t reloff;
    #     uint32_t nreloc;
    #     uint32_t flags;
    #     uint32_t reserved1; /* offset or index */
    #     uint32_t reserved2; /* count or sizeof */
    #     uint32_t reserved3;
    # };
    section32_f = "16s16sIIIIIIIII"
    section64_f = "16s16sQQIIIIIIII"
    section_n   = "sectname segname addr size offset align reloff nreloc flags index count"

    def __init__(self, parent, flavor):
        if flavor == 32:
            self._properties = parent.unpack(self.section32_f, self.section_n)
        else:
            self._properties = parent.unpack(self.section64_f, self.section_n)

        # strip NULs from the segment and section names
        self._properties['segname'] = self._properties['segname'].rstrip("\x00")
        self._properties['sectname'] = self._properties['sectname'].rstrip("\x00")

        # give ourselves a canonical name
        self._properties['name'] = "{},{}".format(self['segname'], self['sectname'])

        # register so that we can be found by either name or index
        self._parent = parent
        parent.add_section(self)

    # allow properties to be fetched by indexing us
    # and for a slice to index the 'bytes' property
    def __getitem__(self, index):

        if isinstance(index, slice):
            return self['bytes'][index]

        if index not in self._properties:
            if index == 'bytes':
                # can't mess with bytes in a zerofill section
                if (self['flags'] & SECTION_TYPE) == S_ZEROFILL:
                    return None
#                self._parent.debug("read {}/{}".format(self['offset'], self['size']))
                v = self._parent.readbytes(self['size'], offset=self['offset'])
#                print ord(v[0]), ord(v[1]), ord(v[2]), ord(v[3])
            elif index == 'end':
                v = self['addr'] + self['size'] - 1
            else:
                return None
            self._properties[index] = v
        return self._properties[index]

    # allow some properties to be changed
    def __setitem__(self, index, value):
        if isinstance(index, slice):
            # prefetch the 'bytes' property
            value = self['bytes']
            self._properties['bytes'][index] = value

        if index == 'bytes':
            if len(value) != self['size']:
                raise RuntimeError("wrong size for section data")
            self._properties['bytes'] = value
            self._properties['__dirty'] = True
        else:
            raise RuntimeError("section attribute '{}' is not writable", index)

    # if we have been dirtied by e.g. variable patching, write our bytes back
    def update(self):
        if self['__dirty']:
            self._parent.writebytes(self['bytes'], self['offset'])
            self._properties['__dirty'] = False
            return True
        return False

class Symtab(LoadCommand):
    """symbol table"""

    # struct symtab_command
    # {
    #     uint_32 cmd;
    #     uint_32 cmdsize;
    #     uint_32 symoff;
    #     uint_32 nsyms;
    #     uint_32 stroff;
    #     uint_32 strsize;
    # };
    symtab_f = "IIIIII"
    symtab_n = "cmd cmdsize symoff nsyms stroff strsize"

    @classmethod
    def for_load_command(cls, cmd, parent):
        if cmd in cls.lc_names:
            if cls.lc_names[cmd] in ("LC_SYMTAB"):
                return cls(parent)
        return None

    def __init__(self, parent):
        self._nlist = []

        # read the symtab header
        self._properties = parent.unpack(self.symtab_f, self.symtab_n)

        # read the string table first
        parent.stringtab = StringTable(parent, self['stroff'], self['strsize'])

        # read the name list
        self._nlist.append(Nlist(parent, self['symoff']))
        for n in range(2, self['nsyms']):
            self._nlist.append(Nlist(parent))

#        print "symtab: {} symbols".format(len(self._nlist))

        parent.symtab = self

    # find symbols matching a dictionary
    def matching(self, matching_dict):
        result = []

        # walk the symbol set looking for matches
        for sym in self._nlist:
#            print "  {}:".format(sym['name'])

            if sym.matches(matching_dict):
                result.append(sym)

        return result


class StringTable(object):
    """a string table"""

    _strings = {}

    def __init__(self, parent, offset, size):
        # read the raw string table from the file
        bytes = parent.readbytes(size, offset=offset)

        # parse out NUL-separate strings and index them
        index = 0
        ptr = -1
        s = ""
        for c in bytes:
            ptr += 1
            if c == 0:
                if len(s) > 0:
#                    print "strtab: {} '{}'".format(index, s)
                    self._strings[index] = s
                    parent.debug("string {}:'{}'".format(index, s))
                    s = ""
            else:
                if len(s) == 0:
                    index = ptr;
                s += chr(c)

    def __getitem__(self, index):
        if index in self._strings:
            return self._strings[index]
        elif index == 0:
            return ""
        else:
            return None

class UUIDCommand(LoadCommand):
    """handles segment commands"""

    # struct uuid_command
    # {
    #     uint32_t cmd;
    #     uint32_t cmdsize;
    #     uint8_t uuid[16];
    # };
    uuid_command_f = "II16s"
    uuid_command_n   = "cmd cmdsize uuid"

    @classmethod
    def for_load_command(cls, cmd, parent):
        if cmd in cls.lc_names:
            if cls.lc_names[cmd] == "LC_UUID":
                return cls(cmd, parent)
        return None

    def __init__(self, cmd, parent):
        self._properties = parent.unpack(self.uuid_command_f, self.uuid_command_n)

        parent.debug('uuid: {}'.format(self._properties))

        # advertise to the parent
        parent.uuid = self._properties['uuid']

class Nlist(object):
    """a symbol table entry"""

    # struct nlist
    # {
    #     union {
    #        int32_t n_strx;    /* string table offset */
    #     } n_un;
    #     uint8_t n_type;       /* symbol type */
    #     uint8_t n_sect;       /* section index */
    #     int16_t n_desc;       /* debug description */
    #     uint32_t n_value;     /* address/value */
    # };
    # struct nlist_64
    # {
    #     union {
    #        uint32_t n_strx;
    #     } n_un;
    #     uint8_t n_type;
    #     uint8_t n_sect;
    #     uint16_t n_desc;
    #     uint64_t n_value;
    # };
    nlist32_f = "IBBHI"
    nlist64_f = "IBBHQ"
    nlist_n = "n_strx n_type n_sect n_desc n_value"

    def __init__(self, parent, offset=-1):
        if parent.flavor == 32:
            self._properties = parent.unpack(self.nlist32_f, self.nlist_n, offset=offset)
        elif parent.flavor == 64:
            self._properties = parent.unpack(self.nlist64_f, self.nlist_n, offset=offset)
        else:
            raise RuntimeError("bad nlist flavor")

        # save a reference to the parent, we need it for lazy name evaluation
        self._parent = parent
        self['name']
        parent.debug("symbol {}".format(self._properties))

    # match the symbol against a dictionary of same
    def matches(self, matching_dict):

        # walk the matching dictionary
        for key in matching_dict:

            # fetch the symbol's value for this key; if we don't
            # have a value we cannot match
            v = self[key]
            if v is None:
                return False
            c = matching_dict[key]

            # simple integer match?
            if isinstance(c, int):
                if c != v:
                    return False
                continue

            # integer masking match?
            if isinstance(c, tuple) and (len(c) == 3):
                mask, require, prohibit = c
                v &= mask
                if (v & prohibit) != 0:
                    return False
                if (v & require) != require:
                    return False
                continue

            # string match?
            if isinstance(c, str):
                r = re.compile(c)
                m = r.match(v)
                if m is None:
                    return False
                continue

            # don't actually know how to handle this...
            raise RuntimeError("don't know how to match '{}' using '{}'".format(key, v))

        return True

    # Allow properties to be fetched by indexing us
    #
    # As some properties aren't known as parse time (e.g. section names)
    # we look them up lazily.
    #
    def __getitem__(self, index):
        if index not in self._properties:
            v = None
            if index == 'name':
                v = self._parent.stringtab[self['n_strx']]
            elif index == 'sectname':
                i = self['n_sect']
                if i not in self._parent.sections:
                    return None
                v = self._parent.sections[i]['name']
            elif index == 'section':
                stype = self._properties['n_type'] & N_TYPE
                if stype == N_ABS or stype == N_DATA:
                    v = self._parent.section_for_address(self['n_value'])
                elif stype == N_SECT:
                    v = self._parent.sections[self['n_sect']]
            self._properties[index] = v
        return self._properties[index]

    # Read from whatever the symbol points to
    def unpack(self, format, names):

        # look up the section we're operating on
        sect = self['section']
        addr = self['n_value']
        offset = addr - sect['addr']
#        self._parent.debug("addr 0x{:x} section addr 0x{:x} offset 0x{:x}".format(addr, sect['addr'], offset))
        bytes = sect['bytes']

        if format == 'C': # Non-standard C String format
            chunk = bytes[offset:bytes.index('\0', offset)]
            s = struct.Struct('%ds' % len(chunk))
        else:
            # slice the requested range and decode
            s = struct.Struct(format)
#            print "{}:{}".format(offset, offset + s.size)
            chunk = bytes[offset:(offset + s.size)]

        return dict_from_struct(chunk, s, names)

    # write to whatever the symbol points to
    #
    # XXX needs to be updated to take a dict as argument
    def pack(self, format, values):

        # look up the section we're operating on
        sect = self['section']
        addr = self['n_value']
        offset = addr - sect['addr']

        # encode the values
        self._parent.debug("packing {}".format(values))
        s = struct.Struct(format)
        chunk = s.pack(*values)

        # and update the section
        bytes = sect['bytes']
        bytes[offset:(offset + s.size)] = chunk
        sect['bytes'] = bytes

    # dereference the symbol and get a single value it points to
    def deref(self, format):
        contents = self.unpack(format, 'value')
        return contents['value']

    # assign a value to the thing the symbol references
    def assign(self, format, value):
        self.pack(format, [value])


def to_binary(macho, output, verbose=False):
        '''emit a compact in-memory binary representation of the mach-o file'''

        output.seek(0)
        data_offset = 0
        for section in macho.sections_inorder():

                # We assume that we simply want to pack all of the
                # text/data sections in the input file into the output, and 
                # that we should stop once we hit a zero-filled section because
                # we've run out of data.

                if (((section['flags'] & S_ZEROFILL) != 0) or
                    (section['sectname'] == '__common') or 
                    (section['sectname'] == '__bss')):
                   break

                # Make a note of where we are in the output file when
                # we start processing sections in the __DATA segment
                if data_offset == 0 and section['segname'] == '__DATA':
                        data_offset = output.tell()

                # pad sections to maintain alignment
                alignment = 1 << section['align']
                if section['segname'] == '__DATA':
                        skew = data_offset
                else:
                        skew = 0
                padding = (alignment - ((output.tell() - skew) % alignment)) % alignment

                zeros = bytearray.fromhex('00' * padding)
                output.write(zeros)

                # write the section contents
                if section['size'] > 0:
                    if verbose is True:
                        print '{}/{} 0x{:x}/0x{:x} @ 0x{:x}'.format(section['segname'],
                                                        section['sectname'],
                                                        section['addr'],
                                                        section['size'],
                                                        output.tell())
                    output.write(section['bytes'])


#
# Be a useful utility.
#
if __name__ == "__main__":
        import argparse

        parser = argparse.ArgumentParser(description='mach-o utility')
        parser.add_argument('-m', '--macho', required=True,
                            help='path to the mach-o file')
        parser.add_argument('-o', '--output',
                            help='path to the output file')
        parser.add_argument('-c', '--convert', dest='format', choices=['binary', 'TBD'],
                            help='convert to the specified format')
        parser.add_argument('-v', '--verbose', default=False, action='store_true',
                            help='enable verbose output')
        parser.add_argument('-d', '--debug', default=False, action='store_true',
                            help='enable debug output')

        args = parser.parse_args()

        try:
                f = File(io.open(args.macho, 'rb'), args.debug)
        except:
                print "macho: failed opening " + args.macho
                raise

        if args.format == 'binary' or args.format == 'objcopy':
                if args.output is None:
                        raise RuntimeError("missing -o argument to specify output name")
                o = io.open(args.output, 'wb')

                to_binary(f, o, args.verbose)
                o.close()

