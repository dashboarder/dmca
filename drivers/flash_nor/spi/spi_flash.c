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
#include <drivers/flash_nor.h>
#include <drivers/spi.h>
#include <lib/blockdev.h>
#include <platform.h>
#include <platform/gpio.h>
#if WITH_TARGET_CONFIG
# include <target/gpiodef.h>
#endif
#include <platform/gpiodef.h>
#include <sys.h>

#define SPI_FLASH_CMD_WRSR		(0x01)
#define SPI_FLASH_CMD_BYTE_PROG		(0x02)
#define SPI_FLASH_CMD_READ		(0x03)
#define SPI_FLASH_CMD_WRDI		(0x04)
#define SPI_FLASH_CMD_RDSR		(0x05)
#define SPI_FLASH_CMD_WREN		(0x06)
#define SPI_FLASH_CMD_ERASE_SECTOR	(0x20)
#define SPI_FLASH_CMD_EWSR		(0x50)
#define SPI_FLASH_CMD_EBSY		(0x70)
#define SPI_FLASH_CMD_DBSY		(0x80)
#define SPI_FLASH_CMD_JEDEC_ID		(0x9f)
#define SPI_FLASH_CMD_SST_AAI_WORD_PROG	(0xad)

#define SPI_FLASH_STATUS_BUSY		(0x01)
#define SPI_FLASH_STATUS_BP		(0x3c)
#define SPI_FLASH_STATUS_ATMEL_SPW	(0x0c)
#define SPI_FLASH_STATUS_ST_SPW		(0x1c)
#define SPI_FLASH_STATUS_BPL		(0x80)

#define SPI_FLASH_JEDEC_ID_SST_25VF080	(0xbf258e)	// 1M
#define SPI_FLASH_JEDEC_ID_SST_25WF080	(0xbf2505)	// 1M, replaces 25VF080
#define SPI_FLASH_JEDEC_ID_SST_2M	(0xbf2541)
#define SPI_FLASH_JEDEC_ID_ATMEL	(0x1f4502)
#define SPI_FLASH_JEDEC_ID_ST_M25PX32	(0x207116)
#define SPI_FLASH_JEDEC_ID_ST_M25PE80	(0x208014)
#define SPI_FLASH_JEDEC_ID_ST_M25P32	(0x202016)
#define SPI_FLASH_JEDEC_ID_ST_M25P64	(0x202017)
#define SPI_FLASH_JEDEC_ID_ST_M25P128	(0x202018)
#define SPI_FLASH_JEDEC_ID_ST_M25PX64	(0x207117)
#define SPI_FLASH_JEDEC_ID_ST_M25PX128	(0x207118)
#define SPI_FLASH_JEDEC_ID_S25FL128P	(0x012018)
#define SPI_FLASH_JEDEC_ID_MX25L25735	(0xc22019)
#define SPI_FLASH_JEDEC_ID_EEPROM	(0xFFFFFF)

#define SPI_FLASH_FLAG_PROG_SST		(1 << 0)

struct spi_ndev {
	struct nor_blockdev ndev;

	u_int32_t spiBus;
	u_int32_t spiChipSelect;
	u_int32_t spiFrequency;
	u_int32_t spiMode;

	u_int32_t jedecID;
	u_int32_t flags;

	u_int32_t blockSize;
	u_int32_t blockCount;

	u_int32_t softWriteProtectMask;
	u_int32_t progMaxLength;
	u_int32_t progAlignMask;

	u_int32_t defaultTimeout;
	u_int32_t byteProgramTimeout;
	u_int32_t eraseSectorTimeout;
};

static int flash_spi_probe(struct spi_ndev *sdev);
static int flash_spi_read_range(uintptr_t handle, u_int8_t *ptr, u_int32_t offset, u_int32_t length);
static int flash_spi_write_range(uintptr_t handle, const u_int8_t *ptr, u_int32_t offset, u_int32_t length);
static int flash_spi_program_chunk(struct spi_ndev *sdev, const u_int8_t *buf, u_int32_t chunk_off, u_int32_t chunk_len);
static int flash_spi_program_chunk_sst(struct spi_ndev *sdev, const u_int8_t *buf, u_int32_t chunk_off, u_int32_t chunk_len);
static int flash_spi_erase_range(uintptr_t handle, u_int32_t offset, u_int32_t length);
static void flash_spi_setup_bus(struct spi_ndev *sdev);
static int flash_spi_wait_busy(struct spi_ndev *sdev, u_int32_t timeout);
static u_int8_t flash_spi_read_rdsr(struct spi_ndev *sdev);
static void flash_spi_write_enable(struct spi_ndev *sdev, bool enable);
static void flash_spi_write_wrsr(struct spi_ndev *sdev, u_int8_t wrsr);
static u_int32_t flash_spi_read_jedec_id(struct spi_ndev *sdev);
static void flash_spi_select(struct spi_ndev *sdev, bool enable);


