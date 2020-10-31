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
#include <sys.h>
#include <unittest.h>

static int irq_disable_count;

#if DEBUG_CRITICAL_SECTIONS
void _enter_critical_section(const char *from)
{
#else
void enter_critical_section(void)
{
#endif
	irq_disable_count++;
}

#if DEBUG_CRITICAL_SECTIONS
void _exit_critical_section(const char *from)
{
#else
void exit_critical_section(void)
{
#endif
	TEST_ASSERT_GT(irq_disable_count, 0);
	irq_disable_count--;
}

void reset_critical_section(void)
{
	irq_disable_count = 0;
}

TEST_SETUP_HOOK(reset_critical_section);
