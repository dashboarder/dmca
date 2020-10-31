#ifndef __PROFILE_H
#define __PROFILE_H

#if WITH_PROFILE

#include <platform/memmap.h>
#include <sys/types.h>

struct profile_event {
	uint32_t time;
	addr_t pc;
	uint32_t data1;
	uint32_t data2;
};

struct profile_buffer {
	uint32_t magic;
	uint32_t count;
	uint32_t pc_bytes;
	uint32_t timer_ticks_per_sec;
	struct profile_event events[];
};

#define PROFILE_2(_data1, _data2) \
	do { \
		addr_t current_pc = 0; \
		__asm__ volatile ("L_%=: adr %0, L_%=" : "=r"(current_pc)); \
		profile(current_pc, (_data1), (_data2)); \
	} while(0)

#define PROFILE_INIT() profile_init()
#define PROFILE_HANDOFF() profile_handoff()

#else // !WITH_PROFILE

#define PROFILE_2(data1, data2) do {} while(0)
#define PROFILE_INIT() do {} while(0)
#define PROFILE_HANDOFF() do {} while(0)

#endif

#define PROFILE_1(data1)		PROFILE_2((data1), 0)
#define PROFILE()			PROFILE_2(0, 0)
#define PROFILE_ENTER(tag)		PROFILE_2(((tag) & 0xFFFFFF) | ('<' << 24), 0)
#define PROFILE_EXIT(tag)		PROFILE_2(((tag) & 0xFFFFFF) | ('>' << 24), 0)

void profile_init(void);
void profile(addr_t pc, uint32_t data1, uint32_t data2);
void profile_relocate(void);
void profile_handoff(void);

#endif