int flash_spi_init(int which_bus)
{
	struct spi_ndev *sdev;
	int result;

	sdev = calloc(1, sizeof(struct spi_ndev));

	switch (which_bus) {
#ifdef SPI_NOR0
	case SPI_NOR0:
		sdev->spiBus = SPI_NOR0;
# ifdef GPIO_SPI0_CS
		sdev->spiChipSelect = GPIO_SPI0_CS;
# endif
		break;
#endif
#ifdef SPI_NOR1
	case SPI_NOR1:
		sdev->spiBus = SPI_NOR1;
# ifdef GPIO_SPI1_CS
		sdev->spiChipSelect = GPIO_SPI1_CS;
# endif
		break;
#endif
#ifdef SPI_NOR3
	case SPI_NOR3:
		sdev->spiBus = SPI_NOR3;
# ifdef GPIO_SPI3_CS
		sdev->spiChipSelect = GPIO_SPI3_CS;
# endif
		break;
#endif
	default:
		free(sdev);
		return -1;
	}

	// Get the SPI frequency based upon the platform & boot configuration.
	sdev->spiFrequency = platform_get_spi_frequency();

	sdev->spiMode = (8 << 16) | (0 << 8) | (0 << 0);	// Default to 8 bit and mode 0

	sdev->defaultTimeout = 50 * 1000;			// Default timeout is 50ms

	result = flash_spi_probe(sdev);
	if (result < 0) {
		free(sdev);
		return result;
	}

	sdev->ndev.handle = (uintptr_t)sdev;
	sdev->ndev.readRange = flash_spi_read_range;
#if !READ_ONLY
	sdev->ndev.writeRange = flash_spi_write_range;
	sdev->ndev.eraseRange = flash_spi_erase_range;
#endif /* ! READ_ONLY */

	return flash_nor_register(&sdev->ndev, "nor0", sdev->blockCount * sdev->blockSize, sdev->blockSize);
}

static int flash_spi_read_range(uintptr_t handle, u_int8_t *ptr, u_int32_t offset, u_int32_t length)
{
	int result;
	u_int8_t addr[4];
	u_int32_t cnt;
	struct spi_ndev *sdev = (struct spi_ndev *)handle;

	flash_spi_setup_bus(sdev);

	result = flash_spi_wait_busy(sdev, sdev->defaultTimeout);
	if (result < 0) return -1;

	if (0 /*sdev->jedecID == SPI_FLASH_JEDEC_ID_EEPROM*/) {
		for (cnt = 0; cnt < length; cnt++) {
			addr[0] = SPI_FLASH_CMD_READ;
			addr[1] = ((offset + cnt) >> 16) & 0xff;
			addr[2] = ((offset + cnt) >>  8) & 0xff;
			addr[3] = ((offset + cnt) >>  0) & 0xff;

			flash_spi_select(sdev, true);
			spi_write_etc(sdev->spiBus, addr, 4, 1, 0); // wait, no rx
			spi_read_etc(sdev->spiBus, ptr + cnt, 1, 1, 0); // wait, no tx
			flash_spi_select(sdev, false);
		}
	} else {
		addr[0] = SPI_FLASH_CMD_READ;
		addr[1] = (offset >> 16) & 0xff;
		addr[2] = (offset >>  8) & 0xff;
		addr[3] = (offset >>  0) & 0xff;

	        flash_spi_select(sdev, true);
	        spi_write_etc(sdev->spiBus, addr, 4, 1, 0); // wait, no rx
		spi_read_etc(sdev->spiBus, ptr, length, 1, 0); // wait, no tx
	        flash_spi_select(sdev, false);
	}

	return 0;
}

#if !READ_ONLY

