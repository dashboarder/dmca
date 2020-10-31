// *****************************************************************************
//
// File: H2fmi.c
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2008-2009 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "WMROAM.h"
#include <FIL.h>
#include <FILTypes.h>

#include "H2fmi_private.h"
#include "H2fmi.h"
#if WMR_BUILDING_IBOOT
#include "nandid.h"
#else
#include "NandSpec.h"
#endif

#include "H2fmi_ppn.h"

// =============================================================================
// external global variable definitions
// =============================================================================
extern UInt8 nand_whitening_key[16];


#if (defined (AND_COLLECT_FIL_STATISTICS) && AND_COLLECT_FIL_STATISTICS)
#define AND_FIL_STAT(x)    (x)
FILStatistics stFILStatistics;
#else
#define AND_FIL_STAT(x)
#endif


// =============================================================================
// private global variable definitions
// =============================================================================

UInt32* g_pScratchBuf = NULL;

h2fmi_t g_fmi0;
h2fmi_t g_fmi1;

static BOOL32 useWhitening = TRUE32;
static BOOL32 useWhiteningMetadata = FALSE32;

static h2fmi_virt_to_phys_map_t virtPhysMap[ NAND_MAX_CE_COUNT_TOTAL ];

static const UInt8 kPpnChipId[] = { 0x50, 0x50, 0x4E };


// =============================================================================
// private local variable definitions
// =============================================================================
static FILDeviceInfo stDeviceInfo;

static UInt8 nand_default_key[16] = { 0xf6, 0x5d, 0xae, 0x95,
                                      0x0e, 0x90, 0x6c, 0x42,
                                      0xb2, 0x54, 0xcc, 0x58,
                                      0xfc, 0x78, 0xee, 0xce };

static UInt64 fmi_current_lba_base = 0;
static UInt8* fmi_current_dest_ptr = 0;
static UInt32 fmi_current_num_blks = 0;

static UInt32 metaLookupTable[METADATA_LOOKUP_SIZE];
static UInt32 metaContent;

#if SUPPORT_PPN
static ppn_feature_entry_t _ppnFeatureList[] =
{
#if H2FMI_PPN_VERIFY_SET_FEATURES
    {
        PPN_VERSION_1_0_0 ,
        PPN_FEATURE__DRIVE_STRENGTH ,
        2 ,
        1 ,
    } ,
#endif // H2FMI_PPN_VERIFY_SET_FEATURES
};
#endif

// =============================================================================
// private implementation function declarations
// =============================================================================

static void h2fmi_setup_whitening(BOOL32 encrypt, h2fmi_t* fmi, UInt32* seeds);
static void h2fmi_setup_default_encryption(BOOL32 encrypt, void* destPtr, h2fmi_t* fmi);
static void h2fmi_choose_aes(BOOL32 enable, BOOL32 encrypt, void* destPtr, h2fmi_t* fmi, UInt32* seeds);
static void h2fmi_calc_default_iv(void* arg, UInt32 chunk_index, void* iv_buffer);
static UInt32 h2fmi_generate_meta_content(void);
static void h2fmi_generate_meta_table(void);
static void h2fmi_encrypt_metadata(UInt32 page, UInt8* pabMetaBuf);
static void h2fmi_decrypt_metadata(UInt32 page, UInt8* pabMetaBuf);
static BOOL32 h2fmi_get_nand_layout(void * buffer, UInt32 *size);
static BOOL32 h2fmiGetChipIdStruct(void * buffer, UInt32 *size, UInt8 addr);
static void h2fmiMapVirtualCEToBusAndEnable( UInt32 virtualCE, UInt32 bus, UInt32 enable);
static BOOL32 h2fmi_init_minimal(h2fmi_t* fmi, UInt32   interface);
static void h2fmi_set_initial_timings(h2fmi_t* fmi);
static void h2fmi_init_raw_state(h2fmi_t *fmi, const NandInfo* nandInfo, NandRequiredTimings *requiredTiming);
static Int32 _initRawNand(h2fmi_t **fmi_list, const UInt32 num_fmi, const NandChipId *id_list, NandFmcSettings *actual);
static void _init_whimory_state(h2fmi_t* fmi, const NandInfo *nandInfo, UInt32 numDataBus, UInt16 ceCount);
#if (defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)
static BOOL32 h2fmiGetPPNUID(PpnUIDReadRequest* ppnUIDRead, UInt32* pdwStructSize, BOOL32* setPattern, BOOL32* toBreak);
static BOOL32 h2fmiGetRAWUID(GenericReadRequest* genericRead, UInt32* pdwStructSize, BOOL32* setPattern, BOOL32* toBreak);
#endif //(defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)
#if SUPPORT_PPN
static Int32 _initPPN(h2fmi_t **fmi_list, const UInt32 num_fmi, const NandChipId *id_list, NandFmcSettings *actual, UInt32 ppn_version);
static BOOL32 h2fmiPpnFirmwareIsBlacklisted(const UInt8 *mfg_id, const UInt8 *fw_version);
static void checkForWorkarounds(UInt8* mfg_id, h2fmi_t **fmi_list, const UInt32 num_fmi);
static void setPpnFeatures(h2fmi_t **fmiList, const UInt32 fmiCount);
#endif

// =============================================================================
// Nand info macros
// =============================================================================

#define _GetPagesPerBlock()         (stDeviceInfo.wPagesPerBlock)
#define _SetPagesPerBlock(_x)       do { stDeviceInfo.wPagesPerBlock = _x; } while (0)
#define _GetBlocksPerCS()           (stDeviceInfo.wBlocksPerCS)
#define _SetBlocksPerCS(_x)         do { stDeviceInfo.wBlocksPerCS = _x; } while (0)
#define _GetPagesPerCS()            (_GetPagesPerBlock() * _GetBlocksPerCS())
#define _GetNumOfCS()               (stDeviceInfo.wNumOfCS)
#define _SetNumOfCS(_x)             do { stDeviceInfo.wNumOfCS = _x; } while (0)
#define _GetDiesPerCS()             (stDeviceInfo.wDiesPerCS)
#define _SetDiesPerCS(_x)           do { stDeviceInfo.wDiesPerCS = _x; } while (0)
#define _GetBanksPerCS()            (stDeviceInfo.wBanksPerCS)
#define _SetBanksPerCS(_x)          do { stDeviceInfo.wBanksPerCS = _x; } while (0)
#define _GetNumOfBanks()            (_GetNumOfCS() * _GetBanksPerCS())
#define _GetBytesPerPageSpare()     (stDeviceInfo.wSpareBytesPerPage)
#define _SetBytesPerPageSpare(_x)   do { stDeviceInfo.wSpareBytesPerPage = _x; } while (0)
#define _GetSectorsPerPage()        (stDeviceInfo.wSectorsPerPage)
#define _SetSectorsPerPage(_x)      do { stDeviceInfo.wSectorsPerPage = _x; } while (0)
#define _GetBytesPerPageMain()      (_GetSectorsPerPage() * H2FMI_BYTES_PER_SECTOR)
#define _GetPagesPerSuBlk()         (_GetNumOfBanks() * _GetPagesPerBlock())
#define _GetInitialBadType()        (stDeviceInfo.checkInitialBadType)
#define _SetInitialBadType(_x)      do { stDeviceInfo.checkInitialBadType = _x; } while (0)
#define _GetECCThresholdBits()      (stDeviceInfo.bECCThresholdBits)
#define _SetECCThresholdBits(_x)    do { stDeviceInfo.bECCThresholdBits = _x; } while (0)
#define _GetECCCorrectableBits()    (stDeviceInfo.bECCCorrectableBits)
#define _SetECCCorrectableBits(_x)  do { stDeviceInfo.bECCCorrectableBits = _x; } while (0)
#define _GetVendorSpecificType()    (stDeviceInfo.vendorSpecificType)
#define _SetVendorSpecificType(_x)  do { stDeviceInfo.vendorSpecificType = _x; } while (0)
#define DUMP_VAR(variable)          _WMR_PRINT("%s : %u\n", #variable, (UInt32)(variable))

// =============================================================================
// public interface function definitions
// =============================================================================
typedef struct
{
    UInt8 mfg_id[NAND_DEV_PARAM_LEN_PPN];
    UInt8 fw_version[NAND_DEV_PARAM_LEN_PPN];
}ControllerInfo;

struct {
    h2fmi_chipid_t chipId;
    ControllerInfo contInfo;
}gConfigInfo;

Int32 h2fmiInit(void)
{
    Int32 nRet = FIL_SUCCESS;
    Int32 tmpRet;
    BOOL32 have_fmi0;
    BOOL32 have_fmi1;
    h2fmi_chipid_t* fmi0ChipIds;
    h2fmi_chipid_t* fmi1ChipIds;
    UInt16 wIdx;
    UInt16 numDataBus;
    UInt16 ceCount;
    BOOL32 is_ppn;
    h2fmi_t *fmi_list[H2FMI_MAX_NUM_BUS] = {0};
    UInt32 fmi_idx;
    NandFmcSettings actual;


    WMR_PRINT(LOG, "[FIL:LOG] h2fmiInit()\n");

    // initialize scratch space
    if (g_pScratchBuf == NULL)
    {
        UInt32 bufSize = WMR_MAX(NAND_MAX_CE_COUNT_TOTAL * sizeof(h2fmi_chipid_t) * H2FMI_MAX_NUM_BUS,
                                 H2FMI_BYTES_PER_SECTOR);
        bufSize = WMR_MAX(bufSize, PPN_MAX_PAGES * H2FMI_MAX_META_PER_LBA * H2FMI_MAX_LBA_PER_PAGE);
        g_pScratchBuf = WMR_MALLOC(bufSize);
        if (g_pScratchBuf == NULL)
        {
            nRet = FIL_CRITICAL_ERROR;
            return nRet;
        }
    }

    fmi0ChipIds = (h2fmi_chipid_t*)g_pScratchBuf;
    fmi1ChipIds = fmi0ChipIds + NAND_MAX_CE_COUNT_TOTAL;

    // perform minimum initialization on specified FMI interface
    h2fmi_init_minimal(&g_fmi0, 0);
    h2fmi_init_minimal(&g_fmi1, 1);

// Only for H5X - // <rdar://problem/11677742> Software workaround for Samsung reset and bad chip ID isssue
#if FMI_VERSION >= 5
    {
        have_fmi0 = h2fmi_nand_reset_all(&g_fmi0);
        have_fmi1 = h2fmi_nand_reset_all(&g_fmi1);

        if(have_fmi0)
        {
            have_fmi0 = h2fmi_nand_read_id_all(&g_fmi0, fmi0ChipIds, CHIPID_ADDR);
        }
        if(have_fmi1)
        {
            have_fmi1 = h2fmi_nand_read_id_all(&g_fmi1, fmi1ChipIds, CHIPID_ADDR);
        }
    }
#else
    {
        have_fmi0 = h2fmi_reset_and_read_chipids(&g_fmi0, fmi0ChipIds, CHIPID_ADDR);
        have_fmi1 = h2fmi_reset_and_read_chipids(&g_fmi1, fmi1ChipIds, CHIPID_ADDR);
    }
#endif // FMI_VERSION >= 5

    WMR_MEMCPY(&(gConfigInfo.chipId), &fmi0ChipIds[0], sizeof(h2fmi_chipid_t));

    // record chip IDs in the devicetree if we fail
    reportDbgChipIds(g_pScratchBuf, NAND_MAX_CE_COUNT_TOTAL * sizeof(h2fmi_chipid_t) * H2FMI_MAX_NUM_BUS);

    // fail if any one of the FMI interfaces could not be initialized
    if (!have_fmi0 || !have_fmi1)
    {
        nRet = FIL_CRITICAL_ERROR;
        return nRet;
    }

    if (WMR_MEMCMP(&fmi0ChipIds[0], kPpnChipId, sizeof(kPpnChipId)) == 0)
    {
        WMR_PRINT(INIT, "[FIL] PPN Device Detected\n");
#if !SUPPORT_PPN
        nRet = FIL_CRITICAL_ERROR;
        return nRet;
#else //!SUPPORT_PPN
        is_ppn = TRUE32;
#endif //!SUPPORT_PPN
    }
    else
    {
        is_ppn = FALSE32;
    }

    
    h2fmi_build_ce_bitmask(&g_fmi0, fmi0ChipIds, &fmi0ChipIds[0], CHIPID_ADDR);
    h2fmi_build_ce_bitmask(&g_fmi1, fmi1ChipIds, &fmi0ChipIds[0], CHIPID_ADDR);

    // Add FMI1 chip IDs to FMI0
    for (wIdx = 0; wIdx < H2FMI_MAX_CE_TOTAL; ++wIdx)
    {
        if (g_fmi1.valid_ces & (1 << wIdx))
        {
            WMR_MEMCPY(&fmi0ChipIds[wIdx], &fmi1ChipIds[wIdx], sizeof(h2fmi_chipid_t));
        }
        else if (!(g_fmi0.valid_ces & (1 << wIdx)))
        {
            WMR_MEMSET(&fmi0ChipIds[wIdx], 0x0, sizeof(h2fmi_chipid_t));
        }
    }

    ceCount = g_fmi0.num_of_ce + g_fmi1.num_of_ce;
    numDataBus = 0;
    if (g_fmi0.num_of_ce)
    {
        fmi_list[numDataBus] = &g_fmi0;
        numDataBus += 1;
    }
    if (g_fmi1.num_of_ce)
    {
        fmi_list[numDataBus] = &g_fmi1;
        numDataBus += 1;
    }

    // We need to initialize the CE map on errors in case the PPN firmware was
    // horked.  We can't update the firmware properly without the CE map.
    tmpRet = h2fmiInitVirtToPhysMap(numDataBus, ceCount);

#if SUPPORT_PPN
    if (is_ppn)
    {
        UInt32 ppn_version = ((fmi0ChipIds[0][3] << 16) |
                              (fmi0ChipIds[0][4] << 8) |
                              (fmi0ChipIds[0][5] << 0));

        if (PPN_VERSION_BAD_FW == ppn_version)
        {
            // This device has missing or corrupted firmware: build the virtual CE map
            // so we can update the firmware properly, but don't bring up any more of
            // the stack.  All it'll do is GEB from here.
            WMR_PRINT(ERROR, "PPN device has missing or corrupted firmware\n");
            nRet = FIL_CRITICAL_ERROR;
        }
        else if (PPN_VERSION_ERROR == ppn_version)
        {
            WMR_PRINT(ERROR, "PPN device reported an error PPN version 0x%06lx\n", ppn_version);
            nRet = FIL_CRITICAL_ERROR;
        }
        else if (PPN_VERSION_UNSUPPORTED <= ppn_version)
        {
            WMR_PRINT(ERROR, "PPN device reported unsupported PPN version 0x%06lx\n", ppn_version);
            nRet = FIL_CRITICAL_ERROR;
        }
        else
        {
            const NandChipId *id_list = (const NandChipId*)fmi0ChipIds;

#if SUPPORT_TOGGLE_NAND
            if (PPN_VERSION_1_5_0 <= ppn_version)
            {
                if (targetSupportsToggle())
                {
                    for(fmi_idx = 0; fmi_idx < numDataBus; ++fmi_idx)
                    {
                        h2fmi_t *fmi = fmi_list[fmi_idx];
                        fmi->is_toggle_system = TRUE32;
                        fmi->useDiffDQS = targetSupportsDiffDQS();
                        fmi->useDiffRE = targetSupportsDiffRE();
                        fmi->useVREF = targetSupportsVREF();
                    }
                    setToggleMode();
                    WMR_PRINT(INIT, "[FIL] Using DDR mode.\n");
                }
                else
                {
                    WMR_PRINT(INIT, "[FIL] Using SDR mode.\n");
                }
            }

#endif // SUPPORT_TOGGLE_NAND

            nRet = _initPPN(fmi_list, numDataBus, id_list, &actual, ppn_version);
        }
    }
    else
#endif // SUPPORT_PPN
    {
        const NandChipId *id_list = (const NandChipId*)fmi0ChipIds;
        nRet = _initRawNand(fmi_list, numDataBus, id_list, &actual);
    }

    if (FIL_SUCCESS == nRet)
    {
        // Setup the full-speed bus timings that we will use
        for(fmi_idx = 0; fmi_idx < numDataBus; ++fmi_idx)
        {
            h2fmi_t *fmi = fmi_list[fmi_idx];

            fmi->if_ctrl = (FMC_IF_CTRL__DCCYCLE(actual.read_dccycle_clocks) |
                            FMC_IF_CTRL__REB_SETUP(actual.read_setup_clocks) |
                            FMC_IF_CTRL__REB_HOLD(actual.read_hold_clocks) |
                            FMC_IF_CTRL__WEB_SETUP(actual.write_setup_clocks) |
                            FMC_IF_CTRL__WEB_HOLD(actual.write_hold_clocks));
            fmi->if_ctrl_normal = fmi->if_ctrl;

#if SUPPORT_TOGGLE_NAND
            if (fmi->is_toggle_system)
            {
#if SUPPORT_NAND_DLL
                UInt32 dllLock;

                if (!h2fmiTrainDLL(&dllLock))
                {
                    nRet = FIL_CRITICAL_ERROR;
                    fmi->dqs_ctrl = FMC_DQS_TIMING_CTRL__DEFAULT_VAL;
                }
                else
                {
                    fmi->dqs_ctrl = FMC_DQS_TIMING_CTRL__USE_DLL |
                                    FMC_DQS_TIMING_CTRL__DELTAV_SUSPENDS_READS;
                }
#else
                fmi->dqs_ctrl = FMC_DQS_TIMING_CTRL__DEFAULT_VAL;
#endif
                fmi->timing_ctrl_1 = (FMC_TOGGLE_CTRL_1_DDR_RD_PRE_TIME(actual.read_pre_clocks) |
                                      FMC_TOGGLE_CTRL_1_DDR_RD_POST_TIME(actual.read_post_clocks) |
                                      FMC_TOGGLE_CTRL_1_DDR_WR_PRE_TIME(actual.write_pre_clocks) |
                                      FMC_TOGGLE_CTRL_1_DDR_WR_POST_TIME(actual.write_post_clocks));
                fmi->timing_ctrl_2 = (FMC_TOGGLE_CTRL_2_CE_SETUP_TIME(actual.ce_setup_clocks) |
                                      FMC_TOGGLE_CTRL_2_CE_HOLD_TIME(actual.ce_hold_clocks) |
                                      FMC_TOGGLE_CTRL_2_NAND_TIMING_ADL(actual.adl_clocks) |
                                      FMC_TOGGLE_CTRL_2_NAND_TIMING_WHR(actual.whr_clocks));
                fmi->toggle_if_ctrl = (FMC_IF_CTRL__DCCYCLE(0) |
                                       FMC_IF_CTRL__REB_SETUP(actual.ddr_half_cycle_clocks) |
                                       FMC_IF_CTRL__REB_HOLD(0) |
                                       FMC_IF_CTRL__WEB_SETUP(actual.write_setup_clocks) |
                                       FMC_IF_CTRL__WEB_HOLD(actual.write_hold_clocks));

                WMR_PRINT(ALWAYS, "FMC_DQS_TIMING_CTRL = 0x%x\n", fmi->dqs_ctrl);
                WMR_PRINT(ALWAYS, "FMC_TOGGLE_CTRL_1 = 0x%x\n", fmi->timing_ctrl_1);
                WMR_PRINT(ALWAYS, "FMC_TOGGLE_CTRL_2 = 0x%x\n", fmi->timing_ctrl_2);
                WMR_PRINT(ALWAYS, "FMC_IF_CTRL = 0x%x\n", fmi->if_ctrl);
                WMR_PRINT(ALWAYS, "FMC_IF_CTRL [DDR] = 0x%x\n", fmi->toggle_if_ctrl);
            }
#endif // SUPPORT_TOGGLE_NAND

            restoreTimingRegs(fmi);
        }

#if SUPPORT_TOGGLE_NAND
        if (g_fmi0.is_toggle_system)
        {
            reportToggleModeFMCTimingValues(actual.ddr_half_cycle_clocks,
                                            actual.ce_setup_clocks,
                                            actual.ce_hold_clocks,
                                            actual.adl_clocks,
                                            actual.whr_clocks,
                                            actual.read_pre_clocks,
                                            actual.read_post_clocks,
                                            actual.write_pre_clocks,
                                            actual.write_post_clocks,
                                            g_fmi0.dqs_ctrl);
        }
#endif        
        // Tell the target OS what values to use
        reportFMCTimingValues(actual.read_setup_clocks,
                              actual.read_hold_clocks,
                              actual.read_dccycle_clocks,
                              actual.write_setup_clocks,
                              actual.write_hold_clocks);
    }

    nRet = (FIL_SUCCESS == nRet ? tmpRet : nRet);
    h2fmi_generate_meta_table();

    if (FIL_SUCCESS == nRet)
    {
        // no need to record chip IDs, and the scratch buffer will be reused
        reportDbgChipIds(NULL, 0);
    }

    return nRet;
}

