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
#include <debug.h>
#include <lib/devicetree.h>
#include <lib/syscfg.h>
#include <lib/env.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "devicetree_private.h"

/*
 * Device Tree Parsing Library
 *
 * This library implements the device tree serializer/updater/deserializer.
 * 
 * There should be more stuff here, but for now a quick note.
 *
 * The dt_size variable contains the size the device tree will have upon
 * serialization. Any modification to the deserialized device tree must
 * update this variable so that it continues to contain the size the device
 * tree will have on re-serialization.  This is because other modules need
 * to know the size of the device tree in order to properly lay out memory
 * (for example, in order to allocate a region of the kernel's memory map)
 */

static void free_node(dt_node_t *node);
void dt_free(dt_node_t *dt);

static size_t dt_size;
static dt_node_t *dt_root;
static void *dt_data;
static bool dt_sealed;

#define ROUND_SIZE(s) (((s) + 3) & ~3)

void dt_init(void)
{
	dt_size = 0;
	if (dt_root != NULL) {
		dt_free(dt_root);
		dt_root = NULL;
	}
	dt_data = NULL;
	dt_sealed = false;
}

size_t dt_get_size(void)
{
	return dt_size;
}

static void free_node(dt_node_t *node)
{
	if (node->props != NULL) {
		for (uint32_t i = 0; i < node->nprops; i++) {
			if (node->props[i].name_malloced)
				free(node->props[i].name);
			if (node->props[i].data_malloced)
				free(node->props[i].data);
		}
		free(node->props);
		node->props = NULL;
	}
	if (node->children != NULL) {
		for (uint32_t i = 0; i < node->nchildren; i++) {
			free_node(&node->children[i]);
		}
		free(node->children);
		node->children = NULL;
	}
}

/* Replaces placeholder values during deserialization. The placeholders are normal strings,
 * but the size field has bit 31 set to indicate it's a placeholder. If iBoot successfully
 * finds the requested data, it replaces the placeholder value with the requested data
 * (and sets bit 31 of the size back to 0). If iBoot doesn't find the requested data,
 * it removes the property from the device tree.
 *
 * Two kinds of placeholders are currently supported:
 *      syscfg/<SKey>       - looks up the key 'SKey' in syscfg
 *      syscfg/<SKey>/<len> - looks up the key 'SKey' in syscfg, and pads/truncates to specified
 *                          length (for compatibility with old-style devicetree placeholders)
 *      macaddr/<envname>   - looks up the mac address envname in the environment using env_get_ethaddr
 *      zeroes/<bytes>      - creates a property with bytes zero bytes
 *
 * Multiple placeholders can be specified, with options separated by commas. The first
 * placeholder found is used.
 */
