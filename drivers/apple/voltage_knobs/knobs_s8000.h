#ifndef __APPLE_KNOBS_S8000_H
#define __APPLE_KNOBS_S8000_H

// from Antigua PMU spec as iBoot is not touching them
// Check latest OTP settings for POR values
#define KNOB_PMU_BUCK3_POR      (0xE0)    // 1.8V
#define KNOB_PMU_BUCK4_POR      (0xA0)    // 1.1V
#define KNOB_PMU_BUCK5_POR      (0x50)    // 0.85V

#endif