#if SUPPORT_NAND_DLL
BOOL32 h2fmiTrainDLL(UInt32 *lock)
{
    UInt64 startTicks;
    UInt64 timeoutTicks = H2FMI_DEFAULT_TIMEOUT_MICROS * WMR_GET_TICKS_PER_US();
    UInt32 status;
    BOOL32 ret = TRUE32;

    WMR_DLL_CLOCK_GATE(TRUE32);

    //<rdar://problem/9934708> Need to implement SW workaround for H5P DLL lock status bug
    //Due to the above bug, we need to wait till locked bit clears
    startTicks = WMR_CLOCK_TICKS();
    do
    {
        if (WMR_HAS_TIME_ELAPSED_TICKS(startTicks, timeoutTicks))
        {
            WMR_PRINT(ERROR, "Timed out waiting for DLL LOCKED bit to clear!\n");
            ret = FALSE32;
            break;
        }
        else
        {
            // There is a bug in H5P, whereby we have to read timeout register before we can read the status register
            h2fmi_dll_rd(NAND_DLL_TIMEOUT_DELAY);
        }
        status = h2fmi_dll_rd(NAND_DLL_STATUS);
    }while(NAND_DLL_STATUS__DLL_LOCKED_GET(status));

    if (ret)
    {
        UInt32 nand_dll_control = NAND_DLL_CONTROL__REFERENCE(NAND_DLL_CONTROL__REFERENCE_DEFAULT) |
                                  NAND_DLL_CONTROL__STEP_SIZE(NAND_DLL_CONTROL__STEP_SIZE_DEFAULT) |
                                  NAND_DLL_CONTROL__START_POINT(NAND_DLL_CONTROL__START_POINT_DEFAULT) |
                                  NAND_DLL_CONTROL__HALF |
                                  NAND_DLL_CONTROL__VOLTAGE_SHIFT_START |
                                  NAND_DLL_CONTROL__SOFTWARE_START |
                                  NAND_DLL_CONTROL__ENABLE;

        // start training
        h2fmi_dll_wr(NAND_DLL_CONTROL, nand_dll_control);

        WMR_SLEEP_US(NAND_DLL_TRAIN_TIME_US);

        // There is a bug in H5P, whereby we have to read timeout register before we can read the status register
        (void)h2fmi_dll_rd(NAND_DLL_TIMEOUT_DELAY);
        status = h2fmi_dll_rd(NAND_DLL_STATUS);

        if (ret)
        {
            *lock = NAND_DLL_STATUS__LOCK_VALUE_GET(status);
      
            // The lock value should never be less than the start point. It will also help
            // us in detecting <rdar://problem/9665811> FMI Master DLL unstable at low voltage
            if (*lock < NAND_DLL_CONTROL__START_POINT_DEFAULT)
            {
                WMR_PRINT(ERROR, "DLL lock < start point!\n");
                ret = FALSE32;
            }
        }
    }

    return ret;
}
#endif

void h2fmiPrintConfig(void)
{
    h2fmi_chipid_t *id = &(gConfigInfo.chipId);
    h2fmi_t* fmi;
    UInt32 ce;

    for (ce = 0 ; ce < H2FMI_MAX_CE_TOTAL ; ce++)
    {
        if ( 0 != ((1UL << ce) & g_fmi0.valid_ces))
        {
            fmi = &g_fmi0;
        }
        else if ( 0 != ((1UL << ce) & g_fmi1.valid_ces))
        {
            fmi = &g_fmi1;
        }
        else
        {
            continue;
        }

        WMR_PRINT(INIT, "Chip ID %02X %02X %02X %02X %02X %02X on FMI%d:CE%d\n",
                  (*id)[0], (*id)[1], (*id)[2], (*id)[3], (*id)[4], (*id)[5],
                  fmi->bus_id,
                  ce);
    }

#if SUPPORT_PPN

    if (g_fmi0.is_ppn)
    {
        UInt8 *mfgId = gConfigInfo.contInfo.mfg_id;
        UInt8 *fwVer = gConfigInfo.contInfo.fw_version;
        WMR_PRINT(INIT, "PPN Manufacturer ID: %02X %02X %02X %02X %02X %02X\n",
               mfgId[0], mfgId[1], mfgId[2], mfgId[3], mfgId[4], mfgId[5]);

        WMR_PRINT(INIT, "PPN Controller Version: %16.16s\n", fwVer);
    }

#endif //SUPPORT_PPN

}

void 
_init_whimory_state(h2fmi_t* fmi, const NandInfo *nandInfo, UInt32 numDataBus, UInt16 ceCount)
{
    UInt32 roundedBlocksPerDie;
    UInt32 roundedPagesPerBlock;
    
    WMR_MEMSET(&stDeviceInfo, 0, sizeof(stDeviceInfo));
    
    stDeviceInfo.wNumOfBusses = numDataBus;
    stDeviceInfo.wNumOfCS = ceCount;
    
    stDeviceInfo.wBlocksPerCS = fmi->blocks_per_ce;
    stDeviceInfo.wPagesPerBlock = fmi->pages_per_block;
    stDeviceInfo.wSectorsPerPage = fmi->sectors_per_page;
    stDeviceInfo.wSpareBytesPerPage = fmi->bytes_per_spare;
    
    stDeviceInfo.wBanksPerCS = fmi->banks_per_ce;
    stDeviceInfo.wDiesPerCS = fmi->dies_per_cs;
    stDeviceInfo.wBlocksPerDie = fmi->blocks_per_ce / fmi->dies_per_cs;

    if (stDeviceInfo.wBlocksPerCS & (stDeviceInfo.wBlocksPerCS - 1)) // non-pow2 blocks per cs 
    {            
        roundedBlocksPerDie = 1 << WMR_LOG2(stDeviceInfo.wBlocksPerDie);
        stDeviceInfo.wDieStride = (stDeviceInfo.wBlocksPerDie % roundedBlocksPerDie)? (roundedBlocksPerDie << 1) : roundedBlocksPerDie;
        stDeviceInfo.wLastBlock = (stDeviceInfo.wDiesPerCS - 1) * stDeviceInfo.wDieStride + stDeviceInfo.wBlocksPerDie;
        stDeviceInfo.wUserBlocksPerCS = 1 << WMR_LOG2(stDeviceInfo.wBlocksPerCS);
    }
    else
    {
        stDeviceInfo.wDieStride = stDeviceInfo.wBlocksPerDie;
        stDeviceInfo.wLastBlock = stDeviceInfo.wBlocksPerCS;
        stDeviceInfo.wUserBlocksPerCS = stDeviceInfo.wBlocksPerCS;
    }
	roundedPagesPerBlock = 1 << WMR_LOG2(stDeviceInfo.wPagesPerBlock);
    // MSB, or MSBx2 if non-pow2
    stDeviceInfo.wBlockStride = (stDeviceInfo.wPagesPerBlock % roundedPagesPerBlock)? (roundedPagesPerBlock << 1) : roundedPagesPerBlock;

    if (fmi->is_ppn)
    {
        stDeviceInfo.dwBlocksPerCAU        = fmi->ppn->device_params.blocks_per_cau;
        stDeviceInfo.dwCAUsPerCE           = fmi->ppn->device_params.caus_per_channel;
        stDeviceInfo.dwSLCPagesPerBlock    = fmi->ppn->device_params.pages_per_block_slc;
        stDeviceInfo.dwMLCPagesPerBlock    = fmi->ppn->device_params.pages_per_block;
        stDeviceInfo.dwMatchOddEvenCAUs    = fmi->ppn->device_params.block_pairing_scheme;
        stDeviceInfo.dwBitsPerCAUAddress   = fmi->ppn->device_params.cau_bits;
        stDeviceInfo.dwBitsPerPageAddress  = fmi->ppn->device_params.page_address_bits;
        stDeviceInfo.dwBitsPerBlockAddress = fmi->ppn->device_params.block_bits;
        stDeviceInfo.ppn                   = 1;
#if SUPPORT_TOGGLE_NAND
        stDeviceInfo.toggle                = fmi->is_toggle_system ? 1 : 0;
#endif
        stDeviceInfo.dwMaxTransactionSize  = fmi->ppn->device_params.prep_function_buffer_size;

        stDeviceInfo.checkInitialBadType = INIT_BBT_PPN;
        stDeviceInfo.vendorSpecificType = 0;
        stDeviceInfo.bAddrBitsForBitsPerCell = fmi->ppn->device_params.address_bits_bits_per_cell;
    }
    else
    {
        stDeviceInfo.dwBlocksPerCAU        = stDeviceInfo.wBlocksPerDie;
        stDeviceInfo.dwCAUsPerCE           = stDeviceInfo.wDiesPerCS;
        stDeviceInfo.dwSLCPagesPerBlock    = stDeviceInfo.wPagesPerBlock;
        stDeviceInfo.dwMLCPagesPerBlock    = stDeviceInfo.wPagesPerBlock;
        stDeviceInfo.dwMatchOddEvenCAUs    = 0;
        stDeviceInfo.dwBitsPerCAUAddress   = 1;
        stDeviceInfo.dwBitsPerPageAddress  = WMR_LOG2(stDeviceInfo.wPagesPerBlock - 1) + 1;
        stDeviceInfo.dwBitsPerBlockAddress = WMR_LOG2(stDeviceInfo.wPagesPerBlock - 1) + 1;
        stDeviceInfo.ppn                   = 0;
        stDeviceInfo.dwMaxTransactionSize  = stDeviceInfo.wBanksPerCS * stDeviceInfo.wPagesPerBlock;
        
        stDeviceInfo.checkInitialBadType = nandInfo->geometry->initialBBType;
        stDeviceInfo.vendorSpecificType = nandInfo->boardSupport->vsType;
        stDeviceInfo.bAddrBitsForBitsPerCell = 0;
    }

    stDeviceInfo.dwValidMetaPerLogicalPage = nandInfo->format->validMetaPerLogicalPage;
    stDeviceInfo.dwTotalMetaPerLogicalPage = nandInfo->format->metaPerLogicalPage;
    stDeviceInfo.dwLogicalPageSize = nandInfo->format->logicalPageSize;
    
    
    stDeviceInfo.bECCThresholdBits = fmi->refresh_threshold_bits;
    stDeviceInfo.bECCCorrectableBits = fmi->correctable_bits;
}

