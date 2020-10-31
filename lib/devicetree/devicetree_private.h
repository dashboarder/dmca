#ifndef DEVICETREE_PRIVATE_H
#define DEVICETREE_PRIVATE_H
#include <stdint.h>
#include <stdbool.h>

#define DT_PROP_FLAG_PLACEHOLDER	(1 << 31)
#define DT_PROP_FLAGS_MASK		(0xf0000000)
#define DT_PROP_SIZE_MASK		(~DT_PROP_FLAGS_MASK)

typedef struct dtprop {
	char *name;
	void *data;
	uint32_t flags;
	uint32_t size;
	bool name_malloced;
	bool data_malloced;
} dtprop_t;

struct dtnode {
	dtprop_t *props;
	dt_node_t *children;
	uint32_t nprops;
	uint32_t nchildren;
	// caches name property
	void *name;
	// caches size of name property
	uint32_t name_size;
};



#endif