static int flash_spi_write_range(uintptr_t handle, const u_int8_t *ptr, u_int32_t offset, u_int32_t length)
{
	int result;
	const u_int8_t *buf;
	u_int8_t rdsr, *tmpBuf = 0;
	u_int32_t cur_len, cur_off, chunk_len, chunk_off, block_mask, tmp, left = length;
	struct spi_ndev *sdev = (struct spi_ndev *)handle;

	flash_spi_setup_bus(sdev);

	result = flash_spi_wait_busy(sdev, sdev->defaultTimeout);
	if (result < 0) {
		return result;
	}

	rdsr = flash_spi_read_rdsr(sdev);
	if (((rdsr & sdev->softWriteProtectMask) != 0) && ((rdsr & SPI_FLASH_STATUS_BPL) != 0)) {
		dprintf(DEBUG_CRITICAL, "flash_spi_write_range: device is read-only\n");
		return -1;
	}

	if ((rdsr & sdev->softWriteProtectMask) == sdev->softWriteProtectMask) rdsr |= SPI_FLASH_STATUS_BP;

	flash_spi_write_wrsr(sdev, 0x00); // Set BP[3:0] = 0

	block_mask = sdev->blockSize - 1;

	cur_off = offset;
	while (left != 0) {
		cur_len = left;							// Assume maximum size
		buf = ptr + cur_off - offset;					// Assume source buffer
		chunk_off = cur_off & ~block_mask;				// Find the base offset
		chunk_len = sdev->blockSize;					// Assume single block

		if ((cur_off & block_mask) != 0) {				// Clip size to rest of block
			cur_len = sdev->blockSize - (cur_off & block_mask);	// when not at the beginning of the block
			if (cur_len > left) cur_len = left;
		} else if (cur_len >= sdev->blockSize) {
			cur_len = cur_len & ~block_mask;			// Round to multiple of block size
			chunk_len = cur_len;
		}

		if (cur_len < sdev->blockSize) {				// Handle partial blocks
			if (tmpBuf == 0) tmpBuf = malloc(sdev->blockSize);

			tmp = sdev->blockSize;					// Pre-read the whole block
			result = flash_spi_read_range(handle, tmpBuf, chunk_off, tmp);
			if (result < 0) break;
										// Copy in the changed bytes
			memcpy(tmpBuf + (cur_off & block_mask), buf, cur_len);

			buf = tmpBuf;						// Select the side buffer
		}

		tmp = chunk_len;						// Erase the whole chunk
		result = flash_spi_erase_range(handle, chunk_off, tmp);
		if (result < 0) break;

		if (sdev->flags & SPI_FLASH_FLAG_PROG_SST) {
			result = flash_spi_program_chunk_sst(sdev, buf, chunk_off, chunk_len);
		} else {
			result = flash_spi_program_chunk(sdev, buf, chunk_off, chunk_len);
		}

		if (result < 0) break;

		cur_off += cur_len;
		left -= cur_len;
	}

	flash_spi_write_wrsr(sdev, rdsr); // Restore BP[3:0]

	if (tmpBuf != 0) free(tmpBuf);

	if (result < 0) {
		dprintf(DEBUG_CRITICAL, "flash_spi_write_range: write failed: %d\n", result);
		return result;
	}

	return 0;
}

static int flash_spi_program_chunk(struct spi_ndev *sdev, const u_int8_t *buf, u_int32_t chunk_off, u_int32_t chunk_len)
{
	int		result = 0;
	u_int8_t	addr[4];
	u_int32_t	prog_len, prog_off = 0;

	while (chunk_len != 0) {
		prog_len = (prog_off | sdev->progAlignMask) - prog_off + 1;
		if (prog_len > sdev->progMaxLength) prog_len = sdev->progMaxLength;
		if (prog_len > chunk_len) prog_len = chunk_len;

		flash_spi_write_enable(sdev, true);

		addr[0] = SPI_FLASH_CMD_BYTE_PROG;
		addr[1] = ((chunk_off + prog_off) >> 16) & 0xff;
		addr[2] = ((chunk_off + prog_off) >>  8) & 0xff;
		addr[3] = ((chunk_off + prog_off) >>  0) & 0xff;

		flash_spi_select(sdev, true);		// Write the command and bytes
		spi_write_etc(sdev->spiBus, addr, 4, 1, 0);
		spi_write_etc(sdev->spiBus, buf + prog_off, prog_len, 1, 0);
		flash_spi_select(sdev, false);
		spin(1);					// CS must be de-asserted for at least 100ns

								// Wait for the bytes to be written
		result = flash_spi_wait_busy(sdev, sdev->byteProgramTimeout);

		if (result < 0) break;

		prog_off += prog_len;
		chunk_len -= prog_len;
	}

	flash_spi_write_enable(sdev, false);

	return result;
}

