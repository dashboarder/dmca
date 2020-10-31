/*****************************************************************************/
/*                                                                           */
/* COMPONENT   : Rainbow				                                     */
/* MODULE      : FTL				                                         */
/* NAME    	   : FTL header file			                                 */
/* FILE        : FTL.h		                                                 */
/* PURPOSE 	   : This file contains the definition and protypes of exported  */
/*           	 functions for FTL. 				                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2005 SAMSUNG ELECTRONICS CO., LTD.                */
/*                          ALL RIGHTS RESERVED                              */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing a product based on Samsung                 */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representation or warranties     */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/*   19-JUL-2005 [Jaesung Jung] : first writing                              */
/*                                                                           */
/*****************************************************************************/
#include "VFL.h"
#ifndef _FTL_H_
#define _FTL_H_

/*****************************************************************************/
/* Return value of FTL_XXX()                                                 */
/*****************************************************************************/
#define     FTL_SUCCESS                     ANDErrorCodeOk
#define     FTL_CRITICAL_ERROR              ANDErrorCodeHwErr
#define     FTL_USERDATA_ERROR              ANDErrorCodeUserDataErr
#define     FTL_OUT_OF_RANGE_ERROR          ANDErrorCodeOutOfRangeErr
#define     FTL_ALIGNMENT_ERROR             ANDErrorCodeAlignmentErr

/*****************************************************************************/
/* exported function prototype of FTL                                        */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    UInt64 lba;
    UInt64 span;
} FTLExtent_t;


typedef struct
{
    Int32 (*Init)(VFLFunctions *pVFLFunctions);
    Int32 (*Open)(UInt32 *pTotalScts, UInt32 * pdwSectorSize, BOOL32 nandFullFormat, BOOL32 justFormatted, UInt32 minor_ver, UInt32 dwOptions);

    Int32 (*Read)(UInt32 nLpn, UInt32 nNumOfScts, UInt8 *pBuf);
    Int32 (*ReadSpans)(FTLExtent_t *extents, UInt32 numExtents, UInt8 *pBuf);
    void (*Close)(void);
    BOOL32 (*GetStruct)(UInt32 dwStructType, void * pvoidStructBuffer, UInt32 * pdwStructSize);

#ifndef AND_READONLY
    UInt32 (*ConvertUserMBtoFTLSuperblocks)(VFLFunctions *pVFLFunctions, UInt32 dwUserMBs);
    Int32 (*Write)(UInt32 nLpn, UInt32 nNumOfScts, UInt8 *pBuf, BOOL32 isStatic);
    Int32 (*Format)(UInt32 dwOptions);
    Int32 (*WearLevel)(void);
    Int32 (*IdleGC)();
    Int32 (*Unmap)(FTLExtent_t *extents, UInt32 numExtents);
    BOOL32 (*GarbageCollect)(void);
    BOOL32 (*ShutdownNotify)(BOOL32 boolMergeLogs);
    BOOL32 (*Drain)(void);
    BOOL32 (*WriteStats)(void);
#endif
    
    UInt32 (*GetMinorVersion)(void);
} FTLFunctions;

void    FTL_Register(FTLFunctions * pFTLFunctions);
#if (defined (AND_SUPPORT_YAFTL) && AND_SUPPORT_YAFTL)
void    YAFTL_Register(FTLFunctions * pFTLFunctions);
#define YAFTL_PPN_MAJOR_VER (0)
FTLFunctions *YAFTL_PPN_Register(void);
#define SFTL_PPN_MAJOR_VER (1)
FTLFunctions *SFTL_PPN_Register(void);
#endif

/*****************************************************************************/
/* statistics gathering constants                                            */
/*****************************************************************************/
    
#define FTL_RC_BIN_SIZE(l)    ((l)/FTL_NUM_RC_BINS)
#define FTL_NUM_RC_BINS       (100)
    
#define FTL_EC_BIN_SIZE    (FTL_MAX_EC_BIN_VAL/FTL_NUM_EC_BINS)
#define FTL_MAX_EC_BIN_VAL (5000)
#define FTL_NUM_EC_BINS    (100)

typedef struct
{
    UInt32 maxValue;
    UInt32 binCount;
    UInt16 usage[0];
} FTLBinsStruct;

// FTL statistics X-macro:
// this defines all of the stats currently known to SFTL in key name / serialized
// key number form.  Stats users (parsers, savers, etc) define X_FTL_doStat to do
// something meaningful.
// The trivial case is the construction of s_statkey_e enum, which provides to C
// code an enumeration of the key name to the key number.
// A stat saver, for instance, compare a key from a blob to the key macro which
// is currently being expanded; if they match, put into a variable inside a struct
// that matches the name of the stat.
// A parser, on the other hand, would convert the symbol into a C string using
// the # preprocessor operator.  It then has a string representation of the key
// name that it can print or add to internal data structures for future use.
//
// Let's dissect the enum creation:
// #undef X_FTL_doStat
//   -- this undefines the X macro, in case it was previously defined
// #define X_FTL_doStat(_part, _val) S_STATKEY_ ## _part = (_val),
//   -- Each time X_FTL_doStat is invoked below, it will take the token, paste
//   -- S_STATKEY_ in front, and assign it the value.  For instance, this:
//   --     X_FTL_doStat(lbas_read, 1)
//   -- becomes:
//   --     S_STATKEY_ ## lbas_read = (1),
//   -- which is interpreted by the preprocessor to mean:
//   --     S_STATKEY_lbas_read = (1),
//   -- Thus, we've created an enumeration without repeating the origial content.
// typedef enum {
//     FTL_doAllStats
//   -- this invokes the X macro for all of the ones in the doAllStats list
//     S_STATKEY_VFL = 255,
//     S_STATKEY_FIL = 254,
//   -- these are special cases
// } s_statkey_e;
//   -- and we're done!  We now have something that looks like:
// typedef enum {
//     S_STATKEY_lbas_read = (1),
//     S_STATKEY_lbas_written = (2),
//     ...
//     S_STATKEY_VFL = 255,
//     S_STATKEY_VFL = 254,
// } s_statkey_e;
// The neat part is the knowledge of key name/key number is centralized;
// no repetition is required.