Int32
_initRawNand(h2fmi_t **fmi_list, const UInt32 num_fmi, const NandChipId *id_list, NandFmcSettings *actual)
{
    NandInfo nandInfo;
    h2fmi_t *fmi;
    UInt32 fmi_idx;
    NandRequiredTimings required;
    Int32 ret = FIL_SUCCESS;
    UInt32 ce_count = 0;
    
    if (findNandInfo(id_list, &nandInfo, num_fmi))
    {
        for(fmi_idx = 0; fmi_idx < num_fmi; ++fmi_idx)
        {
            fmi = fmi_list[fmi_idx];
            fmi->is_ppn = FALSE32;
            ce_count += fmi->num_of_ce;
            h2fmi_init_raw_state(fmi, &nandInfo, &required);
        }
        // Tell the chip ID library these numbers so it can pass them
        // to the device tree
        setECCLevels(fmi_list[0]->correctable_bits, fmi_list[0]->refresh_threshold_bits);
        
#if FMI_VERSION > 2
        if (!NandRequirementToIFCTRL(&required, actual))
        {
            ret = FIL_CRITICAL_ERROR;
        }
#else
        if (!Legacy_NandRequirementToIFCTRL(&required, actual))
        {
            ret = FIL_CRITICAL_ERROR;
        }
#endif

        _init_whimory_state(fmi_list[0], &nandInfo, num_fmi, ce_count);
    }
    else
    {
        WMR_PRINT(ERROR,
                  "[FIL:ERR] FMI - No recognized NAND found!\n");
        ret = FIL_CRITICAL_ERROR;
    }
    return ret;
}

#if SUPPORT_PPN
Int32 
_initPPN(h2fmi_t **fmi_list, const UInt32 num_fmi, const NandChipId *id_list, NandFmcSettings *actual, UInt32 ppn_version)
{
    NandInfo nandInfo;
    NandRequiredTimings required;
    const h2fmi_t *first_fmi = fmi_list[0];
    UInt32 fmi_idx;
    UInt32 ce_count = 0;
    UInt32 ceMap = 0;

    if (!h2fmi_ppn_fil_init())
    {
        return FIL_CRITICAL_ERROR;
    }

    // Strict check CE and bus symmetry
    if (((num_fmi != 1) || !targetSupportsSingleCE()) &&
        (num_fmi != 2) && (num_fmi != 4))
    {
        WMR_PRINT(ERROR, "Invalid fmi count: %d\n", num_fmi);
        return FIL_CRITICAL_ERROR;
    }
    for (fmi_idx = 0; fmi_idx < num_fmi; fmi_idx++)
    {
        h2fmi_t *current_fmi = fmi_list[fmi_idx];
        if (current_fmi->num_of_ce != first_fmi->num_of_ce)
        {
            return FIL_CRITICAL_ERROR;
        }
        // Verify that no two buses claim the same CE
        if (current_fmi->valid_ces & ceMap)
        {
            return FIL_CRITICAL_ERROR;
        }

        ceMap |= current_fmi->valid_ces;
    }
    if (!checkPpnLandingMap(ceMap))
    {
        return FIL_CRITICAL_ERROR;
    }

    WMR_MEMSET(&nandInfo, 0, sizeof(nandInfo));
    // Fetch device info for all channels
    // before issuing Normal-Mode on any of them
    for(fmi_idx = 0; fmi_idx < num_fmi; ++fmi_idx)
    {
        h2fmi_t *current_fmi = fmi_list[fmi_idx];
        current_fmi->is_ppn = TRUE32;
        current_fmi->clock_speed_khz = WMR_BUS_FREQ_HZ() / 1000;

        if (!h2fmi_ppn_init_channel(current_fmi, FALSE32, 0, FALSE32, 0, 0, ppn_version))
        {
            WMR_PRINT(ERROR, "h2fmi_ppn_init_channel for channel %d failed!\n", fmi_idx);
            return FIL_CRITICAL_ERROR;
        }
        ce_count += current_fmi->num_of_ce;

        h2fmi_ppn_post_rst_pre_pwrstate_operations(current_fmi);

        if (0 != current_fmi->valid_ces)
        {
            if (!h2fmi_ppn_get_device_params(current_fmi,
                                             (h2fmi_ce_t)WMR_LSB1(current_fmi->valid_ces),
                                             &current_fmi->ppn->device_params))
            {
                return FIL_CRITICAL_ERROR;
            }
        }
    }

    h2fmiPpnGetControllerInfo(0, gConfigInfo.contInfo.mfg_id, gConfigInfo.contInfo.fw_version);
    if (!h2fmiPpnValidateManufacturerIds(gConfigInfo.contInfo.mfg_id, ce_count))
    {
        WMR_PRINT(ERROR, "Inconsistent PPN controller manufacturer ids\n");
        return FIL_CRITICAL_ERROR;
    }
    else if (!h2fmiPpnValidateFirmwareVersions(gConfigInfo.contInfo.fw_version, ce_count))
    {
        WMR_PRINT(ERROR, "Inconsistent PPN controller firmware versions\n");
        return FIL_CRITICAL_ERROR;
    }
    else if (h2fmiPpnFirmwareIsBlacklisted(gConfigInfo.contInfo.mfg_id, gConfigInfo.contInfo.fw_version))
    {
        WMR_PRINT(ERROR, "PPN Device FW is unsupported.\n");
        return FIL_CRITICAL_ERROR;
    }

    checkForWorkarounds(gConfigInfo.contInfo.mfg_id, fmi_list, num_fmi);

    // Set all channels to Normal Mode without sending
    // any other commands to any other channels
    for(fmi_idx = 0; fmi_idx < num_fmi; ++fmi_idx)
    {
        h2fmi_t *current_fmi = fmi_list[fmi_idx];
        if (!h2fmi_ppn_set_channel_power_state(current_fmi,
#if SUPPORT_TOGGLE_NAND
                                               (current_fmi->is_toggle_system) ? PPN_PS_TRANS_LOW_POWER_TO_DDR :
#endif
                                               PPN_PS_TRANS_LOW_POWER_TO_ASYNC))
        {
            WMR_PRINT(ERROR, "Setting power state failed on FMI %d!\n", fmi_idx);
            return FIL_CRITICAL_ERROR;
        }

#if SUPPORT_TOGGLE_NAND
        if (current_fmi->is_toggle_system)
        {
            current_fmi->is_toggle = TRUE32;
            turn_on_fmc(current_fmi);
            restoreTimingRegs(current_fmi);
        }
#endif

        // Parse device parameters into FMI structures
        h2fmi_ppn_fill_nandinfo(current_fmi, &nandInfo, &required);
        nandInfo.ppn = &current_fmi->ppn->nandPpn;
    }

#if H2FMI_PPN_VERIFY_SET_FEATURES
    for(fmi_idx = 0; fmi_idx < num_fmi; ++fmi_idx)
    {
        h2fmi_t *current_fmi = fmi_list[fmi_idx];
        h2fmi_ppn_verify_feature_shadow(current_fmi);
    }
#endif // H2FMI_PPN_VERIFY_SET_FEATURES

#if SUPPORT_TOGGLE_NAND
    if (fmi_list[0]->is_toggle_system)
    {
        if (!Toggle_NandRequirementToIFCTRL(&required, actual))
        {
            WMR_PRINT(ERROR, "Unable to derive toggle bus timings\n");
            return FIL_CRITICAL_ERROR;
        }
        WMR_PRINT(INIT, "ddr_rs %d rpre %d rpst %d wpre %d wpst %d cs %d ch %d adl %d whr %d\n", 
            actual->ddr_half_cycle_clocks,
            actual->read_pre_clocks,
            actual->read_post_clocks,
            actual->write_pre_clocks,
            actual->write_post_clocks,
            actual->ce_setup_clocks,
            actual->ce_hold_clocks,
            actual->adl_clocks,
            actual->whr_clocks);
    }
#endif //SUPPORT_TOGGLE_NAND
    if (!NandRequirementToIFCTRL(&required, actual))
    {
        WMR_PRINT(ERROR, "Unable to derive bus timings\n");
        return FIL_CRITICAL_ERROR;
    }
    WMR_PRINT(INIT, "rs %d rh %d dc %d ws %d wh %d\n", 
        actual->read_setup_clocks,
        actual->read_hold_clocks,
        actual->read_dccycle_clocks,
        actual->write_setup_clocks,
        actual->write_hold_clocks);

    setPpnFeatures(fmi_list, num_fmi);

    for(fmi_idx = 0; fmi_idx < num_fmi; ++fmi_idx)
    {
        h2fmi_t *current_fmi = fmi_list[fmi_idx];
        if (nandInfo.format->logicalPageSize == 0)
        {
            current_fmi->logical_page_size = current_fmi->bytes_per_page;
            if (current_fmi->logical_page_size > 8 * 1024)
            {
                // lba is > 8KB
                WMR_PRINT(ERROR, "%d sized LBAs are not supported\n", current_fmi->logical_page_size);
                return FIL_UNSUPPORTED_ERROR;
            }
        }
        else
        {
            current_fmi->logical_page_size = nandInfo.format->logicalPageSize;
        }
        current_fmi->valid_bytes_per_meta = nandInfo.format->validMetaPerLogicalPage;
        current_fmi->total_bytes_per_meta = nandInfo.format->metaPerLogicalPage;
        current_fmi->fmi_config_reg    =  h2fmi_ppn_calculate_fmi_config(current_fmi);
#if FMI_VERSION > 0
        current_fmi->fmi_data_size_reg = h2fmi_ppn_calculate_fmi_data_size(current_fmi);
#endif /* FMI_VERSION > 0*/
    }

    // Only setup the device tree if our firmware version passes validation.
    // But we still need to call _init_whimory_state() to get our CE map right to
    // make the firmware update stuff work to fix the busted part later.
    setNandIdPpnConfig(id_list, &nandInfo, num_fmi, ce_count);

    _init_whimory_state(fmi_list[0], &nandInfo, num_fmi, ce_count);

    return FIL_SUCCESS;
}

static const ControllerInfo _ppn_fw_blacklist[] =
{
    { {0xEC, 0x00, 0x05, 0x49, 0x11, 0x02,}, "040201E01050108F" }, //rdar://9072970
    { {0xEC, 0x00, 0x05, 0x49, 0x11, 0x03,}, "040201E01050108F" }, //rdar://9072970
#if TARGET_IPAD3
    { {0xAD, 0x82, 0x01, 0x22, 0x01, 0x44,}, {0x30, 0x32, 0x30, 0x30, 0x37, 0x36, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} }, //rdar://9528679
    { {0xEC, 0x00, 0x05, 0x49, 0x11, 0x02,}, "040601P01090108F" }, //rdar://9498342
    { {0xEC, 0x00, 0x05, 0x49, 0x11, 0x03,}, "040601P01090108F" }, //rdar://9498342
#endif
};


BOOL32 h2fmiPpnFirmwareIsBlacklisted(const UInt8 *mfg_id, const UInt8 *fw_version)
{
    const ControllerInfo *entry;
    UInt32 idx;
    for (idx = 0; idx < sizeof(_ppn_fw_blacklist) / sizeof(ControllerInfo); ++idx)
    {
        entry = &_ppn_fw_blacklist[idx];
        if ((0 == WMR_MEMCMP(entry->mfg_id, mfg_id, PPN_DEVICE_ID_LEN)) &&
            (0 == WMR_MEMCMP(entry->fw_version, fw_version, NAND_DEV_PARAM_LEN_PPN)))
        {
            return TRUE32;
        }
    }
    return FALSE32;
}

void checkForWorkarounds(UInt8* mfg_id, h2fmi_t **fmi_list, const UInt32 num_fmi)
{
    static const ControllerInfo _toshiba_24[] = // rdar://10543574
    {
        {{0x98, 0x00, 0x04, 0x16, 0x14, 0x01}, {} }, //24nm 64GB
        {{0x98, 0x00, 0x04, 0x18, 0x14, 0x02}, {} }, //24nm 32GB
        {{0x98, 0x00, 0x04, 0x1A, 0x14, 0x04}, {} }, //24nm 16GB
        {{0x98, 0x00, 0x04, 0x14, 0x10, 0x01}, {} }, //24nm 8GB
    };

    UInt32 info_idx;
    UInt32 fmi_idx;

    BOOL32 match = FALSE32;
    for (info_idx = 0; info_idx < sizeof(_toshiba_24) / sizeof(ControllerInfo); ++info_idx)
    {
        if (0 == WMR_MEMCMP(_toshiba_24[info_idx].mfg_id, mfg_id, PPN_DEVICE_ID_LEN))
        {
            match = TRUE32;
            break;
        }
    }

    for(fmi_idx = 0; fmi_idx < num_fmi; ++fmi_idx)
    {
        fmi_list[fmi_idx]->retire_on_invalid_refresh = match;
    }
    
    setPPNOptions(match); 
}

static void setPpnFeatures(h2fmi_t **fmiList, const UInt32 fmiCount)
{
    UInt32 featCount = NUMELEMENTS(_ppnFeatureList);
    UInt32 fmiIdx, featIdx;

    // special handling
    for (featIdx = 0 ; featIdx < featCount ; featIdx++)
    {
        ppn_feature_entry_t *entry = &_ppnFeatureList[featIdx];

        switch (entry->feature)
        {
            case PPN_FEATURE__NUM_DIE_PROGRAM_PARALLEL :
                // fall-through
            case PPN_FEATURE__NUM_DIE_READ_PARALLEL :
                // fall-through
            case PPN_FEATURE__NUM_DIE_ERASE_PARALLEL :
            {
                entry->value = WMR_MIN(entry->value, fmiList[0]->ppn->device_params.dies_per_channel);
                break;
            }

            default:
                // do nothing
                break;
        }
    }

    for(fmiIdx = 0; fmiIdx < fmiCount; ++fmiIdx)
    {
        h2fmi_t *fmi = fmiList[fmiIdx];
        h2fmi_ppn_set_feature_list(fmi, _ppnFeatureList, featCount);
    }

    reportPpnFeatures(_ppnFeatureList, sizeof(_ppnFeatureList));
}

#endif //SUPPORT_PPN

