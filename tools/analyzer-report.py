#!/usr/bin/env python
#
# Copyright (c) 2011 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

import os
import sys
import linecache
import cgi
import operator
import xml.sax.saxutils as saxutils


Usage='''
analyzer-report build_output_dir suppressfile
'''
ignored_hashes = None
def get_source_line(filename, linenum):
    return linecache.getline(filename, int(linenum)).strip();

def process_report_file(filename):
    buginfo={}
    def populate(comment_name, friendly_name, line):
        if friendly_name not in buginfo and comment_name in line:
            front_slice = len("<!-- %s " % comment_name)
            end_slice = len(" -->")
            buginfo[friendly_name] = line[front_slice:-end_slice].strip()
            return True;
        return False;
    actions = { "*****Not a filename*******": "report_file", "BUGDESC": "description", "BUGTYPE": "type", 
                "BUGCATEGORY": "category", "BUGFILE": "source_file",
                "BUGLINE": "line_num"}
    buginfo['report_file'] = filename
    with open(filename) as f:
        for line in f:
            try:
                for (cn, fn) in actions.items():
                    if populate(cn, fn, line): raise ValueError, "Continue"
            except ValueError: continue # Lazy hack. Nothing in here uses ValueError.
    if set(actions.values()) == set(buginfo.keys()):
        buginfo["source_line"] = get_source_line(buginfo['source_file'], buginfo['line_num']);
        return buginfo
    return None


'''
Turns a buginfo dict into a "stable" bug hash. This is meant to be used to
mark problems as ignored.
'''
def bughash(buginfo):
    return "[%s # %s # %s]" % (buginfo['description'], buginfo['source_line'], os.path.basename(buginfo['source_file']))


def generate_report_file(buginfos, misc, outfile):
    with(open(outfile, "w")) as out:
        print >>out, "<html><head><title>Clang analyzer report for %s</title></head>" % sys.argv[1]
        print >>out, "<table border=1><tr><th>Source file<th>Bug description</th><th>bughash</th>"
        for bug in sorted(buginfos, key=operator.itemgetter('source_file', 'description')):
            if is_ignored(bughash(bug)): continue
            print >>out, "  <tr>\n"
            print >>out, "    <td><a href={}>{}</a></td>\n".format(saxutils.quoteattr(bug['report_file']), saxutils.escape(bug['source_file']))
            print >>out, "    <td>{}</td>\n".format(saxutils.escape(bug['description']))
            print >>out, "    <td><input size=30 readonly=\"true\" value={}></input></td>\n".format(saxutils.quoteattr(bughash(bug)))
            print >>out, "  </tr>\n"
        print >>out, "</table><hr><pre>%s</pre></html>" % saxutils.escape(misc)
def is_ignored(i):
    global ignored_hashes
    if ignored_hashes is None:
        ignored_hashes = set()
        for line in open(ignorefile):
            if line.strip() and not line.startswith("#"):
                ignored_hashes.add(line.strip())
    return i in ignored_hashes
def run_misc(buginfos):
    import cStringIO
    s = cStringIO.StringIO()
    ignored = [bughash(i) for i in buginfos if is_ignored(bughash(i))]
    not_ignored = [bughash(i) for i in buginfos if not is_ignored(bughash(i))]
    print >>s, "Ignored %d bug(s) because of suppress file" % len(ignored)
    
    if ignored_hashes:
        for h in ignored_hashes:
            if h not in ignored:
                print >>s, "Warning: Possible stale bughash in suppress file: %s" % h
    return s.getvalue()

ignorefile = os.path.abspath(sys.argv[2])
os.chdir(sys.argv[1])
buginfos=[]
for folder, subs, files in os.walk("."):
    for fn in files:
        if fn.endswith(".html"):
            buginfo = process_report_file(os.path.join(folder,fn))
            if(buginfo):
                buginfos.append(buginfo)


generate_report_file(buginfos, run_misc(buginfos), "index.html")
