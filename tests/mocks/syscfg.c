#include <list.h>
#include <stdlib.h>
#include <string.h>
#include <lib/syscfg.h>

struct mock_syscfg_value {
	struct list_node list_node;

	struct syscfgMemEntry entry;
};

struct list_node mock_syscfg_list = LIST_INITIAL_VALUE(mock_syscfg_list);

void mock_syscfg_add(uint32_t tag, void *data, uint32_t size)
{
	struct mock_syscfg_value *new_value;

	new_value = calloc(1, sizeof(*new_value));
	new_value->entry.seTag = tag;
	new_value->entry.seDataSize = size;
	// store the pointer itself in the array for inline data... dirty but it works
	memcpy(new_value->entry.seData, &data, sizeof(data));

	list_add_tail(&mock_syscfg_list, &new_value->list_node);
}

void mock_syscfg_reset(void)
{
	struct mock_syscfg_value *value;

	while ((value = list_remove_tail_type(&mock_syscfg_list, struct mock_syscfg_value, list_node)) != NULL) {
		free(value);
	}
}

bool syscfgFindByTag(uint32_t tag, struct syscfgMemEntry *entry)
{
	struct mock_syscfg_value *value;

	list_for_every_entry(&mock_syscfg_list, value, struct mock_syscfg_value, list_node) {
		if (value->entry.seTag == tag) {
			memcpy(entry, &value->entry, sizeof(*entry));
			return true;
		}
	}

	return false;
}

void *syscfgGetData(struct syscfgMemEntry *entry)
{
	void *ptr;

	memcpy(&ptr, entry->seData, sizeof(ptr));

	return ptr;
}

uint32_t syscfgGetSize(struct syscfgMemEntry *entry)
{
	return entry->seDataSize;
}

bool
syscfg_find_tag(uint32_t tag, void **data_out, uint32_t *size_out)
{
	static struct syscfgMemEntry result;

	if (syscfgFindByTag(tag, &result)) {
		if (data_out) {
			*data_out = syscfgGetData(&result);
		}
		if (size_out) {
			*size_out = syscfgGetSize(&result);
		}

		return true;
	} else {
		return false;
	}
}
