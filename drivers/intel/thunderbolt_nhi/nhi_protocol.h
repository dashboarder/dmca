#ifndef NHI_PROTOCOL_H
#define NHI_PROTOCOL_H

#define NHI_CLASS_CODE		(0x088000)
#define NHI_VENDOR_ID		(0x8086)

typedef struct nhi_rx_buffer_desc {
	uint32_t addr_lo;
	uint32_t addr_hi;
	union {
		struct {
			uint32_t length:12;
			uint32_t eof_pdf:4;
			uint32_t sof_pdf:4;
			uint32_t crc_error:1;
			uint32_t descriptor_done:1;
			uint32_t request_status:1;
			uint32_t interrupt_enable:1;
			uint32_t offset:8;
		};
		uint32_t details;
	};
	uint32_t timestamp;
} nhi_rx_buffer_desc_t;

typedef struct nhi_tx_buffer_desc {
	uint32_t addr_lo;
	uint32_t addr_hi;
	uint32_t length:12;
	uint32_t eof_pdf:4;
	uint32_t sof_pdf:4;
	uint32_t isochronous_dma_enable:1;
	uint32_t descriptor_done:1;
	uint32_t request_status:1;
	uint32_t interrupt_enable:1;
	uint32_t offset:8;
	uint32_t fetch_time;
} nhi_tx_buffer_desc_t;

typedef struct nhi_ring_desc {
	uint32_t addr_lo;
	uint32_t addr_hi;
	// The NHI documentation says that controller registers
	// must be read/written as 32-bit values
	uint32_t consumer_idx:16;
	uint32_t producer_idx:16;
	uint32_t ring_size:16;
	uint32_t reserved:16;
} nhi_ring_desc_t;

#define NHI_REG_TX_RING_ADDR_LO(ring)		(0x00000 + (ring) * 0x10)
#define NHI_REG_TX_RING_ADDR_HI(ring)		(0x00004 + (ring) * 0x10)
#define NHI_REG_TX_RING_INDEXES(ring)		(0x00008 + (ring) * 0x10)
#define NHI_REG_TX_RING_SIZE(ring)		(0x0000C + (ring) * 0x10)
#define NHI_REG_RX_RING_ADDR_LO(ring)		(0x08000 + (ring) * 0x10)
#define NHI_REG_RX_RING_ADDR_HI(ring)		(0x08004 + (ring) * 0x10)
#define NHI_REG_RX_RING_INDEXES(ring)		(0x08008 + (ring) * 0x10)
#define NHI_REG_RX_RING_SIZE_BUFFER_SIZE(ring)	(0x0800C + (ring) * 0x10)
#define NHI_REG_TX_TABLE_FLAGS(ring)		(0x19800 + (ring) * 0x20)
#define NHI_REG_RX_TABLE_FLAGS(ring)		(0x29800 + (ring) * 0x20)
#define NHI_REG_RX_TABLE_PDF_BITMASKS(ring)	(0x29804 + (ring) * 0x20)

#define NHI_RX_RING_DESC_RING_SIZE_BUFFER_SIZE(ring_size, buffer_size) \
	((ring_size & (0xffff)) | (((buffer_size) & (0xfff)) << 16))

#define NHI_TX_TABLE_VALID			(1 << 31)
#define NHI_TX_TABLE_RAW_MODE			(1 << 30)

#define NHI_RX_TABLE_VALID			(1 << 31)
#define NHI_RX_TABLE_RAW_MODE			(1 << 30)

#endif // NHI_PROTOCOL_H