static int flash_spi_program_chunk_sst(struct spi_ndev *sdev, const u_int8_t *buf, u_int32_t chunk_off, u_int32_t chunk_len)
{
	int		result = 0;
	u_int8_t	addr[6];
	u_int32_t	cnt, tmp;

	flash_spi_write_enable(sdev, true);

	for (cnt = 0; cnt < chunk_len; cnt += 2) {
		if (cnt == 0) {
			addr[0] = SPI_FLASH_CMD_SST_AAI_WORD_PROG;
			addr[1] = (chunk_off >> 16) & 0xff;
			addr[2] = (chunk_off >>  8) & 0xff;
			addr[3] = (chunk_off >>  0) & 0xff;
			addr[4] = buf[cnt + 0];
			addr[5] = buf[cnt + 1];
			tmp = 6;
		} else {
			addr[0] = SPI_FLASH_CMD_SST_AAI_WORD_PROG;
			addr[1] = buf[cnt + 0];
			addr[2] = buf[cnt + 1];
			tmp = 3;
		}


		flash_spi_select(sdev, true);		// Write the command and bytes
		spi_write_etc(sdev->spiBus, addr, tmp, 1, 0);	// wait, no rx
		flash_spi_select(sdev, false);
		spin(1);					// CS must be de-asserted for at least 100ns

								// Wait for the bytes to be written
		result = flash_spi_wait_busy(sdev, sdev->byteProgramTimeout);

		if (result < 0) break;
	}

	flash_spi_write_enable(sdev, false);

	return result;
}

static int flash_spi_erase_range(uintptr_t handle, u_int32_t offset, u_int32_t length)
{
	int result;
	u_int8_t rdsr, addr[4];
	u_int32_t block_mask, left = length;
	struct spi_ndev *sdev = (struct spi_ndev *)handle;

//	if (sdev->jedecID == SPI_FLASH_JEDEC_ID_EEPROM) return 0;

	block_mask = sdev->blockSize - 1;

	if (((offset & block_mask) != 0) || ((left & block_mask) != 0)) {
		dprintf(DEBUG_CRITICAL, "flash_spi_erase_range: offset or length not a multiple of erase block size\n");
		return -1;
	}

	flash_spi_setup_bus(sdev);

        result = flash_spi_wait_busy(sdev, sdev->defaultTimeout);
        if (result < 0) {
		return result;
	}

	rdsr = flash_spi_read_rdsr(sdev);
	if (((rdsr & 0x3C) != 0) && ((rdsr & 0x80) != 0)) {
		dprintf(DEBUG_CRITICAL, "flash_spi_erase_range: device is read-only\n");
		return -1;
	}

	if ((rdsr & sdev->softWriteProtectMask) == sdev->softWriteProtectMask) rdsr |= SPI_FLASH_STATUS_BP;

	flash_spi_write_wrsr(sdev, 0x00); // Set BP[3:0] = 0

	while (left != 0) {
		flash_spi_write_enable(sdev, true);

		addr[0] = SPI_FLASH_CMD_ERASE_SECTOR;
		addr[1] = (offset >> 16) & 0xff;
		addr[2] = (offset >>  8) & 0xff;
		addr[3] = (offset >>  0) & 0xff;

		flash_spi_select(sdev, true);
		spi_write_etc(sdev->spiBus, addr, 4, 1, 0); // wait, no rx
		flash_spi_select(sdev, false);

		result = flash_spi_wait_busy(sdev, sdev->eraseSectorTimeout);
		if (result < 0) break;

		offset += sdev->blockSize;
		left -= sdev->blockSize;
	}

	flash_spi_write_wrsr(sdev, rdsr); // Restore BP[3:0]

	if (result < 0) {
		dprintf(DEBUG_CRITICAL, "flash_spi_erase_range: erase failed\n");
		return result;
	}

	return 0;
}

#endif /* ! READ_ONLY */

