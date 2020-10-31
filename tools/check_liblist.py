#!/usr/bin/env python2.7

# Check generated build/library_list for any library~arch which will be
# built with more than one set of flags. The flags are not used to
# uniquely identify a built library, so we instead end up with a
# non-deterministic build where one set of flags builds last, and ends
# up linked into all application/product executables.

import sys

def main():
    lib_flags_sets = {}
    for line in open(sys.argv[1], 'r').readlines():
        # Split each line by '~' delimiter
        parts = line.strip().split('~')
        # Re-join first two components in target library
        lib = ' '.join(parts[0:2])
        # All other components into flags
        flags = ' '.join(parts[2:])
        # Note that this lib has these flags in use
        lib_flags_sets.setdefault(lib, {})[flags] = None
    # Check if any library is attempting to be built with more than one
    # set of flags.
    ok = True
    for lib, flags_set in lib_flags_sets.items():
        if len(flags_set) == 1:
            continue
        ok = False
        sys.stderr.write('Multiple flag sets for library %s:\n' % lib)
        for flags in flags_set.keys():
            sys.stderr.write('  %s  \n' % flags)
    return 0 if ok else 1

if __name__ == '__main__':
    sys.exit(main())
