/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#include <debug.h>
#include <list.h>
#include <lib/env.h>
#include <stdlib.h>
#include <string.h>
#include <sys/menu.h>

static struct list_node env_list = LIST_INITIAL_VALUE(env_list);

#define ENV_VAR_ENTRY_NAME_LEN 64

struct env_var_entry {
	struct list_node node;
	struct env_var var;
	char name[ENV_VAR_ENTRY_NAME_LEN];
};

static int str2ip(const char *str, uint32_t *ip);
static int str2mac(const char *str, uint8_t addr[6]);

#define kIBootHiddenEnvDomain "com.apple.System."

#if DEBUG_BUILD
static bool nvram_whitelist_override = false;
#else
static const bool nvram_whitelist_override = false;
#endif

static bool hide_key(const char *str)
{
	if (0 == strncmp(str, kIBootHiddenEnvDomain, sizeof(kIBootHiddenEnvDomain)-1))
		return true;
	else
		return false;
}

static struct env_var_entry *find_var_entry(const char *name)
{
	struct env_var_entry *entry;

	list_for_every_entry(&env_list, entry, struct env_var_entry, node) {
		if (!strcmp(entry->name, name))
			return entry;
	}
	return NULL;
}

const struct env_var *env_get_etc(const char *name)
{
	struct env_var_entry *entry;

	entry = find_var_entry(name);
	if (!entry)
		return NULL;

	if ((entry->var.flags & ENV_SHADOW) && !nvram_whitelist_override) {
		dprintf(DEBUG_CRITICAL, "\x1b[1;5;31mBlocked read\x1b[m from shadowed variable \"%s\"\n", name);
		dprintf(DEBUG_CRITICAL, "Disable NVRAM whitelist with DEBUG iBoot command envprot off\n");
		return NULL;
	}

	return &entry->var;
}

const char *env_get(const char *name)
{
	const struct env_var *var;

	var = env_get_etc(name);
	if (!var)
		return NULL;

	return var->str;
}

size_t env_get_uint(const char *name, size_t default_val)
{
	const struct env_var *var;

	var = env_get_etc(name);
	if (!var)
		return default_val;

	return var->u;
}

bool env_get_bool(const char *name, bool default_val)
{
	const struct env_var *var;

	var = env_get_etc(name);
	if (NULL == var)
		return default_val;

	if (!strcmp(var->str, "true"))
		return true;
	if (0 != var->u)
		return true;
	return false;
}

static int str2ip(const char *str, uint32_t *ip)
{
	char numstr[16];
	int num;
	int field = 0;
	const char *ptr;

	if (str == NULL)
		return -1;

	*ip = 0;

	ptr = str;
	for (field = 0; field < 4; field++) {
		char *dot = strchr(ptr, '.');

		if (dot) {
			strlcpy(numstr, ptr, __min((unsigned)(dot - ptr + 1), sizeof(numstr)));
		} else {
			strlcpy(numstr, ptr, sizeof(numstr));
		}

//		printf("numstr '%s'\n", numstr);

		num = strtol(numstr, NULL, 0);
		if (num < 0 || num > 255) {
			return -1;
		}
		*ip |= (num << (8 * field));

		if (dot)
			ptr = dot + 1;
		else
			break;
	}	

	return 0;
}

int env_get_ipaddr(const char *name, uint32_t *ip)
{
	const char *env;

	env = env_get(name);
	if (!env)
		return -1;

	if (str2ip(env, ip) < 0)
		return -1;

	return 0;
}

static int str2mac(const char *str, uint8_t addr[6])
{
	char numstr[16];
	int num;
	int field = 0;
	const char *ptr;
	char *endptr;

	if (str == NULL)
		return -1;

	addr[0] = addr[1] = addr[2] = addr[3] = addr[4] = addr[5] = 0;

	ptr = str;
	for (field = 0; field < 6; field++) {
		char *sep = strchr(ptr, ':');

		if (sep) {
			strlcpy(numstr, ptr, __min((unsigned)(sep - ptr + 1), sizeof(numstr)));
		} else {
			strlcpy(numstr, ptr, sizeof(numstr));
		}

//		printf("numstr '%s'\n", numstr);

		num = strtol(numstr, &endptr, 16);
		if (endptr == numstr) {
			dprintf(DEBUG_INFO, "malformed MAC address %s\n", str);
			return -1;
		}
		if (num < 0 || num > 255) {
			dprintf(DEBUG_INFO, "makformed MAC address %s\n", str);
			return -1;
		}
		addr[field] = num;

		if (sep)
			ptr = sep + 1;
		else
			break;
	}

	return 0;
}

