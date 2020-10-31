/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <stdio.h>
#include <string.h>
#include <sys.h>
#include <sys/menu.h>

#if defined(WITH_MENU) && WITH_MENU

/* "md", "mdh", "mdb" */
int do_memdump(int argc, struct cmd_arg *args)
{
	addr_t address;
	size_t count;
	int width;
	size_t i;

	/* default dump values */
	static addr_t last_address = 0;
	static size_t last_count = 0x100;

	address = last_address;
	count = last_count;

	if (!strcmp(args[0].str, "md")) {
		width = 32;
	} else if (!strcmp(args[0].str, "mdh")) {
		width = 16;
	} else {
		width = 8;
	}

	if (argc >= 2) 
		address = args[1].u;
	if (argc >= 3)
		count = args[2].u;
	
//	printf("dumping memory at 0x%x, len 0x%x, width %d\n", address, count, width);

	i = 0;
	while (i < count) {
		if ((i % 16) == 0) {
			if (i != 0)
				puts("\n");
			printf("%p: ", (void *)(address + i));
		}

		switch (width) {
			case 32:
				printf("%08x ", *(uint32_t *)(address + i));
				if ( 4 == (i & 0xf))
				  printf(" ");
				i += 4;
				break;
			case 16:
				printf("%04x ", *(uint16_t *)(address + i));
				if ( 6 == (i & 0xf))
				  printf(" ");
				i += 2;
				break;
			case 8:
				printf("%02x ", *(uint8_t *)(address + i));
				if ( 7 == (i & 0xf))
				  printf(" ");
				i += 1;
				break;
		}
	}
	puts("\n");

	/* save the values so we can continue next time */
	last_count = count;
	last_address = address + count;

	return 0;
}

MENU_COMMAND_DEBUG(md, do_memdump, "memory display - 32bit", NULL);
MENU_COMMAND_DEBUG(mdh, do_memdump, "memory display - 16bit", NULL);
MENU_COMMAND_DEBUG(mdb, do_memdump, "memory display - 8bit", NULL);

/* "mw", "mwh", "mwb", "mws" */
int do_memwrite(int argc, struct cmd_arg *args)
{
	static addr_t last_address = ~0UL;
	addr_t address;
	size_t length;
	uint32_t data;
	const char *buffer;
	int width;

	switch (argc) {
	case 2:
		if (~0UL == last_address) {
			printf("%s - need an address\n", args[0].str);
			return -1;
		}
		address = last_address;
		data = args[1].u;
		buffer = args[1].str;
		break;
	case 3:
		address = args[1].u;
		last_address = address;
		data = args[2].u;
		buffer = args[2].str;
		break;
	default:
		printf("%s [<address>] <data>\n", args[0].str);
		return -1;
	}

	if (!strcmp(args[0].str, "mw")) {
		width = 32;
		length = 4;
	} else if (!strcmp(args[0].str, "mwh")) {
		width = 16;
		length = 2;
	} else if (!strcmp(args[0].str, "mwb")) {
		width = 8;
		length = 1;
	} else {
		width = 255;
		length = strlen(buffer) + 1;
	}

//	printf("writing memory at 0x%x, data 0x%x\n", address, data);

	switch (width) {
		case 32:
			*(uint32_t *)address = data;
			break;
		case 16:
			*(uint16_t *)address = data;
			break;
		case 8:
			*(uint8_t *)address = data;
			break;

		case 255:
			strlcpy((char *)address, buffer, length);
			break;
	}

	return 0;
}

MENU_COMMAND_DEBUG(mw, do_memwrite, "memory write - 32bit", NULL);
MENU_COMMAND_DEBUG(mwh, do_memwrite, "memory write - 16bit", NULL);
MENU_COMMAND_DEBUG(mwb, do_memwrite, "memory write - 8bit", NULL);
MENU_COMMAND_DEBUG(mws, do_memwrite, "memory write - string", NULL);

int do_panic(int argc, struct cmd_arg *args)
{
	panic("command prompt");
}
MENU_COMMAND_DEBUG(panic, do_panic, "...", NULL);

int do_hang(int argc, struct cmd_arg *args)
{
	for (;;) 
		;

	return(0);
}
MENU_COMMAND_DEBUG(hang, do_hang, "spin forever, hanging the system", NULL);


#endif
