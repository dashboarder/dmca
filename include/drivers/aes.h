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
#ifndef __DRIVERS_AES_H
#define __DRIVERS_AES_H

#include <sys/types.h>

__BEGIN_DECLS

#define AES_CMD_ENC		(0x00000000)
#define AES_CMD_DEC		(0x00000001)
#define AES_CMD_DIR_MASK	(0x0000000F)
#define AES_CMD_ECB		(0x00000000)
#define AES_CMD_CBC		(0x00000010)
#define AES_CMD_MODE_MASK	(0x000000F0)

#define AES_KEY_TYPE_USER	(0x00000000)
#define AES_KEY_TYPE_UID0	(0x00000100)
#define AES_KEY_TYPE_GID0	(0x00000200)
#define AES_KEY_TYPE_GID1	(0x00000201)
#define AES_KEY_TYPE_MASK	(0x00000FFF)

#define AES_KEY_SIZE_128	(0x00000000)
#define AES_KEY_SIZE_192	(0x10000000)
#define AES_KEY_SIZE_256	(0x20000000)
#define AES_KEY_SIZE_MASK	(0xF0000000)

#define AES_BLOCK_SIZE		(16)

/*
 *  aes_crypto_cmd - provides access to AES functions
 *
 *    cmd  - operation mode: ENC or DEC and EBC or CBC
 *    src  - source buffer and destination buffer for in place operations
 *    dst  - desination buffer, or NULL for in place operations
 *    len  - size in bytes of the operation
 *    opts - key type and size
 *    key  - key buffer or NULL for non USER key types
 *    iv   - iv buffer or NULL if iv should be all zeros
 *
 * AES operations may return an error if the requested operation depends
 * on hardware resources that are not available.
 *
 * Bad arguments passed to AES operations will result in a panic.
 */
int aes_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv);

#define aes_cbc_encrypt(src, dst, len, opts, key, iv) \
		aes_crypto_cmd(AES_CMD_ENC | AES_CMD_CBC, src, dst, len, opts, key, iv)
#define aes_cbc_decrypt(src, dst, len, opts, key, iv) \
		aes_crypto_cmd(AES_CMD_DEC | AES_CMD_CBC, src, dst, len, opts, key, iv)

/* exported interface from hardware AES driver */
int aes_hw_crypto_cmd(u_int32_t cmd, void *src, void *dst, size_t len, u_int32_t opts, const void *key, void *iv);

/* old AES hardware driver protocol, not for new platforms */
enum aes_key_type {
	AES_KEY_TYPE_REGISTER = 0,
	AES_KEY_TYPE_GLOBAL,
	AES_KEY_TYPE_CHIP
};

int AES_CBC_DecryptInPlace(u_int8_t* pAddr, u_int32_t u32Len, u_int32_t keyType, u_int8_t* pKey, u_int8_t* pIv);
bool AES_CBC_EncryptInPlace(u_int8_t* pAddr, u_int32_t u32Len, u_int32_t keyType, u_int8_t* pKey, u_int8_t* pIv);

__END_DECLS

#endif /* __DRIVERS_AES_H */
