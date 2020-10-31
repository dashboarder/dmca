/*
 * Copyright (C) 2008, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/mib.h>
#include <list.h>
#include <lib/libc.h>
#include <lib/libiboot.h>
#include <sys/menu.h>
#include <string.h>

static struct mib_node *mib_find_node(u_int32_t oid, bool nofollow);

/*
 * Find a node in the MIB.
 */
static struct mib_node *
mib_find_node(u_int32_t oid, bool nofollow)
{
	void **cursor;
	struct mib_node	*node;

restart:
	node = NULL;
	
	/* Scan the static list for non-weak nodes */
	LINKER_SET_FOREACH(cursor, mib) {
		node = (struct mib_node *)*cursor;

		if ((oid == node->node_oid) && !(node->node_type & kOIDWeak))
			goto found;
	}

	/* and finally give weak nodes a chance */
	LINKER_SET_FOREACH(cursor, mib) {
		node = (struct mib_node *)*cursor;

		if (oid == node->node_oid)
			goto found;
	}

	// not found
	return NULL;

found:
	/* handle indirect nodes */
	if (!nofollow && (NULL != node) && (node->node_type == kOIDTypeOIDIndirect)) {
		oid = node->node_data.v32;
		goto restart;
	}
	return(node);
}

/*
 * Verify whether a node exists or not
 */
bool mib_exists(uint32_t oid, void *value)
{
	bool retVal = false;
	struct mib_node *node;
	if(NULL != (node = mib_find_node(oid, false))) {
		retVal = true;
		if(NULL != value) {
			// value of node has been requested
			switch (node->node_type & kOIDTypeMask) {
			case kOIDTypeUInt32:
				*(u_int32_t *)value = node->node_data.v32;
				break;
			case kOIDTypeInt32:
				*(int32_t *)value = node->node_data.v32;
				break;
			case kOIDTypeUInt64:
				*(u_int64_t *)value = node->node_data.v64;
				break;
			case kOIDTypeInt64:
				*(int64_t *)value = node->node_data.v64;
				break;
			case kOIDTypeBoolean:
				*(bool *)value = node->node_data.v_bool;
				break;
				case kOIDTypeString:
			case kOIDTypeStruct:
				*(void **)value = node->node_data.v_ptr;
				break;
			default:
				panic("unexpected node type 0x%08x for oid 0x%08x", node->node_type, oid);
			}
		}
	}
	return retVal;
}
/*
 * Look for a node and return its value if its present.
 */
bool
_mib_find(u_int32_t oid, u_int32_t node_type, void *value)
{
	struct mib_node *node;

	/* find the node */
	if (NULL == (node = mib_find_node(oid, false)))
		return(false);

	/* if we have been given a type hint, check it against the node */
	if ((0 != node_type) && (node_type != (node->node_type & kOIDTypeMask))) {
		panic("MIB node type mismatch: requested 0x%08x actual 0x%08x for oid 0x%08x",
			node_type, node->node_type, oid);
		return(false);
	}

	if (node->node_type & kOIDFunction) {
		/* call node handler function */
		node->node_data.v_func.func(oid, node->node_data.v_func.arg, value);
		
	} else if (node->node_type & kOIDDataIndirect) {
		/* node data is indirected through v_ptr */
		switch (node->node_type & kOIDTypeMask) {
		case kOIDTypeUInt32:
			*(u_int32_t *)value = *(u_int32_t *)node->node_data.v_ptr;
			break;
		case kOIDTypeUInt64:
			*(u_int64_t *)value = *(u_int64_t *)node->node_data.v_ptr;
			break;
		case kOIDTypeBoolean:
			*(bool *)value = *(bool *)node->node_data.v_ptr;
			break;
		case kOIDTypeString:
		case kOIDTypeStruct:
			*(void **)value = *(void **)node->node_data.v_ptr;
			break;
		default:
			panic("unexpected MIB node type 0x%08x for oid 0x%08x", node->node_type, oid);
		}
		
	} else {
		/* node data is in the structure */
		switch (node->node_type & kOIDTypeMask) {
		case kOIDTypeUInt32:
			*(u_int32_t *)value = node->node_data.v32;
			break;
		case kOIDTypeInt32:
			*(int32_t *)value = node->node_data.v32;
			break;
		case kOIDTypeUInt64:
			*(u_int64_t *)value = node->node_data.v64;
			break;
		case kOIDTypeInt64:
			*(int64_t *)value = node->node_data.v64;
			break;
		case kOIDTypeBoolean:
			*(bool *)value = node->node_data.v_bool;
			break;
		case kOIDTypeString:
		case kOIDTypeStruct:
			*(void **)value = node->node_data.v_ptr;
			break;
		default:
			panic("unexpected node type 0x%08x for oid 0x%08x", node->node_type, oid);
		}
	}
	return(true);
}

