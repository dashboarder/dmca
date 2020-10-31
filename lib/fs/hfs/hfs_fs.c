/* Copyright 2007-2011 Apple Inc. All rights Reserved */
/* Copyright 2006 Apple Computer Inc, All Rights Reserved */

/*
 * Interface shim between the iBoot fs/bdev layers and the HFS code.
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <lib/libc.h>
#include <lib/fs.h>
#include <lib/blockdev.h>
#ifndef HFS_UNITTEST
#include <sys/task.h>
#endif
#include "hfs.h"
#include "hfs_format.h"

#if 0
# define HFSdebug(fmt, args...)	printf("%s: " fmt "\n", __FUNCTION__ , ##args)
#else
# define HFSdebug(fmt, args...) do { } while (0)
#endif

/*******************************************************************************
 * fs interface
 */

static int	hfs_mount(void **ctxt, const char *devname);
static void	hfs_unmount(void *ctxt);
static int	hfs_fsstat(void *ctxt, struct statfs *stat);

static void	*hfs_open(void *ctxt, const char *path, uint8_t flags);
static int	hfs_read(void *filectxt, void *buf, size_t len);
static int	hfs_fstat(void *filectxt, struct stat *st);
static off_t	hfs_seek(void *filectxt, off_t offset, int whence);
static void	hfs_close(void *filectxt);

static void	*hfs_opendir(void *ctxt, const char *path);
static int	hfs_readdir(void *dirctxt, struct dirent *ent);
static void	hfs_rewinddir(void *dirctxt);
static void	hfs_closedir(void *dirctxt);

struct fs_ops hfs_fs_ops = {
	hfs_mount,
	hfs_unmount,
	hfs_fsstat,

	hfs_open,
	hfs_read,
	hfs_fstat,
	hfs_seek,
	hfs_close,

	hfs_opendir,
	hfs_readdir,
	hfs_rewinddir,
	hfs_closedir
};

struct hfs_context {
	struct blockdev		*dev;
	struct statfs		s;
	uint32_t		blocksize;
};

struct hfs_openfile {
	struct hfs_context	*fs;
	char			*path;
	off_t			pos;
	struct stat		s;
};

struct hfs_opendir {
	struct hfs_context	*fs;
	char			*path;
	off_t			pos;
};

/*
 * Locking.  Since the HFS code is not re-entrant, we must ensure
 * that only one task calls into it at a time.
 */
#ifndef HFS_UNITTEST
static struct task *hfs_owner_task;

#define LOCK()								\
	do {								\
		while (hfs_owner_task != NULL)				\
			task_yield();					\
		hfs_owner_task = current_task;				\
	} while(0);

#define UNLOCK()							\
	do {								\
		hfs_owner_task = NULL;					\
	} while(0);
#else
#define LOCK() do {} while(0)
#define UNLOCK() do {} while(0)
#endif



static int
hfs_mount(void **ctxt, const char *devname)
{
	struct hfs_context		*cp;
	struct HFSPlusVolumeHeader	*vh;
	int				result;

	HFSdebug("mounting '%s'", devname);
	result = -1;
	
	LOCK();
	cp = malloc(sizeof(struct hfs_context));
	
	// find the device
	if ((cp->dev = lookup_blockdev(devname)) == NULL)
		goto out;

	// make sure it's something we recognise
	if (HFSInitPartition(cp))
		goto out;

//	HFSDetect();
	
	// save FS stat info while we're at it
	vh = HFSGetVolumeHeader();
	cp->s.f_bsize = vh->blockSize;
	cp->s.f_blocks = vh->totalBlocks;
	cp->s.f_bfree = vh->freeBlocks;
	
	result = 0;
out:
	if (result) {
		free(cp);
		HFSdebug("mount failed");
	} else {
		HFSdebug("mounted fs %p", cp);
		*ctxt = cp;
	}
	UNLOCK();
	return(result);
}

static void
hfs_unmount(void *ctxt)
{
	free(ctxt);
}

static int
hfs_fsstat(void *ctxt, struct statfs *stat)
{
	struct hfs_context		*cp = (struct hfs_context *)ctxt;

	*stat = cp->s;

	return(0);
}

static void *
hfs_open(void *ctxt, const char *path, uint8_t open_flags)
{
	struct hfs_context	*cp = (struct hfs_context *)ctxt;
	struct hfs_openfile	*of;
	uint32_t		flags;
	uint32_t		path_length;
	time_t			time;
	off_t			size;

	HFSdebug("%s: openfile", path);
	if (path[0] == '/')
		path++;
	of = NULL;
	LOCK();
	
	// check for file existence
	if (HFSGetFileInfo(cp, path, &flags, &time, &size))
		goto out;
	if ((flags & kFileTypeMask) != kFileTypeFlat)
		goto out;

	// alloc file and populate path
	path_length = strlen(path) + 1;
	of = malloc(sizeof(struct hfs_openfile) + path_length);
	of->path = (char *)(of + 1);
	strlcpy(of->path, path, path_length);
	of->pos = 0;
	of->fs = cp;

	// Since we have stat information courtesy of checking the file's existence,
	// save it away for the inevitable stat.
	of->s.st_ino = 0;	// XXX
	of->s.st_size = size;
	of->s.st_blocks = size / cp->s.f_bsize + ((size % cp->s.f_bsize) ? 1 : 0);
	of->s.st_type = FILE_TYPE_FILE;
	of->s.st_flags = 0;	// XXX
	of->s.st_ctime = of->s.st_mtime = of->s.st_atime = time;
out:
	UNLOCK();
	if (of == NULL) 
		HFSdebug("openfile failed");

	return(of);	
}