Int32
h2fmiInitVirtToPhysMap(UInt16 busCount, UInt16 ceCount)
{
    UInt32 ce;
    UInt32 bus;
    h2fmi_ce_t enable[H2FMI_MAX_CE_TOTAL];
    UInt8  busIndex[H2FMI_MAX_NUM_BUS] = {0, 0};
    BOOL32 mapped = FALSE32;
    h2fmi_t* fmi;

    enable[ 0 ] = 0;
    enable[ 1 ] = 8;
    
    WMR_MEMSET(virtPhysMap, 0xFF, sizeof(virtPhysMap));

    WMR_PRINT(INF, "[FIL:INF] Mapping %d bus and %d CE\n", (UInt32)busCount, (UInt32)ceCount);
    for (ce = 0; ce < ceCount;)
    {
        mapped = FALSE32;

        // in order to best interleave across busses, map the next
        // virtual CE to the next active enable from each bus that
        // has any remaining active enables (i.e. stripe across busses)
        for (bus = 0; (bus < H2FMI_MAX_NUM_BUS) && (ce < ceCount); bus++)
        {
            // if there is still an active enable available on this bus,
            // assign it the next virtual CE to this bus and enable
            fmi = (bus == 0) ? &g_fmi0 : &g_fmi1;
            if (fmi->valid_ces & (1 << enable[bus]))
            {
                h2fmiMapVirtualCEToBusAndEnable(ce, bus, enable[bus]);
                fmi->ceMap[busIndex[bus]] = enable[bus];
                ce++;
                busIndex[bus]++;
                mapped = TRUE32;
            }
            enable[bus]++;
        }
    }

    // at least one CE must be mapped per stripe
    if(!mapped)
    {
        WMR_PRINT(ERROR, "CE Map failed!\n");
        return FIL_CRITICAL_ERROR;
    }
    else
    {
        return FIL_SUCCESS;
    }
}

void
h2fmiMapVirtualCEToBusAndEnable( UInt32 virtualCE, UInt32 bus, UInt32 enable )
{
    WMR_PRINT(INF, "[FIL:INF] Map virtual ce %d to bus %d, ce %d\n", virtualCE, bus, enable );

    virtPhysMap[ virtualCE ].ce  = enable;
    virtPhysMap[ virtualCE ].bus = bus;
}

h2fmi_t*
h2fmiTranslateVirtualCEToBus( UInt32 virtualCE )
{
    UInt32 bus = virtPhysMap[virtualCE].bus;
    switch(bus)
    {
        case 0:
            return &g_fmi0;
        case 1:
            return &g_fmi1;
        default:
            return NULL;
    }
}

h2fmi_ce_t
h2fmiTranslateVirtualCEToCe( UInt32 virtualCE )
{
    return virtPhysMap[virtualCE].ce;
}

#if !(AND_FPART_ONLY)
void
h2fmiSetDeviceInfo(UInt32 dwParamType, UInt32 dwVal)
{
    WMR_PRINT(LOG, "[FIL:LOG] h2fmiSetDeviceInfo()\n");

    switch(dwParamType)
    {
        case AND_DEVINFO_VENDOR_SPECIFIC_TYPE:
            // VS writes are not implemented in boot context
            break;

        case AND_DEVINFO_BANKS_PER_CS:
            g_fmi0.banks_per_ce = dwVal;
            g_fmi1.banks_per_ce = dwVal;
            stDeviceInfo.wBanksPerCS = ((UInt16)dwVal);
            break;

        default:
            WMR_PANIC("unsupported 0x%08x\n", dwParamType);
            break;
    }
}
#endif //!AND_FPART_ONLY

UInt32
h2fmiGetDeviceInfo(UInt32 dwParamType)
{
    UInt32 dwRetVal = 0xFFFFFFFF;

    WMR_PRINT(LOG, "[FIL:LOG] h2fmiGetDeviceInfo()\n");

    switch(dwParamType)
    {
    
    case AND_DEVINFO_DIES_PER_CS:
        dwRetVal = (UInt32)stDeviceInfo.wDiesPerCS;
        break;

    case AND_DEVINFO_BLOCKS_PER_DIE:
        dwRetVal = (UInt32)stDeviceInfo.wBlocksPerDie;
        break;

    case AND_DEVINFO_DIE_STRIDE:
        dwRetVal = (UInt32)stDeviceInfo.wDieStride;
        break;

    case AND_DEVINFO_BLOCK_STRIDE:
        dwRetVal = (UInt32)stDeviceInfo.wBlockStride;
        break;

    case AND_DEVINFO_LAST_BLOCK:
        dwRetVal = (UInt32)stDeviceInfo.wLastBlock;
        break;

    case AND_DEVINFO_USER_BLOCKS_PER_CS:
        dwRetVal = (UInt32)stDeviceInfo.wUserBlocksPerCS;
        break;

    case AND_DEVINFO_PAGES_PER_BLOCK:
        dwRetVal = (UInt32)stDeviceInfo.wPagesPerBlock;
        break;

        case AND_DEVINFO_NUM_OF_CS:
            dwRetVal = (UInt32)stDeviceInfo.wNumOfCS;
            break;

        case AND_DEVINFO_BBT_TYPE:
            dwRetVal = (UInt32)stDeviceInfo.checkInitialBadType;
            break;

        case AND_DEVINFO_BLOCKS_PER_CS:
            dwRetVal = (UInt32)stDeviceInfo.wBlocksPerCS;
            break;

        case AND_DEVINFO_BYTES_PER_PAGE:
            dwRetVal = (UInt32)(stDeviceInfo.wSectorsPerPage * H2FMI_BYTES_PER_SECTOR);
            break;

        case AND_DEVINFO_BYTES_PER_SPARE:
            dwRetVal = (UInt32)stDeviceInfo.wSpareBytesPerPage;
            break;

        case AND_DEVINFO_VENDOR_SPECIFIC_TYPE:
            dwRetVal = (UInt32)stDeviceInfo.vendorSpecificType;
            break;

        case AND_DEVINFO_REFRESH_THRESHOLD:
            dwRetVal = (UInt32)stDeviceInfo.bECCThresholdBits;
            break;

        case AND_DEVINFO_CORRECTABLE_SIZE:
            dwRetVal = (UInt32)H2FMI_BYTES_PER_SECTOR;
            break;

        case AND_DEVINFO_BYTES_PER_BL_PAGE:
            dwRetVal = H2FMI_BOOT_BYTES_PER_PAGE;
            break;

        case AND_DEVINFO_CORRECTABLE_BITS:
            dwRetVal = g_fmi0.correctable_bits;            
            break;

        case AND_DEVINFO_NUM_OF_BANKS:
            dwRetVal = (UInt32)stDeviceInfo.wNumOfCS * stDeviceInfo.wBanksPerCS;
            break;

        case AND_DEVINFO_BLOCKS_PER_CAU:
            dwRetVal = stDeviceInfo.dwBlocksPerCAU;
            break;

        case AND_DEVINFO_CAUS_PER_CE:
            dwRetVal = stDeviceInfo.dwCAUsPerCE;
            break;

        case AND_DEVINFO_SLC_PAGES_PER_BLOCK:
            dwRetVal = stDeviceInfo.dwSLCPagesPerBlock;
            break;

        case AND_DEVINFO_MLC_PAGES_PER_BLOCK:
            dwRetVal = stDeviceInfo.dwMLCPagesPerBlock;
            break;

        case AND_DEVINFO_MATCH_ODDEVEN_CAUS:
            dwRetVal = stDeviceInfo.dwMatchOddEvenCAUs;
            break;

        case AND_DEVINFO_BITS_PER_CAU_ADDRESS:
            dwRetVal = stDeviceInfo.dwBitsPerCAUAddress;
            break;

        case AND_DEVINFO_BITS_PER_PAGE_ADDRESS:
            dwRetVal = stDeviceInfo.dwBitsPerPageAddress;
            break;

        case AND_DEVINFO_BITS_PER_BLOCK_ADDRESS:
            dwRetVal = stDeviceInfo.dwBitsPerBlockAddress;
            break;

        case AND_DEVINFO_NUM_OF_CHANNELS:
            dwRetVal = stDeviceInfo.wNumOfBusses;
            break;

        case AND_DEVINFO_NUM_OF_CES_PER_CHANNEL:
            if (stDeviceInfo.wNumOfBusses > 0)
            {
                dwRetVal = stDeviceInfo.wNumOfCS / stDeviceInfo.wNumOfBusses;
            }
            else
            {
                dwRetVal = 0;
            }
            break;

        case AND_DEVINFO_PPN_DEVICE:
            dwRetVal = stDeviceInfo.ppn;
            break;

        case AND_DEVINFO_TOGGLE_DEVICE:
#if SUPPORT_TOGGLE_NAND
            dwRetVal = stDeviceInfo.toggle;
#else
            dwRetVal = 0;
#endif
            break;

        case AND_DEVINFO_BANKS_PER_CS:
            dwRetVal = stDeviceInfo.wBanksPerCS;
            break;

        case AND_DEVINFO_FIL_LBAS_PER_PAGE:
            if (stDeviceInfo.dwLogicalPageSize == 0)
            {
                dwRetVal = 1;
            }
            else
            {
                dwRetVal = (stDeviceInfo.wSectorsPerPage * H2FMI_BYTES_PER_SECTOR) /  stDeviceInfo.dwLogicalPageSize;
            }

            break;

        case AND_DEVINFO_FIL_META_VALID_BYTES:
            dwRetVal = stDeviceInfo.dwValidMetaPerLogicalPage;
            break;

        case AND_DEVINFO_FIL_META_BUFFER_BYTES:
            dwRetVal = stDeviceInfo.dwTotalMetaPerLogicalPage;
            break;

        case AND_DEVINFO_FIL_PREP_BUFFER_ENTRIES:
            dwRetVal = stDeviceInfo.dwMaxTransactionSize;
            break;

        case AND_DEVINFO_ADDR_BITS_BITS_PER_CELL:
            dwRetVal = stDeviceInfo.bAddrBitsForBitsPerCell;
            break;

        case AND_DEVINFO_STREAM_BUFFER_MAX:
            dwRetVal= 0;
            break;

        default:
            WMR_PANIC("unsupported 0x%08x\n", dwParamType);
            break;
    }
    return dwRetVal;
}

static BOOL32 h2fmi_get_nand_layout(void * buffer, UInt32 *size)
{
    ANDNandLayoutStruct * pLayout = (ANDNandLayoutStruct*) buffer;
    UInt32 idx;
    UInt32 cursor;
    const UInt32 dwParityBytes = h2fmi_calculate_ecc_output_bytes(&g_fmi0);

    if (!size)
    {
        return FALSE32;
    }
    else if (pLayout && (*size < sizeof(ANDNandLayoutStruct)))
    {
        // It's OK just to ask the size
        return FALSE32;
    }
    
    *size = sizeof(ANDNandLayoutStruct);

    if (!pLayout)
    {
        return TRUE32;
    }

    WMR_ASSERT((_GetSectorsPerPage() + 1UL) <= sizeof(pLayout->pastrSegments));
    
    // Meta data is at the end of the first envelope
    pLayout->dwMetaSegmentIndex = 1;
    pLayout->dwNumSegments = _GetSectorsPerPage() + 1;

    pLayout->pastrSegments[0].dwOffset = 0;
    pLayout->pastrSegments[0].dwLength = H2FMI_BYTES_PER_SECTOR;
    cursor = H2FMI_BYTES_PER_SECTOR;

    pLayout->pastrSegments[1].dwOffset = cursor;
    pLayout->pastrSegments[1].dwLength = g_fmi0.valid_bytes_per_meta;
    cursor += g_fmi0.valid_bytes_per_meta + dwParityBytes;

    for (idx = 2; idx < pLayout->dwNumSegments; ++idx)
    {
        pLayout->pastrSegments[idx].dwOffset = cursor;
        pLayout->pastrSegments[idx].dwLength = H2FMI_BYTES_PER_SECTOR;
        cursor += H2FMI_BYTES_PER_SECTOR + dwParityBytes;
    }
    
    return TRUE32;
}

static BOOL32 h2fmiGetChipIdStruct(void * buffer, UInt32 *size, UInt8 addr)
{
    BOOL32 boolRes;
    h2fmi_chipid_t* fmi0ChipIds, * fmi1ChipIds;
    UInt16 wIdx;
    UInt32 valid_ces;
    const UInt32 dwRequiredBufferSize = H2FMI_MAX_CE_TOTAL * sizeof(UInt64);
    
    if (!size)
    {
        boolRes = FALSE32;
    }
    else if (buffer && (*size < dwRequiredBufferSize))
    {
        // It's OK just to ask the size
        boolRes = FALSE32;
    }
    else if (!buffer)
    {
        *size = dwRequiredBufferSize;    
        boolRes = TRUE32;
    }
    else
    {
        fmi0ChipIds = (h2fmi_chipid_t*)g_pScratchBuf;
        fmi1ChipIds = fmi0ChipIds + H2FMI_MAX_CE_TOTAL;
        WMR_MEMSET(g_pScratchBuf, 0, sizeof(h2fmi_chipid_t) * NAND_MAX_CE_COUNT_TOTAL * 2);

#if SUPPORT_TOGGLE_NAND
        if (g_fmi0.is_toggle_system)
        {
            WMR_ASSERT(g_fmi1.is_toggle_system || (0 == g_fmi1.num_of_ce));
            g_fmi0.is_toggle = FALSE32;
            g_fmi1.is_toggle = FALSE32;
        }
#endif
   
        h2fmi_reset(&g_fmi0);
        h2fmi_reset(&g_fmi1);

// Only for H5X - // <rdar://problem/11677742> Software workaround for Samsung reset and bad chip ID isssue
#if FMI_VERSION >= 5
        {
            BOOL32 reset_result_ch0, reset_result_ch1;
            reset_result_ch0 = h2fmi_nand_reset_all(&g_fmi0);
            reset_result_ch1 = h2fmi_nand_reset_all(&g_fmi1);

            if(reset_result_ch0)
            {
                h2fmi_nand_read_id_all(&g_fmi0, fmi0ChipIds, addr);
            }
            if(reset_result_ch1)
            {
                h2fmi_nand_read_id_all(&g_fmi1, fmi1ChipIds, addr);
            }
        }
#else
        {
            h2fmi_reset_and_read_chipids(&g_fmi0, fmi0ChipIds, addr);
            h2fmi_reset_and_read_chipids(&g_fmi1, fmi1ChipIds, addr);
        }
#endif // FMI_VERSION >= 5

        h2fmi_build_ce_bitmask(&g_fmi0, fmi0ChipIds, &fmi0ChipIds[0], addr);
        h2fmi_build_ce_bitmask(&g_fmi1, fmi1ChipIds, &fmi0ChipIds[0], addr);

        valid_ces = g_fmi0.valid_ces | g_fmi1.valid_ces;

#if SUPPORT_PPN
        if (g_fmi0.is_ppn)
        {
            WMR_ASSERT(g_fmi1.is_ppn || (0 == g_fmi1.num_of_ce));
            h2fmi_ppn_post_rst_pre_pwrstate_operations(&g_fmi0);
            if(g_fmi1.num_of_ce)
            {
                h2fmi_ppn_post_rst_pre_pwrstate_operations(&g_fmi1);
            }
        }
#endif

        WMR_MEMSET(buffer, 0, H2FMI_MAX_CE_TOTAL * sizeof(UInt64));

        for (wIdx = 0; wIdx < H2FMI_MAX_CE_TOTAL; ++wIdx)
        {
            h2fmi_chipid_t* srcID = &fmi0ChipIds[0];
            *((UInt64 *)srcID) = (*(UInt64 *)srcID) & MAX_ID_SIZE_MASK;	
            UInt64* destID = ((UInt64*)buffer) + wIdx;
            if( valid_ces & (1 << wIdx))
            {
                WMR_MEMCPY(destID, srcID, sizeof(UInt64));
            }
        }
        *size = dwRequiredBufferSize;

		// There's probably a better way to do this, but FMI currently resets all devices in the same path that reads the chip ID.
		// On a PPN device, that'll kick us back into low-power mode.  Force it back to ASYNC/DDR mode...
#if SUPPORT_PPN
        if (g_fmi0.is_ppn)
        {
            h2fmi_ppn_set_channel_power_state(&g_fmi0, 
#if SUPPORT_TOGGLE_NAND
                   (g_fmi0.is_toggle_system) ? PPN_PS_TRANS_LOW_POWER_TO_DDR : 
#endif
                   PPN_PS_TRANS_LOW_POWER_TO_ASYNC);

            if(g_fmi1.num_of_ce)
            {
                WMR_ASSERT(g_fmi1.is_ppn);
                h2fmi_ppn_set_channel_power_state(&g_fmi1, 
#if SUPPORT_TOGGLE_NAND
                   (g_fmi1.is_toggle_system) ? PPN_PS_TRANS_LOW_POWER_TO_DDR : 
#endif
                   PPN_PS_TRANS_LOW_POWER_TO_ASYNC);
	    }

#if H2FMI_PPN_VERIFY_SET_FEATURES
            if(g_fmi0.num_of_ce)
            {
                h2fmi_ppn_verify_feature_shadow(&g_fmi0);
            }
            if(g_fmi1.num_of_ce)
            {
                h2fmi_ppn_verify_feature_shadow(&g_fmi1);
            }
#endif // H2FMI_PPN_VERIFY_SET_FEATURES

#if SUPPORT_TOGGLE_NAND
            if (g_fmi0.is_toggle_system)
            {
                g_fmi0.is_toggle = TRUE32;
                
                h2fmi_reset(&g_fmi0);
                if(g_fmi1.num_of_ce)
                {
                    WMR_ASSERT(g_fmi1.is_toggle_system);
                    g_fmi1.is_toggle = TRUE32;
                }
                h2fmi_reset(&g_fmi1);
            }
#endif

        }       
#endif //SUPPORT_PPN
        boolRes = TRUE32;
    }
    return boolRes;
}

