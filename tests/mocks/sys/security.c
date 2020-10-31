#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

bool security_allow_memory(const void *address, size_t length)
{
	return true;
}

bool security_allow_modes(uint32_t modes)
{
	return true;
}
