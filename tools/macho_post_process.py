#!/usr/bin/python
#
# Copyright (C) 2013-2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

#
# RTXC Mach-o post processing
#

from __future__ import print_function
from argparse import ArgumentParser
import macho
import io
from struct import Struct
import subprocess
from uuid import UUID


def memory_slices(data, size):
    m = memoryview(data)

    for index in xrange(0, len(data), size):
        yield m[index:index+size]


def hexdump(bytes, stride=16):
    for i, data in enumerate(memory_slices(bytes, stride)):
        dataString = ' '.join(map('{:02X}'.format, data.tolist()))
        print('{:04x}: {}'.format(i * stride, dataString))

from collections import namedtuple

def magicToInt(string, format='<L'):
    return Struct(format).unpack(string)[0]

class RTXCId(namedtuple('RTXCId', ['rtxc_version',
                                     'client_version',
                                     'uuid'])):
    STRUCT = Struct('<4L16s')
    MAGIC = magicToInt('uuid')
    VERSION = 0x01

    @property
    def size(self):
        return self.STRUCT.size

    @property
    def bytes(self):
        return self.STRUCT.pack(self.MAGIC,
                                self.VERSION,
                                self.rtxc_version,
                                self.client_version,
                                self.uuid.bytes)

def get_symbol_addr(mf, name):
    symbol_match = mf.symtab.matching({'name': name})
    if symbol_match:
        return symbol_match[0]['n_value']
    else:
        return None


def patch_uuid(mf, file_path, build_tag, verbose=False):
    '''
    Patch the UUID with the value provided by the linker
    '''
    uuid_address = get_symbol_addr(mf, '_UUID')
    if uuid_address is None:
        print('WARNING: No UUID symbol found. will not patch {}'.format(file_path))
    elif verbose:
        print('Found UUID container at address 0x{:x}'.format(uuid_address))

    tag_address = get_symbol_addr(mf, '_build_tag_string_contents')
    if tag_address is None:
        print('WARNING: No build tag string symbol found. will not patch {}'.format(file_path))
    elif verbose:
        print('Found build tag string container at address 0x{:x}'.format(tag_address))

    text_section = mf.section_for_name('__TEXT,__text')

    if verbose:
        print('Found text section at address 0x{:x}'.format(text_section['addr']))

    uuid = UUID(bytes=mf.uuid)

    if verbose:
        print('Identified UUID: {}'.format(uuid))

    # We're using the RTXC ID format used by other firmware teams, but setting
    # the version numbers to 0
    rtxcid = RTXCId(0,
                    0,
                    uuid)

    if verbose:
        print('Full ID structure:')
        hexdump(rtxcid.bytes)

    data = text_section['bytes']

    if uuid_address is not None:
        data_patch_addr = uuid_address - text_section['addr']
        data[data_patch_addr:data_patch_addr+rtxcid.size] = rtxcid.bytes

    if tag_address is not None and build_tag is not None:
        data_patch_addr = tag_address - text_section['addr']
        data[data_patch_addr:data_patch_addr+len(build_tag)] = build_tag

    text_section['bytes'] = data

    mf.update()


def main():
    parser = ArgumentParser(description="RTXC post processing for Mach-o files")
    parser.add_argument('-v', '--verbose',
                        action='store_true')
    parser.add_argument('--build-tag',
                        default=None,
                        help="Name of the XBS build tag or local build tag")
    parser.add_argument('file',
                        help="Mach-o file to operate on")
    args = parser.parse_args()

    with io.open(args.file, "r+b") as f:
        mf = macho.File(f, debug=False)
        patch_uuid(mf, args.file, args.build_tag, args.verbose)

if __name__ == '__main__':
    main()
