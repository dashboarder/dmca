/*
 * Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <sys/menu.h>
#include <drivers/iic.h>

#if defined(WITH_MENU) && WITH_MENU


static int do_iic(int argc, struct cmd_arg *args)
{
	if (!security_allow_modes(kSecurityModeHWAccess)) {
		printf("Permission Denied\n");
		return -1;
	}

	if (argc < 3) {
notenoughargs:
		puts("not enough arguments.\n");
usage:
		printf("%s read <bus> <iicaddr> <len>\n", args[0].str);
		printf("%s readreg <bus> <iicaddr> <reg> <len>\n", args[0].str);
		printf("%s creadreg <bus> <iicaddr> <reg> <len>\n", args[0].str);
		printf("%s readreg16 <bus> <iicaddr> <reg> <len>\n", args[0].str);
		printf("%s write <bus> <iicaddr> <data0> <data1> ...\n", args[0].str);
		printf("%s probe <bus>\n", args[0].str);
		return -1;
	}

	int bus = args[2].u;
	int addr = args[3].u;

	if (bus < 0 || bus > 3) {
		puts("unsupported bus\n");
		return -1;
	}

	if (!strcmp("read", args[1].str)) {
		uint8_t data[64];
		int err;
		size_t len = args[4].u;

		if (len == 0 || len > 64) {
			puts("invalid len\n");
			return -1;
		}

		err = iic_read(bus, addr, NULL, 0, data, len, IIC_NORMAL);
		if (err < 0) {
			printf("error %d reading from iic\n", err);
		} else {
			hexdump(data, len);
		}
	} else if (!strcmp("readreg", args[1].str)) {
		uint8_t data[64];
		uint8_t outdata;
		int err;
		size_t len = args[5].u;

		if (argc < 6)
			goto notenoughargs;

		if (len == 0 || len > 64) {
			puts("invalid len\n");
			return -1;
		}

		outdata = args[4].u & 0xff;
	
		err = iic_read(bus, addr, &outdata, 1, data, len, IIC_NORMAL);
		if (err < 0) {
			printf("error %d reading from iic\n", err);
		} else {
			hexdump(data, len);
		}
	} else if (!strcmp("creadreg", args[1].str)) {
		uint8_t data[64];
		uint8_t outdata;
		int err;
		size_t len = args[5].u;

		if (argc < 6)
			goto notenoughargs;

		if (len == 0 || len > 64) {
			puts("invalid len\n");
			return -1;
		}

		outdata = args[4].u & 0xff;
	
		err = iic_read(bus, addr, &outdata, 1, data, len, IIC_COMBINED);
		if (err < 0) {
			printf("error %d reading from iic\n", err);
		} else {
			hexdump(data, len);
		}
	} else if (!strcmp("readreg16", args[1].str)) {
		uint8_t data[64];
		uint16_t outdata;
		int err;
		size_t len = args[5].u;

		if (argc < 6)
			goto notenoughargs;

		if (len == 0 || len > 64) {
			puts("invalid len\n");
			return -1;
		}

		outdata = swap16(args[4].u & 0xffff);

		err = iic_read(bus, addr, &outdata, 2, data, len, IIC_NORMAL);
		if (err < 0) {
			printf("error %d reading from iic\n", err);
		} else {
			hexdump(data, len);
		}
	} else if (!strcmp("write", args[1].str)) {
		u_int8_t data[64];
		size_t len, i;
		int err;

		len = argc - 4;
		if (len > sizeof(data)) len = sizeof(data);

		for (i = 0; i < len; i++) {
			data[i] = args[i+4].u;
		}

		printf("writing %zu bytes of data\n", len);
		err = iic_write(bus, addr, data, len);
		if (err < 0)
			printf("error %d writing to iic\n", err);
	} else if (!strcmp("probe", args[1].str)) {

		for (addr = 1; addr < 0x7e; addr++) {
			if (iic_probe(bus, addr))
				printf("0x%02x: found\n", addr);
		}
	} else {
		puts("unrecognized command.\n");
		goto usage;
	}

	return 0;
}

MENU_COMMAND_DEVELOPMENT(iic, do_iic, "iic read/write", NULL);

#endif
