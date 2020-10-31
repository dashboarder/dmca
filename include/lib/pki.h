/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __LIB_PKI_H
#define __LIB_PKI_H

#include <sys/types.h>

__BEGIN_DECLS

int verify_signature_with_chain(
			void *chain_blob, size_t chain_blob_length,
			void *sig_blob, size_t sig_blob_len,
			void *hash_blob, size_t hash_blob_len,
			void **img3_spec_blob, size_t *img3_spec_blob_len,
			void **ticket_spec_blob, size_t *ticket_spec_blob_len);

__END_DECLS

#endif
