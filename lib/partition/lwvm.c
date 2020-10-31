/*
 * Copyright (C) 2010-2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <compiler.h>

#include <lib/partition.h>
#include <lib/blockdev.h>
#include <lib/cksum.h>

/*
 * Cutdown types from LwVMFormat.h
 */

#pragma pack(push, 1)

#define kLwVMMaxChunks                           1024
#define kLwVMMaxPartitions                       8
#define kLwVMPartitionAttributeEffaceable        (1ULL<<48)

#define LwVM_CME_TO_PARTITION(_CME)      (((_CME) >> 12) & 0xf)
#define LwVM_CME_TO_INDEX(_CME)          ((_CME) & 0x3ff)

typedef unsigned char   uuid_t[16];
typedef uint16_t        LwVMChunkMapEntry;
typedef uint16_t        LwVMChunkIndex;

static const uuid_t LwVMMediaUUIDv1 = {
        0xB1, 0x89, 0xA5, 0x19, 0x4F, 0x59, 0x4B, 0x1D,
        0xAD, 0x44, 0x1E, 0x12, 0x7A, 0xAF, 0x45, 0x39
};

static const uuid_t LwVMMediaUUIDv2 = {
	0x6A, 0x90, 0x88, 0xCF, 0x8A, 0xFD, 0x63, 0x0A,
	0xE3, 0x51, 0xE2, 0x48, 0x87, 0xE0, 0xB9, 0x8B
};

static const uint16_t UpgradePartitionName[] = {
    'U', 'p', 'd', 'a', 't', 'e', '\0',
};

typedef struct {
        uuid_t          miLwVMUUID;
        uuid_t          miThisUUID;
        uint64_t        miBaseMediaSize;
        uint32_t        miPartitionCount;
	uint32_t	miCRC;		// not present in v1
} LwVMMediaInfo;

typedef struct {
        uuid_t          peType;
        uuid_t          peUUID;
        uint64_t        peStart;
        uint64_t        peEnd;
        uint64_t        peAttributes;
        uint16_t        peName[36];
} LwVMPartitionEntry;

typedef struct
{
        LwVMMediaInfo            mhInfo;
        uint8_t                 _pad1[512 - sizeof(LwVMMediaInfo)];
        LwVMPartitionEntry       mhPartitionTable[kLwVMMaxPartitions];
        uint8_t                 _pad2[1536 - 
                                      (sizeof(LwVMPartitionEntry) * 
                                              kLwVMMaxPartitions)];
        LwVMChunkMapEntry        mhChunkMap[kLwVMMaxChunks];
} LwVMMediaHeader;

#pragma pack(pop)


/*
 * LwVM blockdev
 */
struct lwvm_partition {
        struct blockdev         thisdev;
        struct blockdev         *parent;
        uint32_t                chunk_size;                     /* chunk size in blocks */
        uint32_t                chunk_shift;                    /* log2(chunk_size) */
        uint32_t                chunk_count;
        uint16_t                chunk_map[kLwVMMaxChunks];      /* XXX could dynamically allocate */
};

static void     lwvm_scan_partition(struct blockdev *bdev, LwVMMediaHeader *header, int partition);
static int      lwvm_read_block(struct blockdev *dev, void *ptr, block_addr block, uint32_t count);

int
lwvm_scan(struct blockdev *dev)
{
        LwVMMediaHeader         *header;
        uint32_t                i;
        int                     ret;
	uint32_t		crc;
	int			tried;
	uint32_t		chunk_shift, chunk_size;

        ret = -1;
        header = NULL;
        header = memalign(sizeof(*header), dev->alignment);
        memset(header, 0, sizeof(*header));

        /* do required geometry calculations */
	chunk_shift = 32 - clz((uint32_t)((dev->total_len - 1) / kLwVMMaxChunks));
        chunk_size = 1 << chunk_shift;

        /* read the media header */
	for (tried = 0; tried < 2; tried++) {
		/*
		 * Read either the main header from 0, or the backup header from the last block of the 
		 * first chunk.
		 */
		if (blockdev_read(dev, 
				  header, 
				  (tried ? (((off_t)chunk_size << dev->block_shift) - dev->block_size) : 0),
				  sizeof(*header))
		    != sizeof(*header))
			continue;

		/* check to see whether it's LwVM */
		if (memcmp(header->mhInfo.miLwVMUUID, LwVMMediaUUIDv2, 16) == 0) {
			crc = header->mhInfo.miCRC;
			header->mhInfo.miCRC = 0;
			if (crc != crc32((void *)header, sizeof(*header))) {
				/* v2 but corrupt */
				continue;
			}

			if (header->mhInfo.miPartitionCount > kLwVMMaxPartitions)
				continue;

			goto valid;
		}
		if (memcmp(header->mhInfo.miLwVMUUID, LwVMMediaUUIDv1, 16) == 0) {
			if (header->mhInfo.miPartitionCount > kLwVMMaxPartitions)
				continue;

			goto valid;
		}
	}
	goto out;

valid:
        /* create partitions */
        for (i = 0; i < header->mhInfo.miPartitionCount; i++)
                lwvm_scan_partition(dev, header, i);

        ret = header->mhInfo.miPartitionCount;
out:
        if (NULL != header)
                free(header);
        return(ret);
}

