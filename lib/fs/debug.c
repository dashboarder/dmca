/*
 * Copyright (C) 2007-2009, 2014  Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple  Inc.
 */
#include <debug.h>
#include <lib/fs.h>
#include <lib/env.h>
#include <sys/menu.h>

int
fs_cat_file(const char *path)
{
	int filefd;
	char buf[512];
	struct stat stat;
	off_t resid, readsize;
	int i;

	/* open the file */
	filefd = fs_open(path, 0);
	if (filefd < 0)
		return -1;

	/* stat it */
	fs_stat(filefd, &stat);

	resid = stat.st_size;
	while (resid) {
		readsize = sizeof(buf);
		if (readsize > resid)
			readsize = resid;
		fs_read(filefd, (void *)buf, readsize);
		for (i = 0; i < readsize; i++)
			printf("%c", buf[i]);
		resid -= readsize;
	}
	printf("\n");

	fs_close(filefd);
	return(0);
}

void
fs_stat_dump(const struct stat *st)
{
	printf("STAT (%p):\n", (void *) st);
	printf(" st_ino:    %x\n", st->st_ino);
	printf(" st_size    %x\n", st->st_size);
	printf(" st_blocks: %x\n", st->st_blocks);
	printf(" st_flags:  %x\n", st->st_flags);
	printf(" st_type: ");
	switch (st->st_type) {
		case FILE_TYPE_FILE:
			printf("FILE\n");
			break;
		case FILE_TYPE_DIR:
			printf("DIR\n");
			break;
		default:
			printf("UNKNOWN\n");
	}
	
	printf(" st_ctime:  %x\n", st->st_ctime);
	printf(" st_mtime:  %x\n", st->st_mtime);
	printf(" st_atime:  %x\n", st->st_atime);
}

void
fs_statfs_dump(const struct statfs *st)
{
	printf("STATFS (%p):\n", (void *) st);
	printf(" f_bsize:  %x\n", st->f_bsize);
	printf(" f_blocks: %x\n", st->f_blocks);
	printf(" f_bfree:  %x\n", st->f_bfree);
}

int fs_dump_dir(const char *dirpath, bool recurse)
{
	int dirfd;
	struct dirent ent;

	printf("fs_dump_dir: dumping contents of directory '%s'\n", dirpath);

	dirfd = fs_opendir(dirpath);
	if (dirfd < 0) {
		printf("error opening dir '%s'\n", dirpath);
		return -1;
	}

	while (fs_readdir(dirfd, &ent) >= 0) {
		printf("\tname '%s', size %d, type 0x%x\n", ent.d_name, ent.d_reclen, ent.d_type);
	}
	fs_closedir(dirfd);

	return 0;
}

int do_fs(int argc, struct cmd_arg *args)
{
	int	err;
	size_t	max_file_size;

	if (!security_allow_modes(kSecurityModeDebugCmd)) {
		printf("Permission Denied\n");
		return -1;
	}

	if (argc < 2) {
notenoughargs:
		printf("not enough arguments.\n");
usage:
		printf("usage:\n");
		printf("%s mount <fs> <device> <dir>\n", args[0].str);
		printf("%s unmount <dir>\n", args[0].str);
		printf("%s mounts\n", args[0].str);
		printf("%s dir <path>\n", args[0].str);
		printf("%s load <path> <loadaddress>\n", args[0].str);
		printf("%s cat <path>\n", args[0].str);

		return 0;
	}

	if (!strcmp(args[1].str, "mount")) {
		if (argc < 5)
			goto notenoughargs;

		err = fs_mount(args[3].str, args[2].str, args[4].str);
		printf("mount returned %d\n", err);
	} else if (!strcmp(args[1].str, "unmount")) {
		if (argc < 3)
			goto notenoughargs;

		fs_unmount(args[2].str);
	} else if (!strcmp(args[1].str, "mounts")) {
		fs_dump_mounts();
	} else if (!strcmp(args[1].str, "dir")) {
		if (argc < 3)
			goto notenoughargs;

		fs_dump_dir(args[2].str, false);	
	} else if (!strcmp(args[1].str, "load")) {
		if (argc < 4)
			goto notenoughargs;

		/* Allow file to be up to 32 MB. */
		max_file_size = 0x02000000;
		if (!security_allow_memory((void *)args[3].u, max_file_size)) {
			printf("Permission Denied\n");
			return -1;
		}

		err = fs_load_file(args[2].str, args[3].u, &max_file_size);
		printf("fs_load_file() returns %d\n", err);
		if (err < 0)
			return err;

		env_set_uint("loadaddr", args[3].u, 0);
		env_set_uint("filesize", max_file_size, 0);
	} else if (!strcmp(args[1].str, "cat")) {
		if (argc < 3)
			goto notenoughargs;

		err = fs_cat_file(args[2].str);
	} else {
		printf("unrecognized subcommand.\n");
		goto usage;
	}

	return 0;
}