static bool expand_placeholder_prop(dtprop_t *prop)
{
	uint32_t alloc_size;
	uint8_t ethaddr[6];
	uint8_t dummy = 0;
	void *data = NULL;
	uint32_t data_size = 0;
	uint32_t data_pad_size = 0;
	char *placeholder;
	char *next_placeholder;
	// make sure placeholder is null-terminated
	if (memchr(prop->data, '\0', prop->size) == NULL) {
		panic("devicetree placeholder property '%s' missing null terminator", prop->name);
	}

	placeholder = prop->data;
	next_placeholder = placeholder;

	while ((placeholder = strsep(&next_placeholder, ",")) != NULL) {
		if (memcmp(placeholder, "syscfg/", 7) == 0) {
			char *details = placeholder + 7;
			uint8_t *tag_str;
			char *len_str;
			uint32_t len;
			uint32_t tag;

			tag_str = (uint8_t *)details;

			len_str = details;
			strsep(&len_str, "/");

			if (strlen(details) != 4) {
				panic("dt: wrong length on placeholder %s", placeholder);
			}

			tag = tag_str[0] << 24 | tag_str[1] << 16 | tag_str[2] << 8 | tag_str[3];

			if (syscfg_find_tag(tag, &data, &data_size)) {
				// For compatibility with old-style devicetree placeholders, a length
				// can be specified to truncate/pad the syscfg data to
				if (len_str != NULL) {
					len = (uint32_t)strtoul(len_str, NULL, 0);
					if (len > data_size) {
						data_pad_size = len - data_size;
					} else {
						data_size = len;
					}
				}

				break;
			}
		} else if (memcmp(placeholder, "macaddr/", 8) == 0) {
			char *details = placeholder + 8;

			if (env_get_ethaddr(details, ethaddr) == 0) {
				data = ethaddr;
				data_size = sizeof(ethaddr);
				break;
			}
		} else if (memcmp(placeholder, "zeroes/", 7) == 0) {
			char *details = placeholder + 7;

			// Set data to a dummy pointer so that data != NULL but request
			// copying 0 bytes. The padding will all get zeroed, resulting
			// in a property value with the requested number of zeroes
			data = &dummy;
			data_size = 0;
			data_pad_size = (uint32_t)strtoul(details, NULL, 0);
		}

		placeholder = next_placeholder;
	}

	uint32_t original_size = ROUND_SIZE(prop->size);
	
	if (data != NULL) {
		alloc_size = ROUND_SIZE(data_size + data_pad_size);

		// Since this function is called during initial parsing, we know
		// that the storage for the placeholder was in the original devicetree
		// buffer and not malloced in a new buffer. We only need a new buffer
		// if the new data is longer than the placeholder
		if (alloc_size > original_size) {
			prop->data = malloc(alloc_size);
			prop->data_malloced = true;
		}
		memcpy(prop->data, data, data_size);
		// zero-fill any padding bytes
		memset((uint8_t *)prop->data + data_size, 0, alloc_size - data_size);

		prop->size = data_size + data_pad_size;

		return true;
	} else {
		return false;
	}
}

static bool parse_node(dt_node_t *node, uint8_t **addrptr, const uint8_t *endaddr)
{
	uint8_t *addr = *addrptr;

	if ((size_t)(endaddr - addr) < 2 * sizeof(uint32_t))
		goto error;

	memcpy(&node->nprops, addr, 4);
	memcpy(&node->nchildren, addr + 4, 4);
	addr += 2 * sizeof(uint32_t);
	dt_size += 2 * sizeof(uint32_t);

	// Sanity checks
	if (node->nprops > 1024) {
		dprintf(DEBUG_INFO, "dt: too many properties\n");
		goto error;
	}
	if (node->nchildren > 1024) {
		dprintf(DEBUG_INFO, "dt: too many children\n");
		goto error;
	}

	if (node->nprops > 0)
		node->props = calloc(sizeof(*node->props), node->nprops);
	if (node->nchildren > 0)
		node->children = calloc(sizeof(*node->children), node->nchildren);

	// Parse each property, recording its size, the address of the name,
	// and the address of its data.
	// We could find the "name" property and cache it here, but instead
	// we'll do that if we ever traverse this node looking for a node
	// by path.
	for(uint32_t i = 0; i < node->nprops; i++) {
		dtprop_t *prop;
		uint32_t size_and_flags;
		bool prop_removed = false;

		ASSERT(addr <= endaddr);

		prop = &node->props[i];

		if((size_t)(endaddr - addr) < kPropNameLength + sizeof(uint32_t)) {
			dprintf(DEBUG_INFO, "dt: not enough bytes for property name '%s'\n", (char *)addr);
			goto error;
		}
		if (memchr(addr, 0, kPropNameLength) == NULL) {
			dprintf(DEBUG_INFO, "dt: unterminated property name '%s'\n", (char *)addr);
			goto error;
		}

		prop->name = (char *)addr;

		memcpy(&size_and_flags, addr + kPropNameLength, sizeof(uint32_t));
		addr += kPropNameLength + sizeof(uint32_t);

		prop->size = size_and_flags & DT_PROP_SIZE_MASK;

		if ((size_t)(endaddr - addr) < ROUND_SIZE(prop->size)) {
			dprintf(DEBUG_INFO, "dt: not enough bytes for property '%s'\n", prop->name);
			goto error;
		}

		prop->data = addr;
		addr += ROUND_SIZE(prop->size);

		if (size_and_flags & DT_PROP_FLAG_PLACEHOLDER) {
			// if placeholder expansion fails, then recycle the property slot
			// we used for the placeholder
			prop_removed = !expand_placeholder_prop(prop);
			if (prop_removed) {
				node->nprops--;
				i--;
			}
		}

		// Account for the size of the property after any placeholder expansion has occurred
		if (!prop_removed)
			dt_size += kPropNameLength + sizeof(uint32_t) + ROUND_SIZE(prop->size);
	}

	for (uint32_t i = 0; i < node->nchildren; i++) {
		if (!parse_node(&node->children[i], &addr, endaddr))
			goto error;
	}

	*addrptr = addr;

	return true;
error:
	free_node(node);

	return false;
}