static int flash_spi_probe(struct spi_ndev *sdev)
{
	int result;

	flash_spi_setup_bus(sdev);

	result = flash_spi_wait_busy(sdev, sdev->defaultTimeout);
	if (result < 0) return result;

	sdev->flags = 0;

	/* default to 4K page, 1MiB device */
	sdev->blockSize = 0x1000;
	sdev->blockCount = 0x100;

#if !READ_ONLY
	sdev->jedecID = flash_spi_read_jedec_id(sdev);

	dprintf(DEBUG_INFO, "flash_spi_probe: jedecID: 0x%06lx\n", sdev->jedecID);

	switch (sdev->jedecID) {
		case SPI_FLASH_JEDEC_ID_SST_25VF080 :
		case SPI_FLASH_JEDEC_ID_SST_2M :
			sdev->flags |= SPI_FLASH_FLAG_PROG_SST;
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_BP;
			sdev->progMaxLength = 1;		// One byte buffer
			sdev->progAlignMask = 1;		// No aligment restriction
			sdev->byteProgramTimeout = 10;		// Byte program timeout is 10us
			sdev->eraseSectorTimeout = 25 * 1000;	// Sectory Erase timeout is 25ms
			break;

		case SPI_FLASH_JEDEC_ID_SST_25WF080 :
			sdev->flags |= SPI_FLASH_FLAG_PROG_SST;
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_BP;
			sdev->progMaxLength = 1;		// One byte buffer
			sdev->progAlignMask = 1;		// No aligment restriction
			sdev->byteProgramTimeout = 25;		// Byte program timeout is 25us
			sdev->eraseSectorTimeout = 30 * 1000;	// Sectory Erase timeout is 30ms
			break;

		case SPI_FLASH_JEDEC_ID_ATMEL :
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_ATMEL_SPW;
			sdev->progMaxLength = 256;		// 256 byte buffer
			sdev->progAlignMask = 256 - 1;		// 256 byte alignment requirement
			sdev->byteProgramTimeout = 5 * 1000;    // Byte program timeout is 5ms
			sdev->eraseSectorTimeout = 200 * 1000;	// Sectory Erase timeout is 200ms
			break;

		case SPI_FLASH_JEDEC_ID_ST_M25P128 :
		case SPI_FLASH_JEDEC_ID_ST_M25P64 :
		case SPI_FLASH_JEDEC_ID_ST_M25P32 :
		case SPI_FLASH_JEDEC_ID_ST_M25PX128 :
		case SPI_FLASH_JEDEC_ID_ST_M25PX64 :
		case SPI_FLASH_JEDEC_ID_ST_M25PX32 :
        	case SPI_FLASH_JEDEC_ID_ST_M25PE80 :  
			switch (sdev->jedecID) {
			case SPI_FLASH_JEDEC_ID_ST_M25P128 :
			case SPI_FLASH_JEDEC_ID_ST_M25PX128 :
				sdev->blockCount = 0x1000;		// 16MiB
				break;
			case SPI_FLASH_JEDEC_ID_ST_M25P32 :		// 64 x 64Kib
				sdev->blockSize = 0x10000;
				sdev->blockCount = 0x40;
				break;
			case SPI_FLASH_JEDEC_ID_ST_M25P64 :		// 128 x 64Kib
				sdev->blockSize = 0x10000;
				sdev->blockCount = 0x80;
				break;
			case SPI_FLASH_JEDEC_ID_ST_M25PX64 :
				sdev->blockCount = 0x800;		// 8MiB
				break;
			}
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_ST_SPW;
			sdev->progMaxLength = 256;		// 256 byte buffer
			sdev->progAlignMask = 256 - 1;		// 256 byte alignment requirement
			sdev->byteProgramTimeout = 5 * 1000;    // Byte program timeout is 5ms
			sdev->eraseSectorTimeout = 150 * 1000;	// Sectory Erase timeout is 150ms
			break;

		case SPI_FLASH_JEDEC_ID_S25FL128P :
			sdev->blockSize = 0x1000;		// 64KiB
			sdev->blockCount = 0x1000;		// 16MiB
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_ST_SPW;
			sdev->progMaxLength = 256;		// 256 byte buffer
			sdev->progAlignMask = 256 - 1;		// 256 byte alignment requirement
			sdev->byteProgramTimeout = 5 * 1000;    // Byte program timeout is 5ms
			sdev->eraseSectorTimeout = 150 * 1000;	// Sectory Erase timeout is 150ms
			break;

		case SPI_FLASH_JEDEC_ID_MX25L25735:
			sdev->blockSize = 0x1000;		// 4KiB
			sdev->blockCount = 0x2000;		// 32 MiB
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_BP;
			sdev->progMaxLength = 256;		// 256 byte buffer
			sdev->progAlignMask = 256 - 1;		// 256 byte alignment requirement
			sdev->byteProgramTimeout = 5 * 1000;    // Byte program timeout is 5ms
			sdev->eraseSectorTimeout = 150 * 1000;	// Sectory Erase timeout is 150ms
			break;

#if 0
		case SPI_FLASH_JEDEC_ID_EEPROM :
			sdev->softWriteProtectMask = SPI_FLASH_STATUS_ATMEL_SPW;
			sdev->progMaxLength = 128;		// 128 byte buffer
			sdev->progAlignMask = 128 - 1;		// 128 byte alignment requirement
			sdev->byteProgramTimeout = 5 * 1000;    // Byte program timeout is 5ms
			sdev->eraseSectorTimeout = 1 * 1000;	// Sectory Erase timeout is 1ms
			break;
#endif

		default :
			return -1;
	}

#endif /* ! READ_ONLY */

	return 0;
}

