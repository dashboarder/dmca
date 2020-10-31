#!/usr/bin/python
import os
import hashlib
import hmac
import argparse
import sys

argparser = argparse.ArgumentParser(description='Generates obfuscated logging file IDs for iBoot release builds')
argparser.add_argument('--src', required=True, help='iBoot source tree')

args = argparser.parse_args()

id_file_map = {}

directory = args.src

base_hmac = hmac.new('obfuscation of filename for release build logging', '', hashlib.sha1)

for (path, dirnames, filenames) in os.walk(directory):
    # strip the part of the pathname we supplied off the pathname
    shortened_path = path[len(directory)+1:]
    # don't iterate into paths we don't care about

    thisdir = os.path.split(shortened_path)[1]

    if thisdir in ['build', '.git', '.svn']:
        del dirnames[:]
        continue
    for filename in filenames:
        # ignore file types we don't care about
        extension = os.path.splitext(filename)[1]
        if extension not in ['.c', '.cpp', '.s', '.S', '.h']:
            continue

        full_path = os.path.join(shortened_path, filename)

        current_hmac = base_hmac.copy()
        current_hmac.update(full_path)
        
        file_id = current_hmac.hexdigest()[:16]

        if file_id in id_file_map and id_file_map[file_id] != full_path:
            print >> sys.stderr, 'Error: Files "{}" and "{}" both have the same ID {}'.format(full_path, id_file_map[file_id], file_id)
            sys.exit(1)
        else:
            id_file_map[file_id] = full_path
            print '{}:{}'.format(full_path, file_id)