dt_node_t *dt_deserialize(void *dtaddr, size_t dtlength)
{
	uint8_t *startptr;
	uint8_t *endptr;
	dt_node_t *node;

	dt_init();

	node = calloc(sizeof(*node), 1);
	startptr = dtaddr;
	endptr = dtaddr + dtlength;

	if (!parse_node(node, &startptr, endptr)) {
		dt_size = 0;
		free(node);
		return NULL;
	}

	dt_root = node;

	return node;
}

void dt_free(dt_node_t *dt)
{
	free_node(dt);
	free(dt);
}

static void write_prop(char *name, void *data, uint32_t data_size, uint8_t **bufptr, uint8_t *end)
{
	uint8_t *buf = *bufptr;
	uint32_t prop_buf_size = ROUND_SIZE(data_size);
	uint32_t total_size = kPropNameLength + sizeof(uint32_t) + prop_buf_size;

	// no overflow
	if (prop_buf_size < data_size || total_size < prop_buf_size) {
		panic("devicetree integer overflow");
	}

	// Make sure there's room for the property name and value
	if ((size_t)(end - buf) < total_size) {
		panic("devicetree buffer overflow");
	}

	// Write property name, padding with zeroes to 32 bytes
	memset(buf, 0, kPropNameLength);
	strlcpy((char *)buf, name, kPropNameLength);
	buf += kPropNameLength;

	// Then property size
	memcpy(buf, &data_size, sizeof(uint32_t));
	buf += sizeof(uint32_t);

	// And property value, which won't be present if size is 0
	if (prop_buf_size != 0) {
		memcpy(buf, data, data_size);
		// Pad if needed to multiple of 4 bytes
		if (data_size != prop_buf_size)
			memset(buf + data_size, 0, prop_buf_size - data_size);
		buf += prop_buf_size;
	}

	RELEASE_ASSERT(buf <= end);

	// let caller know how far we wrote
	*bufptr = buf;
}

static void write_node(dt_node_t *node, uint8_t **bufptr, uint8_t *end)
{
	uint8_t *buf = *bufptr;

	RELEASE_ASSERT(*bufptr < end);

	// Make sure there's room for the node header
	if ((size_t)(end - buf) < 2 * sizeof(uint32_t))
		panic("devicetree buffer overflow");
	// Write the node header (number of properties, then number of children)
	memcpy(buf, &node->nprops, 4);
	memcpy(buf + 4, &node->nchildren, 4);
	buf += 8;

	// Write out each property
	for (uint32_t i = 0; i < node->nprops; i++) {
		dtprop_t *prop = &node->props[i];

		write_prop(prop->name, prop->data, prop->size, &buf, end);
	}

	// Write out all of this node's children
	for (uint32_t i = 0; i < node->nchildren; i++) {
		write_node(&node->children[i], &buf, end);
	}

	// Let the caller know how far we wrote
	*bufptr = buf;

	RELEASE_ASSERT(*bufptr <= end);
}

