/*
 * Copyright (C) 2008-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _MIB_H
#define _MIB_H

#include <stdbool.h>
#include <lib/mib/mib_nodes.h>
#include <sys/linker_set.h>
#include <sys/types.h>

/*
 * Standard MIB application/product definitions.
 */
#define MIB_APP_PROD_UNKNOWN		(0)
#define MIB_APP_PROD_SECUREROM		(1)
#define MIB_APP_PROD_IBSS		(2)
#define MIB_APP_PROD_IBEC		(3)
#define MIB_APP_PROD_IBOOT		(4)
#define MIB_APP_PROD_LLB		(5)
#define MIB_APP_PROD_EMBEDDEDIOP	(8)

__BEGIN_DECLS

#define OID_SUBSYSTEM(_o)	((_o)>>24)
#define OID_NODE(_o)		(((_o)>>16)&0xff)

#define OID_SUBSYSTEM_MASK	(0xff<<24)
#define OID_NODE_MASK		(0xff<<16)

typedef void	(* mib_func_t)(uint32_t oid, void *arg, void *value);

struct mib_func_spec {
	mib_func_t	func;
	void		*arg;
};

union mib_value {
	uint32_t	v32;
	uint64_t	v64;
	bool		v_bool;
	void		*v_ptr;
	struct mib_func_spec v_func;
};

struct mib_node {
	uint32_t	node_oid;
	uint32_t	node_type;
	union mib_value	node_data;
};

/* node data types */
#define kOIDTypeUInt32		(0x00<<16)	/* node data is uint32_t */
#define kOIDTypeInt32		(0x01<<16)	/* node data is int32_t */
#define kOIDTypeUInt64		(0x02<<16)	/* node data is uint64_t */
#define kOIDTypeInt64		(0x03<<16)	/* node data is int64_t */
#if defined(__LP64__)
#define kOIDTypeAddr		kOIDTypeUInt64	/* node data is uint64_t */
#define kOIDTypeSize		kOIDTypeUInt64	/* node data is uint64_t */
#else
#define kOIDTypeAddr		kOIDTypeUInt32	/* node data is uint32_t */
#define kOIDTypeSize		kOIDTypeUInt32	/* node data is uint32_t */
#endif
#define kOIDTypeBoolean		(0x04<<16)	/* node data is bool */
#define kOIDTypeString		(0x05<<16)	/* node data is const char * */
#define kOIDTypeStruct		(0x06<<16)	/* node data is void * */
#define kOIDTypeOIDIndirect	(0x07<<16)	/* node data is oid (reference to another node) */

#define kOIDTypeMask		(0xff<<16)
#define kOIDStructSizeMask	(0xffff)

/* node data qualifiers */
#define kOIDDataIndirect	(1<<24)		/* data is indirected through v_ptr */
#define kOIDFunction		(2<<24)		/* data is obtained by calling v_func */
#define kOIDWeak		(4<<24)		/* node only valid if no other node present */