void h2fmi_write_bus_timings(NandTimingParams *timing)
{
#if SUPPORT_TOGGLE_NAND
    if (g_fmi0.is_toggle_system)
    {
        g_fmi0.toggle_if_ctrl &= (~FMC_IF_CTRL__REB_SETUP_MASK);
        g_fmi0.toggle_if_ctrl |= FMC_IF_CTRL__REB_SETUP(timing->ddr_tHALFCYCLE);

        g_fmi1.toggle_if_ctrl &= (~FMC_IF_CTRL__REB_SETUP_MASK);
        g_fmi1.toggle_if_ctrl |= FMC_IF_CTRL__REB_SETUP(timing->ddr_tHALFCYCLE);

        h2fmi_set_if_ctrl(&g_fmi0, g_fmi0.toggle_if_ctrl);
        h2fmi_set_if_ctrl(&g_fmi1, g_fmi1.toggle_if_ctrl);
    }
    else
    {
#endif
        g_fmi0.if_ctrl = ((FMC_IF_CTRL__WPB & (g_fmi0.if_ctrl)) |
                          (FMC_IF_CTRL__RBBEN & (g_fmi0.if_ctrl)) |
                          FMC_IF_CTRL__DCCYCLE(timing->sdrTimings.DCCYCLE) |
                          FMC_IF_CTRL__REB_SETUP(timing->sdrTimings.tRP) |
                          FMC_IF_CTRL__REB_HOLD(timing->sdrTimings.tREH) |
                          FMC_IF_CTRL__WEB_SETUP(timing->sdrTimings.tWP) |
                          FMC_IF_CTRL__WEB_HOLD(timing->sdrTimings.tWH));
        g_fmi0.if_ctrl_normal = g_fmi0.if_ctrl;
    
        g_fmi1.if_ctrl = ((FMC_IF_CTRL__WPB & (g_fmi1.if_ctrl)) |
                          (FMC_IF_CTRL__RBBEN & (g_fmi1.if_ctrl)) |
                          FMC_IF_CTRL__DCCYCLE(timing->sdrTimings.DCCYCLE) |
                          FMC_IF_CTRL__REB_SETUP(timing->sdrTimings.tRP) |
                          FMC_IF_CTRL__REB_HOLD(timing->sdrTimings.tREH) |
                          FMC_IF_CTRL__WEB_SETUP(timing->sdrTimings.tWP) |
                          FMC_IF_CTRL__WEB_HOLD(timing->sdrTimings.tWH));
        g_fmi1.if_ctrl_normal = g_fmi1.if_ctrl;

        h2fmi_set_if_ctrl(&g_fmi0, g_fmi0.if_ctrl);
        h2fmi_set_if_ctrl(&g_fmi1, g_fmi1.if_ctrl);
#if SUPPORT_TOGGLE_NAND
    }
#endif
}

void h2fmi_read_bus_timings(NandTimingParams *timing)
{
#if SUPPORT_TOGGLE_NAND
#if APPLICATION_EMBEDDEDIOP
    if (g_fmi0.is_toggle)
#else
    if (g_fmi0.is_toggle_system)
#endif
    {
        timing->ddr_tHALFCYCLE = FMC_IF_CTRL__GET_REB_SETUP(g_fmi0.toggle_if_ctrl);
    }
    else
    {
#endif
        timing->sdrTimings.DCCYCLE  = FMC_IF_CTRL__GET_DCCYCLE(g_fmi0.if_ctrl);
        timing->sdrTimings.tRP      = FMC_IF_CTRL__GET_REB_SETUP(g_fmi0.if_ctrl);
        timing->sdrTimings.tREH     = FMC_IF_CTRL__GET_REB_HOLD(g_fmi0.if_ctrl);
        timing->sdrTimings.tWP      = FMC_IF_CTRL__GET_WEB_SETUP(g_fmi0.if_ctrl);
        timing->sdrTimings.tWH      = FMC_IF_CTRL__GET_WEB_HOLD(g_fmi0.if_ctrl);
#if SUPPORT_TOGGLE_NAND
    }
#endif
}



BOOL32 h2fmiSetStruct(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 dwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType & AND_STRUCT_FIL_MASK)
    {
    case AND_STRUCT_FIL_SET_TIMINGS:
        {
            NandTimingParams *timing = pvoidStructBuffer;
            if((dwStructSize != sizeof(NandTimingParams)) || (NULL == pvoidStructBuffer))
            {
                boolRes = FALSE32;
                break;
            }
            h2fmi_write_bus_timings(timing);
            boolRes = TRUE32;
            break;
        }

    default:
        {
            break;
        }
    }
    return boolRes;
}

#if SUPPORT_PPN
static BOOL32 h2fmiGetDeviceParameters(UInt32 dwStructType, void* pvoidStructBuffer, UInt32* pdwStructSize)
{    
    BOOL32 boolRes = FALSE32;
    UInt16 ceIdx;
    UInt8 phyCE;
    const UInt32 requiredSize = NAND_DEV_PARAM_LEN_PPN * _GetNumOfCS();
    UInt8 *cursor = (UInt8*)pvoidStructBuffer;

    if ((pvoidStructBuffer == NULL) || (pdwStructSize == NULL) || (*pdwStructSize < requiredSize))
    {
        return FALSE32;
    }
    else
    {
        for(ceIdx = 0; ceIdx< _GetNumOfCS(); ceIdx++)
        {
            phyCE = h2fmiTranslateVirtualCEToCe(ceIdx);
            h2fmi_t *whichFmi = h2fmiTranslateVirtualCEToBus(ceIdx);
    		
            if(whichFmi)
            {
                switch (dwStructType & AND_STRUCT_FIL_MASK)
                {
                    case AND_STRUCT_FIL_FW_VERSION:
                    {
                        boolRes = h2fmi_ppn_get_fw_version(whichFmi, phyCE, cursor); 
                        break;
                    }

                    case AND_STRUCT_FIL_PKG_ASSEMBLY_CODE:
                    {
                        boolRes = h2fmi_ppn_get_package_assembly_code(whichFmi, phyCE, cursor);
                        break;
                    }

                    case AND_STRUCT_FIL_CONTROLLER_UID:
                    {
                        boolRes = h2fmi_ppn_get_controller_unique_id(whichFmi, phyCE, cursor);
                        break;
                    }

                    case AND_STRUCT_FIL_CONTROLLER_HW_ID:
                    {
                        boolRes = h2fmi_ppn_get_controller_hw_id(whichFmi, phyCE, cursor);
                        break;
                    }
                    default:
                        WMR_PRINT(ERROR, "[FIL:DBG]  FNAND_h2fmiGetDeviceParameters 0x%X is not identified as FIL data struct identifier!\n", dwStructType);
                        boolRes = FALSE32;
                }
                
                cursor += NAND_DEV_PARAM_LEN_PPN;
            }
        }
        *pdwStructSize = NAND_DEV_PARAM_LEN_PPN * _GetNumOfCS();
    }
    return boolRes;
}
#endif //SUPPORT_PPN

BOOL32 h2fmiGetStruct(UInt32 dwStructType, void* pvoidStructBuffer, UInt32* pdwStructSize)
{
    BOOL32 boolRes = FALSE32;

    switch (dwStructType & AND_STRUCT_FIL_MASK)
    {
#if !(AND_FPART_ONLY)
#if AND_COLLECT_FIL_STATISTICS
        case AND_STRUCT_FIL_STATISTICS:
        {
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &stFILStatistics, sizeof(stFILStatistics));
            break;
        }
#endif

        case AND_STRUCT_FIL_CHIPID:
        {
            boolRes = h2fmiGetChipIdStruct(pvoidStructBuffer, pdwStructSize, CHIPID_ADDR);
            break;
        }

        case AND_STRUCT_FIL_MFG_ID:
        {
            boolRes = h2fmiGetChipIdStruct(pvoidStructBuffer, pdwStructSize, MfgID_ADDR);            
            break;
        }

#if (defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)
        case AND_STRUCT_FIL_UNIQUEID:
        {
            BOOL32 setPattern = FALSE32;
            BOOL32 toBreak = FALSE32;
            PpnUIDReadRequest *ppnUIDRead = pvoidStructBuffer;
            GenericReadRequest *genericRead = pvoidStructBuffer;
			
            if(stDeviceInfo.ppn)
            {
                boolRes = h2fmiGetPPNUID(ppnUIDRead, pdwStructSize, &setPattern, &toBreak);
            }
            else
            {
                boolRes = h2fmiGetRAWUID(genericRead, pdwStructSize, &setPattern, &toBreak);
            }
            
            if(toBreak == TRUE32)
            {
                break;
            }
			
            if(setPattern)
            {
                UInt32 i;
                UInt8 pattern = 0x0; // UID is stored as data followed by (!data), so this is just checking that
                for (i=0; i<((stDeviceInfo.ppn)?(NAND_UID_PPN_BYTES_TO_READ):(NAND_UID_RAW_PAGE_BYTES_TO_READ)); i+=((stDeviceInfo.ppn)?(NAND_UID_LEN_PPN):(NAND_UID_LEN_RAW)))
                {
                    WMR_MEMSET( ( (stDeviceInfo.ppn)?(&((ppnUIDRead->buf[i]))):(&((genericRead->buf[i]))) ), pattern, ((stDeviceInfo.ppn)?(NAND_UID_LEN_PPN):(NAND_UID_LEN_RAW)) );
                    pattern = ~pattern;
                }
                boolRes = TRUE32;
            }
            break;
        }			
        
#endif // (defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)

        case AND_STRUCT_FIL_NAND_LAYOUT:
        {
            boolRes = h2fmi_get_nand_layout(pvoidStructBuffer, pdwStructSize);
            break;
        }

        case AND_STRUCT_FIL_SPARE_SIZE:
        {
            UInt32 dwSpareSize = _GetBytesPerPageSpare();
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwSpareSize, sizeof(dwSpareSize));
            break;
        }
                
        case AND_STRUCT_FIL_BITS_PER_CELL:
        {
            UInt32 dwBitsPerCell;
            if (_GetPagesPerBlock() < 128)
            {
                dwBitsPerCell = 1;
            }
            else if (0 == (_GetPagesPerBlock() % 3))
            {
                dwBitsPerCell = 3;
            }
            else
            {
                dwBitsPerCell = 2;
            }
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwBitsPerCell, sizeof(dwBitsPerCell));
            break;
        }

        case AND_STRUCT_FIL_LBAS_PER_PAGE:
        {
            UInt32 dwLbasPerPage;
            if (stDeviceInfo.dwLogicalPageSize == 0)
            {
                dwLbasPerPage = 1;
            }
            else
            {
                dwLbasPerPage = (stDeviceInfo.wSectorsPerPage * H2FMI_BYTES_PER_SECTOR) /  stDeviceInfo.dwLogicalPageSize;
            }
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &dwLbasPerPage, sizeof(dwLbasPerPage));
            break;
        }

        case AND_STRUCT_FIL_DIES_PER_CE:
        {
            UInt32 diesPerCE = _GetDiesPerCS();
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &diesPerCE, sizeof(diesPerCE));
            break;
        }

        case AND_STRUCT_FIL_BLOCKS_PER_CE:
        {
            UInt32 blocksPerCE = _GetBlocksPerCS();
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &blocksPerCE, sizeof(blocksPerCE));
            break;
        }

        case AND_STRUCT_FIL_GET_TIMINGS:
        {
            NandTimingParams timing;
            if (*pdwStructSize != sizeof(NandTimingParams))
            {
                boolRes = FALSE32;
                break;
            }
            h2fmi_read_bus_timings(&timing);
            boolRes = WMR_FILL_STRUCT(pvoidStructBuffer, pdwStructSize, &timing, sizeof(timing));
            break;
        }
#endif // !(AND_FPART_ONLY)
        case AND_STRUCT_FIL_GET_CE_INFO:
        {
            PPNChipEnableStruct *ceInfo;
            h2fmi_t             *fmi;

            ceInfo = (PPNChipEnableStruct *)pvoidStructBuffer;
            fmi = (ceInfo->channel == 0) ? &g_fmi0 : &g_fmi1;

            ceInfo->physical_chip_enable = fmi->ceMap[ceInfo->chip_enable_idx];
            boolRes = TRUE32;
            break;
        }
#if SUPPORT_PPN
        case AND_STRUCT_FIL_FW_VERSION:
        case AND_STRUCT_FIL_PKG_ASSEMBLY_CODE:
        case AND_STRUCT_FIL_CONTROLLER_UID:
        case AND_STRUCT_FIL_CONTROLLER_HW_ID:
        {
            boolRes = h2fmiGetDeviceParameters(dwStructType, pvoidStructBuffer, pdwStructSize); 
            break;
        }    
#endif
        default:
            WMR_PRINT(ERROR, "[FIL:DBG]  FNAND_GetStruct 0x%X is not identified as FIL data struct identifier!\n", dwStructType);
            boolRes = FALSE32;
    }
    return boolRes;
}

void h2fmiReset(void)
{
    WMR_PRINT(LOG, "[FIL:LOG] h2fmiReset()\n");

    AND_FIL_STAT(stFILStatistics.ddwResetCallCnt++);

    // do nothing
}

