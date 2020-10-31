#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <lib/devicetree.h>
#include <sys/menu.h>
#include "devicetree_private.h"

void print_indent(int level)
{
	for (int i = 0; i < level; i++)
		putchar(' ');
}

void print_property(struct dtprop *prop, bool verbose)
{
	uint32_t prop_size;
	bool truncated = false;

	printf("%s [0x%x]:", prop->name, prop->size);

	prop_size = prop->size;
	if (!verbose && prop_size > 16) {
		prop_size = 16;
		truncated = true;
	}

	for (uint32_t i = 0; i < prop_size; i++)
		printf(" %02x", ((uint8_t *)prop->data)[i]);

	if (truncated)
		printf(" ...");

	putchar('\n');
}

void dump_tree_int(dt_node_t *node, int indent, bool verbose)
{
	const char *node_name;

	node_name = dt_get_node_name(node);
	if (node_name == NULL)
		node_name = "<UNNAMED>";

	print_indent(indent);
	printf("%s {\n", node_name);
	indent += 4;

	for (unsigned i = 0; i < node->nprops; i++) {
		print_indent(indent);
		print_property(&node->props[i], verbose);
	}

	for (unsigned i = 0; i < node->nchildren; i++)
		dump_tree_int(&node->children[i], indent, verbose);

	indent -= 4;
	print_indent(indent);
	printf("}\n");
}

#if DEBUG_BUILD
void dt_dump(bool verbose)
{
	dump_tree_int(dt_get_root(), 0, verbose);
}
#endif