static int
hfs_read(void *filectxt, void *buf, size_t len)
{
	struct hfs_openfile	*of = (struct hfs_openfile *)filectxt;
	int			result;

	if (of->s.st_type != FILE_TYPE_FILE)
		return(-1);

	HFSdebug("%s: read %zu bytes to %p", of->path, len, buf);

	result = HFSReadFile(of->fs, of->path, buf, of->pos, len);

	if (result)
		HFSdebug("read returns %d", result);

	if (result > 0)
		of->pos += result;

	return(result);
}

static int
hfs_fstat(void *filectxt, struct stat *st)
{
	struct hfs_openfile	*of = (struct hfs_openfile *)filectxt;
	
	*st = of->s;
	return(0);
}

static off_t
hfs_seek(void *filectxt, off_t offset, int whence)
{
	struct hfs_openfile	*of = (struct hfs_openfile *)filectxt;
	off_t			newpos;

	switch(whence) {
	case FILE_SEEK_SET:
		newpos = offset;
		break;
		
	case FILE_SEEK_CUR:
		newpos = of->pos + offset;
		break;
		
	case FILE_SEEK_END:
		newpos = of->s.st_size + offset;
		break;
	default:
		return(-1);
	}
	if ((newpos < 0) || (newpos > of->s.st_size))
		return(-1);

	HFSdebug("%s: seek to 0x%llx", of->path, newpos);
	return((of->pos = newpos));
}

static void
hfs_close(void *filectxt)
{
	free(filectxt);
}

static void *
hfs_opendir(void *ctxt, const char *path)
{
	struct hfs_context	*cp = (struct hfs_context *)ctxt;
	struct hfs_opendir	*od;
	uint32_t		flags;
	uint32_t		path_length;

	HFSdebug("%s: opendir", path);
	od = NULL;
	if (path[0] == '/')
		path++;
	LOCK();
	
	// check for directory existence (/ always exists)
	if (path[0] != '\0') {
		if (HFSGetFileInfo(cp, path, &flags, 0, 0)) {
			HFSdebug("%s: can't get directory info", path);
			goto out;
		}
	}

	// alloc file and populate path
	path_length = strlen(path) + 1;
	od = malloc(sizeof(struct hfs_opendir) + path_length);
	od->path = (char *)(od + 1);
	strlcpy(od->path, path, path_length);
	od->pos = 0;
	od->fs = cp;
out:
	UNLOCK();
	if (od == NULL) {
		HFSdebug("opendir failed");
	} else {
		HFSdebug("opendir OK");
	}
	return(od);
}

static int
hfs_readdir(void *dirctxt, struct dirent *ent)
{
	struct hfs_opendir	*od = (struct hfs_opendir *)dirctxt;
	uint32_t		flags;
	time_t			time;
	off_t			size;
	int			result;

	if (od->pos == -1)
		return(-1);	// end of directory
	HFSdebug("%s: readdir at 0x%llx", od->path, od->pos);

	LOCK();
	if (HFSGetDirEntry(od->fs, od->path, &od->pos, ent->d_name, sizeof(ent->d_name), &flags, &time, &size) >= 0) {

		ent->d_fileno = od->pos;	// kinda dubious
		RELEASE_ASSERT(size <= INT32_MAX);
		ent->d_reclen = size;		// ?! seems to be what it's for
		switch(flags & kFileTypeMask) {
		case kFileTypeFlat:
			ent->d_type = FILE_TYPE_FILE;
			break;
		case kFileTypeDirectory:
			ent->d_type = FILE_TYPE_DIR;
			break;
		default:
			ent->d_type = FILE_TYPE_UNKNOWN;
			break;
		}
		ent->d_name[sizeof(ent->d_name) - 1] = '\0';
		ent->d_namlen = strlen(ent->d_name);
		ent->d_ctime = ent->d_atime = ent->d_atime = time;
		result = 0;
	} else {
		result = -1;
	}
	UNLOCK();

	return(result);
}

static void
hfs_rewinddir(void *dirctxt)
{
	struct hfs_opendir	*od = (struct hfs_opendir *)dirctxt;

	od->pos = 0;
}

static void
hfs_closedir(void *dirctxt)
{
	free(dirctxt);
}


/*******************************************************************************
 * bdev interface
 *
 * Note that the blockdev layer deblocks for us, so this interface is trivial.
 */

int
HFSBlockRead(CICell *ih, void *buf, off_t offset, uint32_t length)
{
	struct hfs_context	*cp = (struct hfs_context *)ih;

	HFSdebug("HFS physical read from 0x%llx/0x%x to %p", (unsigned long long)offset, length, buf);

	return(blockdev_read(cp->dev, buf, offset, length));
}