/*
 * Get the value of a node.  Panic if it doesn't exist.
 */
void
_mib_get(u_int32_t oid, u_int32_t node_type, void *value)
{
	if (false == _mib_find(oid, node_type, value))
		panic("mib: oid 0x%08x type 0x%08x not found", oid, node_type);
}

static struct mib_description *
mib_find_ident(u_int32_t oid)
{
	int	i;

	for (i = 0; mib_desc[i].desc != NULL; i++)
		if (mib_desc[i].oid == oid)
			return(mib_desc + i);
	return(NULL);
}

static void
mib_find_oid_ident(u_int32_t oid, const char **subsystem, const char **node)
{
	struct mib_description	*md;;

	*subsystem = "????";
	*node = "????";

	if (NULL != (md = mib_find_ident(oid & OID_SUBSYSTEM_MASK))) {
		*subsystem = md->desc;

		if (NULL != (md = mib_find_ident(oid)))
			*node = md->desc;
	}
}

void
print_mib_node(struct mib_node *node)
{
	size_t	ssize;
	const char *s, *n;
	u_int32_t ntype;
	int cols;
	char f = ' ', i = ' ', w = ' ';

	mib_find_oid_ident(node->node_oid, &s, &n);

	if (node->node_type & kOIDDataIndirect) {
		i = 'I';
	}

	if (node->node_type & kOIDFunction) {
		f = 'F';
	}

	if (node->node_type & kOIDWeak) {
		w = 'W';
	}

	printf("  0x%08x %c%c%c %s.%s ", node->node_oid, f, i, w, s, n);
	cols = strlen(s) + strlen(n) + 16;
	if (cols < 50)
		printf("%*s", 53 - cols, "= ");

	ntype = node->node_type & kOIDTypeMask;

	switch (ntype) {
	case kOIDTypeUInt32:
		printf("0x%08x / %u / %d\n",
			mib_get_u32(node->node_oid),
			mib_get_u32(node->node_oid),
			mib_get_u32(node->node_oid));
		break;

	case kOIDTypeInt32:
		printf("0x%08x / %u / %d\n",
			mib_get_s32(node->node_oid),
			mib_get_s32(node->node_oid),
			mib_get_s32(node->node_oid));
		break;

	case kOIDTypeUInt64:
		printf("0x%016llx / %llu / %lld\n",
			mib_get_u64(node->node_oid),
			mib_get_u64(node->node_oid),
			mib_get_u64(node->node_oid));
		break;

	case kOIDTypeInt64:
		printf("0x%016llx / %llu / %lld\n",
			mib_get_s64(node->node_oid),
			mib_get_s64(node->node_oid),
			mib_get_s64(node->node_oid));
		break;

	case kOIDTypeBoolean:
		printf("%s\n",
		    mib_get_bool(node->node_oid) ? "true" : "false");
		break;

	case kOIDTypeString:
		s = (const char *)mib_get_str(node->node_oid);
		printf("%s\n", s);
		break;

	case kOIDTypeOIDIndirect:
		printf("-> 0x%08x\n",
		    node->node_data.v32);
		break;

	default:
		if (kOIDTypeStruct == ntype) {
			ssize = node->node_type & kOIDStructSizeMask;
			printf("struct\n");
			if (ssize > 0)
				hexdump(mib_get_ptr(node->node_oid), (ssize <= 128) ? ssize : 128);
			if (ssize > 128)
				printf(" ...\n");
			break;
		}
		printf("unknown type 0x%08x\n", node->node_type);
		break;
	}
}

int
do_mib(int argc, struct cmd_arg *args)
{
	void **cursor;
	struct mib_node	*node;

	printf("MIB dump:\n");
	
	/* Scan the static list */
	/* XXX note that this will print duplicates for static nodes that are overridden */
	LINKER_SET_FOREACH(cursor, mib) {
		node = (struct mib_node *)*cursor;
		print_mib_node(node);
	}
	return(0);
}

