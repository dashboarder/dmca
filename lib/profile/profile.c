#include <lib/profile.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/timer.h>
#include <string.h>

#define MAX_PROFILE_ENTRIES ((PROFILE_BUF_SIZE - sizeof(struct profile_buffer)) / sizeof(struct profile_event))

#if PRODUCT_IBSS || PRODUCT_LLB
#if defined PROFILE_BUF_BASE
static struct profile_buffer *profile_buffer = (struct profile_buffer *)PROFILE_BUF_BASE;
#else
#define PROFILE_BUF_SIZE	(0x400)
union {
	uint8_t	buffer[PROFILE_BUF_SIZE];
	struct profile_buffer profile_buffer;
} profile_union;
static struct profile_buffer *profile_buffer = &profile_union.profile_buffer;
#endif
#else
#define PROFILE_BUF_SIZE	PANIC_SIZE
static struct profile_buffer *profile_buffer = (struct profile_buffer *)PANIC_BASE;
#endif

void profile_init(void)
{
#if PRODUCT_IBSS || PRODUCT_LLB
	profile_buffer->magic = 'BTRC';
	profile_buffer->count = 0;
	profile_buffer->pc_bytes = sizeof(addr_t);
	/* At startup this value isn't known. We'll fill it in at handoff
	   at which time it should definitely be known */
	profile_buffer->timer_ticks_per_sec = 0;
#endif
	PROFILE_1('INIT');
}

void profile(addr_t pc, uint32_t data1, uint32_t data2)
{
	struct profile_event *next_entry;
	if (profile_buffer->count < MAX_PROFILE_ENTRIES) {
		next_entry = &profile_buffer->events[profile_buffer->count++];
		next_entry->time = timer_get_ticks();
		next_entry->pc = pc;
		next_entry->data1 = data1;
		next_entry->data2 = data2;
	};
}

extern uint32_t invalidate_time;

void profile_handoff(void)
{
	PROFILE_ENTER('HOF');
	if ((void *)profile_buffer != (void *)PANIC_BASE) {
		size_t buffer_size;

		profile_buffer->timer_ticks_per_sec = timer_get_tick_rate();
		buffer_size = PROFILE_BUF_SIZE > PANIC_SIZE ? PANIC_SIZE : PROFILE_BUF_SIZE;

		bcopy(profile_buffer, (void *)PANIC_BASE, buffer_size);
		profile_buffer = (struct profile_buffer *)PANIC_BASE;
	}
	PROFILE_EXIT('HOF');
}
