/*
 * Copyright (C) 2007-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <sys/menu.h>
#include <lib/pki.h>
#include <drivers/sha1.h>

#if defined(WITH_MENU) && WITH_MENU

int do_pki(int argc, struct cmd_arg *args)
{
	int ret_val;

	if (!security_allow_modes(kSecurityModeDebugCmd)) {
		printf("Permission Denied\n");
		return -1;
	}

	if (argc < 2) {
		printf("not enough arguments.\n");
		usage:
		printf("%s verify <cert-chain-as-concatenated-der-blobs-data> <length>  "
                        "<signature-blob-data> <length> <hash-blob-data> <length>\n", args[0].str);
		printf("\tverify-image <8900-1.0-image-data> <length>\n");
		return -1;
	}

        void *chain_data, *sig_blob, *hash_blob, *spec_blob;
        size_t chain_len, sig_blob_len, hash_blob_len, spec_blob_len;
        chain_data = sig_blob = hash_blob = spec_blob = NULL;
        chain_len = sig_blob_len = hash_blob_len = spec_blob_len = 0;

	if (!strcmp("verify", args[1].str)) {

		if (argc < 8) {
			printf("not enough arguments.\n");
			goto usage;
		}

		chain_data = (void *)args[2].u;
		chain_len = args[3].u;
		
		sig_blob = (void *)args[4].u;
		sig_blob_len = args[5].u;
		
		hash_blob = (void *)args[6].u;
		hash_blob_len = args[7].u;

	} else if (!strcmp("verify-image", args[1].str)) {

                if (argc < 4) {
                        printf ("not enough arguments.\n");
                        goto usage;
                }
                
                if (!strcmp("89001.0", (void*)args[2].u)) {
                        printf ("invalid image (only 8900 1.0 images)\n");
                        return -1;
                }

                uint8_t *payload = (uint8_t*)(args[2].u + 0x800);
                uint8_t *signature = payload + *(uint32_t*)(args[2].u + 16);
                uint8_t *chain = payload + *(uint32_t*)(args[2].u + 20);

                chain_data = chain;
                chain_len = args[2].u + args[3].u - (size_t)chain;

                sig_blob = signature;
                sig_blob_len = chain - signature;
                
                /* abuse part of header as scratch */
                uint8_t sha1_buf[20];
                /* header + image are hashed for rsa verification */
                sha1_calculate((uint8_t*)args[2].u, (size_t)signature - args[2].u, sha1_buf);
                //sha1_calculate(payload, signature - payload, sha1_buf);

                hash_blob = sha1_buf;
                hash_blob_len = sizeof(sha1_buf);

	} else
                return -1;

        ret_val = verify_signature_with_chain(chain_data, chain_len, 
                sig_blob, sig_blob_len, hash_blob, hash_blob_len,
                &spec_blob, &spec_blob_len, NULL, NULL);

        printf("signature verification returned: %d\n", ret_val);

	return 0;
}

// Turned off to save space, enable as needed
// MENU_COMMAND_DEBUG(pki, do_pki, "PKI", NULL);

#endif