void dt_serialize(dt_node_t *dt, void *buffer, size_t bufferlen)
{
	uint8_t *startptr = buffer;
	uint8_t *endptr = startptr + bufferlen;

	if (dt == NULL)
		dt = dt_root;

	RELEASE_ASSERT(dt != NULL);

	if (bufferlen < dt_size)
		panic("devicetree length %zu > buffer size %zu", dt_size, bufferlen);

	write_node(dt, &startptr, endptr);

	// Make sure dt_size is accurate
	ASSERT(startptr == (uint8_t *)buffer + dt_size);
}

dt_node_t *dt_get_root(void)
{
	return dt_root;
}

bool dt_find_node(dt_node_t *root, const char *path, dt_node_t **node)
{
	uint32_t path_len;
	const char *rest;

	if (root == NULL) {
		if (dt_root == NULL)
			return false;
		root = dt_root;
	}

	if(path[0] == '\0') {
		*node = root;
		return true;
	}

	rest = strchr(path, '/');
	if (rest == NULL) {
		rest = "";
		path_len = strlen(path);
	} else {
		path_len = rest - path;
		rest++;
	}

	for (uint32_t i = 0; i < root->nchildren; i++) {
		dt_node_t *child = &root->children[i];
		if (child->name == NULL) {
			char *prop_name = "name";
			if (!dt_get_prop(child, &prop_name, &child->name, &child->name_size)) {
				dprintf(DEBUG_CRITICAL, "dt: found node with no name: %p\n", child);
				continue;
			}
		}
		if (child->name_size == path_len + 1 && memcmp(path, child->name, path_len) == 0) {
			return dt_find_node(child, rest, node);
		}
	}

	return false;
}

static bool find_prop(dt_node_t *node, const char *name, dtprop_t **prop_out)
{
	ASSERT(node != NULL);
	ASSERT(name != NULL);

	for (uint32_t i = 0; i < node->nprops; i++) {
		dtprop_t *prop = &node->props[i];

		if (strncmp(name, prop->name, kPropNameLength) == 0) {
			if (prop_out != NULL)
				*prop_out = prop;
			return true;
		}
	}

	return false;
}

bool dt_get_prop(dt_node_t *node, char **prop_name, void **prop_data, uint32_t *prop_size)
{
	dtprop_t *prop;

	ASSERT(node != NULL);
	ASSERT(prop_name != NULL && *prop_name != NULL);

	if(find_prop(node, *prop_name, &prop)) {
		*prop_name = prop->name;
		if (prop_size != NULL)
			*prop_size = prop->size;
		if (prop_data != NULL)
			*prop_data = prop->data;
		return true;
	} else {
		return false;
	}
}

// Like dt_find_prop, but with a less obnoxious interface on prop_name
bool dt_find_prop(dt_node_t *node, const char *prop_name, void **prop_data, uint32_t *prop_size)
{
	dtprop_t *prop;

	ASSERT(node != NULL);
	ASSERT(prop_name != NULL);

	if(find_prop(node, prop_name, &prop)) {
		if (prop_size != NULL)
			*prop_size = prop->size;
		if (prop_data != NULL)
			*prop_data = prop->data;
		return true;
	} else {
		return false;
	}
}

bool dt_has_prop(dt_node_t *node, const char *prop_name)
{
	return dt_find_prop(node, prop_name, NULL, NULL);
}

const char *dt_get_node_name(dt_node_t *node)
{
	if (node->name == NULL) {
		char *prop_name = "name";
		dt_get_prop(node, &prop_name, &node->name, &node->name_size);
	}

	return node->name;
}

static dtprop_t *add_prop(dt_node_t *node, const char *name)
{
	uint32_t nprops = node->nprops + 1;
	dtprop_t *prop;

	ASSERT(strlen(name) < kPropNameLength);

	node->props = realloc(node->props, nprops * sizeof(*node->props));

	prop = &node->props[nprops - 1];
	memset(prop, 0, sizeof(*prop));

	prop->name = calloc(kPropNameLength, 1);
	strlcpy(prop->name, name, kPropNameLength);
	prop->name_malloced = true;

	node->nprops = nprops;

	// The device tree will grow by the size of the property name and length fields
	// The caller of this function will account for the growth from the value
	dt_size += 4 + kPropNameLength;

	return prop;
}

