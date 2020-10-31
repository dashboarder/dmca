/*
 * Copyright (C) 2007-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/aes.h>
#include <sys/menu.h>
#include <sys.h>

#if WITH_CORECRYPTO

#include <corecrypto/ccaes.h>
#include <corecrypto/ccmode.h>

static const struct ccmode_cbc *aes_get_ccaes_cbc_decrypt_mode()
{
#if WITH_VFP
	return ((const struct ccmode_cbc *)ccaes_cbc_decrypt_mode());
#else
	return (&ccaes_gladman_cbc_decrypt_mode);
#endif	
}

static const struct ccmode_cbc *aes_get_ccaes_cbc_encrypt_mode()
{
#if WITH_VFP
	return ((const struct ccmode_cbc *)ccaes_cbc_encrypt_mode());
#else
	return (&ccaes_gladman_cbc_encrypt_mode);
#endif	
}

static const struct ccmode_ecb *aes_get_ccaes_ecb_decrypt_mode()
{
#if WITH_VFP
	return ((const struct ccmode_ecb *)ccaes_ecb_decrypt_mode());
#else
	return (&ccaes_ltc_ecb_decrypt_mode);
#endif	
}

static const struct ccmode_ecb *aes_get_ccaes_ecb_encrypt_mode()
{
#if WITH_VFP
	return ((const struct ccmode_ecb *)ccaes_ecb_encrypt_mode());
#else
	return (&ccaes_ltc_ecb_encrypt_mode);
#endif	
}

#endif // WITH_CORECRYPTO

int
aes_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv)
{
	/* argument sanity */
	if (0 != (len % AES_BLOCK_SIZE))
		goto fail;
	if ((AES_KEY_TYPE_USER == (opts & AES_KEY_TYPE_MASK)) && (NULL == key))
		goto fail;
	switch (cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC :	break;
		case AES_CMD_DEC :	break;
		default :		goto fail;
	}

#if WITH_CORECRYPTO
	u_int32_t key_size;

	// Ops with explicit user key will be handled by libcorecrypto (SW AES)
	if (AES_KEY_TYPE_USER == (opts & AES_KEY_TYPE_MASK)) {
		switch (opts & AES_KEY_SIZE_MASK) {
			case AES_KEY_SIZE_128:
				key_size = CCAES_KEY_SIZE_128;
			break;

			case AES_KEY_SIZE_192:
				key_size = CCAES_KEY_SIZE_192;
			break;

			case AES_KEY_SIZE_256:
				key_size = CCAES_KEY_SIZE_256;
			break;

			default:
				panic("invalid AES key size");
			break;
		}

		if (likely((cmd & AES_CMD_MODE_MASK) == AES_CMD_CBC)) {
			const struct ccmode_cbc *mode;

			if (likely((cmd & AES_CMD_DIR_MASK) == AES_CMD_DEC))
				mode = aes_get_ccaes_cbc_decrypt_mode();
			else
				mode = aes_get_ccaes_cbc_encrypt_mode();

			cccbc_one_shot(mode, key_size, key, iv, (len / AES_BLOCK_SIZE), src, dst);
		}
		else {
			const struct ccmode_ecb *mode;

			if (likely((cmd & AES_CMD_DIR_MASK) == AES_CMD_DEC))
				mode = aes_get_ccaes_ecb_decrypt_mode();
			else
				mode = aes_get_ccaes_ecb_encrypt_mode();

			ccecb_one_shot(mode, key_size, key, len, src, dst);
		}
		
		return 0;
	}
#endif

#if WITH_HW_OLD_AES
	u_int32_t keyType = 0;

	/* Only supports UID0, GID0 and USER keys */
	switch (opts & AES_KEY_TYPE_MASK) {
	       case AES_KEY_TYPE_USER : keyType = AES_KEY_TYPE_REGISTER; break;
	       case AES_KEY_TYPE_UID0 : keyType = AES_KEY_TYPE_CHIP; break;
	       case AES_KEY_TYPE_GID0 : keyType = AES_KEY_TYPE_GLOBAL; break;
	       default: goto fail;
	}

	/* Only 128 bit keys are supported */
	if ((opts & AES_KEY_SIZE_MASK) != AES_KEY_SIZE_128) goto exit;

	/* Only CBC mode is supported */
	if ((cmd & AES_CMD_MODE_MASK) != AES_CMD_CBC) goto exit;

	if (dst == NULL)
		dst = src;
	if (dst != src)
		memcpy(dst, src, len);

	switch (cmd & AES_CMD_DIR_MASK) {
		case AES_CMD_ENC :
			(void)AES_CBC_EncryptInPlace(dst, len, keyType, (void *)key, iv);
			break;

		case AES_CMD_DEC :
			(void)AES_CBC_DecryptInPlace(dst, len, keyType, (void *)key, iv);
			break;
	}
