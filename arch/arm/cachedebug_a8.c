
#include <debug.h>
#include <sys/menu.h>
#include <arch/arm/arm.h>

void arm_l1cache_dump(int dcache, unsigned int mva)
{
	register unsigned index = (mva >> 6) & 0x7f;
	unsigned way, addr, word, data0, data1;
	unsigned tag, i, parity, tmp;
	unsigned lookup;
	unsigned cr = arm_read_cr();

	arm_write_cr(cr & ~((1 << 12) | (1 << 2)));
	lookup = (mva != -1UL);
	for (way=0; way<4; way++) {
		if (!lookup) index = 0;
		do {
			addr = (way << 30) | (index << 6);
			if (dcache) {
				__asm__ volatile("mcr p15,0,%0,c15,c2,6" ::"r"(addr));
				__asm__ volatile("mrc p15,0,%0,c15,c0,0" :"=r"(tag));
			} else {
				__asm__ volatile("mcr p15,0,%0,c15,c3,6" ::"r"(addr));
				__asm__ volatile("mrc p15,0,%0,c15,c1,0" :"=r"(tag));
			}
			tag <<= 12;
			for (word = 0; word < 16; word++) {
				addr = (way << 30) | (index << 6) | (word << 2);
				if (dcache) {
					__asm__ volatile("mcr p15,0,%0,c15,c2,7" ::"r"(addr));
					__asm__ volatile("mrc p15,0,%0,c15,c0,0" :"=r"(data0));
					__asm__ volatile("mrc p15,0,%0,c15,c0,1" :"=r"(data1));
				} else {
					__asm__ volatile("mcr p15,0,%0,c15,c3,7" ::"r"(addr));
					__asm__ volatile("mrc p15,0,%0,c15,c1,0" :"=r"(data0));
					__asm__ volatile("mrc p15,0,%0,c15,c1,1" :"=r"(data1));
				}
				if (word == 0)
					printf("PA %08x: %c ", tag | ((index & 0x3f) << 6),
					       dcache && (data1&1) ? 'D':' ');
				parity = 0;
				tmp = data0;
				for (i=0; i<8; i++) {
					parity ^= tmp & 0x01010101;
					tmp >>= 1;
				}
				parity &= 0x01010101;
				parity = (parity >> 21) | (parity >> 14) | (parity >> 7) | parity;
				if ((parity ^ (data1 >> 2)) & 0xf)
					printf(" >%08x-%x<", data0, (data1>>2)&0xf);
				else
					printf(" %08x", data0);
			}
			printf("\n");
		} while (!lookup && (++index < 128));
	}
	arm_clean_dcache();
	arm_write_cr(cr);
}

void dump_l1caches(int disable)
{
	unsigned cr;

	cr = arm_read_cr();

	if (disable) {
		arm_write_cr(cr & ~((1<<12)|(1<<2)));
		arm_clean_dcache();
	}

	arm_l1cache_dump(0, -1);
	arm_l1cache_dump(1, -1);

	if (disable)
		arm_write_cr(cr);
}

#if defined(WITH_MENU) && WITH_MENU
static int do_l1dump(int argc, struct cmd_arg *args) { dump_l1caches(1); return 0; }
MENU_COMMAND_DEVELOPMENT(l1dump, do_l1dump, "dump L1 i/d contents", NULL);
#endif
