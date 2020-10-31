# Copyright (C) 2014 Apple, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple, Inc.
#
function mktemp(template, _cmd, _tmpfile) {
	_cmd = "mktemp " template ".XXXXXX"
	_cmd | getline _tempfile
	close(_cmd)
	return _tempfile
}

BEGIN {
	FS="[ \t]*"
	if (ARGV[1] == "") {
		print "ERROR: Missing path argument for output\n"
		exit 1
	}
	if (system("test -d " ARGV[1])) {
		print "ERROR: " ARGV[1] " is not a directory"
		exit 1
	}
	headerfile = mktemp(ARGV[1] "/mib_nodes.h")
	print "/* AUTOMATICALLY GENERATED - DO NOT EDIT */\n\n" > headerfile

	ssnum = 0
	ssinc = 2 ^ 24
	nnum = 0
	nninc = 2 ^ 16
}

$1=="subsystem" {
	subsystem = $2
	ssnum += ssinc

	printf "#define kMIBSubsystem%s	0x%08x\n", subsystem, ssnum >> headerfile
}

$1=="node" {
	node = $2
	type = $3
	nnum += nninc

	printf "#define kMIB%s%s	0x%08x\n", subsystem, node, ssnum + nnum >> headerfile
}

END {
	system("mv " headerfile " " ARGV[1] "/mib_nodes.h")
}