int env_get_ethaddr(const char *name, uint8_t ethaddr[6])
{
	const char *env;

	env = env_get(name);
	if (!env)
		return -1;

	if (str2mac(env, ethaddr) < 0)
		return -1;

	return 0;
}

int env_set(const char *name, const char *val, uint32_t flags)
{
	struct env_var_entry *entry;

	if (flags & ENV_PERSISTENT) {
		if (env_blacklist_nvram(name))
			flags |= ENV_SHADOW;
	}

	/* see if it already exists */
	entry = find_var_entry(name);
	if (entry) {
		if ((flags & ENV_SHADOW) && !(entry->var.flags & ENV_SHADOW) && !nvram_whitelist_override) {
			dprintf(DEBUG_CRITICAL, "\x1b[1;5;31mBlocked shadowed write\x1b[m to variable \"%s\"\n", name);
			dprintf(DEBUG_CRITICAL, "Disable NVRAM whitelist with DEBUG iBoot command envprot off\n");
			// If the variable exists, is not on the whitelist for NVRAM variables,
			// and the value was generated internally (probably from the default environment),
			// refuse to modify the existing value.
			// This will result in the variable getting removed from nvram on saveenv, but
			// protects us from people removing things from the default environment.
			return -1;
		} else {
			// If the variable exists and either is on the whitelist or was not
			// generated internally, delete the old value
			env_unset(name);
		}
	}

	entry = malloc(sizeof(struct env_var_entry));

	strlcpy(entry->name, name, sizeof(entry->name));

	entry->var.str = strdup(val);
	if (entry->var.str == NULL) {
		free(entry);
		return -1;
	}

	entry->var.u = strtoul(entry->var.str, NULL, 0);

	/* save the flags */
	entry->var.flags = flags;

	list_add_tail(&env_list, &entry->node);

	return 0;
}

int env_set_uint(const char *name, size_t val, uint32_t flags)
{
	char numbuf[(sizeof(size_t) * 2) + 3];		// plus '3' to store "0x" and null terminator

	snprintf(numbuf, sizeof(numbuf), "0x%lx", val);

	return env_set(name, numbuf, flags);
}

int env_unset(const char *name)
{
	struct env_var_entry *entry;

	entry = find_var_entry(name);
	if (!entry)
		return 0;

	list_delete(&entry->node);

	free(entry->var.str);
	free(entry);

	return 1;
}

/* 
 * used by the nvram code to write the environment out to nvram.
 * serialize all of the persistent environment variables with a simple
 * <name>=<value>NULL...
 * strategy.
 */
size_t env_serialize(uint8_t *buf, size_t buf_len)
{
	size_t offset;
	struct env_var_entry *entry;

	offset = 0;
	list_for_every_entry(&env_list, entry, struct env_var_entry, node) {
		if (entry->var.flags & ENV_PERSISTENT) {
			/* print one var into the buffer, avoiding overflow */
			offset += snprintf((void *)(buf + offset), buf_len - offset, "%s=%s", entry->name, entry->var.str) + 1;

			/* effectively require there to always be at least one free byte */
			if (offset >= buf_len)
				return(0);
		}
	}

	return offset;
}

/* opposite of the above */
int env_unserialize(const uint8_t *buf, size_t buf_len)
{
	size_t offset;

#if 0
		dprintf(DEBUG_SPEW, "serialized environment vars:\n");
		dhexdump(DEBUG_SPEW, buf, buf_len);
#endif

	offset = 0;
	while (offset < buf_len) {
		char envname[64];
		char envval[256];
		uint32_t var_start, var_end;
		uint32_t val_start, val_end;

		if (buf[offset] == 0) {
			offset++;
			continue;
		}

		/* find the start and end of the variable name */
		var_start = offset;
		for (var_end = var_start; buf[var_end] != '=' && buf[var_end] != 0; var_end++)
			;
		if (buf[var_end] == 0) // malformed variable, no '='
			break;

		/* find the start and end of the value component */
		val_start = var_end + 1; /* skip the '=' */
		for (val_end = val_start; buf[val_end] != 0; val_end++)
			;

		/* copy them out */
		strlcpy(envname, (const char *)&buf[var_start], __min(var_end - var_start + 1, sizeof(envname)));
		strlcpy(envval, (const char *)&buf[val_start], __min(val_end - val_start + 1, sizeof(envval)));

		dprintf(DEBUG_SPEW, "'%s' = '%s'\n", envname, envval);

		/* load the environment */
		if (strlen(envname) > 0 && strlen(envval) > 0) {
			uint32_t flags = ENV_PERSISTENT;
			if (env_blacklist_nvram(envname))
				flags |= ENV_SHADOW;
			env_set(envname, envval, flags);
		}

		offset = val_end + 1; 
	}

	return 0;
}

/* for unit testing testing purposes only, remove every environment
 * variable
 */
