#if WITH_HW_POWER && WITH_TARGET_CONFIG
#include <limits.h>
#include <debug.h>
#include <drivers/gasgauge.h>
#include <target/powerconfig.h>
#include <target.h>

int target_get_precharge_gg_flag_mask(void)
{
#ifdef TARGET_PRECHARGE_GG_FLAG_MASK
	return TARGET_PRECHARGE_GG_FLAG_MASK;
#else
	return kHDQRegFlagsMaskSOC1;
#endif
}

#endif // WITH_TARGET_CONFIG
