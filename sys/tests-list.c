/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <unittest.h>
#include <list.h>

void test_list_is_empty(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);
	struct list_node list2;

	struct list_node item;
	struct list_node item2;

	TEST_ASSERT(list_is_empty(&list));

	list_add_head(&list, &item);

	TEST_ASSERT(!list_is_empty(&list));

	list_initialize(&list2);

	TEST_ASSERT(list_is_empty(&list2));

	list_add_head(&list2, &item2);

	TEST_ASSERT(!list_is_empty(&list2));
}

void test_list_add_head(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);
	struct list_node *iter;

	struct list_node item1;
	struct list_node item2;
	struct list_node item3;

	struct list_node* items[] = {&item1, &item2, &item3};
	int i;

	list_add_head(&list, &item1);

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 1);
		TEST_ASSERT_PTR_EQ(items[1 - (i + 1)], iter);
		i++;
	}

	list_add_head(&list, &item2);

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 2);
		TEST_ASSERT_PTR_EQ(items[2 - (i + 1)], iter);
		i++;
	}

	list_add_head(&list, &item3);

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 3);
		TEST_ASSERT_PTR_EQ(items[3 - (i + 1)], iter);
		i++;
	}
}

void test_list_add_tail(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);
	struct list_node *iter;

	struct list_node item1;
	struct list_node item2;
	struct list_node item3;

	struct list_node* items[] = {&item1, &item2, &item3};
	int i;

	list_add_tail(&list, &item1);

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 1);
		TEST_ASSERT_PTR_EQ(items[i], iter);
		i++;
	}

	list_add_tail(&list, &item2);

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 2);
		TEST_ASSERT_PTR_EQ(items[i], iter);
		i++;
	}

	list_add_tail(&list, &item3);

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 3);
		TEST_ASSERT_PTR_EQ(items[i], iter);
		i++;
	}
}

void test_list_remove_head(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);
	struct list_node *iter;
	struct list_node *removed;

	struct list_node item1;
	struct list_node item2;
	struct list_node item3;

	struct list_node* items[] = {&item1, &item2, &item3};
	int i;

	list_add_tail(&list, &item1);
	list_add_tail(&list, &item2);
	list_add_tail(&list, &item3);

	removed = list_remove_head(&list);
	TEST_ASSERT_PTR_EQ(removed, &item1);
	TEST_ASSERT(!list_in_list(removed));

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 2);
		TEST_ASSERT_PTR_EQ(items[i + 1], iter);
		i++;
	}

	removed = list_remove_head(&list);
	TEST_ASSERT_PTR_EQ(removed, &item2);
	TEST_ASSERT(!list_in_list(removed));

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 1);
		TEST_ASSERT_PTR_EQ(items[i + 2], iter);
		i++;
	}

	removed = list_remove_head(&list);
	TEST_ASSERT_PTR_EQ(removed, &item3);
	TEST_ASSERT(!list_in_list(removed));
	TEST_ASSERT(list_is_empty(&list));

	removed = list_remove_head(&list);
	TEST_ASSERT_NULL(removed);
	TEST_ASSERT(list_is_empty(&list));
}

void test_list_remove_tail(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);
	struct list_node *iter;
	struct list_node *removed;

	struct list_node item1;
	struct list_node item2;
	struct list_node item3;

	struct list_node* items[] = {&item1, &item2, &item3};
	int i;

	list_add_tail(&list, &item1);
	list_add_tail(&list, &item2);
	list_add_tail(&list, &item3);

	removed = list_remove_tail(&list);
	TEST_ASSERT_PTR_EQ(removed, &item3);
	TEST_ASSERT(!list_in_list(removed));

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 2);
		TEST_ASSERT_PTR_EQ(items[i], iter);
		i++;
	}

	removed = list_remove_tail(&list);
	TEST_ASSERT_PTR_EQ(removed, &item2);
	TEST_ASSERT(!list_in_list(removed));

	i = 0;
	list_for_every(&list, iter) {
		TEST_ASSERT_LT(i, 1);
		TEST_ASSERT_PTR_EQ(items[i], iter);
		i++;
	}

	removed = list_remove_tail(&list);
	TEST_ASSERT_PTR_EQ(removed, &item1);
	TEST_ASSERT(!list_in_list(removed));
	TEST_ASSERT(list_is_empty(&list));

	removed = list_remove_head(&list);
	TEST_ASSERT_NULL(removed);
	TEST_ASSERT(list_is_empty(&list));
}

void test_safe_unlink_double_free(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);

	struct list_node item1;
	struct list_node item2;

	list_add_tail(&list, &item1);
	list_add_tail(&list, &item2);

	list_delete(&item1);

	TEST_EXPECT_PANIC();
	list_delete(&item1);
	TEST_EXPECT_PANICKED();
}

void test_safe_unlink_next_corrupt(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);

	struct list_node item1;
	struct list_node item2;

	list_add_tail(&list, &item1);
	list_add_tail(&list, &item2);

	item1.next = item1.next->next;

	TEST_EXPECT_PANIC();
	list_delete(&item1);
	TEST_EXPECT_PANICKED();
}

void test_safe_unlink_prev_corrupt(uintptr_t param)
{
	struct list_node list = LIST_INITIAL_VALUE(list);

	struct list_node item1;
	struct list_node item2;

	list_add_tail(&list, &item1);
	list_add_tail(&list, &item2);

	item1.prev = item1.prev->prev;

	TEST_EXPECT_PANIC();
	list_delete(&item1);
	TEST_EXPECT_PANICKED();
}

struct test_suite linked_list_suite = {
	.name = "linked_list",
	.test_cases = {
		{ "list_is_empty", test_list_is_empty, 0, "Tests list_is_empty" },
		{ "list_add_head", test_list_add_head, 0, "Tests list_add_head" },
		{ "list_add_tail", test_list_add_tail, 0, "Tests list_add_tail" },
		{ "list_remove_head", test_list_remove_head, 0, "Tests list_remove_head" },
		{ "list_remove_tail", test_list_remove_tail, 0, "Tests list_remove_tail" },
		{ "safe_unlink_double_free", test_safe_unlink_double_free, 0, "Tests removing an item twice is detected" },
		{ "safe_unlink_next_corrupt", test_safe_unlink_next_corrupt, 0, "Tests corrupting the next pointer is detected" },
		{ "safe_unlink_prev_corrupt", test_safe_unlink_prev_corrupt, 0, "Tests corrupting the next pointer is detected" },
		TEST_CASE_LAST
	}
};

TEST_SUITE(linked_list_suite);