# define MIB_CONSTANT(_oid, _type, _value)				\
static const struct mib_node __attribute__((used))			\
__mib_node_ ## _oid = {							\
	.node_oid	= _oid,						\
	.node_type	= _type,					\
	.node_data.v64	= (uint64_t)(_value)				\
};									\
LINKER_SET_ENTRY(mib, __mib_node_ ## _oid)

# define MIB_CONSTANT_WEAK(_oid, _type, _value)				\
static const struct mib_node __attribute__((used))			\
__mib_node_weak_ ## _oid = {						\
	.node_oid	= _oid,						\
	.node_type	= _type | kOIDWeak,				\
	.node_data.v64	= (uint64_t)(_value)				\
};									\
LINKER_SET_ENTRY(mib, __mib_node_weak_ ## _oid)

# define MIB_CONSTANT_PTR(_oid, _type, _value)				\
static const struct mib_node __attribute__((used))			\
__mib_node_ ## _oid = {							\
	.node_oid	= _oid,						\
	.node_type	= _type,					\
	.node_data.v_ptr = (_value)					\
};									\
LINKER_SET_ENTRY(mib, __mib_node_ ## _oid)

# define MIB_CONSTANT_PTR_WEAK(_oid, _type, _value)			\
static const struct mib_node __attribute__((used))			\
__mib_node_weak_ ## _oid = {						\
	.node_oid	= _oid,						\
	.node_type	= _type | kOIDWeak,				\
	.node_data.v_ptr = (_value)					\
};									\
LINKER_SET_ENTRY(mib, __mib_node_weak_ ## _oid)

# define MIB_VARIABLE(_oid, _type, _var)				\
static const struct mib_node __attribute__((used))			\
__mib_node_##_oid = {							\
	.node_oid	= _oid,						\
	.node_type	= _type | kOIDDataIndirect,			\
	.node_data.v_ptr = (void *)(&_var)				\
};									\
LINKER_SET_ENTRY(mib, __mib_node_ ## _oid)

# define MIB_VARIABLE_WEAK(_oid, _type, _var)				\
static const struct mib_node __attribute__((used))			\
__mib_node_weak_ ## _oid = {						\
	.node_oid	= _oid,						\
	.node_type	= _type | kOIDDataIndirect | kOIDWeak,		\
	.node_data.v_ptr = (void *)(&_var)				\
};									\
LINKER_SET_ENTRY(mib, __mib_node_weak_ ## _oid)

# define MIB_FUNCTION(_oid, _type, _func, _arg)				\
static const struct mib_node __attribute__((used))			\
__mib_node_##_oid = {							\
	.node_oid		= _oid,					\
	.node_type		= _type | kOIDFunction,			\
	.node_data.v_func	= {_func, _arg}				\
};									\
LINKER_SET_ENTRY(mib, __mib_node_ ## _oid)

# define MIB_FUNCTION_WEAK(_oid, _type, _func, _arg)			\
static const struct mib_node __attribute__((used))			\
__mib_node_weak_ ## _oid = {						\
	.node_oid		= _oid,					\
	.node_type		= _type | kOIDFunction | kOIDWeak,	\
	.node_data.v_func	= {_func, _arg}				\
};									\
LINKER_SET_ENTRY(mib, __mib_node_weak_ ## _oid)


/*
 * Find a node in the MIB.
 */
union mib_value *mib_find_oid(uint32_t oid);

/*
 * Find a node of an expected type in the MIB and return its value.
 * Returns false if the OID is not present.
 *
 * This interface should only be used for optional nodes; if a node
 * is expected to exist, use the mib_get_* interfaces to avoid duplicating
 * error checking.
 */
bool _mib_find(uint32_t oid, uint32_t node_type, void *value);
bool mib_exists(uint32_t oid, void *value);
/*
 * Return the value of an OID expected to be in the MIB; the
 * MIB will panic on behalf of the caller if the node is not
 * found.
 */
void _mib_get(uint32_t oid, uint32_t node_type, void *value);

#define MIB_GET_TEMPLATE(stype, type, oid_type)	 	\
	static inline type				\
	mib_get_##stype(uint32_t oid) {			\
		type	ret;				\
		_mib_get(oid, oid_type, &ret);		\
		return ret;				\
	}						\
							\
	static inline type				\
	mib_get_##stype##_opt(uint32_t oid, type def) {	\
		type	ret = def;			\
		_mib_find(oid, oid_type, &ret);		\
		return ret;				\
	}

MIB_GET_TEMPLATE(u32, uint32_t, kOIDTypeUInt32)
MIB_GET_TEMPLATE(s32, int32_t, kOIDTypeInt32)
MIB_GET_TEMPLATE(u64, uint64_t, kOIDTypeUInt32)
MIB_GET_TEMPLATE(s64, int64_t, kOIDTypeInt64)
MIB_GET_TEMPLATE(addr, uintptr_t, kOIDTypeAddr)
MIB_GET_TEMPLATE(size, size_t, kOIDTypeSize)
MIB_GET_TEMPLATE(bool, bool, kOIDTypeBoolean)
MIB_GET_TEMPLATE(str, const char *, kOIDTypeString)
MIB_GET_TEMPLATE(ptr, const void *, kOIDTypeStruct)

#undef MIB_GET_TEMPLATE

/*
 * MIB node descriptions are automatically generated at build time.
 */
struct mib_description {
	uint32_t	oid;
	const char	*desc;
};

extern struct mib_description mib_desc[];

__END_DECLS

#endif /* _MIB_H */