static void flash_spi_setup_bus(struct spi_ndev *sdev)
{
	spi_setup(sdev->spiBus, sdev->spiFrequency, (sdev->spiMode >> 16) & 0xff, 1, (sdev->spiMode >> 8) & 0xff, (sdev->spiMode >> 0) & 0xff);
}

static int flash_spi_wait_busy(struct spi_ndev *sdev, u_int32_t timeout)
{
	u_int8_t data;
	bool timeOut;
	utime_t startTime = system_time();
	int result = -2;

	do {
		timeOut = time_has_elapsed(startTime, timeout);
		data = flash_spi_read_rdsr(sdev);

		if ((data & SPI_FLASH_STATUS_BUSY) == 0) result = 0;
	} while ((result < 0) && !timeOut);

	if (result < 0) dprintf(DEBUG_INFO, "flash_spi_wait_busy: result: %d\n", result);

	return result;
}

static u_int8_t flash_spi_read_rdsr(struct spi_ndev *sdev)
{
	u_int8_t data = SPI_FLASH_CMD_RDSR;

	flash_spi_select(sdev, true);
	spi_write_etc(sdev->spiBus, &data, 1, 1, 0); // wait, no rx
	spi_read_etc(sdev->spiBus, &data, 1, 1, 0); // wait, no tx
	flash_spi_select(sdev, false);

	return data;
}

static void flash_spi_select(struct spi_ndev *sdev, bool enable)
{
#if defined(GPIO_SPI0_CS) || defined(GPIO_SPI1_CS)
	/* this is still bogus */
	gpio_write(sdev->spiChipSelect, enable ? 0 : 1);
#else
	spi_select(sdev->spiBus, enable);
#endif
}

#if !READ_ONLY

static void flash_spi_write_enable(struct spi_ndev *sdev, bool enable)
{
	u_int8_t data;

	data = enable ? SPI_FLASH_CMD_WREN : SPI_FLASH_CMD_WRDI;
	flash_spi_select(sdev, true);
	spi_write_etc(sdev->spiBus, &data, 1, 1, 0); // wait, no rx
	flash_spi_select(sdev, false);
}

static void flash_spi_write_wrsr(struct spi_ndev *sdev, u_int8_t wrsr)
{
	u_int8_t data[2];
	u_int8_t done;

	flash_spi_write_enable(sdev, true);

	data[0] = SPI_FLASH_CMD_WRSR;
	data[1] = wrsr;
	flash_spi_select(sdev, true);
	spi_write_etc(sdev->spiBus, data, 2, 1, 0); // wait, no rx
	flash_spi_select(sdev, false);

	done = 1;
	while (done) {
		done = flash_spi_read_rdsr(sdev) & 1;
		spin(1);
	}
}

static u_int32_t flash_spi_read_jedec_id(struct spi_ndev *sdev)
{
	u_int8_t  addr = SPI_FLASH_CMD_JEDEC_ID;
	u_int32_t data = 0;

	flash_spi_select(sdev, true);
	spi_write_etc(sdev->spiBus, &addr, 1, 1, 0); // wait, no rx
	spi_read_etc(sdev->spiBus, &data, 3, 1, 0); // wait, no tx
	flash_spi_select(sdev, false);

	return (data & 0x0000FF00) | ((data >> 16) & 0x000000FF) | ((data << 16) & 0x00FF0000);
}

#endif /* ! READ_ONLY */
