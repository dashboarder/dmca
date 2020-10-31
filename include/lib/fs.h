/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __LIB_FS_H
#define __LIB_FS_H

#include <list.h>
#include <sys/types.h>

__BEGIN_DECLS

/* limits */
#define FS_MAXNAMLEN	255	/* maximum component name */
#define FS_MAXPATHLEN	512	/* maximum path length */


/* statfs structure. */
struct statfs {
	uint32_t f_bsize;	/* Size of a cluster in bytes. */
	uint32_t f_blocks;	/* Number of clusters. */
	uint32_t f_bfree;	/* Number of free clusters. */
};

enum file_type {
	FILE_TYPE_UNKNOWN = 0,
	FILE_TYPE_FILE,
	FILE_TYPE_DIR,
};

/* directory entry. */
struct dirent {
	uint32_t d_fileno;
	uint32_t d_reclen;
	uint8_t d_type;
	uint8_t d_namlen;
	char d_name[FS_MAXNAMLEN + 1];

	int32_t d_ctime;
	int32_t d_atime;
	int32_t d_mtime;
};

/* stat structure. */
struct stat {
	uint32_t st_ino;	/* Inode number. */
	uint32_t st_size;	/* Lenght of file (bytes). */
	uint32_t st_blocks;	/* Length of file (blocks). */
	enum file_type st_type;
	uint32_t st_flags;
	int32_t st_ctime;	/* Creation time. */
	int32_t st_mtime;	/* Modification time. */
	int32_t st_atime;	/* Access time. */
};

/* File I/O */
#define FILE_SEEK_SET	0
#define FILE_SEEK_CUR	1
#define FILE_SEEK_END	2

/* read only fs ops */
struct fs_ops {
	int (*mount)(void **ctxt, const char *dev);
	void (*unmount)(void *ctxt);
	int (*fsstat)(void *ctxt, struct statfs *stat);

	void *(*open)(void *ctxt, const char *path, uint8_t flags);
	int (*read)(void *filectxt, void *buf, size_t len);
	int (*fstat)(void *filectxt, struct stat *st);
	off_t (*seek)(void *filectxt, off_t offset, int whence);
	void (*close)(void *filectxt);

	void *(*opendir)(void *ctxt, const char *path);
	int (*readdir)(void *dirctxt, struct dirent *ent);
	void (*rewinddir)(void *dirctxt);
	void (*closedir)(void *dirctxt);
};

struct fs_struct {
	const char *name;
	const struct fs_ops *ops;	
};

/* user fs api */
int fs_mount(const char *dev, const char *fs_name, const char *mountpoint);
void fs_unmount(const char *dev);
int fs_fsstat(void *fscookie, struct statfs *stat);

int fs_open(const char *path, uint8_t flags);
int fs_read(int handle, void *buf, size_t len);
int fs_stat(int handle, struct stat *st);
off_t fs_seek(int handle, off_t offset, int whence);
int fs_close(int handle);

int fs_opendir(const char *path);
int fs_readdir(int handle, struct dirent *ent);
int fs_rewinddir(int handle);
int fs_closedir(int handle);
const char *fs_get_fsname(const int index);


/* some debug routines */
void fs_stat_dump(const struct stat *st);
void fs_statfs_dump(const struct statfs *st);
int fs_dump_dir(const char *dirpath, bool recurse);
void fs_dump_mounts(void);

/* convenience routines */
int fs_load_file(const char *path, addr_t addr, size_t *maxsize);

__END_DECLS

#endif

