/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/security.h>
#include <lib/fs.h>
#include <list.h>
#include <platform.h>
 
#ifndef MAX_FDS
/** Maximum number of open files/directories */
# define MAX_FDS	16
#endif

//#if WITH_HFS
extern const struct fs_ops hfs_fs_ops;
//#endif

static const struct fs_struct fs_list[] = {
//#if WITH_HFS
	{ "hfs", &hfs_fs_ops },
//#endif
	{ NULL, NULL }
};

struct fscookie {
	const struct fs_struct *fs;
	void *ctxt;
};

#define FILEMAGIC 'film'
#define DIRMAGIC  'dirm'

struct filecookie {
	struct list_node node;
	struct fscookie *fscookie;
	int fd;
	void *ctxt;
	bool is_dir;
};

struct fsmount {
	struct list_node node;
	struct fscookie *cookie;
	size_t mountpoint_len;
	char mountpoint[256];
};

/** list of active mountpoints */
static struct list_node mounts = LIST_INITIAL_VALUE(mounts);

/** list of open files */
static struct list_node files = LIST_INITIAL_VALUE(files);

/**
 * Given a file descriptor, obtain the corresponding filecookie.
 *
 * \param[in]	handle		The file descriptor to look up.
 *
 * \return		      	Pointer to the corresponding filecookie structure.
 * \retval	NULL		The file descriptor is not currently allocated.
 */
static struct filecookie *fhtocp(int handle)
{
	struct filecookie *file;

	list_for_every_entry(&files, file, struct filecookie, node) {
		if (file->fd == handle)
			return file;
	}
	return NULL;
}

/**
 * Find a valid, unused file descriptor.
 *
 * \return			A file descriptor not currently in use.
 * \retval	-1		No file descriptors are available.
 */
static int getfd(void)
{
	int	fd;

	for (fd = 0; fd < MAX_FDS; fd++) {
		if (NULL == fhtocp(fd))
			return fd;
	}
	return -1;	
}

struct fsmount *path_to_mount(const char *path, const char **outpath)
{
	struct fsmount *mount;

	/* take a path and see if it lies on a mountpoint */
	list_for_every_entry(&mounts, mount, struct fsmount, node) {
		if (!strncmp(path, mount->mountpoint, mount->mountpoint_len)) {
			if (path[mount->mountpoint_len] == '/') {
				/* the leading chars matched the mountpoint exactly and the next char was a / */
				*outpath = &path[mount->mountpoint_len];
//				printf("path_to_mount: path '%s, found mount %p, path '%s', outpath '%s'\n",
//						path, mount, mount->mountpoint, *outpath);
				return mount;
			}
			if (path[mount->mountpoint_len] == 0) {
				/* the leading chars matched the mountpoint and there was no more string,
				 * return a / for the file system to chew on */
				*outpath = "/";
				return mount;
			}
		}
	}
	
	return NULL;
}

int fs_mount(const char *dev, const char *fs_name, const char *mountpoint)
{
	struct fscookie *cookie;
	struct fsmount *mount;
	const struct fs_struct *fs;
	void *ctxt;
	int err;

	ASSERT(dev);
	ASSERT(fs_name);
	
	for (fs = fs_list; fs->name != NULL; fs++) {
		if (!strcmp(fs_name, fs->name))
			break;
	}
	if (fs->name == NULL) {
		dprintf(DEBUG_CRITICAL, "fs_mount: fs_name '%s' not in registered fs list\n", fs_name);
		return -1;
	}

	list_for_every_entry(&mounts, mount, struct fsmount, node) {
		if (!strcmp(mountpoint, mount->mountpoint)) {
			dprintf(DEBUG_CRITICAL, "fs_mount: mountpoint '%s' already occupied\n", mountpoint);
			return -1;
		}
	}

	err = fs->ops->mount(&ctxt, dev);
	if (err < 0)
		return err;

	cookie = malloc(sizeof(struct fscookie));

	cookie->fs = fs;
	cookie->ctxt = ctxt;

	/* create a mount structure */
	mount = malloc(sizeof(struct fsmount));	

	mount->cookie = cookie;
	strlcpy(mount->mountpoint, mountpoint, sizeof(mount->mountpoint));
	mount->mountpoint_len = strlen(mount->mountpoint); // used in path comparisons

	list_add_tail(&mounts, &mount->node);

	return 0;
}

void fs_unmount(const char *dev)
{
	struct fsmount *mount;
	const char *shortpath;

	ASSERT(dev);

	mount = path_to_mount(dev, &shortpath);
	if (!mount)
		return;

	ASSERT(mount->cookie != NULL);

	mount->cookie->fs->ops->unmount(mount->cookie->ctxt);

	/* remove the mount structure */
	list_delete(&mount->node);
	free(mount->cookie);
	free(mount);
}

int fs_fsstat(void *fscookie, struct statfs *stat)
{
	struct fscookie *cookie = (struct fscookie *)fscookie;

	ASSERT(cookie);
	ASSERT(stat);

	return cookie->fs->ops->fsstat(cookie->ctxt, stat);
}