void dt_set_prop(dt_node_t *node, const char *name, const void *data, uint32_t size)
{
	dtprop_t *prop;
	size_t cur_alloc_size;
	size_t new_alloc_size;

	ASSERT(node != NULL);
	ASSERT(name != NULL);
	ASSERT(size == 0 || data != NULL);
	ASSERT(!dt_sealed);

	if (!find_prop(node, name, &prop))
		prop = add_prop(node, name);

	// if we're changing the name, get rid of the cache
	if (prop->data == node->name) {
		node->name = NULL;
		node->name_size = 0;
	}

	// Data is always serialized with length padded to 4 bytes
	new_alloc_size = ROUND_SIZE(size);
	cur_alloc_size = ROUND_SIZE(prop->size);

	// zero-length properties are legal, but malloc(0) isn't
	// rest of this file handles prop.data == NULL and prop.size == 0 properly
	if (new_alloc_size > 0) {
		// Only re-allocate if needed. data_malloced will be false for
		// newly created properties because add_prop zeroes the struct
		// before returning it
		if (cur_alloc_size < new_alloc_size) {
			if (!prop->data_malloced)
				prop->data = malloc(new_alloc_size);
			else
				prop->data = realloc(prop->data, new_alloc_size);
			prop->data_malloced = true;
		}
		memcpy(prop->data, data, size);
		// zero-fill any padding bytes
		memset((uint8_t *)prop->data + size, 0, new_alloc_size - size);
	}

	prop->size = size;

	// add_prop accounted for the name and length fields, we just need
	// to fgure out the difference from the value. Since add_prop zeroes
	// out the struct, for new properties, cur_alloc_size will be 0
	dt_size += new_alloc_size - cur_alloc_size;
}

void dt_set_prop_32(dt_node_t *node, const char *name, uint32_t value)
{
	dt_set_prop(node, name, &value, sizeof(value));
}

void dt_set_prop_64(dt_node_t *node, const char *name, uint64_t value)
{
	dt_set_prop(node, name, &value, sizeof(value));
}

void dt_set_prop_addr(dt_node_t *node, const char *name, uintptr_t value)
{
	dt_set_prop(node, name, &value, sizeof(value));
}

void dt_set_prop_str(dt_node_t *node, const char *name, const char *str)
{
	dt_set_prop(node, name, str, strlen(str) + 1);
}

bool dt_remove_prop(dt_node_t *node, const char *name)
{
	ASSERT(node != NULL);
	ASSERT(name != NULL);
	ASSERT(!dt_sealed);

	for (uint32_t i = 0; i < node->nprops; i++) {
		dtprop_t *prop = &node->props[i];

		if (strncmp(name, prop->name, kPropNameLength) == 0) {
			// unlikely, but if we're removing the name, get rid of the cache
			if (prop->data == node->name) {
				node->name = NULL;
				node->name_size = 0;
			}

			dt_size -= ROUND_SIZE(prop->size) + 4 + kPropNameLength;

			memmove(prop, prop + 1, (node->nprops - i - 1) * sizeof(*prop));
			node->nprops--;

			return true;
		}
	}

	return false;
}

bool dt_rename_prop(dt_node_t *node, const char *name, const char *new_name)
{
	ASSERT(node != NULL);
	ASSERT(name != NULL);
	ASSERT(new_name != NULL);
	ASSERT(!dt_sealed);

	dtprop_t *prop;
	size_t new_name_len = strlen(new_name);

	ASSERT(new_name_len < kPropNameLength - 1);

	if (find_prop(node, name, &prop)) {
		// unlikely, but if we're renaming the name property, get rid of the cache
		if (prop->data == node->name) {
			node->name = NULL;
			node->name_size = 0;
		}

		memset(prop->name, 0, kPropNameLength);
		memcpy(prop->name, new_name, new_name_len);

		return true;
	} else {
		return false;
	}
}

void dt_seal(void)
{
	dt_sealed = true;
}