#elif WITH_HW_AES
	if (aes_hw_crypto_cmd(cmd, src, dst, len, opts, key, iv))
		return(-1);
	goto exit;
#else
#error	"aes_crypto_cmd: no valid implementation\n";
#endif

exit:
	dprintf(DEBUG_SPEW, "aes_crypto_cmd: cmd: %08x, src: %p, dst: %p, len: %08zx, opts: %08x, key: %p, iv: %p \n",
		cmd, src, dst, len, opts, key, iv);

	return(0);

fail:
	panic("AES: bad arguments");
}

int
do_aes_cmd(int argc, struct cmd_arg *args)
{
	u_int8_t	pt[32];
	u_int8_t	ct[32];
	u_int8_t	k[16];
	u_int8_t	iv[16];
	int		i;

	/* seed the plaintext */
	for (i = 0; i < 32; i++) { 
		pt[i] = system_time() & 0xff;
	}
  	
	memset(k, 0x55, sizeof(k));
	memset(iv, 0, sizeof(iv));

	if (argc > 1) {
		if (strcmp(args[1].str, "uid0") == 0) {
			printf("uid0\n");
			aes_cbc_encrypt(k, k, sizeof(k), AES_KEY_TYPE_UID0, NULL, NULL);
		}
		else if (strcmp(args[1].str, "gid0") == 0) {
			printf("gid0\n");
			aes_cbc_encrypt(k, k, sizeof(k), AES_KEY_TYPE_GID0, NULL, NULL);
		}
		else if (strcmp(args[1].str, "gid1") == 0) {
			printf("gid1\n");
			aes_cbc_encrypt(k, k, sizeof(k), AES_KEY_TYPE_GID1, NULL, NULL);
		}
	}

	printf("AES plaintext input:\n");
	hexdump(pt, 32);
	memset(ct, 0, 32);
	aes_crypto_cmd(
		AES_CMD_ENC | AES_CMD_CBC,
		&pt[0],
		&ct[0],
		32,
		AES_KEY_TYPE_USER | AES_KEY_SIZE_128,
		k,
		iv);
	printf("AES ciphertext result:\n");
	hexdump(ct, 32);
	memset(pt, 0, 32);
	aes_crypto_cmd(
		AES_CMD_DEC | AES_CMD_CBC,
		&ct[0],
		&pt[0],
		32,
		AES_KEY_TYPE_USER | AES_KEY_SIZE_128,
		k,
		iv);
	printf("AES plaintext result:\n");
	hexdump(pt, 32);

	return 0;
}

int
do_aes_golden_vec_cmd(int argc, struct cmd_arg *args)
{
	uint8_t ct[16];
	uint8_t pt[16];

	printf("encrypt:\n");

	for (int i = 0; i < 10; i++) {
		memset(pt, 0, sizeof(pt));
		memset(ct, 0x55, sizeof(pt));
		pt[0] = i;

		printf("src: [%p]", pt);
		for (unsigned int j = 0; j < sizeof(pt); j++)
			printf(" %02x", pt[j]);
		printf("\n");

		if (aes_crypto_cmd(AES_CMD_ENC | AES_CMD_CBC, pt, ct, sizeof(pt), AES_KEY_TYPE_GID0 | AES_KEY_SIZE_256, NULL, NULL) != 0) {
			printf("encyrpt failed\n");
			return -1;
		}

		printf("dst: [%p]", ct);
		for (unsigned int j = 0; j < sizeof(ct); j++)
			printf(" %02x", ct[j]);
		printf("\n");
	}

	printf("decrypt\n");

	for (int i = 0; i < 10; i++) {
		memset(ct, 0, sizeof(pt));
		memset(pt, 0x55, sizeof(pt));
		ct[0] = i;

		printf("src: [%p]", ct);
		for (unsigned int j = 0; j < sizeof(ct); j++)
			printf(" %02x", ct[j]);
		printf("\n");

		if (aes_crypto_cmd(AES_CMD_DEC | AES_CMD_CBC, ct, pt, sizeof(pt), AES_KEY_TYPE_GID0 | AES_KEY_SIZE_256, NULL, NULL) != 0) {
			printf("decrypt failed\n");
			return -1;
		}

		printf("dst: [%p]", pt);
		for (unsigned int j = 0; j < sizeof(pt); j++)
			printf(" %02x", pt[j]);
		printf("\n");
	}

	return 0;
}


// Turned off to save space, enable as needed
//MENU_COMMAND_DEBUG(aes, do_aes_cmd, "AES test", NULL);
//MENU_COMMAND_DEBUG(aes_golden_vec, do_aes_golden_vec_cmd, "AES golden vector test", NULL);
