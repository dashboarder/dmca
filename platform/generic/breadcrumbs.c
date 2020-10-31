#include <lib/env.h>
#include <stdlib.h>
#include <lib/nvram.h>
#include <debug.h>
#include <platform.h>
#include <arch.h>


#define BREADCRUMB_BUFFER_SIZE 256	// This is arbitrarily sized, should be large enough for 
					// sensible options of key/value
#define BREADCRUMB_MIN_SIZE    3	// Smallest size of a breadcrumb

typedef struct {
	char* 		buffer;		// Storage for the breadcrumbs
	int		buffer_len; 	// Length of breadcrumb buffer
	int 		wrpos;		// Current write position
} breadcrumb_store_t;

static bool initialized = false;

breadcrumb_store_t breadcrumb_store;


static char* bc_cur_pos (void)
{
	ASSERT(breadcrumb_store.wrpos <= breadcrumb_store.buffer_len);
	return &breadcrumb_store.buffer[breadcrumb_store.wrpos];
}

static int bc_remaining (void)
{
	ASSERT(breadcrumb_store.wrpos <= breadcrumb_store.buffer_len);
	return breadcrumb_store.buffer_len - breadcrumb_store.wrpos;
}

void _platform_init_breadcrumbs_internal(void)
{
	if (!initialized) {
		breadcrumb_store.buffer = malloc(BREADCRUMB_BUFFER_SIZE);
		breadcrumb_store.wrpos = 0;
		breadcrumb_store.buffer_len = BREADCRUMB_BUFFER_SIZE;

		initialized = (breadcrumb_store.buffer != NULL);
		dprintf(DEBUG_INFO, "breadcrumb store initialized at %p\n", breadcrumb_store.buffer);
		platform_record_breadcrumb_marker("BOOT");
	}
}

void _platform_record_breadcrumb_internal(const char* key, const char* val)
{
	if (initialized && bc_remaining() > BREADCRUMB_MIN_SIZE) {
		int count = (int)snprintf(bc_cur_pos(), bc_remaining(), "%s=%s, ", key, val);

		if (count > 0)
			breadcrumb_store.wrpos += strlen(bc_cur_pos());
	}
}

void _platform_record_breadcrumb_marker_internal(const char* val)
{
	if (initialized && bc_remaining() > BREADCRUMB_MIN_SIZE) {
		int count = (int)snprintf(bc_cur_pos(), bc_remaining(), "<%s> ", val);

		if (count > 0)
			breadcrumb_store.wrpos += strlen(bc_cur_pos());
	}
}

void _platform_record_breadcrumb_int_internal(const char* key, int val)
{
	if (initialized && bc_remaining() > BREADCRUMB_MIN_SIZE) {
		int count = (int)snprintf(bc_cur_pos(), bc_remaining(), "%s=%d, ", key, val);

		if (count > 0)
			breadcrumb_store.wrpos += strlen(bc_cur_pos());
	}
}

void _platform_commit_breadcrumbs_internal(const char* envvar)
{
	if (initialized) {
		platform_record_breadcrumb_marker("DONE");
		// We expect this buffer to always be NULL terminated since it was built using
		// snprintf(), but just to be sure, make sure the end of the buffer is terminated.
		breadcrumb_store.buffer[breadcrumb_store.buffer_len - 1] = '\0';

		const char* existing_breadcrumbs = env_get(envvar);
		if (existing_breadcrumbs) {
			dprintf(DEBUG_INFO, "Existing breadcrumbs found: '%s'\n", existing_breadcrumbs);
			// If there's already iBoot breadcrumbs, we should preserve them chronologically
			// (earliest first). Since we need to prepend, we might as well swap it with
			// a new buffer.
			if (bc_remaining() < (int)strlen(existing_breadcrumbs)) {
				// Existing breadcrumbs would not fit. Drop the oldest breadcrumbs first
				existing_breadcrumbs += ((int)strlen(existing_breadcrumbs) - bc_remaining());
			}
			char* tmp_buf = malloc(BREADCRUMB_BUFFER_SIZE);
			if (tmp_buf) {
				int count = snprintf(tmp_buf, BREADCRUMB_BUFFER_SIZE, "%s%s", existing_breadcrumbs, breadcrumb_store.buffer);
				if (count >= 0 && count < BREADCRUMB_BUFFER_SIZE)
					breadcrumb_store.wrpos = count;
				else
					breadcrumb_store.wrpos = 0;

				free(breadcrumb_store.buffer);
				breadcrumb_store.buffer = tmp_buf;
			}
		}
		dprintf(DEBUG_INFO, "Committing breadcrumbs [used %d of %d bytes]: Setting '%s'='%s'\n", breadcrumb_store.wrpos, breadcrumb_store.buffer_len, envvar, breadcrumb_store.buffer);
		env_set(envvar, breadcrumb_store.buffer, ENV_PERSISTENT);
		nvram_save();
		initialized=false; // Stop recording breadcrumbs and make redundant commits a no-op.
	}
}