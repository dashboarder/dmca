#!/usr/bin/python
import re
import os
import hashlib
import hmac
import argparse
import sys

argparser = argparse.ArgumentParser(description='Parses obfuscated logging from iBoot release builds')
argparser.add_argument('--src', required=True, help='iBoot source tree corresponding to log')
argparser.add_argument('-v', '--verbose', action='store_true', default=False, help='Attempts to load log string from source file instead of simplying printing the source file and line number')
argparser.add_argument('filename', nargs='?', default=None, help='File to read log from (omit for stdin)')

args = argparser.parse_args()

id_file_map = {}

directory = args.src

base_hmac = hmac.new('obfuscation of filename for release build logging', '', hashlib.sha1)

for (path, dirnames, filenames) in os.walk(directory):
    # strip the part of the pathname we supplied off the pathname
    shortened_path = path[len(directory)+1:]
    # don't iterate into paths we don't care about
    try:
        dirnames.remove('.svn')
    except ValueError:
        pass
    # only look at .c and .cpp files in the build directory, be more permissive elsewhere
    if shortened_path.startswith('build/'):
        filenames = [x for x in filenames if x.endswith('.c') or x.endswith('.cpp')]
    for filename in filenames:
        full_path = os.path.join(shortened_path, filename)

        current_hmac = base_hmac.copy()
        current_hmac.update(full_path)
        
        file_id = current_hmac.hexdigest()[:16]

        if file_id in id_file_map and id_file_map[file_id] != full_path:
            print 'Warning: Files "{}" and "{}" both have the same ID {}'.format(full_path, id_file_map[file_id], file_id)
        else:
            id_file_map[file_id] = full_path

file_cache = {}
def get_src_line(filename, line_number):

    if filename not in file_cache:
        file_path = os.path.join(directory, filename)
        with open(file_path) as f:
            lines = [x.strip() for x in f]
            file_cache[filename] = lines

    return file_cache[filename][line_number - 1]

def parse_src_line(src_line):
    result = re.match(r'dprintf\s*\(\s*[a-zA-Z0-9_]+,\s*"([^"]*)".*', src_line)

    if result is not None:
        return result.group(1)
    else:
        return src_line

def parse_log(f):
    log_re = re.compile(r'^(.*\|)?([0-9a-f]{16}):(\d+)$')
    for line in f:
        line = line.rstrip()
        result = log_re.match(line)
        if result is not None:
            timestamp = result.group(1) or ''
            file_id = result.group(2)
            line_number = int(result.group(3))
            if file_id in id_file_map:
                filename = id_file_map[file_id]
                if args.verbose:
                    src_line = get_src_line(filename, line_number)
                    print '{}{} [{}:{}]'.format(timestamp, parse_src_line(src_line), filename, line_number)
                else:
                    print '{}{}:{}'.format(timestamp, filename, line_number)
                continue
        print line

if args.filename is not None:
    with open(args.filename) as f:
        parse_log(f)
else:
    parse_log(sys.stdin)