int fs_open(const char *path, uint8_t flags)
{
	struct fsmount *mount;
	void *ctxt;
	struct filecookie *fcookie;
	const char *shortpath;
	int fd;

	ASSERT(path);

	mount = path_to_mount(path, &shortpath);
	if (!mount)
		return -1;

	if (-1 == (fd = getfd()))
		return -1;

	ctxt = mount->cookie->fs->ops->open(mount->cookie->ctxt, shortpath, flags);
	if (!ctxt)
		return -1;

	fcookie = malloc(sizeof(struct filecookie));

	fcookie->fscookie = mount->cookie;
	fcookie->ctxt = ctxt;
	fcookie->is_dir = false;
	fcookie->fd = fd;

	list_add_head(&files, &fcookie->node);

	return fcookie->fd;
}

int fs_read(int handle, void *buf, size_t len)
{
	struct filecookie *fcookie = fhtocp(handle);

	ASSERT(fcookie);
	ASSERT(!fcookie->is_dir);

	return fcookie->fscookie->fs->ops->read(fcookie->ctxt, buf, len);
}

int fs_stat(int handle, struct stat *st)
{
	struct filecookie *fcookie = fhtocp(handle);

	ASSERT(fcookie);
	ASSERT(!fcookie->is_dir);

	return fcookie->fscookie->fs->ops->fstat(fcookie->ctxt, st);
}

off_t fs_seek(int handle, off_t offset, int whence)
{
	struct filecookie *fcookie = fhtocp(handle);

	ASSERT(fcookie);
	ASSERT(!fcookie->is_dir);

	return fcookie->fscookie->fs->ops->seek(fcookie->ctxt, offset, whence);
}

int fs_close(int handle)
{
	struct filecookie *fcookie = fhtocp(handle);

	ASSERT(fcookie);
	ASSERT(!fcookie->is_dir);
	ASSERT(fcookie->fscookie != NULL);
	ASSERT(fcookie->fscookie->fs != NULL);
	ASSERT(fcookie->fscookie->fs->ops != NULL);
	ASSERT(fcookie->fscookie->fs->ops->close != NULL);

	fcookie->fscookie->fs->ops->close(fcookie->ctxt);

	list_delete(&fcookie->node);
	free(fcookie);

	return 0;
}

int fs_opendir(const char *path)
{
	struct fsmount *mount;
	void *ctxt;
	struct filecookie *fcookie;
	const char *shortpath;
	int fd;

	mount = path_to_mount(path, &shortpath);
	if (!mount)
		return -1;

	if (-1 == (fd = getfd()))
		return -1;

	ctxt = mount->cookie->fs->ops->opendir(mount->cookie->ctxt, shortpath);
	if (!ctxt)
		return -1;

	fcookie = malloc(sizeof(struct filecookie));

	fcookie->fscookie = mount->cookie;
	fcookie->ctxt = ctxt;
	fcookie->is_dir = true;
	fcookie->fd = fd;

	list_add_head(&files, &fcookie->node);

	return fcookie->fd;
}

int fs_readdir(int handle, struct dirent *ent)
{
	struct filecookie *fcookie = fhtocp(handle);

	RELEASE_ASSERT(fcookie);
	RELEASE_ASSERT(fcookie->is_dir);

	return fcookie->fscookie->fs->ops->readdir(fcookie->ctxt, ent);
}

int fs_rewinddir(int handle)
{
	struct filecookie *fcookie = fhtocp(handle);

	RELEASE_ASSERT(fcookie);
	RELEASE_ASSERT(fcookie->is_dir);

	fcookie->fscookie->fs->ops->rewinddir(fcookie->ctxt);

	return 0;
}

int fs_closedir(int handle)
{
	struct filecookie *fcookie = fhtocp(handle);

	RELEASE_ASSERT(fcookie);
	RELEASE_ASSERT(fcookie->is_dir);

	fcookie->fscookie->fs->ops->closedir(fcookie->ctxt);

	list_delete(&fcookie->node);
	free(fcookie);

	return 0;
}

int fs_load_file(const char *path, addr_t addr, size_t *maxsize)
{
	int filefd;
	int result;
	struct stat stat;

	RELEASE_ASSERT(maxsize != NULL);

	/* open the file */
	filefd = fs_open(path, 0);
	if (filefd < 0) {
		platform_record_breadcrumb("fs_load_file", "fs_open_failed");
		return -1;
	}

	/* stat it */
	fs_stat(filefd, &stat);

	/* make sure it is not too large */
	if (*maxsize < stat.st_size) {
		platform_record_breadcrumb("fs_load_file", "maxsize_exceeded");
		return -1;
	}

	if (!security_allow_memory((void *)addr, stat.st_size)) {
		platform_record_breadcrumb("fs_load_file", "permission_denied");
		printf("Permission Denied\n");
		return -1;
	}

	/* read the entire thing in */
	result = fs_read(filefd, (void *)addr, stat.st_size);
	fs_close(filefd);

	if (result < 0 || (unsigned)result != stat.st_size) {
		platform_record_breadcrumb("fs_load_file", "read_failed");
		// Zero the buffer in case of a partial read
		memset((void *)addr, *maxsize, 0);

		return -1;
	}
	
	*maxsize = stat.st_size;

	return 0;
}

void fs_dump_mounts(void)
{
	struct fsmount *mount;

	printf("active fs mounts:\n");
	list_for_every_entry(&mounts, mount, struct fsmount, node) {
		printf("\t'%s'\n", mount->mountpoint);
	}
}

const char *fs_get_fsname(const int index)
{
	int	i;

	for (i = 0; i < index; i++)
		if (fs_list[i].name == NULL)
			return(NULL);
	return(fs_list[i].name);
}
