# Copyright (c) 2007 Apple Inc. All Rights Reserved
#
# Customise the protocol header file to reflect a specific build of the firmware.
#
# The following environment variables are expected:
#
# OBJFILE	Firmware object file
# BINFILE	Binary ROM data file
# NM		An 'nm' suitable for the object file
#

BEGIN { FS = ":" }

# eat lines that ask for it
/EATLINE/ {
	next
}

# by default, copy input to output
$1!="/*SUBST" { 
	print $0 
	next
}

# read a string value from the environment
$1=="/*SUBST" && $2=="ENVSTR"	{ 
	if (ENVIRON[$4] != "") {
		printf "%s\"%s\"%s\n", $3, ENVIRON[$4], $5
	}
	next
}

# read a variable from the environment
$1=="/*SUBST" && $2=="ENV"	{ 
	if (ENVIRON[$4] != "") {
		printf "%s%s%s\n", $3, ENVIRON[$4], $5
	}
	next
}

# look up a variable in the object file
$1=="/*SUBST" && $2=="VAR"	{ 
	cmd = sprintf("%s %s | grep \" %s\" | cut -f 1 -d ' '", ENVIRON["NM"], ENVIRON["OBJFILE"], $4)
	if (cmd | getline value == 0) {
	    value = "ffffffff"
	}
	printf "%s0x%s%s\n", $3, value, $5
	next
}

# format the binary file as hex and insert it
$1=="/*SUBST" && $2=="FILE"	{
	format = "cat %s | dd obs=16 conv=osync 2>/dev/null | hexdump -v -e '~\t~ 16/1 ~0x%%02x,~ ~\n~'"
	gsub(/~/, "\"", format)
	cmd = sprintf(format, ENVIRON["BINFILE"])
	system(cmd)
	next
}

# insert the file named via the environment
$1=="/*SUBST" && $2=="INSERT"	{
	if (ENVIRON[$3] != "") {
		cmd = sprintf("cat %s", ENVIRON[$3])
		system(cmd)
	}
	next
}

# eat the next line
$1=="/*SUBST" && $2=="EATLINE"	{
	next
}

