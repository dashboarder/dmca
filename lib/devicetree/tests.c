/*
 * Copyright (C) 2014-2015 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <unittest.h>
#include <mock_syscfg.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct dt_test_data {
	void *input;
	const void *expected;
	size_t input_size;
	size_t expected_size;
	uint32_t misc;
} dt_test_data;

#define DT_TESTCASE(in, out) {			\
	.input = (void *)in,			\
	.input_size = sizeof(in) - 1,		\
	.expected = out,			\
	.expected_size = sizeof(out) - 1,	\
	.misc = 0				\
}

#define DT_TESTCASE_MISC(in, out, misc) {	\
	.input = in,				\
	.input_size = sizeof(in) - 1,		\
	.expected = out,			\
	.expected_size = sizeof(out) - 1,	\
	.misc = misc				\
}

#define DT_TESTCASE_FAIL(in) {			\
	.input = in,				\
	.input_size = sizeof(in) - 1,		\
	.expected = NULL,			\
	.expected_size = 0,			\
	.misc = 0				\
}

#define DT_TESTCASE_FAIL_MISC(in, misc) {	\
	.input = in,				\
	.input_size = sizeof(in) - 1,		\
	.expected = NULL,			\
	.expected_size = 0,			\
	.misc = misc				\
}

// handy for dumping two buffers side-by-side when tests fail
static void hexdump2(uint8_t *buf1, uint8_t *buf2, size_t size) __unused;
static void hexdump2(uint8_t *buf1, uint8_t *buf2, size_t size) {
	for (size_t i = 0; i < size; i += 8) {
		printf("%04zx: ", i);
		for (size_t j = 0; j < 8; j++) {
			if (i + j < size)
				printf(" %02x", buf1[i + j]);
			else
				printf("   ");
		}

		printf("    ");

		for (size_t j = 0; j < 8; j++) {
			if (i + j < size)
				printf(" %02x", buf2[i + j]);
			else
				printf("   ");
		}

		printf("    ");

		for (size_t j = 0; j < 8; j++) {
			if (i + j < size) {
				uint8_t c = buf1[i + j];
				if (!isprint(c))
					c = '.';
				printf("%c", c);
			} else {
				printf(" ");
			}
		}

		printf("    ");

		for (size_t j = 0; j < 8; j++) {
			if (i + j < size) {
				uint8_t c = buf2[i + j];
				if (!isprint(c))
					c = '.';
				printf("%c", c);
			} else {
				printf(" ");
			}
		}

		printf("\n");
	}
}

dt_node_t *test_common_deserialize(dt_test_data *data)
{
	dt_init();

	return dt_deserialize((void *)data->input, data->input_size);
}

void test_common_serialize(dt_test_data *data)
{
	void *serialize_buf;

	TEST_ASSERT_EQ(dt_get_size(), data->expected_size);

	serialize_buf = calloc(data->expected_size, 1);
	dt_serialize(0, serialize_buf, data->expected_size);
	TEST_ASSERT_MEM_EQ(serialize_buf, data->expected, data->expected_size);
	free(serialize_buf);
}

uint8_t test_dt1[] =
	// root node
	"\x01\x00\x00\x00" // 1 property
	"\x03\x00\x00\x00" // 3 children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 1st child
	"\x03\x00\x00\x00" // 3 properties
	"\x00\x00\x00\x00" // 0 children

	// 1st property of 1st child
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x07\x00\x00\x00" // property length 7
	"child1\0\0"

	// 2nd property of 1st child
	"property1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x09\x00\x00\x00" // property length 9
	"abcdefghi\0\0\0"

	// 3rd property of 1st child
	"property2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x08\x00\x00\x00" // property length 8
	"01234567"

	// 2nd child
	"\x03\x00\x00\x00" // 3 properties
	"\x01\x00\x00\x00" // 1 child

	// 1st property of 2nd child
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x07\x00\x00\x00" // property length 7
	"child2\0\0"

	// 2nd property of 2nd child
	"property1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x00\x00\x00\x00" // property length 0
	""

	// 3rd property of 2nd child
	"property2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x01\x00\x00\x00" // property length 1
	"X\x00\x00\x00"

	// 1st grandchild
	"\x02\x00\x00\x00" // 2 properties
	"\x00\x00\x00\x00" // 0 children

	// 1st property of 1st grandchild
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0b\x00\x00\x00" // property length 11
	"grandchild\x00\x00" // 

	// 2nd property of 1st grandchild
	"theprop\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"YYYY"

	// 3rd child
	"\x01\x00\x00\x00" // 1 property
	"\x00\x00\x00\x00" // 0 children

	// 1st property of 3rd child
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x07\x00\x00\x00" // property length 7
	"child3\0\0";

void test_find_property(dt_node_t *node, char *propName, void *expected, uint32_t expected_len)
{
	char *propNameInOut = propName;
	void *propData;
	uint32_t propSize;

	if (expected != NULL) {
		TEST_ASSERT_EQ(FindProperty(node, &propNameInOut, &propData, &propSize), true);
		// result should point into the device tree's backing store
		TEST_ASSERT_PTR_NEQ(propNameInOut, propName);
		TEST_ASSERT_MEM_EQ(propData, expected, expected_len);
		TEST_ASSERT_EQ(propSize, expected_len);
	} else {
		TEST_ASSERT_EQ(FindProperty(node, &propNameInOut, &propData, &propSize), false);
	}
}

void test_devicetree_legacy(uintptr_t param)
{
	dt_node_t *node;
	dt_node_t *root;
	dt_node_t *child1, *child2, *child3, *grandchild;

	dt_init();

	// - 1 for null terminator of the string
	dt_deserialize(&test_dt1[0], sizeof(test_dt1) - 1);

	// we should be able to find the root of the device tree
	TEST_ASSERT_EQ(FindNode(0, "", &root), true);

	// An empty path returns the same node
	TEST_ASSERT_EQ(FindNode(root, "", &node), true);
	TEST_ASSERT_PTR_EQ(root, node);

	// none of these should be found
	TEST_ASSERT_EQ(FindNode(0, "root", &node), false);
	TEST_ASSERT_EQ(FindNode(0, "/", &node), false);
	TEST_ASSERT_EQ(FindNode(0, "child0", &node), false);
	TEST_ASSERT_EQ(FindNode(0, "child1/grandchild", &node), false);
	TEST_ASSERT_EQ(FindNode(0, "child2/grandchild2", &node), false);
	TEST_ASSERT_EQ(FindNode(0, "child2/grandchild/child", &node), false);

	TEST_ASSERT_EQ(FindNode(0, "child1", &child1), true);
	TEST_ASSERT_NOT_NULL(child1);
	test_find_property(child1, "name", "child1", strlen("child1") + 1);
	test_find_property(child1, "property1", "abcdefghi", 9);
	test_find_property(child1, "property2", "01234567", 8);
	test_find_property(child1, "someprop", NULL, 0);

	TEST_ASSERT_EQ(FindNode(0, "child2", &child2), true);
	TEST_ASSERT_NOT_NULL(child2);
	test_find_property(child2, "name", "child2", strlen("child2") + 1);
	test_find_property(child2, "property1", "", 0);
	test_find_property(child2, "property2", "X", 1);
	test_find_property(child2, "someprop", NULL, 0);

	TEST_ASSERT_EQ(FindNode(0, "child3", &child3), true);
	TEST_ASSERT_NOT_NULL(child3);
	test_find_property(child3, "name", "child3", strlen("child3") + 1);
	test_find_property(child3, "property1", NULL, 0);
	test_find_property(child3, "property2", NULL, 0);

	TEST_ASSERT_EQ(FindNode(0, "child2/grandchild", &grandchild), true);
	TEST_ASSERT_NOT_NULL(grandchild);
	test_find_property(grandchild, "name", "grandchild", strlen("grandchild") + 1);
	test_find_property(grandchild, "theprop", "YYYY", 4);
}

void test_dt_get_prop(dt_node_t *node, char *propName, void *expected, uint32_t expected_len)
{
	char *propNameInOut = propName;
	void *propData;
	uint32_t propSize;

	if (expected != NULL) {
		TEST_ASSERT_EQ(dt_get_prop(node, &propNameInOut, &propData, &propSize), true);
		// result should point into the device tree's backing store
		TEST_ASSERT_PTR_NEQ(propNameInOut, propName);
		TEST_ASSERT_MEM_EQ(propData, expected, expected_len);
		TEST_ASSERT_EQ(propSize, expected_len);
	} else {
		TEST_ASSERT_EQ(dt_get_prop(node, &propNameInOut, &propData, &propSize), false);
	}
}

void test_dt_deserialize(uintptr_t context)
{
	dt_node_t *node;
	dt_node_t *root;
	dt_node_t *child1, *child2, *child3, *grandchild;

	dt_init();

	// - 1 for null terminator of the string
	root = dt_deserialize(&test_dt1[0], sizeof(test_dt1) - 1);
	TEST_ASSERT_NOT_NULL(root);

	// size should match the size of the source buffer
	TEST_ASSERT_EQ(dt_get_size(), sizeof(test_dt1) - 1);

	// we should be able to find the root of the device tree
	TEST_ASSERT_PTR_EQ(dt_get_root(), root);

	// An empty path returns the same node
	TEST_ASSERT_EQ(dt_find_node(root, "", &node), true);
	TEST_ASSERT_PTR_EQ(root, node);

	// none of these should be found
	TEST_ASSERT_EQ(dt_find_node(0, "root", &node), false);
	TEST_ASSERT_EQ(dt_find_node(0, "/", &node), false);
	TEST_ASSERT_EQ(dt_find_node(0, "child0", &node), false);
	TEST_ASSERT_EQ(dt_find_node(0, "child1/grandchild", &node), false);
	TEST_ASSERT_EQ(dt_find_node(0, "child2/grandchild2", &node), false);
	TEST_ASSERT_EQ(dt_find_node(0, "child2/grandchild/child", &node), false);

	TEST_ASSERT_EQ(dt_find_node(0, "child1", &child1), true);
	TEST_ASSERT_NOT_NULL(child1);
	test_dt_get_prop(child1, "name", "child1", strlen("child1") + 1);
	test_dt_get_prop(child1, "property1", "abcdefghi", 9);
	test_dt_get_prop(child1, "property2", "01234567", 8);
	test_dt_get_prop(child1, "someprop", NULL, 0);

	TEST_ASSERT_EQ(dt_find_node(0, "child2", &child2), true);
	TEST_ASSERT_NOT_NULL(child2);
	test_dt_get_prop(child2, "name", "child2", strlen("child2") + 1);
	test_dt_get_prop(child2, "property1", "", 0);
	test_dt_get_prop(child2, "property2", "X", 1);
	test_dt_get_prop(child2, "someprop", NULL, 0);

	TEST_ASSERT_EQ(dt_find_node(0, "child3", &child3), true);
	TEST_ASSERT_NOT_NULL(child3);
	test_dt_get_prop(child3, "name", "child3", strlen("child3") + 1);
	test_dt_get_prop(child3, "property1", NULL, 0);
	test_dt_get_prop(child3, "property2", NULL, 0);

	TEST_ASSERT_EQ(dt_find_node(0, "child2/grandchild", &grandchild), true);
	TEST_ASSERT_NOT_NULL(grandchild);
	test_dt_get_prop(grandchild, "name", "grandchild", strlen("grandchild") + 1);
	test_dt_get_prop(grandchild, "theprop", "YYYY", 4);
}

// Deserialize and then reserialize device tree,
// verify output is identical to input
void test_dt_serialize(uintptr_t context)
{
	dt_test_data data = DT_TESTCASE(test_dt1, test_dt1);

	test_common_deserialize(&data);
	test_common_serialize(&data);
}

// Deserialize and then reserialize device tree using a buffer
// too small for the result, verify that this results in a panic
void test_dt_serialize_too_small(uintptr_t context)
{
	size_t out_buffer_size;
	void *serialize_buf;

	dt_test_data data = DT_TESTCASE(test_dt1, test_dt1);

	test_common_deserialize(&data);
	dt_init();


	if ((int)context >= 0) {
		out_buffer_size = context;
	} else {
		out_buffer_size = data.expected_size - (size_t)(-1 * (int)context);
	}
	serialize_buf = calloc(out_buffer_size, 1);

	TEST_EXPECT_PANIC();

	dt_serialize(0, serialize_buf, out_buffer_size);
	free(serialize_buf);

	TEST_EXPECT_PANICKED();
}

uint8_t test_dt2[] =
	// root node
	"\x02\x00\x00\x00" // 2 properties
	"\x01\x00\x00\x00" // 1 child

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root
	"prop\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"QWER"

	// 1st child
	"\x03\x00\x00\x00" // 3 properties
	"\x00\x00\x00\x00" // 0 children

	// 1st property of 1st child
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x07\x00\x00\x00" // property length 7
	"child1\0\0"

	// 2nd property of 1st child
	"prop\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"ASDF"

	// 3rd property of 1st child
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x00\x00\x00\x00"; // property length 0

uint8_t test_dt2_out[] =
	// root node
	"\x01\x00\x00\x00" // 1 property
	"\x01\x00\x00\x00" // 1 child

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 1st child
	"\x09\x00\x00\x00" // 9 properties
	"\x00\x00\x00\x00" // 0 children

	// 1st property of 1st child
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0e\x00\x00\x00" // property length 14
	"renamed-child\0\0\0"

	// 2nd property of 1st child
	"renamed-prop\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"ASDF"

	// 3rd property of 1st child
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x00\x00\x00\x00" // property length 0

	// 4th property of 1st child
	"newprop32\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"\xcd\xab\x34\x12"

	// 5th property of 1st child
	"newprop64\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x08\x00\x00\x00" // property length 8
	"\xef\xcd\xab\x90\x78\x56\x34\x12"

	// 6th property of 1st child
	"newpropaddr\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x08\x00\x00\x00" // property length 8
	"\x88\x77\x66\x55\x44\x33\x22\x11"

	// 7th property of 1st child
	"newpropstr\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x08\x00\x00\x00" // property length 8
	"strprop\0"

	// 8th property of 1st child
	"newpropempty\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x00\x00\x00\x00" // property length 0

	// 9th property of 1st child
	"newpropdata\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0b\x00\x00\x00" // property length 11
	"somedata\0xy\0";

void test_dt_update(uintptr_t context)
{
	char *prop_name;
	dt_node_t *root;
	dt_node_t *node;

	dt_test_data data = DT_TESTCASE(test_dt2, test_dt2_out);

	root = test_common_deserialize(&data);
	TEST_ASSERT_NOT_NULL(root);

	// removing a property returns true if it existed
	TEST_ASSERT(dt_remove_prop(root, "prop"));
	// and false if not
	TEST_ASSERT(!dt_remove_prop(root, "prop"));
	TEST_ASSERT(!dt_remove_prop(root, "prop2"));

	TEST_ASSERT(dt_find_node(root, "child1", &node));

	dt_set_prop_32(node, "newprop32", 0x1234abcd);
	dt_set_prop_64(node, "newprop64", 0x1234567890abcdefULL);
	dt_set_prop_addr(node, "newpropaddr", 0x1122334455667788ULL);
	dt_set_prop_str(node, "newpropstr", "strprop");
	dt_set_prop(node, "newpropempty", "", 0);
	dt_set_prop(node, "newpropdata", "somedata\0xy", 11);

	// After changing the node's name, it should be findable under
	// the new name, and not under the old one
	dt_set_prop_str(node, "name", "renamed-child");
	TEST_ASSERT(!dt_find_node(root, "child1", &node));
	TEST_ASSERT(dt_find_node(root, "renamed-child", &node));

	// After changing the property's name, it should be findable under
	// the new name, and not under the old one
	prop_name = "prop";
	TEST_ASSERT(dt_get_prop(node, &prop_name, NULL, NULL));
	TEST_ASSERT(dt_rename_prop(node, "prop", "renamed-prop"));
	prop_name = "prop";
	TEST_ASSERT(!dt_get_prop(node, &prop_name, NULL, NULL));
	prop_name = "renamed-prop";
	TEST_ASSERT(dt_get_prop(node, &prop_name, NULL, NULL));

	test_common_serialize(&data);
}

uint8_t test_dt_update_shorter_in[] =
	// root node
	"\x04\x00\x00\x00" // 4 properties
	"\x00\x00\x00\x00" // 0 children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root
	"prop1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x08\x00\x00\x00" // property length 8
	"12345678"

	// 3rd property of root
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x07\x00\x00\x00" // property length 7
	"1234567\0"

	// 4th property of root
	"prop3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x09\x00\x00\x00" // property length 9
	"123456789\0\0\0"

	// 5th property of root
	"prop4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x08\x00\x00\x00" // property length 8
	"12345678";

uint8_t test_dt_update_shorter_out[] =
	// root node
	"\x05\x00\x00\x00" // 5 properties
	"\x00\x00\x00\x00" // 0 children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root
	"prop1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"abcd"

	// 3rd property of root
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"ABCD"

	// 4th property of root
	"prop3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"QWER"

	// 5th property of root
	"prop4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"ASDFG\0\0\0";

void test_dt_update_shorter(uintptr_t context)
{
	dt_node_t *root;

	dt_test_data data = DT_TESTCASE(test_dt_update_shorter_in, test_dt_update_shorter_out);

	root = test_common_deserialize(&data);
	TEST_ASSERT_NOT_NULL(root);

	dt_set_prop(root, "prop1", "abcd", 4);
	dt_set_prop(root, "prop2", "ABCD", 4);
	dt_set_prop(root, "prop3", "QWER", 4);
	dt_set_prop(root, "prop4", "ASDFG", 5);

	test_common_serialize(&data);
}

uint8_t test_dt_placeholders_data[] =
	// root node
	"\x09\x00\x00\x00" // num properties
	"\x00\x00\x00\x00" // num children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root (valid keys should get filled in)
	"prop1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0c\x00\x00\x80" // placeholder property length 12
	"syscfg/ABCD\0"

	// 3rd property of root (invalid keys should get elided)
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0c\x00\x00\x80" // placeholder property length 12
	"syscfg/NNNN\0"

	// 4th property of root (valid key after invalid key should get filled in)
	"prop3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x18\x00\x00\x80" // placeholder property length 24
	"syscfg/NNNN,syscfg/ABCD\0"

	// 5th property of root (1st value found should get copied)
	"prop4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x18\x00\x00\x80" // placeholder property length 24
	"syscfg/FFFF,syscfg/ABCD\0"

	// 6th property of root (syscg value longer than placeholder)
	"prop5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0c\x00\x00\x80" // placeholder property length 12
	"syscfg/BigE\0"

	// 7th property of root (syscg value truncated)
	"prop6\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0e\x00\x00\x80" // placeholder property length 14
	"syscfg/BigE/6\0\0\0"

	// 8th property of root (syscg value padded)
	"prop7\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0f\x00\x00\x80" // placeholder property length 15
	"syscfg/ABCD/12\0\0"

	// 9th property of root (macaddr from environment)
	"prop8\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x10\x00\x00\x80" // placeholder property length 16
	"macaddr/ethaddr\0";

uint8_t test_dt_placeholders_out[] =
	// root node
	"\x08\x00\x00\x00" // num properties
	"\x00\x00\x00\x00" // num children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root (valid keys should get filled in)
	"prop1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x06\x00\x00\x00" // property length 6
	"found!\0\0"

	// 3rd property of root (valid key after invalid key should get filled in)
	"prop3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x06\x00\x00\x00" // property length 6
	"found!\0\0"

	// 4th property of root (1st value found should get copied)
	"prop4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length 4
	"yess"

	// 5th property of root (syscfg value longer than placeholder)
	"prop5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x1a\x00\x00\x00" // property length 26
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ\0\0"

	// 6th property of root (syscfg value truncated)
	"prop6\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x06\x00\x00\x00" // property length 6
	"ABCDEF\0\0"

	// 7th property of root (syscfg value padded)
	"prop7\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0c\x00\x00\x00" // property length 12
	"found!\0\0\0\0\0\0"

	// 8th property of root (mac address)
	"prop8\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x06\x00\x00\x00" // property length 6
	"\x01\x02\x03\x40\x50\x60\0\0";

void test_dt_placeholders(uintptr_t context)
{
	dt_node_t *root;

	mock_syscfg_reset();
	mock_syscfg_add('ABCD', "found!", strlen("found!"));
	mock_syscfg_add('FFFF', "yess", strlen("yess"));
	mock_syscfg_add('BigE', "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
	env_set("ethaddr", "01:02:03:40:50:60", 0);

	dt_test_data data = DT_TESTCASE(test_dt_placeholders_data, test_dt_placeholders_out);

	root = test_common_deserialize(&data);
	TEST_ASSERT_NOT_NULL(root);

	test_common_serialize(&data);
}

uint8_t test_dt_placeholders_zeroes_data[] =
	// root node
	"\x04\x00\x00\x00" // num properties
	"\x00\x00\x00\x00" // num children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root
	"prop1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x09\x00\x00\x80" // property length
	"zeroes/2\0\0\0\0"

	// 3nd property of root
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x09\x00\x00\x80" // property length
	"zeroes/4\0\0\0\0"

	// 4th property of root
	"prop3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x0c\x00\x00\x80" // property length
	"zeroes/0x20\0";


uint8_t test_dt_placeholders_zeroes_out[] =
	// root node
	"\x04\x00\x00\x00" // num properties
	"\x00\x00\x00\x00" // num children

	// 1st property of root
	"name\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x05\x00\x00\x00" // property length 5
	"root\0\0\0\0"

	// 2nd property of root
	"prop1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x02\x00\x00\x00" // property length
	"\0\0\0\0"

	// 3nd property of root
	"prop2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x04\x00\x00\x00" // property length
	"\0\0\0\0"

	// 4th property of root
	"prop3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\x20\x00\x00\x00" // property length
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";


void test_dt_placeholders_zeroes(uintptr_t context)
{
	dt_node_t *root;

	dt_test_data data = DT_TESTCASE(test_dt_placeholders_zeroes_data, test_dt_placeholders_zeroes_out);

	root = test_common_deserialize(&data);
	TEST_ASSERT_NOT_NULL(root);

	test_common_serialize(&data);
}

void test_common_short(dt_test_data *data)
{
	dt_node_t *root;

	// Start by making sure the data is well-formed
	root = test_common_deserialize(data);
	TEST_ASSERT_NOT_NULL(root);
	test_common_serialize(data);

	// Now chop off a byte at a time, verifying we get a failure each time
	for (size_t i = 0; i < data->input_size; i++) {
		dt_init();

		root = dt_deserialize((void *)data->input, i);
		TEST_ASSERT_NULL(root);
	}
}

void test_dt_short1(uintptr_t context)
{
	uint8_t input_data[] =
		"\x00\x00\x00\x00\00\x00\x00\x00" // no properties, no children
		;

	dt_test_data data = DT_TESTCASE(input_data, input_data);

	test_common_short(&data);
}

void test_dt_short2(uintptr_t context)
{
	uint8_t input_data[] =
		"\x01\x00\x00\x00\x00\x00\x00\x00" // one property, no children
		"abcdefghijklmnopqrstuvqxzy01234\0" // 31-char name
		"\x03\x00\x00\x00" // property len 3
		"abc\0" // property value
		;

	dt_test_data data = DT_TESTCASE(input_data, input_data);

	test_common_short(&data);
}

void test_dt_short3(uintptr_t context)
{
	uint8_t input_data[] =
		"\x01\x00\x00\x00\x01\x00\x00\x00" // one property, one child
		"abcdefghijklmnopqrstuvqxzy01234\0" // 31-char name
		"\x03\x00\x00\x00" // property len 3
		"abc\0" // property value
		"\x00\x00\x00\x00\x00\x00\x00\x00" // no properties, no children
		;

	dt_test_data data = DT_TESTCASE(input_data, input_data);

	test_common_short(&data);
}

void test_dt_bad_name(uintptr_t context)
{
	uint8_t input_data[] =
		"\x01\x00\x00\x00\x00\x00\x00\x00" // one property, no children
		"abcdefghijklmnopqrstuvqxzy012345" // 32-char name
		"\x03\x00\x00\x00" // property len 3
		"abc\0" // property value
		;
	dt_node_t *root;

	dt_test_data data = DT_TESTCASE_FAIL(input_data);

	dt_init();
	root = dt_deserialize(data.input, data.input_size);
	TEST_ASSERT_NULL(root);
}

static struct test_suite devicetree_test_suite = {
	.name = "devicetree",
	.description = "tests the devicetree parser",
	.test_cases = {
		{ "devicetree_legacy", test_devicetree_legacy, 0 },
		{ "dt_deserialize", test_dt_deserialize, 0 },
		{ "dt_serialize", test_dt_serialize, 0 },
		{ "dt_serialize_too_small0", test_dt_serialize, 0 },
		{ "dt_serialize_too_small1", test_dt_serialize, 1 },
		{ "dt_serialize_too_small-1", test_dt_serialize, (uintptr_t)-1LL },
		{ "dt_update", test_dt_update, 0 },
		{ "dt_update_shorter", test_dt_update_shorter, 0 },
		{ "dt_placeholders", test_dt_placeholders, 0 },
		{ "dt_placeholders_zeroes", test_dt_placeholders_zeroes, 0 },
		{ "dt_short1", test_dt_short1, 0 },
		{ "dt_short2", test_dt_short2, 0 },
		{ "dt_short3", test_dt_short3, 0 },
		{ "dt_bad_name", test_dt_bad_name, 0 },
		TEST_CASE_LAST
	}
};

TEST_SUITE(devicetree_test_suite);

bool env_blacklist(const char *name, bool write)
{
	return false;
}

bool env_blacklist_nvram(const char *name)
{
	return false;
}
