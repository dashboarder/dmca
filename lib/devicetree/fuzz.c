#include <unittest.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <lib/env.h>
#include <lib/devicetree.h>
#include <mock_syscfg.h>

struct blockdev *create_mock_blockdev(const char *name, const char *filename, uint32_t block_size);

int do_printenv(int argc, void *args);

int fuzz_main(const char *filename)
{
	void *dt_in_buffer;
	void *dt_out_buffer;
	size_t dt_in_size;
	size_t dt_out_size;
	struct stat st;
	FILE *f;
	int result;
	dt_node_t *root;

	// Set up environment
	mock_syscfg_add('ABCD', "abcd", 4);
	mock_syscfg_add('AAAA', "abcdabcdabcdabcd", 16);
	mock_syscfg_add('AAAB', "abcdabcdabcdabc", 15);
	mock_syscfg_add('AAAC', "abcdabcdabcdabcda", 17);

	env_set("a", "", 0);
	env_set("b", "b", 0);
	env_set("c", "cc", 0);
	env_set("d", "ddd", 0);
	env_set("e", "eeee", 0);
	env_set("f", "fffff", 0);
	env_set("g", "ggggggggggggggggggggggggggggggggggggggggggg", 0);
	env_set("h", "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh", 0);
	env_set("i", "01:02:03:40:50:60", 0);

	// Get input size
	result = stat(filename, &st);
	TEST_ASSERT_EQ(result, 0);
	dt_in_size = st.st_size;

	// Read it
	f = fopen(filename, "r");
	TEST_ASSERT_NOT_NULL(f);

	dt_in_buffer = malloc(dt_in_size);
	result = fread(dt_in_buffer, 1, dt_in_size, f);
	TEST_ASSERT_EQ(result, dt_in_size);

	root = dt_deserialize(dt_in_buffer, dt_in_size);

	if (root != NULL) {
		// serialize the unmodified devicetree
		dt_out_size = dt_get_size();
		dt_out_buffer = malloc(dt_out_size);
		dt_serialize(root, dt_out_buffer, dt_out_size);
		free(dt_out_buffer);

		// make some mods
		dt_set_prop(root, "a", NULL, 0);
		dt_set_prop(root, "b", "abcdefg", 7);
		dt_set_prop(root, "c", "abcdefgh", 8);
		dt_set_prop(root, "d", "abcdefghi", 9);
		dt_remove_prop(root, "e");
		dt_rename_prop(root, "f", "z");

		dt_node_t *node;
		dt_find_node(root, "chosen", &node);
		dt_find_node(NULL, "chosen", &node);
		dt_find_node(root, "product/camera", &node);
		dt_find_node(NULL, "product/camera", &node);
		dt_find_node(root, "product/sdfasd", &node);

		// serialize with the modifications
		dt_out_size = dt_get_size();
		dt_out_buffer = malloc(dt_out_size);
		dt_serialize(root, dt_out_buffer, dt_out_size);
		free(dt_out_buffer);

		// causes deserialized devicetree to be freed
		dt_init();
	}

	free(dt_in_buffer);

	return result;
}

// Mock for lib/env
bool
env_blacklist(const char *name, bool write)
{
        return false;
}

bool
env_blacklist_nvram(const char *name)
{
        return false;
}