void env_reset(void)
{
	struct env_var_entry *entry;
	struct env_var_entry *temp;

	list_for_every_entry_safe(&env_list, entry, temp, struct env_var_entry, node) {
		env_unset(entry->name);
	}
}


/* command prompt routines */
static void dump_var(const struct env_var_entry *entry)
{
	int i, binary = 0;
	uint32_t flags;

	/* filter out non-ascii strings in variable values */
	for (i = 0; i < ENV_VAR_ENTRY_NAME_LEN; i++) {
		if (entry->var.str[i] == 0)
			break;
		if (entry->var.str[i] < 0x20 || entry->var.str[i] > 0x7e)
			binary = 1;
	}

	flags = entry->var.flags;
	printf("%c %s = %s%s%s\n",
			(flags & ENV_SHADOW) ? 'S' : ((flags & ENV_PERSISTENT) ? 'P' : ' '),
			entry->name,
			binary ? "" : "\"",
			binary ? "<DATA>" : entry->var.str,
			binary ? "" : "\"");
}

int do_printenv(int argc, struct cmd_arg *args)
{
	struct env_var_entry *entry;

	if (argc >= 2) {
		/* display a single variable */
		entry = find_var_entry(args[1].str);
		if (entry) {
			if (!hide_key(entry->name))
				dump_var(entry);
		} else {
			printf("variable %s not set\n", args[1].str);
		}
	} else {
		/* display all variables */
		list_for_every_entry(&env_list, entry, struct env_var_entry, node) {
			if (hide_key(entry->name)) continue;
			dump_var(entry);
		}
	}

	return 0;
}

int do_getenv(int argc, struct cmd_arg *args)
{
	struct env_var_entry *entry;
	if(argc != 2)
	{
#if !RELEASE_BUILD
		printf("%s <var>\n", args[0].str);
#endif
		return -1;
	}

	entry = find_var_entry(args[1].str);
	if(!entry)
		return -1;
	if(hide_key(entry->name))
		return -1;
        if(env_blacklist(entry->name, false))
                return -1;
	return env_set("cmd-results", entry->var.str, 0);
}

int do_setenv(int argc, struct cmd_arg *args)
{
	if (argc < 2) {
#if !RELEASE_BUILD
		printf("not enough arguments.\n");
		printf("%s <var> [<value>] ...\n", args[0].str);
#endif
		return -1;
	}

	if (hide_key(args[1].str)) return -1;

        if (env_blacklist(args[1].str, true)) return -1;

	if (argc < 3) {
		env_unset(args[1].str);
	} else {
		size_t buf_size = 512;
		char *envbuf = malloc(buf_size);
		int i;

		/* reassemble a string from all the command line args */
		envbuf[0] = 0;
		for (i = 2; i < argc; i++) {
			strlcat(envbuf, args[i].str, buf_size);
			if (i != (argc - 1))
				strlcat(envbuf, " ", buf_size);
		}

		env_set(args[1].str, envbuf, ENV_PERSISTENT);

		free(envbuf);
	}

	return 0;
}

int do_clearenv(int argc, struct cmd_arg *args)
{
	struct env_var_entry *entry, *pentry;
	
	if (argc < 2) {
		printf("not enough arguments.\n");
		printf("Usage: It's a two step process. Run the following commands to clear\n");
		printf("\t\tclearenv <var>\n");
		printf("\t\tclearenv 1 \t {confirms the clear}\n");
		printf("ex: clearenv auto-boot boot-args\n");
		printf("    clearenv 1\n");
		return -1;
	}
	
	if (args[1].u == 0) {
		printf("Clear pending. Run command \"clearenv 1\" to confirm\n");
		return -1;
	}
	
	/* clear all persistent variables */
	while (1) {
		pentry = 0;
		list_for_every_entry(&env_list, entry, struct env_var_entry, node) {
			if (hide_key(entry->name)) continue;
			if (entry->var.flags & ENV_PERSISTENT) {
				pentry = entry;
				break;
			}
		}
		
		if (pentry == 0) break;
		
		env_unset(pentry->name);
	}
	
	return 0;
}

#if DEBUG_BUILD
int
do_envprot(int argc, struct cmd_arg *args)
{
	int result = 0;

	if (argc != 2) {
		result = -1;
	} else {
		if (strcmp(args[1].str, "off") == 0) {
			nvram_whitelist_override = true;
			printf("Disabled NVRAM whitelist\n");
		} else if (strcmp(args[1].str, "on") == 0) {
			nvram_whitelist_override = false;
			printf("Enabled NVRAM whitelist\n");
		} else {
			result = -1;
		}
	}
		
	if (result < 0)
		printf("Usage: envprot {on | off}\n");

	return result;
}
#endif