#undef X_FTL_doStat
#define FTL_doAllStats \
    X_FTL_doStat(lbas_read, 1) \
    X_FTL_doStat(lbas_written, 2) \
    X_FTL_doStat(lbas_gc, 3) \
    X_FTL_doStat(lbas_flatten, 4) \
    X_FTL_doStat(xacts_read, 5) \
    X_FTL_doStat(xacts_write, 6) \
    X_FTL_doStat(data_gc, 7) \
    X_FTL_doStat(zero_valid_cross, 8) \
    X_FTL_doStat(valid_lbas, 9) \
    X_FTL_doStat(free_sb, 10) \
    X_FTL_doStat(data_sb, 11) \
    X_FTL_doStat(cxt_sb, 12) \
    X_FTL_doStat(dead_sb, 13) \
    X_FTL_doStat(boot_count, 14) \
    X_FTL_doStat(refresh_gcs, 15) \
    X_FTL_doStat(readCount_gcs, 16) \
    X_FTL_doStat(wearLev_gcs, 17) \
    X_FTL_doStat(shutdowns, 18) \
    X_FTL_doStat(lbas_read_1, 19) \
    X_FTL_doStat(lbas_read_2, 20) \
    X_FTL_doStat(lbas_read_3, 21) \
    X_FTL_doStat(lbas_read_4, 22) \
    X_FTL_doStat(lbas_read_5, 23) \
    X_FTL_doStat(lbas_read_6, 24) \
    X_FTL_doStat(lbas_read_7, 25) \
    X_FTL_doStat(lbas_read_8, 26) \
    X_FTL_doStat(lbas_read_over_8, 27) \
    X_FTL_doStat(lbas_write_1, 28) \
    X_FTL_doStat(lbas_write_2, 29) \
    X_FTL_doStat(lbas_write_3, 30) \
    X_FTL_doStat(lbas_write_4, 31) \
    X_FTL_doStat(lbas_write_5, 32) \
    X_FTL_doStat(lbas_write_6, 33) \
    X_FTL_doStat(lbas_write_7, 34) \
    X_FTL_doStat(lbas_write_8, 35) \
    X_FTL_doStat(lbas_write_over_8, 36) \
    X_FTL_doStat(L2V_pool_free, 37) \
    X_FTL_doStat(L2V_pool_count, 38) \
    X_FTL_doStat(lbas_written_static, 39) \
    X_FTL_doStat(lbas_written_dynamic, 40) \
    X_FTL_doStat(unclean_boot_count, 41) \
    X_FTL_doStat(span_xacts_read, 42) \
    X_FTL_doStat(span_freebies_read, 43) \
// blank

#undef X_FTL_doStat
#define X_FTL_doStat(_part, _val) S_STATKEY_ ## _part = (_val),
typedef enum {
    FTL_doAllStats
    S_STATKEY_VFL = 255,
    S_STATKEY_FIL = 254,
} s_statkey_e;
#undef X_FTL_doStat

#define FTL_makeStatParser(_fname, _VFLFunc, _FILFunc) \
void (_fname)(const UInt8 *buf, UInt32 bufSize) \
{ \
    UInt32 len; \
    UInt32 key, size; \
    UInt32 *buf32; \
    UInt64 val64; \
 \
    if (bufSize < sizeof(len)) \
        return; \
 \
    len = *(UInt32*)buf; \
    buf += sizeof(len); \
    bufSize -= sizeof(len); \
 \
    size = (UInt32)(-2); \
    while (len--) { \
        buf += (size + 2) * sizeof(UInt32); /* for prior round */ \
        bufSize -= (size + 2) * sizeof(UInt32); \
        if (bufSize < ((2 * sizeof(UInt32)) + sizeof(UInt64))) \
            break; \
        /* Load key/size/val */ \
        buf32 = (UInt32*)buf; \
        key = buf32[0]; \
        size = buf32[1]; \
        val64 = *(UInt64*)(&buf32[2]); \
 \
        FTL_doAllStats \
 \
        if ((bufSize - (2 * sizeof(UInt32))) < size) \
            break; \
 \
        /* Got here--wasn't caught be easy filter above, so must be some other blob */ \
        if (0 == key) { \
            buf += sizeof(UInt32); \
            continue; \
        } \
        if (S_STATKEY_VFL == key) { \
            (_VFLFunc)(AND_STRUCT_VFL_STATISTICS, &buf32[2], size*sizeof(UInt32)); \
            continue; \
        } \
        if (S_STATKEY_FIL == key) { \
            (_FILFunc)(AND_STRUCT_VFL_FILSTATISTICS, &buf32[2], size*sizeof(UInt32)); \
            continue; \
        } \
    } \
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FTL_H_ */
