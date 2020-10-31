#ifndef __DIALOG_DCOMMON_H
#define __DIALOG_DCOMMON_H

typedef UInt8 eventRegisters[kDIALOG_EVENT_COUNT];
typedef UInt8 statusRegisters[kDIALOG_STATUS_COUNT];
typedef UInt8 faultRegisters[kDIALOG_FAULTLOG_COUNT];

// Test whether any of the 'mask' bits is set in 'registers'
static inline bool REGISTER_TEST_MASK(const UInt8 *events, const UInt8 *mask, unsigned int count) {
    for (unsigned int i = 0 ; i < count; i++) {
	if ((events[i] & mask[i]) != 0) return true;
    }
    return false;
}

#define EVENT_FLAG_MAKE(byte,bit) ((UInt16)((byte)&0xff)<<8 | ((bit)&0xff))
#define EVENT_FLAG_GET_BYTE(flag) ((UInt16)flag>>8)
#define EVENT_FLAG_GET_BIT(flag) (flag&0xff)

/* return a UInt16 if mismatch, which should warn you */
#define EVENT_FLAG_GET_BIT_FOR_BYTE(flag,byte) \
	(((flag) & 0xff) | (EVENT_FLAG_GET_BYTE(flag)==(byte) ? 0 : 0xffff))

#define EVENT_REGISTERS_GET_BYTE(registers,flag) ((registers)[EVENT_FLAG_GET_BYTE(flag)])
#define EVENT_FLAG_TEST(registers,flag) ((EVENT_REGISTERS_GET_BYTE(registers,flag)&EVENT_FLAG_GET_BIT(flag)) != 0)
#define EVENT_REGISTER_TEST_MASK(registers,mask) REGISTER_TEST_MASK(registers,mask,kDIALOG_EVENT_COUNT)

#define STATUS_FLAG_MAKE EVENT_FLAG_MAKE
#define STATUS_FLAG_TEST EVENT_FLAG_TEST
#define STATUS_REGISTER_TEST_MASK(registers,mask) REGISTER_TEST_MASK(registers,mask,kDIALOG_STATUS_COUNT)

#define FAULTLOG_FLAG_MAKE EVENT_FLAG_MAKE
#define FAULTLOG_FLAG_TEST EVENT_FLAG_TEST
#define FAULTLOG_REGISTER_TEST_MASK(registers,mask) REGISTER_TEST_MASK(registers,mask,kDIALOG_FAULTLOG_COUNT)

enum {
    kDIALOG_NOTEXIST_MASK		= EVENT_FLAG_MAKE(0, 0)
};

struct ldo_params {
	UInt16 minv;
	UInt8 step;
	UInt8 maxs;
	UInt8 stepmask;
	UInt8 bypass;
	UInt16 ldoreg;
	UInt16 actreg;
	UInt8 actmask;
};

int pmu_uvwarn_config(int dev, uint32_t thresholdMV);
int pmu_set_data(int dev, uint16_t reg, uint8_t byte, bool do_confirm);
int pmu_get_data(int dev, uint16_t reg, uint8_t *byte);
void pmu_check_events(bool *powersupply_change_event, bool *button_event, bool *other_wake_event);
void pmu_read_events(eventRegisters data);

#endif