Int32 h2fmiReadNoECC(UInt16 wVirtualCE,
                     UInt32 dwPpn,
                     UInt8* pabDataBuf,
                     UInt8* pabSpareBuf)
{
    Int32 nRet    = FIL_SUCCESS;

    h2fmi_t* fmi  = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    h2fmi_ce_t ce = h2fmiTranslateVirtualCEToCe(wVirtualCE);

    WMR_PRINT(READ, "[FIL:LOG] h2fmiReadNoEcc()\n");

    WMR_PREPARE_READ_BUFFER(pabDataBuf, fmi->bytes_per_page);
    WMR_PREPARE_READ_BUFFER(pabSpareBuf, fmi->bytes_per_spare);

    AND_FIL_STAT(stFILStatistics.ddwReadNoECCCallCnt++);
    AND_FIL_STAT(stFILStatistics.ddwPagesReadCnt++);

    h2fmi_choose_aes(FALSE32, FALSE32, pabDataBuf, fmi, &dwPpn);

    if (!h2fmi_read_raw_page(fmi,
                             ce,
                             dwPpn,
                             pabDataBuf,
                             pabSpareBuf))
    {
        nRet = FIL_CRITICAL_ERROR;
    }

    WMR_COMPLETE_READ_BUFFER(pabDataBuf, fmi->bytes_per_page);
    WMR_COMPLETE_READ_BUFFER(pabSpareBuf, fmi->bytes_per_spare);

    return nRet;
}

Int32 h2fmiReadSequentialPages(UInt32* padwPpn,
                               UInt8*  pabDataBuf,
                               UInt8*  pabMetaBuf,
                               UInt16  wNumOfPagesToRead,
                               UInt8*  pbMaxCorrectedBits,
                               UInt8*  pdwaSectorStats,
                               BOOL32  bDisableWhitening)
{
    Int32 nRet   = FIL_SUCCESS;

    const UInt32 banks_total = _GetNumOfBanks();

    UInt16 wBankIdx;
    UInt32 wIdx;
    UInt8* dataPtr;
    UInt8* metaPtr;
    UInt32 cleanCount = 0;
    
    WMR_PRINT(READ, "[FIL:LOG] h2fmiReadSequentialPages()\n");

    if (pbMaxCorrectedBits)
    {
        *pbMaxCorrectedBits = 0;
    }
    
    dataPtr = pabDataBuf;
    metaPtr = pabMetaBuf;

    // It is assumed by the caller that ReadSequential will operate on a multiple of
    // bankCount and the CE's of the padwPpn array will be paired with ascending
    // CE's generated by looping through banks sequentially
    for (wIdx = 0; wIdx < wNumOfPagesToRead; wIdx += banks_total)
    {
        for (wBankIdx = 0; wBankIdx < banks_total; wBankIdx++)
        {
            const UInt32 dwCurrentPpn = padwPpn[wBankIdx] + (wIdx / banks_total);
            const UInt16 wVirtualCE = wBankIdx % banks_total;

            UInt8 maxCorrectedBits = 0;
            Int32 singleStatus;
            
            singleStatus = h2fmiReadSinglePage(wVirtualCE,
                                       dwCurrentPpn,
                                       dataPtr,
                                       metaPtr,
                                       &maxCorrectedBits,
                                       pdwaSectorStats,
                                       bDisableWhitening);
    
            switch (singleStatus)
            {
                case FIL_SUCCESS:
                    // don't overwrite previous failures
                    break;
                case FIL_U_ECC_ERROR:
                    nRet = FIL_U_ECC_ERROR;
                    WMR_PRINT(ERROR, "[FIL:ERR] Uncorrectable page detected during scattered read; continuing.\n");
                    break;
                case FIL_SUCCESS_CLEAN:
                    nRet = FIL_SUCCESS_CLEAN;
                    cleanCount++;
                    break;
                default:
                    nRet = FIL_CRITICAL_ERROR;
                    WMR_PRINT(ERROR, "[FIL:ERR] Hardware error detected during scattered read: 0x%08x\n", singleStatus);
                    break;
            }
    
            if (pbMaxCorrectedBits && (maxCorrectedBits > *pbMaxCorrectedBits))
            {
                // Trace max corrected bits accross all pages in this operation
                *pbMaxCorrectedBits = maxCorrectedBits;
            }
    
            if (pdwaSectorStats)
            {
                pdwaSectorStats += _GetSectorsPerPage();
            }
    
            dataPtr += _GetBytesPerPageMain();
            metaPtr += g_fmi0.total_bytes_per_meta;
        }
    }

    if (cleanCount && (cleanCount != wNumOfPagesToRead))
    {
        // Mix of valid and clean pages.  Treat this like a UECC error
        nRet = FIL_U_ECC_ERROR;
    }

    return nRet;
}

Int32 h2fmiReadScatteredPages(UInt16* pawCEs,
                              UInt32* padwPpn,
                              UInt8*  pabDataBuf,
                              UInt8*  pabMetaBuf,
                              UInt16  wNumOfPagesToRead,
                              UInt8*  pbMaxCorrectedBits,
                              UInt8*  pdwaSectorStats,
                              BOOL32  bDisableWhitening)
{
    Int32 nRet   = FIL_SUCCESS;
    UInt32 idx;
    UInt8* dataPtr;
    UInt8* metaPtr;
    UInt32 cleanCount = 0;

    WMR_PRINT(READ, "[FIL:LOG] h2fmiReadScatteredPages()\n");

    if (pbMaxCorrectedBits)
    {
        *pbMaxCorrectedBits = 0;
    }

    dataPtr = pabDataBuf;
    metaPtr = pabMetaBuf;

    for (idx = 0; idx < wNumOfPagesToRead; idx++)
    {
        UInt8 maxCorrectedBits = 0;
        Int32 singleStatus;
        
        singleStatus = h2fmiReadSinglePage(pawCEs[idx],
                                   padwPpn[idx],
                                   dataPtr,
                                   metaPtr,
                                   &maxCorrectedBits,
                                   pdwaSectorStats,
                                   bDisableWhitening);

        switch (singleStatus)
        {
            case FIL_SUCCESS:
                // don't overwrite previous failures
                break;
            case FIL_U_ECC_ERROR:
                nRet = FIL_U_ECC_ERROR;
                WMR_PRINT(ERROR, "[FIL:ERR] Uncorrectable page detected during scattered read; continuing.\n");
                break;
            case FIL_SUCCESS_CLEAN:
                nRet = FIL_SUCCESS_CLEAN;
                cleanCount++;
                break;
            default:
                nRet = FIL_CRITICAL_ERROR;
                WMR_PRINT(ERROR, "[FIL:ERR] Hardware error detected during scattered read: 0x%08x\n", singleStatus);
                break;
        }

        if (pbMaxCorrectedBits && (maxCorrectedBits > *pbMaxCorrectedBits))
        {
            // Trace max corrected bits accross all pages in this operation
            *pbMaxCorrectedBits = maxCorrectedBits;
        }

            if (pdwaSectorStats)
            {
                pdwaSectorStats += _GetSectorsPerPage();
            }
    
            dataPtr += _GetBytesPerPageMain();
            metaPtr += g_fmi0.total_bytes_per_meta;
    }

    if (cleanCount && (cleanCount != wNumOfPagesToRead))
    {
        nRet = FIL_U_ECC_ERROR;
    }

    return nRet;
}

Int32 h2fmiReadBootpage(UInt16 wVirtualCE,
                        UInt32 dwPpn,
                        UInt8* pabData)
{
    Int32 nRet;
    UInt32 status;
    h2fmi_t*   fmi     = h2fmiTranslateVirtualCEToBus( wVirtualCE );
    h2fmi_ce_t ce      = h2fmiTranslateVirtualCEToCe( wVirtualCE );

    WMR_ASSERT(WMR_CHECK_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE));
    WMR_PREPARE_READ_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    status = h2fmi_read_bootpage_pio(fmi, ce, dwPpn, pabData, NULL);
    
    switch (status)
    {
        case _kIOPFMI_STATUS_BLANK:
            WMR_PRINT(READ, "[FIL:LOG] Clean Page\n");
            nRet = FIL_SUCCESS_CLEAN;
            break;
        case _kIOPFMI_STATUS_SUCCESS:
            WMR_PRINT(READ, "[FIL:LOG] Good page\n");
            nRet = FIL_SUCCESS;
            break;
        case _kIOPFMI_STATUS_AT_LEAST_ONE_UECC:
            WMR_PRINT(ERROR, "UECC ce %d page 0x%08x\n", (UInt32) wVirtualCE, dwPpn);
            nRet = FIL_U_ECC_ERROR;
            break;
        case _kIOPFMI_STATUS_FUSED:
        case _kIOPFMI_STATUS_NOT_ALL_CLEAN:
            nRet = FIL_U_ECC_ERROR;
            break;
        default:
            WMR_PRINT(ERROR, "[FIL:ERR] Hardware error: 0x%08x\n", status);
            nRet = FIL_CRITICAL_ERROR;
            break;
    }
    
    WMR_COMPLETE_READ_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    return nRet;

}


Int32 h2fmiReadSinglePage(UInt16 wVirtualCE,
                          UInt32 dwPpn,
                          UInt8* pabDataBuf,
                          UInt8* pabMetaBuf,
                          UInt8* pbMaxCorrectedBits,
                          UInt8* pdwaSectorStats,
                          BOOL32 bDisableWhitening)
{
    Int32 nRet      = FIL_SUCCESS;
    UInt32 status;
    UInt32 fringe_idx;
    h2fmi_t* fmi    = h2fmiTranslateVirtualCEToBus( wVirtualCE );
    h2fmi_ce_t ce   = h2fmiTranslateVirtualCEToCe( wVirtualCE );
    const UInt32 fringe_count = fmi->total_bytes_per_meta - fmi->valid_bytes_per_meta;
    UInt8 *pabMetaBounceBuffer = (UInt8*) g_pScratchBuf;
    
    WMR_PRINT(READ, "[FIL:LOG] h2fmiReadSinglePage()\n");

    AND_FIL_STAT(stFILStatistics.ddwReadWithECCCallCnt++);
    AND_FIL_STAT(stFILStatistics.ddwPagesReadCnt++);

    if (pbMaxCorrectedBits)
    {
        *pbMaxCorrectedBits = 0;
    }

    WMR_ASSERT(WMR_CHECK_BUFFER(pabDataBuf, fmi->bytes_per_page));
    WMR_PREPARE_READ_BUFFER(pabDataBuf, fmi->bytes_per_page);
    WMR_PREPARE_READ_BUFFER(pabMetaBounceBuffer, fmi->total_bytes_per_meta);

    h2fmi_choose_aes((useWhitening && !bDisableWhitening), FALSE32, pabDataBuf, fmi, &dwPpn);

    status = h2fmi_read_page(fmi,
                             ce,
                             dwPpn,
                             pabDataBuf,
                             pabMetaBounceBuffer,
                             pbMaxCorrectedBits,
                             pdwaSectorStats);

    WMR_MEMCPY(pabMetaBuf, pabMetaBounceBuffer, fmi->total_bytes_per_meta);
    if (useWhiteningMetadata)
    {
        h2fmi_decrypt_metadata(dwPpn, pabMetaBuf);
    }

    switch (status)
    {
        case _kIOPFMI_STATUS_BLANK:
            WMR_PRINT(READ, "[FIL:LOG] Clean Page\n");
            nRet = FIL_SUCCESS_CLEAN;
            break;
        case _kIOPFMI_STATUS_SUCCESS:
            WMR_PRINT(READ, "[FIL:LOG] Good page\n");
            // this allows compatibility of newer 10-byte metadata
            // region with code expecting older 12-byte metadata
            for (fringe_idx = 0; fringe_idx < fringe_count; fringe_idx++)
            {
                pabMetaBuf[fmi->valid_bytes_per_meta + fringe_idx] = 0xFF;
            }
            break;
        case _kIOPFMI_STATUS_AT_LEAST_ONE_UECC:
            WMR_PRINT(ERROR, "UECC ce %d page 0x%08x\n", (UInt32) wVirtualCE, dwPpn);
            nRet = FIL_U_ECC_ERROR;
            break;
        case _kIOPFMI_STATUS_FUSED:
        case _kIOPFMI_STATUS_NOT_ALL_CLEAN:
            nRet = FIL_U_ECC_ERROR;
            break;
        default:
            WMR_PRINT(ERROR, "[FIL:ERR] Hardware error: 0x%08x\n", status);
            nRet = FIL_CRITICAL_ERROR;
            break;
    }

    WMR_COMPLETE_READ_BUFFER(pabDataBuf, fmi->bytes_per_page);
    WMR_COMPLETE_READ_BUFFER(pabMetaBounceBuffer, fmi->total_bytes_per_meta);

    return nRet;
}

Int32 h2fmiReadSinglePageMaxECC(UInt16 wVirtualCE,
                                UInt32 dwPpn,
                                UInt8* pabDataBuf)
{
    Int32 nRet    = FIL_SUCCESS;

    h2fmi_t* fmi    = h2fmiTranslateVirtualCEToBus( wVirtualCE );
    h2fmi_ce_t ce   = h2fmiTranslateVirtualCEToCe( wVirtualCE );
    UInt32 status;

    WMR_PRINT(READ, "[FIL:LOG] h2fmiReadSinglePageMaxECC()\n");

    AND_FIL_STAT(stFILStatistics.ddwReadMaxECCCallCnt++);

    WMR_PREPARE_READ_BUFFER(pabDataBuf, fmi->bytes_per_page);

    h2fmi_choose_aes(useWhitening, FALSE32, pabDataBuf, fmi, &dwPpn);

    status = h2fmi_read_page(fmi,
                             ce,
                             dwPpn,
                             pabDataBuf,
                             (UInt8*)g_pScratchBuf,
                             NULL,
                             NULL);

    switch (status)
    {
        case _kIOPFMI_STATUS_SUCCESS:
            nRet = FIL_SUCCESS;
            break;
        case _kIOPFMI_STATUS_FUSED:
        case _kIOPFMI_STATUS_AT_LEAST_ONE_UECC:
        case _kIOPFMI_STATUS_NOT_ALL_CLEAN:
            nRet = FIL_U_ECC_ERROR;
            break;
        case _kIOPFMI_STATUS_BLANK:
            nRet = FIL_SUCCESS_CLEAN;
            break;
        default:
            nRet = FIL_CRITICAL_ERROR;
            break;
    }

    WMR_COMPLETE_READ_BUFFER(pabDataBuf, fmi->bytes_per_page);

    return nRet;
}

void h2fmiCalcCurrentTimings()
{
/*
    h2fmi_calc_bus_timings(......);

    h2fmi_calc_bus_timings(fmi,
                           stDEVInfo[wDevIdx].wReadCycleTime,
                           stDEVInfo[wDevIdx].wReadSetupTime,
                           stDEVInfo[wDevIdx].wReadHoldTime,
                           stDEVInfo[wDevIdx].wWriteCycleTime,
                           stDEVInfo[wDevIdx].wWriteSetupTime,
                           stDEVInfo[wDevIdx].wWriteHoldTime)
 */
}

void h2fmiSetWhiteningState(BOOL32 enable)
{
    useWhitening = enable;
}

void h2fmiSetWhiteningMetadataState(BOOL32 enable)
{
    useWhiteningMetadata = enable;
}

