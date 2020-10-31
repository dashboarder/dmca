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
#include <target.h>
#include <stdlib.h>
#include <lib/devicetree.h>
#include <lib/image4_spi.h>
#include <debug.h>

/**
 * Stash away properties in a simple data structure while the image is being validated.
 *
 * The image4 wrapper extracts out properties before declaring an overall image valid/invalid,
 * and we also can load the kernelcache before the device tree.
 */
#define N_BOOL_PROPS 20
#define N_INT_PROPS  20
#define N_STRING_PROPS 10
#define MAX_IMG4_STRING_LEN 64
typedef struct {
	uint32_t tag;
	bool val;
} bool_prop_t;
typedef struct {
	uint32_t tag;
	uint64_t val;
} int_prop_t;
typedef struct {
	uint32_t tag;
	size_t length;
	uint8_t data[MAX_IMG4_STRING_LEN];
} string_prop_t;

typedef struct {
	bool_prop_t	bool_props[N_BOOL_PROPS];
	int_prop_t	int_props[N_INT_PROPS];
	string_prop_t	string_props[N_STRING_PROPS];

} image4_property_collection_t;

struct {
	bool is_valid;
	bool cb_encountered_error;
	image4_property_collection_t manifest_properties;
	image4_property_collection_t manifest_object_properties;
} image4_properties;

static bool image4_start_cb(uint32_t type)
{
	return (type == IMAGE_TYPE_KERNELCACHE || type == IMAGE_TYPE_KERNELCACHE_RESTORE);
}
static void image4_update_validity(bool is_valid)
{
	if (!is_valid) {
		bzero(&image4_properties, sizeof(image4_properties));
	}
	image4_properties.is_valid = is_valid;
}
static void image4_bool_cb(uint32_t tag, bool is_object_property, bool val)
{
	bool_prop_t* properties = NULL;
	if (is_object_property) {
		properties = image4_properties.manifest_object_properties.bool_props;
	} else {
		properties = image4_properties.manifest_properties.bool_props;
	}

	for(int i = 0; i < N_BOOL_PROPS; i++)
	{
		if(properties[i].tag == 0)
		{
			properties[i].tag = tag;
			properties[i].val = val;
			return;
		}
	}
	dprintf(DEBUG_CRITICAL, "Ran out of space storing boolean properties -- please tune N_BOOL_PROPS");
	image4_properties.cb_encountered_error = true;
}
static void image4_int_cb(uint32_t tag, bool is_object_property, uint64_t val)
{
	int_prop_t* properties = NULL;
	if (is_object_property) {
		properties = image4_properties.manifest_object_properties.int_props;
	} else {
		properties = image4_properties.manifest_properties.int_props;
	}

	for(int i = 0; i < N_INT_PROPS; i++)
	{
		if(properties[i].tag == 0)
		{
			properties[i].tag = tag;
			properties[i].val = val;
			return;
		}
	}
	dprintf(DEBUG_CRITICAL, "Ran out of space storing integer properties -- please tune N_INT_PROPS");
	image4_properties.cb_encountered_error = true;
}
static void image4_string_cb(uint32_t tag, bool is_object_property, uint8_t* data, size_t length)
{
	string_prop_t* properties = NULL;
	if (is_object_property) {
		properties = image4_properties.manifest_object_properties.string_props;
	} else {
		properties = image4_properties.manifest_properties.string_props;
	}

	for(int i = 0; i < N_STRING_PROPS; i++)
	{
		if(properties[i].tag == 0)
		{
			if (length > MAX_IMG4_STRING_LEN) {
				dprintf(DEBUG_CRITICAL, "image4_string_cb: Property length %lu exceeds max supported length, %u", length, MAX_IMG4_STRING_LEN);
				image4_properties.cb_encountered_error = true;
				return;
			}
			properties[i].tag = tag;
			memcpy(properties[i].data, data, length);
			properties[i].length = length;
			return;
		}
	}
	dprintf(DEBUG_CRITICAL, "Ran out of space storing string properties -- please tune N_STRING_PROPS");
	image4_properties.cb_encountered_error = true;
}
#define IMG4UNTAG(x)		(((x) >> 24) & 0xff),(((x) >> 16) & 0xff),(((x) >> 8) & 0xff),((x) & 0xff)

