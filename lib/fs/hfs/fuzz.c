#include <stdint.h>
#include <unittest.h>
#include <lib/blockdev.h>
#include <lib/fs.h>
#include <stdlib.h>
#include <string.h>

struct blockdev *create_mock_blockdev(const char *name, const char *filename, uint32_t block_size);

void walk(const char *dirpath)
{
	int dirfd;
	struct dirent ent;
	int result;

	dirfd = fs_opendir(dirpath);
	if (dirfd < 0) {
		printf("error opening dir '%s'\n", dirpath);
		return;
	}

	while ((result = fs_readdir(dirfd, &ent)) >= 0) {
		size_t subpath_size = strlen(dirpath) + 1 + strlen(ent.d_name) + 1;
		char *subpath = malloc(subpath_size);
		snprintf(subpath, subpath_size, "%s/%s", dirpath, ent.d_name);
		if (ent.d_type == FILE_TYPE_FILE) {
			int fd;
			printf("%s\n", subpath);

			fd = fs_open(subpath, 0);
			if (fd >= 0) {
				void *buffer;
				struct stat st;
				if (fs_stat(fd, &st) >= 0) {
					unsigned size = st.st_size;
					if (size > 1000000)
						size = 1000000;
					buffer = malloc(size);
					if (fs_read(fd, buffer, size) < 0) {
						printf("error reading file '%s'\n", subpath);
					}
					free(buffer);
				} else {
					printf("error statting file '%s'\n", subpath);
				}
			} else {
				printf("error opening file '%s'\n", subpath);
			}
		} else if (ent.d_type == FILE_TYPE_DIR) {
			printf("%s/\n", subpath);
			walk(subpath);
		}

		free(subpath);
	}
	fs_closedir(dirfd);
}

int fuzz_main(const char *filename)
{
	struct blockdev *bdev;
	int result;

	bdev = create_mock_blockdev("dev", filename, 4096);
	register_blockdev(bdev);

	result = fs_mount("dev", "hfs", "/boot");

	if (result == 0) {
		printf("Mount succeeded\n");
		walk("/boot");
	}
	
	return 0;
}

// Mock 
bool
env_blacklist(const char *name, bool write)
{
        return false;
}

// Mock 
bool
env_blacklist_nvram(const char *name)
{
        return false;
}
