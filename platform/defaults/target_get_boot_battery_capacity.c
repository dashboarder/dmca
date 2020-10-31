#if WITH_HW_POWER && WITH_TARGET_CONFIG
#include <limits.h>
#include <target/powerconfig.h>
#include <target.h>

#ifdef TARGET_BOOT_BATTERY_CAPACITY
int target_get_boot_battery_capacity(void)
{
	return TARGET_BOOT_BATTERY_CAPACITY;
}
#endif
#endif // WITH_TARGET_CONFIG