static void
lwvm_scan_partition(struct blockdev *dev, LwVMMediaHeader *header, int partition)
{
        struct lwvm_partition   *part;
        LwVMChunkMapEntry       cme;
        LwVMChunkIndex          ci;
        int                     i;
        char                    dev_name[32];
        block_addr		total_blocks;
		uint64_t partition_blockcount;
        
		if (header->mhPartitionTable[partition].peAttributes & kLwVMPartitionAttributeEffaceable) {
#if DEBUG_BUILD
			printf("enumerating effaceable LwVM partition\n");
#else
        /* ignore effaceable partitions, as we can't get their keys */
        	return;
#endif
		}
        part = calloc(sizeof(*part), 1);

        part->parent = dev;

        /* do required geometry calculations */
	total_blocks = dev->total_len >> dev->block_shift;
        part->chunk_shift = 32 - clz((uint32_t)((total_blocks - 1) / kLwVMMaxChunks));
        part->chunk_size = 1 << part->chunk_shift;

        /* scan chunk map in header for chunks belonging to this partition */
        for (i = 1; i < kLwVMMaxChunks; i++) {

                cme = header->mhChunkMap[i];

                /* if it matches this partition */
                if (LwVM_CME_TO_PARTITION(cme) == partition) {
                        /* register it in the map */
                        ci = LwVM_CME_TO_INDEX(cme);
                        part->chunk_map[ci] = i;
                        if (ci >= part->chunk_count)
                                part->chunk_count = ci + 1;
                }
        }

		partition_blockcount = header->mhPartitionTable[partition].peEnd - header->mhPartitionTable[partition].peStart; 

        /* construct the device */
        snprintf(dev_name, sizeof(dev_name), "%s%c", dev->name, 'a' + partition);
        construct_blockdev(&part->thisdev, dev_name, partition_blockcount, dev->block_size);
        part->thisdev.read_block_hook = &lwvm_read_block;
        blockdev_set_buffer_alignment(&part->thisdev, dev->alignment);

        /* Check if this is the upgrade partition */
        if (memcmp(UpgradePartitionName, header->mhPartitionTable[partition].peName, sizeof(UpgradePartitionName)) == 0) {
            part->thisdev.flags |= BLOCKDEV_FLAG_UPGRADE_PARTITION;
        }

        /* register the device */
        register_blockdev(&part->thisdev);
}

static int
lwvm_read_block(struct blockdev *dev, void *ptr, block_addr block, uint32_t count)
{
        struct lwvm_partition    *part;
        LwVMChunkIndex           ci;
        uint8_t                 *buf;
        off_t                   offset;
        size_t                  read_size;
        block_addr              chunk_start;
        size_t                  resid;
        int                     ret;

        part = (struct lwvm_partition *)dev;
        buf = (uint8_t *)ptr;
        resid = count;

        /* iterate reading from chunk(s) until we are done */
        while (resid > 0) {

                /*
                 * Compute the chunk index into the partition, and the
                 * block offset into the chunk.
                 */
                ci = block >> part->chunk_shift;
                offset = block & (part->chunk_size - 1);

                /* are we off the end of the partition? */
                if (ci >= part->chunk_count)
                        break;

                /* find the starting block for the indexed chunk */
                chunk_start = (block_addr)part->chunk_map[ci] << part->chunk_shift;;

                /* read no more than one chunk */
                ASSERT(offset >= 0);
                read_size = __min(resid, part->chunk_size - (size_t)offset);

                /* do the read */
                ret = blockdev_read_block(part->parent, buf, chunk_start + offset, read_size);
                if (ret <= 0)
                        return(ret);

                /* update for data read */
                resid -= ret;
                block += ret;
                buf += ret << dev->block_shift;
        }
        return(count - resid); 
}
