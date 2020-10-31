/*
 * Test/debug code to mess with the arm7
 */

#include <debug.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <lib/env.h>
#include <lib/heap.h>
#include <platform/gpio.h>
#include <platform/memmap.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/arm7m.h>
#include <target/gpiodef.h>

#if WITH_MENU
#if DEBUG_BUILD

#define ARM7M_ALIGN		(1 << 12)
#define ARM7M_ALIGN_MASK	(ARM7M_ALIGN - 1)

int do_arm7_go(int argc, struct cmd_arg *args)
{
	addr_t	addr;

	addr = env_get_uint("loadaddr", DEFAULT_LOAD_ADDRESS);

	if (addr & ARM7M_ALIGN_MASK) {
		dprintf(DEBUG_CRITICAL, "load address not on a page boundary, cannot remap\n");
		return(-1);
	}
	dprintf(DEBUG_INFO, "configuring ARM7M for image @ 0x%08x\n", addr);

	/* clock on */
	clock_gate(CLK_ISP, true);
	clock_gate(CLK_MSVDX, true);
	clock_gate(CLK_ARM7, true);
	clock_reset_device(CLK_ARM7);
	
	/* turn the arm7m off and reset it */
	rARM7M_CTRL = 0;
	rARM7M_CTRL = ARM7M_CTRL_RESET;

	/* set up translation */
	rARM7M_AT_BASE = addr;
	rARM7M_AT_RANGE = 0x03ff0000;	/* give the ARM7 1MiB of DDR */

	/* start */
	rARM7M_CTRL = ARM7M_CTRL_ENABLE;

	return(0);
}

MENU_COMMAND_DEBUG(arm7_go, do_arm7_go, "start ARM7M with a downloaded image", NULL);

int do_arm7_stop(int argc, struct cmd_arg *args)
{

	/* turn the arm7m off and reset it */
	rARM7M_CTRL = 0;
	rARM7M_CTRL = ARM7M_CTRL_RESET;

	/* clock off */
	clock_gate(CLK_ARM7, false);
	
	return(0);
}

MENU_COMMAND_DEBUG(arm7_stop, do_arm7_stop, "stop the arm7", NULL);

#ifdef GPIO_ARM7_DEBUG

int do_arm7_jtag_select(int argc, struct cmd_arg *args)
{
#if (GPIO_ARM7_DEBUG_ARM7 >= 0)
	gpio_configure_out(GPIO_ARM7_DEBUG, GPIO_ARM7_DEBUG_ARM7);
#else
	gpio_configure_in(GPIO_ARM7_DEBUG);
#endif
	return(0);
}

MENU_COMMAND_DEVELOPMENT(arm7_jtag, do_arm7_jtag_select, "switch JTAG from Cortex to ARM7M", NULL);

int do_cortex_jtag_select(int argc, struct cmd_arg *args)
{
#if (GPIO_ARM7_DEBUG_CORTEX >= 0)
	gpio_configure_out(GPIO_ARM7_DEBUG, GPIO_ARM7_DEBUG_CORTEX);
#else
	gpio_configure_in(GPIO_ARM7_DEBUG);
#endif
	return(0);
}

MENU_COMMAND_DEVELOPMENT(cortex_jtag, do_cortex_jtag_select, "switch JTAG from ARM7M to Cortex", NULL);

#endif /* GPIO_ARM7_DEBUG */

#endif /* DEBUG_BUILD */
#endif /* WITH_MENU */