Int32 h2fmiPrintParameters(void){

    h2fmi_t *iter[] = {&g_fmi0, &g_fmi1, NULL};
    h2fmi_ppn_device_params_t *params;
    int i;
    Int32 err = 0;

    for(i=0; iter[i] != NULL; i++){
        if (iter[i]->num_of_ce && iter[i]->is_ppn){

            params = &(iter[i]->ppn->device_params);

            _WMR_PRINT("Channel: %d\n",i); 
            DUMP_VAR(params->caus_per_channel);
            DUMP_VAR(params->cau_bits);
            DUMP_VAR(params->blocks_per_cau);
            DUMP_VAR(params->block_bits);
            DUMP_VAR(params->pages_per_block);
            DUMP_VAR(params->pages_per_block_slc);
            DUMP_VAR(params->page_address_bits);
            DUMP_VAR(params->address_bits_bits_per_cell);
            DUMP_VAR(params->default_bits_per_cell);
            DUMP_VAR(params->page_size);
            DUMP_VAR(params->dies_per_channel);
            DUMP_VAR(params->block_pairing_scheme);
            DUMP_VAR(params->bytes_per_row_address);   
 
            DUMP_VAR(params->tRC);
            DUMP_VAR(params->tREA);
            DUMP_VAR(params->tREH);
            DUMP_VAR(params->tRHOH);
            DUMP_VAR(params->tRHZ);
            DUMP_VAR(params->tRLOH);
            DUMP_VAR(params->tRP);
            DUMP_VAR(params->tWC);
            DUMP_VAR(params->tWH);
            DUMP_VAR(params->tWP);
     
            DUMP_VAR(params->read_queue_size);
            DUMP_VAR(params->program_queue_size);
            DUMP_VAR(params->erase_queue_size);
            DUMP_VAR(params->prep_function_buffer_size);
     
            DUMP_VAR(params->tRST_ms);
            DUMP_VAR(params->tPURST_ms);
            DUMP_VAR(params->tSCE_ms);
            DUMP_VAR(params->tCERDY_us);

            DUMP_VAR(params->cau_per_channel2);
            DUMP_VAR(params->dies_per_channel2);
            DUMP_VAR(params->ddr_tRC);
            DUMP_VAR(params->ddr_tREH);
            DUMP_VAR(params->ddr_tRP);
            DUMP_VAR(params->tDQSL_ps);
            DUMP_VAR(params->tDQSH_ps);
            DUMP_VAR(params->tDSC_ps);
            DUMP_VAR(params->tDQSRE_ps);
            DUMP_VAR(params->tDQSQ_ps);
            DUMP_VAR(params->tDVW_ps);
            DUMP_VAR(params->max_interface_speed);
        }
    }
    return err;
}


// =============================================================================
// non-readonly public interface function definitions (not normally
// enabled in iBoot)
// =============================================================================
#if (!H2FMI_READONLY)
Int32 h2fmiWriteMultiplePages(UInt32* padwPpn,
                              UInt8*  pabDataBuf,
                              UInt8*  pabMetaBuf,
                              UInt16  wNumOfPagesToWrite,
                              BOOL32  boolAligned,
                              BOOL32  boolCorrectBank,
                              UInt16* pwFailingCE,
                              BOOL32  bDisableWhitening,
                              UInt32* pwFailingPageNum)
{
    UInt16 wIdx, wBankIdx, wVirtualCEIdx;
    Int32 errStatus = FIL_SUCCESS;

    WMR_PRINT(WRITE, "[FIL:LOG] h2fmiWriteMultiplePages()\n");

    AND_FIL_STAT(stFILStatistics.ddwWriteMultipleCallCnt++);

    // It is assumed by the caller that WriteMultiple will operate on a multiple of
    // bankCount and the controller will write to a bank on each CE before writing
    // to the second bank on the first CE.
    // Note that this swizzling is meant to optimize the overlap case, so the
    // loop (performed via singles) below is a bit redundant
    for (wIdx = 0; wIdx < wNumOfPagesToWrite; wIdx += _GetNumOfBanks())
    {
        for (wBankIdx = 0; wBankIdx < _GetBanksPerCS(); wBankIdx++)
        {
            for (wVirtualCEIdx = 0; wVirtualCEIdx < _GetNumOfCS(); wVirtualCEIdx++)
            {
                const UInt16 wAddrIdx = wIdx + (wBankIdx * _GetNumOfCS()) + wVirtualCEIdx;
                const h2fmi_t *fmi = h2fmiTranslateVirtualCEToBus(wVirtualCEIdx);
                UInt8* tmpDataBuf = pabDataBuf + (wAddrIdx * _GetBytesPerPageMain());
                UInt8* tmpMetaBuf = pabMetaBuf + (wAddrIdx * fmi->total_bytes_per_meta);

                errStatus = h2fmiWriteSinglePage(wVirtualCEIdx,
                                                 padwPpn[wBankIdx * _GetNumOfCS() + wVirtualCEIdx] + (wIdx / _GetNumOfBanks()),
                                                 tmpDataBuf,
                                                 tmpMetaBuf,
                                                 bDisableWhitening);

                if (FIL_SUCCESS != errStatus)
                {
                    if (pwFailingCE)
                    {
                        *pwFailingCE = wVirtualCEIdx;
                    }
                    if (pwFailingPageNum)
                    {
                        *pwFailingPageNum = padwPpn[wBankIdx * _GetNumOfCS() + wVirtualCEIdx] + (wIdx / _GetNumOfBanks());
                    }
                    return errStatus;
                }
            }
        }
    }

    return errStatus;
}

Int32 h2fmiWriteScatteredPages(UInt16 *pawBanks,
                               UInt32 *padwPpn,
                               UInt8  *pabDataBuf,
                               UInt8  *pabMetaBuf,
                               UInt16  wNumOfPagesToWrite,
                               UInt16 *pawFailingCE,
                               BOOL32  bDisableWhitening,
                               UInt32 *pwFailingPageNum)
{
    UInt32    i;
    Int32     ret = FIL_SUCCESS;

    for (i = 0; i < wNumOfPagesToWrite; i++)
    {
        const UInt16 bank = pawBanks[i];
        const UInt32 page = padwPpn[i];
        const h2fmi_t *fmi = h2fmiTranslateVirtualCEToBus(bank);
        UInt8 *dataPtr = pabDataBuf + (i * _GetBytesPerPageMain());
        UInt8 *metaPtr = pabMetaBuf + (i * fmi->total_bytes_per_meta);

        ret = h2fmiWriteSinglePage(bank,
                                   page,
                                   dataPtr,
                                   metaPtr,
                                   bDisableWhitening);
        if (ret != FIL_SUCCESS)
        {
            if (pawFailingCE)
            {
                *pawFailingCE = bank;
            }
            if (pwFailingPageNum)
            {
                *pwFailingPageNum = page;
            }
            return ret;
        }
    }

    return ret;
}


Int32 h2fmiWriteSinglePage(UInt16 wVirtualCE,
                           UInt32 dwPpn,
                           UInt8* pabDataBuf,
                           UInt8* pabMetaBuf,
                           BOOL32 bDisableWhitening)
{
    UInt8 *pabMetaBounceBuffer = (UInt8*) g_pScratchBuf;
    Int32 result  = FIL_SUCCESS;
    h2fmi_t* fmi  = h2fmiTranslateVirtualCEToBus( wVirtualCE );
    h2fmi_ce_t ce = h2fmiTranslateVirtualCEToCe( wVirtualCE );
    BOOL32 status_failed = FALSE32;
    const UInt32 bytes_per_page = fmi->sectors_per_page * H2FMI_BYTES_PER_SECTOR;

    WMR_PRINT(WRITE, "[FIL:LOG] h2fmiWriteSinglePage(%d, 0x%06X, ...)\n", wVirtualCE, dwPpn);

    AND_FIL_STAT(stFILStatistics.ddwPagesWrittenCnt++);
    AND_FIL_STAT(stFILStatistics.ddwWriteSingleCallCnt++);

    WMR_ASSERT(WMR_CHECK_BUFFER(pabDataBuf, fmi->bytes_per_page));
    
    // Don't touch the caller's buffer
    WMR_MEMCPY(pabMetaBounceBuffer, pabMetaBuf, fmi->valid_bytes_per_meta);

    if (useWhiteningMetadata)
    {
        h2fmi_encrypt_metadata(dwPpn, pabMetaBounceBuffer);
    }
    
    h2fmi_choose_aes((useWhitening && !bDisableWhitening), TRUE32, pabDataBuf, fmi, &dwPpn);

    WMR_PREPARE_WRITE_BUFFER(pabDataBuf, bytes_per_page);
    WMR_PREPARE_WRITE_BUFFER(pabMetaBounceBuffer, fmi->valid_bytes_per_meta);


    if (!h2fmi_write_page(fmi,
                          ce,
                          dwPpn,
                          pabDataBuf,
                          pabMetaBounceBuffer,
                          &status_failed))
    {
        if (status_failed)
        {
            result = FIL_WRITE_FAIL_ERROR;
        }
        else
        {
            result = FIL_CRITICAL_ERROR;
        }
    }

    return result;
}

Int32 h2fmiWriteSinglePageMaxECC(UInt16 wVirtualCE,
                             UInt32 dwPpn,
                             UInt8* pabDataBuf)
{
    const h2fmi_t *fmi = h2fmiTranslateVirtualCEToBus(wVirtualCE);
    UInt8 *pabMaxECCMeta = (UInt8*) g_pScratchBuf;
    AND_FIL_STAT(stFILStatistics.ddwWriteMaxECCCallCnt++);

    // g_pScratchBuf is re-used for metadata whitening, but we dont really care
    // what this data ends up being
    WMR_MEMSET(pabMaxECCMeta, 0xA5, fmi->total_bytes_per_meta);

    return h2fmiWriteSinglePage(wVirtualCE, dwPpn,pabDataBuf, pabMaxECCMeta, FALSE32);
}

#endif // !H2FMI_READONLY

// =============================================================================
// non-readonly public interface function definitions that must be available
// to support NAND-backed NVRAM implementation
// =============================================================================

#if (!H2FMI_READONLY || AND_SUPPORT_NVRAM)

Int32 h2fmiWriteBootpage(UInt16 wVirtualCE,
                         UInt32 dwPpn,
                         UInt8 *pabData)
{
    Int32      result = FIL_SUCCESS;
    UInt32     status;
    h2fmi_t*   fmi  = h2fmiTranslateVirtualCEToBus( wVirtualCE );
    h2fmi_ce_t ce   = h2fmiTranslateVirtualCEToCe( wVirtualCE );

    WMR_ASSERT(WMR_CHECK_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE));
    WMR_PREPARE_WRITE_BUFFER(pabData, H2FMI_BOOT_BYTES_PER_PAGE);

    status = h2fmi_write_bootpage(fmi, ce, dwPpn, pabData);
    switch (status)
    {
        case _kIOPFMI_STATUS_SUCCESS:
            result = FIL_SUCCESS;
            break;

        case _kIOPFMI_STATUS_PGM_ERROR:
            WMR_PRINT(ERROR, "WSF ce %d page 0x%08x\n", (UInt32) wVirtualCE, dwPpn);
            result = FIL_WRITE_FAIL_ERROR;
            break;

        default:
            result = FIL_CRITICAL_ERROR;
            break;
    }

    return result;
}

Int32 h2fmiEraseSingleBlock(UInt16 wVirtualCE,
                            UInt32 wPbn)
{
    Int32 result  = FIL_SUCCESS;
    BOOL32 status_failed = FALSE32;

    h2fmi_t*   fmi  = h2fmiTranslateVirtualCEToBus( wVirtualCE );
    UInt16 ce = (UInt16)h2fmiTranslateVirtualCEToCe( wVirtualCE );

    WMR_PRINT(ERASE, "[FIL:LOG] h2fmiEraseSingleBlock()\n");

    AND_FIL_STAT( stFILStatistics.ddwSingleEraseCallCnt++);
    AND_FIL_STAT( stFILStatistics.ddwBlocksErasedCnt++);

    fmi->failureDetails.wCEStatusArray = &fmi->failureDetails.wSingleCEStatus;

    if (!h2fmi_erase_blocks(fmi, 1, &ce, &wPbn, &status_failed))
    {
        if (status_failed)
        {
            WMR_PRINT(ERROR, "ESF ce %d block %d\n", (UInt32) wVirtualCE, wPbn);
            AND_FIL_STAT( stFILStatistics.ddwEraseNANDErrCnt++);
            result = FIL_WRITE_FAIL_ERROR;
        }
        else
        {
            result = FIL_CRITICAL_ERROR;
        }
    }

    return result;
}

#endif // (!H2FMI_READONLY || AND_SUPPORT_NVRAM)

// =============================================================================
// private implementation function definitions
// =============================================================================

static BOOL32 h2fmi_init_minimal(h2fmi_t* fmi,
                                 UInt32   interface)
{
    BOOL32 result = TRUE32;
    
    // only interface numbers 0 and 1 are valid
    if ((interface != 0) && (interface != 1))
    {
        h2fmi_fail(result);
    }
    else
    {
#if SUPPORT_PPN
        if (NULL != fmi->ppn)
        {
            WMR_FREE(fmi->ppn, sizeof(*fmi->ppn));
        }
#endif
        WMR_MEMSET(fmi, 0, sizeof(*fmi));
        // record which interface is being used
        fmi->regs   = (interface == 0) ? FMI0 : FMI1;
        fmi->bus_id = interface;
#if SUPPORT_TOGGLE_NAND
        fmi->is_toggle_system = FALSE32;
        fmi->is_toggle = FALSE32;
#endif
        
        // "get the clock rolling" for the dedicated HCLK clock domain
        WMR_CLOCK_GATE(h2fmi_get_gate(fmi), TRUE32);

        // configure safe initial bus timings
        h2fmi_set_initial_timings(fmi);
        fmi->activeCe = ~0;
        
        // reset the FMI block (not NAND)
        h2fmi_reset(fmi);
        
        h2fmi_init_sys(fmi);
    } 
    
    return result;
}

static void h2fmi_set_initial_timings(h2fmi_t* fmi)
{
    fmi->if_ctrl = H2FMI_IF_CTRL_LOW_POWER;
    fmi->if_ctrl_normal = fmi->if_ctrl;

#if SUPPORT_TOGGLE_NAND
    fmi->dqs_ctrl = FMC_DQS_TIMING_CTRL__DEFAULT_VAL;
    fmi->timing_ctrl_1 = (FMC_TOGGLE_CTRL_1_DDR_RD_PRE_TIME(FMC_TOGGLE_CTRL_1__TIMING_MAX_CLOCKS) |
                          FMC_TOGGLE_CTRL_1_DDR_RD_POST_TIME(FMC_TOGGLE_CTRL_1__TIMING_MAX_CLOCKS) |
                          FMC_TOGGLE_CTRL_1_DDR_WR_PRE_TIME(FMC_TOGGLE_CTRL_1__TIMING_MAX_CLOCKS) |
                          FMC_TOGGLE_CTRL_1_DDR_WR_POST_TIME(FMC_TOGGLE_CTRL_1__TIMING_MAX_CLOCKS));
    fmi->timing_ctrl_2 = (FMC_TOGGLE_CTRL_2_CE_SETUP_TIME(FMC_TOGGLE_CTRL_2__TIMING_MAX_CLOCKS) |
                          FMC_TOGGLE_CTRL_2_CE_HOLD_TIME(FMC_TOGGLE_CTRL_2__TIMING_MAX_CLOCKS) |
                          FMC_TOGGLE_CTRL_2_NAND_TIMING_ADL(FMC_TOGGLE_CTRL_2__TIMING_MAX_CLOCKS) |
                          FMC_TOGGLE_CTRL_2_NAND_TIMING_WHR(FMC_TOGGLE_CTRL_2__TIMING_MAX_CLOCKS));
    fmi->toggle_if_ctrl = (FMC_IF_CTRL__REB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                           FMC_IF_CTRL__WEB_SETUP(FMC_IF_CTRL__TIMING_MAX_CLOCKS) |
                           FMC_IF_CTRL__WEB_HOLD(FMC_IF_CTRL__TIMING_MAX_CLOCKS));
#endif

    restoreTimingRegs(fmi);   
}