static int image4_copy_boolean_properties(DTNode* node, bool_prop_t* props)
{
	char entryName[50];
	char* propName;
	void* propData;
	uint32_t propSize;
	for(int i = 0; (i < N_BOOL_PROPS) && (props[i].tag != 0); i++) {
		snprintf(entryName, sizeof(entryName), "UnusedBooleanProperty%d", i);
		propName = entryName;
		if (!FindProperty(node, &propName, &propData, &propSize)) {
			dprintf(DEBUG_CRITICAL, "Ran out of boolean entries %s", propName);
			return -1;
		}
		snprintf(entryName, sizeof(entryName), "%c%c%c%c", IMG4UNTAG(props[i].tag));		
		strlcpy(propName, entryName, kPropNameLength);
		memcpy(propData, &props[i].val, sizeof(bool));
	}
	return 0;
}
static int image4_copy_integer_properties(DTNode* node, int_prop_t* props)
{
	char entryName[50];
	char* propName;
	void* propData;
	uint32_t propSize;
	for(int i = 0; (i < N_INT_PROPS) && (props[i].tag != 0); i++) {
		snprintf(entryName, sizeof(entryName), "UnusedIntegerProperty%d", i);
		propName = entryName;
		if (!FindProperty(node, &propName, &propData, &propSize)) {
			dprintf(DEBUG_CRITICAL, "Ran out of integer entries %s", propName);
			return -1;
		}
		snprintf(entryName, sizeof(entryName), "%c%c%c%c", IMG4UNTAG(props[i].tag));		
		strlcpy(propName, entryName, kPropNameLength);
		memcpy(propData, &props[i].val, sizeof(uint64_t));
	}
	return 0;
}
static int image4_copy_string_properties(DTNode* node, string_prop_t* props)
{
	char entryName[50];
	char* propName;
	void* propData;
	uint32_t propSize;
	for(int i = 0; (i < N_STRING_PROPS) && (props[i].tag != 0); i++) {
		snprintf(entryName, sizeof(entryName), "UnusedStringProperty%d", i);
		propName = entryName;
		if (!FindProperty(node, &propName, &propData, &propSize)) {
			dprintf(DEBUG_CRITICAL, "Ran out of string entries %s", propName);
			return -1;
		}
		if (propSize < props[i].length) {
			dprintf(DEBUG_CRITICAL, "DT property size too small to fit string property");
			return -2;
		}

		snprintf(entryName, sizeof(entryName), "%c%c%c%c", IMG4UNTAG(props[i].tag));		
		strlcpy(propName, entryName, kPropNameLength);
		memcpy(propData, &props[i].data, props[i].length);
	}
	return 0;
}

static bool initialized;

int target_init_boot_manifest(void)
{
	image4_register_property_capture_callbacks(image4_start_cb, 
  		image4_bool_cb, image4_int_cb, image4_string_cb, image4_update_validity);
	initialized=true;
	return 0;
}
int target_pass_boot_manifest(void)
{
	// You must call init before loading the image4 of interest.
	if (!initialized)
		panic("target_pass_boot_manifest() used without target_init_boot_manifest()");

	if (!image4_properties.is_valid || image4_properties.cb_encountered_error) {
		// Don't touch the DT unless the image is validated.
		return -1;
	}
	const char* ManifestPropertiesPath = "chosen/manifest-properties";
	const char* ManifestObjectPropertiesPath = "chosen/manifest-object-properties";
	DTNode* dtnode;
	if (FindNode(0, ManifestPropertiesPath, &dtnode)) {
		if (image4_copy_boolean_properties(dtnode, image4_properties.manifest_properties.bool_props))
			return -2;

		if (image4_copy_integer_properties(dtnode, image4_properties.manifest_properties.int_props))
			return -3;

		if (image4_copy_string_properties(dtnode, image4_properties.manifest_properties.string_props))
			return -4;
	} else {
		dprintf(DEBUG_CRITICAL, "Can't find manifest properties path %s -- Skipping DT patching\n", ManifestPropertiesPath);
		return -5;
	}
	if (FindNode(0, ManifestObjectPropertiesPath, &dtnode)) {
		if (image4_copy_boolean_properties(dtnode, image4_properties.manifest_object_properties.bool_props))
			return -6;
		if (image4_copy_integer_properties(dtnode, image4_properties.manifest_object_properties.int_props))
			return -7;
		if (image4_copy_string_properties(dtnode, image4_properties.manifest_object_properties.string_props))
			return -8;
	} else {
		dprintf(DEBUG_CRITICAL, "Can't find manifest object properties path %s -- Skipping DT patching\n", ManifestObjectPropertiesPath);
		return -9;
	}
	return 0;
}
