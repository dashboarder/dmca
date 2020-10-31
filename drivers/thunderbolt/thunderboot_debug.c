/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#include <drivers/thunderbolt/thunderboot.h>
#include <sys/menu.h>
#include <lib/cksum.h>
#include <lib/env.h>
#include <platform/memmap.h>


//===================================================================================
//			Thunderboot command stuff
//===================================================================================

#if defined(WITH_MENU) && WITH_MENU

#define USB_CMD_STRING_LEN      1024

#define USB_SERIAL_CMD_START        0xfc
#define USB_SERIAL_CMD_END          0xfd
#define USB_SERIAL_CMD_START_OFFSET 1
#define USB_SERIAL_CMD_END_OFFSET   1
#if WITH_DFU_UPLOAD
#define MAX_NUM_CHAR_IN_INT         (12)
#endif // WITH_DFU_UPLOAD

static u_int8_t usb_cmd_string[USB_CMD_STRING_LEN] __attribute__ ((aligned(CPU_CACHELINE_SIZE)));

static int tbt_cmd_main(int argc, struct cmd_arg * args);


MENU_COMMAND_DEVELOPMENT(tbt, tbt_cmd_main, "run a thunderboot command", NULL);

static void tbt_print_usage(const char * s)
{
	printf("Usage: \n");
	printf("\t%s get <filename> [address] [amount] \n", s);
	printf( "\t%s get tftp://servername.com/file.bin [address] [amount] \n", s );
#if WITH_DFU_UPLOAD
	printf( "\t%s put <filename> <amount> [address] \n", s );
#endif // WITH_DFU_UPLOAD
}

static int tbt_cmd_get(int argc, struct cmd_arg *args)
{
	int ret = -1;
	
	// XXX: We should check to see if a login has happened, and
	// fail if not
    
	const char *prefix = "get ";
	const char * fileName = args[0].str;
	int notifyHostLen = strlen(prefix) + strlen(fileName) +
		USB_SERIAL_CMD_START_OFFSET + USB_SERIAL_CMD_END_OFFSET;
    
	addr_t address = 0;
	uint64_t amount = 0;
	
	if (notifyHostLen > USB_CMD_STRING_LEN) {
		printf("File length too long \n");
		goto error;
	}
	
	if (argc >= 2) {
		address = args[1].u;
		if(argc >= 3) {
			amount = args[2].u;
		}
	}
	
	if (address == 0) {
		address = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);
		printf("Defaulting to address %p\n", address);
	}
	
	if (amount > DEFAULT_RAMDISK_SIZE) {
		printf("Amount too large \n");
		goto error;
	}
	
	if (amount == 0) {
		amount = DEFAULT_RAMDISK_SIZE;
	}
	
	if (!security_allow_memory((void *)address, amount)) {
		dprintf(DEBUG_INFO, "Permission Denied\n");
		goto error;
	}
	
	thunderboot_transfer_prepare((void *)address, amount);
	bzero((void *)usb_cmd_string, USB_CMD_STRING_LEN);
	usb_cmd_string[0] = USB_SERIAL_CMD_START;
	memcpy(usb_cmd_string + USB_SERIAL_CMD_START_OFFSET, prefix, strlen(prefix));
	memcpy(usb_cmd_string + USB_SERIAL_CMD_START_OFFSET + strlen(prefix), fileName,
		   notifyHostLen - USB_SERIAL_CMD_START_OFFSET - USB_SERIAL_CMD_END_OFFSET);
	usb_cmd_string[notifyHostLen - 1] = USB_SERIAL_CMD_END;
	
	ret = thunderboot_serial_send_cmd_string(usb_cmd_string, notifyHostLen);
	
	if (ret != 0) {
		goto error;
	}
	
	ret = thunderboot_transfer_wait();
	
	if (ret <= 0) {
		goto error;
	}
    
	amount = ret;
	
	printf("GET complete, amount %d\n", amount);
	
	// set the file size and return code
	env_set_uint("filesize", amount, 0);
	
	ret = 0;
    
	return 0;
	
    error :
	printf("GET failed. \n");
	env_set_uint("filesize", 0, 0);
	return -1;
}

static int tbt_cmd_main(int argc, struct cmd_arg *args)
{
	int ret = 0;
	const char *s = args[1].str;
    
	if (s == NULL) {
		ret = -1;
	}
	else {
		if (strcmp(s, "get") == 0) {
            
			if (argc < 3 || argc > 5) {
				goto print_usage;
			}
            
			ret = tbt_cmd_get(argc - 2, args + 2);
		}
#if 0 && WITH_DFU_UPLOAD
		else if(!strcmp(s, "put")) {
            
			if((argc < 4) || (argc > 5)) {
				goto print_usage;
			}
            
			ret = usb_cmd_put(args[2].str, args[3].u, (argc == 5) ? args[4].u : 0, 1);
		}
#endif // WITH_DFU_UPLOAD
		else {
			goto print_usage;
		}
	}
    
	return ret;
    
print_usage:
	tbt_print_usage( args[0].str );
	return 0;
    
}

#endif
