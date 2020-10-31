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
#ifndef THUNDERBOLT_NHI_H
#define THUNDERBOLT_NHI_H

#include <drivers/pci.h>
#include <stdint.h>

#define NHI_STATUS_OK				(0)
#define NHI_STATUS_UNKNOWN_ERROR		(-1)
#define NHI_STATUS_CRC_ERROR			(-2)
#define NHI_STATUS_INVALID_DESCRIPTOR		(-3)
#define NHI_STATUS_TIMEOUT			(-4)

typedef struct nhi_sgl {
	void *buffer;
	uint32_t bytes;
	struct nhi_sgl *next;
} nhi_sgl_t;

typedef struct nhi nhi_t;

nhi_t *nhi_init(pci_device_t bridge, uint32_t dart_id);
void nhi_quiesce_and_free(nhi_t *nhi);
void nhi_init_tx_ring(nhi_t *nhi, uint32_t ring_idx, uint32_t num_buffers, uint32_t buffer_size);
void nhi_init_rx_ring(nhi_t *nhi, uint32_t ring_idx, uint32_t num_buffers, uint32_t buffer_size);
void nhi_disable_tx_ring(nhi_t *nhi, uint32_t ring_idx);
void nhi_disable_rx_ring(nhi_t *nhi, uint32_t ring_idx);
int32_t nhi_send_sgl(nhi_t *nhi, uint32_t ring_idx, const nhi_sgl_t *sgl, uint8_t sof_pdf, uint8_t eof_pdf);
int32_t nhi_send_buffer(nhi_t *nhi, uint32_t ring_idx, void *buffer, uint32_t bytes, uint8_t sof_pdf, uint8_t eof_pdf);
bool nhi_rx_buffer_available(nhi_t *nhi, uint32_t ring_idx);
int32_t nhi_rx_buffer(nhi_t *nhi, uint32_t ring_idx, void *buffer, uint32_t bytes, uint8_t *sof_pdf, uint8_t *eof_pdf);

#endif
