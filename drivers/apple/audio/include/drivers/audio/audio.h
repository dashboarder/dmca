
#ifndef __APPLE_AUDIO_H
#define __APPLE_AUDIO_H

#include <platform/soc/hwregbase.h>

#define rAE2_MCSVER		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e0000))
#define rAE2_MCSCSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e0004))
#define rAE2_MCSATB		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e0008))

#define rAE2_MIRQRR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1000))
#define rAE2_MIRQER		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1004))
#define rAE2_MIRQSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1008))
#define rAE2_MIRQCR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e100c))
#define rAE2_MFIQRR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1010))
#define rAE2_MFIQER		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1014))
#define rAE2_MFIQSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1018))
#define rAE2_MFIQCR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e101c))
#define rAE2_MSFISR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1020))
#define rAE2_MSFICR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001e1024))

#define rAE2_ACSCSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a0000))

#define rAE2_AIRQRR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1000))
#define rAE2_AIRQER		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1004))
#define rAE2_AIRQSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1008))
#define rAE2_AIRQCR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a100c))
#define rAE2_AFIQRR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1010))
#define rAE2_AFIQER		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1014))
#define rAE2_AFIQSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1018))
#define rAE2_AFIQCR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a101c))
#define rAE2_ASFISR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1020))
#define rAE2_ASFICR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a1024))

#define rAE2_ADECSR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a2000))
#define rAE2_ADECTR		(*(volatile u_int32_t *)(AUDIO_BASE_ADDR + 0x001a2004))

#define AE2_INT_DEC		0	// Decrementer
#define AE2_INT_DMACINTERR	1	// PL080 error
#define AE2_INT_DMACINTTC	2	// PL080 transaction completion
#define AE2_INT_SW0		4	// Software interrupt 0
#define AE2_INT_SW1		5	// Software interrupt 1
#define AE2_INT_SW2		6	// Software interrupt 2
#define AE2_INT_SW3		7	// Software interrupt 3
#define AE2_INT_BCZ0		8	// Ring buffer 0 byte count exhausted
#define AE2_INT_BCZ1		9	// Ring buffer 1 byte count exhausted
#define AE2_INT_BCZ2		10	// Ring buffer 2 byte count exhausted
#define AE2_INT_BCZ3		11	// Ring buffer 3 byte count exhausted
#define AE2_INT_I2S0		16	// I2S 0 interrupt
#define AE2_INT_I2S1		17	// I2S 1 interrupt
#define AE2_INT_I2S2		18	// I2S 2 interrupt
#define AE2_INT_I2S3		19	// I2S 3 interrupt
#define AE2_INT_SPDIF		20	// SPDIF interrupt
#define AE2_INT_DP0		21	// DisplayPort 0 interrupt
#define AE2_INT_DP1		22	// DisplayPort 1 interrupt
#define AE2_INT_MCA0		23	// MCA0 interrupt

#define AE2_MAX_INTS		24

#endif /* !__APPLE_AUDIO_H */