static void
h2fmi_init_raw_state(h2fmi_t*        fmi,
                     const NandInfo* nandInfo,
                     NandRequiredTimings *requiredTiming)
{
    fmi->blocks_per_ce        = nandInfo->geometry->blocksPerCS;
    fmi->banks_per_ce         = 1; // updated later by SetStruct
    fmi->sectors_per_page     = nandInfo->geometry->dataBytesPerPage / H2FMI_BYTES_PER_SECTOR;
    fmi->pages_per_block      = nandInfo->geometry->pagesPerBlock;
    fmi->bytes_per_spare      = nandInfo->geometry->spareBytesPerPage;
    fmi->valid_bytes_per_meta = nandInfo->format->validMetaPerLogicalPage;
    fmi->total_bytes_per_meta = nandInfo->format->metaPerLogicalPage;
    fmi->bytes_per_page       = nandInfo->geometry->dataBytesPerPage;
    fmi->dies_per_cs          = nandInfo->geometry->diesPerCS;
    fmi->clock_speed_khz      = WMR_BUS_FREQ_HZ() / 1000;
    if (nandInfo->format->logicalPageSize == 0)
    {
        fmi->logical_page_size = fmi->bytes_per_page;
    }
    else
    {
        fmi->logical_page_size = nandInfo->format->logicalPageSize;
    }

    WMR_MEMSET(requiredTiming, 0, sizeof(*requiredTiming));

    requiredTiming->clock_hz = WMR_BUS_FREQ_HZ();

    requiredTiming->soc_tx_prop_ns = FMI_TX_PROP_DELAY_NS;
    requiredTiming->soc_rx_prop_ns = FMI_RX_PROP_DELAY_NS;

    requiredTiming->soc_to_nand_rise_ns = nandInfo->boardInfo->socToNandRiseNanosecs;
    requiredTiming->soc_to_nand_fall_ns = nandInfo->boardInfo->socToNandFallNanosecs;
    requiredTiming->nand_to_soc_rise_ns = nandInfo->boardInfo->nandToSocRiseNanosecs;
    requiredTiming->nand_to_soc_fall_ns = nandInfo->boardInfo->nandToSocFallNanosecs;

    requiredTiming->tRC_ns = nandInfo->configTiming->readCycleNanosecs;
    requiredTiming->tRP_ns = nandInfo->configTiming->readSetupNanosecs;
    requiredTiming->tREH_ns = nandInfo->configTiming->readHoldNanosecs;
    requiredTiming->tREA_ns = nandInfo->configTiming->readDelayNanosecs;
    requiredTiming->tRHOH_ns = nandInfo->configTiming->readValidNanosecs;
    requiredTiming->tRLOH_ns = 0;

    requiredTiming->tWC_ns = nandInfo->configTiming->writeCycleNanosecs;
    requiredTiming->tWP_ns = nandInfo->configTiming->writeSetupNanosecs;
    requiredTiming->tWH_ns = nandInfo->configTiming->writeHoldNanosecs;

    // Cache ECC configuration
    fmi->correctable_bits = h2fmi_calculate_ecc_bits(fmi);
    if (fmi->correctable_bits > 8)
    {
        fmi->refresh_threshold_bits = (fmi->correctable_bits * 8) / 10;
    }
    else
    {
        fmi->refresh_threshold_bits = 8;
    }

    fmi->fmi_config_reg    = h2fmi_calculate_fmi_config(fmi);
#if FMI_VERSION > 0
    fmi->fmi_data_size_reg =  h2fmi_calculate_fmi_data_size(fmi);
#endif /* FMI_VERSION > 0*/

#if FMISS_ENABLED
    fmiss_raw_init_sequences(fmi);
#endif /* FMISS_ENABLED */
}

void
h2fmiRegisterCurrentTransaction(UInt64 ddwLba, UInt32 dwNum, void* pDest)
{
    fmi_current_lba_base = ddwLba;
    fmi_current_dest_ptr = (UInt8*)pDest;
    fmi_current_num_blks = dwNum;
    WMR_PRINT(CRYPTO, "Registered %lld %d %p\n", ddwLba, dwNum, pDest);
}

static void h2fmi_choose_aes(BOOL32 enable, BOOL32 encrypt, void* destPtr, h2fmi_t* fmi, UInt32* seeds)
{
    const UInt8* endPtr = fmi_current_dest_ptr + (fmi_current_num_blks * fmi->bytes_per_page);
    const UInt8* pabDestPtr = (UInt8*)destPtr;
    if (!enable)
    {
        fmi->current_aes_cxt = NULL;
    }
    else if (IS_ADDRESS_IN_RANGE(pabDestPtr, fmi_current_dest_ptr, endPtr))
    {
        WMR_PRINT(CRYPTO, "default enc dest: 0x%08x range: 0x%08x - 0x%08x\n", pabDestPtr, fmi_current_dest_ptr, endPtr);
        h2fmi_setup_default_encryption(encrypt, destPtr, fmi);
    }
    else
    {
        WMR_PRINT(CRYPTO, "dest: 0x%08x range: 0x%08x - 0x%08x\n", pabDestPtr, fmi_current_dest_ptr, endPtr);
        h2fmi_setup_whitening(encrypt, fmi, seeds);
    }
}

static void
h2fmi_setup_default_encryption(BOOL32 encrypt, void* destPtr, h2fmi_t* fmi)
{

    fmi->aes_cxt.chunk_size = fmi->logical_page_size;
    fmi->aes_cxt.iv_func_arg = destPtr;
    fmi->aes_cxt.iv_func = h2fmi_calc_default_iv;
    fmi->aes_cxt.keying = (AES_KEY_TYPE_USER | AES_KEY_SIZE_128);
    fmi->aes_cxt.key = nand_default_key;
    fmi->aes_cxt.command = encrypt ? AES_CMD_ENC : AES_CMD_DEC;
    
    fmi->current_aes_iv_array = NULL;
    
    fmi->current_aes_cxt = &fmi->aes_cxt;

}

#define LFSR32(seed) ((seed) & 1) ? ((seed) >> 1) ^ 0x80000061 : ((seed) >> 1)
void h2fmi_calc_default_iv(void* arg, UInt32 chunk_index, void* iv_buffer)
{
    UInt32* iv = (UInt32*)iv_buffer;
    UInt8* dest_addr = (UInt8*)arg;
    const UInt32 offset = (dest_addr - fmi_current_dest_ptr) / (stDeviceInfo.wSectorsPerPage * H2FMI_BYTES_PER_SECTOR);
    const UInt32 current_lba = fmi_current_lba_base + offset;

    WMR_PRINT(CRYPTO, "Decrypting lba %d\n", current_lba);

    iv[0] = LFSR32(current_lba);
    iv[1] = LFSR32(iv[0]);
    iv[2] = LFSR32(iv[1]);
    iv[3] = LFSR32(iv[2]);
}

static void
h2fmi_setup_whitening(BOOL32 encrypt, h2fmi_t* fmi, UInt32* seeds)
{

    fmi->aes_cxt.chunk_size = fmi->logical_page_size;
    fmi->aes_cxt.iv_func_arg = fmi;
    fmi->aes_cxt.iv_func = h2fmi_aes_iv;
    fmi->aes_cxt.keying = (AES_KEY_TYPE_USER | AES_KEY_SIZE_128);
    fmi->aes_cxt.key = nand_whitening_key;
    fmi->aes_cxt.command = encrypt ? AES_CMD_ENC : AES_CMD_DEC;
    
    fmi->current_aes_iv_array = NULL;
    fmi->iv_seed_array = seeds;

    fmi->current_aes_cxt = &fmi->aes_cxt;

}

static UInt32
h2fmi_generate_meta_content(void)
{
    UInt32 idx;
    for (idx = 0; idx < METADATA_ITERATIONS_TABLE; idx++)
    {
        metaContent = (METADATA_MULTIPLY * metaContent) + METADATA_INCREMENT;
    }
    return metaContent;
}

static void
h2fmi_generate_meta_table(void)
{
    UInt32 i;
    metaContent = 0x50F4546A; // randomly chosen by a fair roll of ssh-keygen

    for (i = 0; i < METADATA_LOOKUP_SIZE; i++)
    {
        metaLookupTable[i] = h2fmi_generate_meta_content();
    }
}

static void
h2fmi_encrypt_metadata(UInt32 page, UInt8* pabMetaBuf)
{
    UInt8 i;
    for (i = 0; i < METADATA_ITERATIONS_CRYPT; i++)
    {
        ((UInt32*)pabMetaBuf)[i] ^= metaLookupTable[(page + i) % METADATA_LOOKUP_SIZE];
    }
}

static void
h2fmi_decrypt_metadata(UInt32 page, UInt8* pabMetaBuf)
{
    UInt8 i;
    for (i = 0; i < METADATA_ITERATIONS_CRYPT; i++)
    {
        ((UInt32*)pabMetaBuf)[i] ^= metaLookupTable[(page + i) % METADATA_LOOKUP_SIZE];
    }
}

#if (defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)
static BOOL32 h2fmiGetPPNUID(PpnUIDReadRequest* ppnUIDRead, UInt32* pdwStructSize, BOOL32* setPattern, BOOL32* toBreak)
{    
    BOOL32 boolRes = FALSE32;
    if ((ppnUIDRead==NULL) || (ppnUIDRead->buf==NULL) || (pdwStructSize==NULL) || (*pdwStructSize < sizeof(PpnUIDReadRequest)))
    {
        *toBreak = TRUE32;
        return FALSE32;
    }
    else if((ppnUIDRead->ce < _GetNumOfCS()) && (ppnUIDRead->die < _GetDiesPerCS()))
    {
        UInt8 phyCE = h2fmiTranslateVirtualCEToCe(ppnUIDRead->ce);
        h2fmi_t *whichFmi = h2fmiTranslateVirtualCEToBus(ppnUIDRead->ce);
		
        if(whichFmi)
        {
            boolRes = h2fmi_ppn_get_uid(whichFmi, phyCE, ppnUIDRead->die, ppnUIDRead->buf);
        }
        *pdwStructSize = sizeof(PpnUIDReadRequest);
    }
    else
    {
        *setPattern = TRUE32;
    }
    return boolRes;
}

static BOOL32 h2fmiGetRAWUID(GenericReadRequest* genericRead, UInt32* pdwStructSize, BOOL32* setPattern, BOOL32* toBreak)
{
    BOOL32 boolRes = FALSE32;
    if ((genericRead==NULL) || (genericRead->buf==NULL) || (pdwStructSize==NULL) || (*pdwStructSize < sizeof(GenericReadRequest)))
    {
        *toBreak = TRUE32;
        return FALSE32;
    }
    if (genericRead->ce >= _GetNumOfCS())
    {
        *setPattern = TRUE32;  
    }
    else
    {
        UInt8 phyCE = h2fmiTranslateVirtualCEToCe(genericRead->ce);
        h2fmi_t *whichFmi = h2fmiTranslateVirtualCEToBus(genericRead->ce);
        boolRes = h2fmiGenericNandReadSequence(whichFmi, phyCE, genericRead); 
    }
    *pdwStructSize = sizeof(GenericReadRequest);
    return boolRes;
}
#endif //(defined(ENABLE_VENDOR_UNIQUE_QUERIES) && ENABLE_VENDOR_UNIQUE_QUERIES)

void h2fmi_ppn_all_channel_power_state_transition(UInt32 ps_tran)
{
    if (g_fmi0.num_of_ce)
    {
        h2fmi_ppn_set_channel_power_state(&g_fmi0, ps_tran);
    }
    if (g_fmi1.num_of_ce)
    {
        h2fmi_ppn_set_channel_power_state(&g_fmi1, ps_tran);
    }
}


#if SUPPORT_TOGGLE_NAND
void transitionWorldFromDDR(UInt32 powerstate_to)
{
    WMR_ASSERT(g_fmi0.is_toggle || (0 == g_fmi0.num_of_ce));
    WMR_ASSERT(g_fmi1.is_toggle || (0 == g_fmi1.num_of_ce));
    WMR_ASSERT(g_fmi0.is_toggle_system || (0 == g_fmi0.num_of_ce));
    WMR_ASSERT(g_fmi1.is_toggle_system || (0 == g_fmi1.num_of_ce));

    h2fmi_ppn_all_channel_power_state_transition(PPN_PS_TRANS_DDR_TO_LOW_POWER);

    if (0 != g_fmi0.num_of_ce)
    {
        g_fmi0.is_toggle = FALSE32;
        h2fmi_reset(&g_fmi0);
    }
    if (0 != g_fmi1.num_of_ce)
    {
        g_fmi1.is_toggle = FALSE32;
        h2fmi_reset(&g_fmi1);
    }

    if (powerstate_to == PPN_FEATURE__POWER_STATE__ASYNC)
    {
        h2fmi_ppn_all_channel_power_state_transition(PPN_PS_TRANS_LOW_POWER_TO_ASYNC);;
    }

#if H2FMI_PPN_VERIFY_SET_FEATURES
    if(g_fmi0.num_of_ce)
    {
        h2fmi_ppn_verify_feature_shadow(&g_fmi0);
    }
    if(g_fmi1.num_of_ce)
    {
        h2fmi_ppn_verify_feature_shadow(&g_fmi1);
    }
#endif // H2FMI_PPN_VERIFY_SET_FEATURES
}

void transitionWorldToDDR(UInt32 powerstate_from)
{
    WMR_ASSERT(!g_fmi0.is_toggle || (0 == g_fmi0.num_of_ce));
    WMR_ASSERT(!g_fmi1.is_toggle || (0 == g_fmi1.num_of_ce));
    WMR_ASSERT(g_fmi0.is_toggle_system || (0 == g_fmi0.num_of_ce));
    WMR_ASSERT(g_fmi1.is_toggle_system || (0 == g_fmi1.num_of_ce));

    if (powerstate_from == PPN_FEATURE__POWER_STATE__ASYNC)
    {
        h2fmi_ppn_all_channel_power_state_transition(PPN_PS_TRANS_ASYNC_TO_LOW_POWER);
    }

    h2fmi_ppn_all_channel_power_state_transition(PPN_PS_TRANS_LOW_POWER_TO_DDR);

    if (0 != g_fmi0.num_of_ce)
    {
        g_fmi0.is_toggle = TRUE32;
        h2fmi_reset(&g_fmi0);
    }
    if (0 != g_fmi1.num_of_ce)
    {
        g_fmi1.is_toggle = TRUE32;
        h2fmi_reset(&g_fmi1);
    }

#if H2FMI_PPN_VERIFY_SET_FEATURES
    if(g_fmi0.num_of_ce)
    {
        h2fmi_ppn_verify_feature_shadow(&g_fmi0);
    }
    if(g_fmi1.num_of_ce)
    {
        h2fmi_ppn_verify_feature_shadow(&g_fmi1);
    }
#endif // H2FMI_PPN_VERIFY_SET_FEATURES
}
#endif


// ********************************** EOF **************************************
