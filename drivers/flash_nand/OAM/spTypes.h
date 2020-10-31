//-----------------------------------------------------------------------------
// 
// (C) Copyright 2001 Apple Computer, All rights reserved
//
//-----------------------------------------------------------------------------
// FILE NAME:    spTypes.h
//
// DESCRIPTION:  
//
// DOCUMENT REF: 
//
// NOTES:        None
//-----------------------------------------------------------------------------
//
#ifndef __SPTYPES_H__ //--------------------
#define __SPTYPES_H__

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
// Types and defines:
//-----------------------------------------------------------------------------
//

#define False           0
#define True            (!False)

#ifdef __cplusplus
#define Null            0
#else
#define Null            ((Void *)0)
#endif

#define MB		0x100000
#define KB		0x400

/* Metrowerks for Windows doesn't support __packed for structures */
#ifdef __INTEL__
	#define __packed
	#pragma options align=packed

#endif

//-----------------------------------------------------------------------------
// SP Standard Types    	Type   Pointer
//                      	------  -------
typedef signed char   		xInt8,   *pInt8;     //  8 bit   signed integer
typedef signed short  		xInt16,  *pInt16;    // 16 bit   signed integer
typedef signed long   		xInt32,  *pInt32;    // 32 bit   signed integer
typedef signed long long	xInt64, *pInt64;		// 64 bit signed integer
typedef unsigned char   	xUInt8,  *pUInt8;    //  8 bit unsigned integer
typedef const unsigned char   	cUInt8,  *pcUInt8;    //  8 bit unsigned integer
typedef unsigned short  	xUInt16, *pUInt16;   // 16 bit unsigned integer
typedef unsigned int		xUInt32, *pUInt32;   // 32 bit unsigned integer
typedef unsigned long long	xUInt64, *pUInt64;	// 64 bit unsigned integer
typedef void            	Void,   *pVoid;     // Void (typeless)
typedef unsigned int    	Bool,   *pBool;     // Boolean (True/False)
typedef char            	Char,   *pChar;     // character, character array ptr
typedef char           		*String, **pString;  // Null terminated 8 bit char str,
typedef const char     		*ConstString,**pConstString;  // Null term 8 bit char str ptr

//-----------------------------------------------------------------------------
// Pointer access macros
//-----------------------------------------------------------------------------

#define mmioReg8( pv )   *((volatile char *)(pv))
#define mmioReg8u( pv )   *((volatile unsigned char *)(pv))
#define mmioReg16( pv )  *((volatile short*)(pv))
#define mmioReg16u( pv )  *((volatile unsigned short*)(pv))
#define mmioReg32( pv )  *((volatile long *)(pv))
#define mmioReg32u( pv )  *((volatile unsigned long *)(pv))

// use mmioReg for types that are already defined with volatile and a proper size and type
#define	mmioReg( pv )	*(pv)

//
// For C++ we have to make sure the rValue is invoked 
// explicitly from the pointers lvalue when we only want the
// side effects for reading and not the actual result.
//
#define mmioReg8_access( pv )   ((void)(0+mmioReg8( pv )))
#define mmioReg8u_access( pv )   ((void)(0+mmioReg8u( pv )))
#define mmioReg16_access( pv )  ((void)(0+mmioReg16( pv )))
#define mmioReg16u_access( pv )  ((void)(0+mmioReg16u( pv )))
#define mmioReg32_access( pv )  ((void)(0+mmioReg32( pv )))
#define mmioReg32u_access( pv )  ((void)(0+mmioReg32u( pv )))

// use mmioReg for types that are already defined with volatile and a proper size and type
#define	mmioReg_access( pv )	((void)(0+mmioReg(pv)))

//-----------------------------------------------------------------------------
// Pointer creation macros
//-----------------------------------------------------------------------------

#define makePtr( c, b, o ) ((c)(((long)b)+((long)o)))

#define memberOffset(t,m) ( &(((t)0)->m))

#define mmioValueAlterMask( pv, mask, setclear )		\
	pv = (setclear) ? 					\
		(pv | (mask)) :					\
		(pv & ~(mask))
#define mmioValueAlterBit( pv, bit, setclear ) mmioValueAlterMask(pv, 1 << (bit), setclear)

//-----------------------------------------------------------------------------
// Error Codes
//-----------------------------------------------------------------------------
//
//  +------+------+------------+------+------------------+
//  |31  28|27  24|23        16|15  12|11               0| bit
//  +------+------+------------+------+------------------+
//  |Class | Type |ComponentTag|Layer |  Error/Progress  |
//  +------+------+------------+------+------------------+

typedef UInt32 spErrorCode_t;
typedef void (*spFn_t)(void);
typedef void (*HandlerFunction)(void);
typedef void (*spFnProgress_t)(  UInt32 pct, pVoid pContext);
typedef spErrorCode_t (*spFnUiCmd_t)( pUInt8 argv[],UInt32 argc);

//-----------------------------------------------------------------------------
// SP_OK is the 32 bit global status value used by all software components
//  to indicate successful function/operation status.  If a non-zero value is
//  returned as status, it should use the component ID formats defined.
//
#define SP_OK               0         // Global success return status

//
// NOTE: Component ID types are defined as unsigned 32 bit integers (UInt32).
//
#define CID_ERR_BITSHIFT    0     // Component error bit shift
#define CID_TAG_BITSHIFT    16    // Component tag bit shift
#define CID_LAYER_BITSHIFT  12    // Component layer bit shift
#define CID_TYPE_BITSHIFT   24    // Component type bit shift
#define CID_CLASS_BITSHIFT  28    // Component class bit shift

#define CID_LAYER_BITMASK   ( 0x00F << CID_LAYER_BITSHIFT) // Layer AND bitmask
#define CID_TAG_BITMASK     ( 0x0FF << CID_TAG_BITSHIFT  ) // Comp tag AND bitmask
#define CID_CLASS_BITMASK   ( 0x00F << CID_CLASS_BITSHIFT) // Class AND bitmsk
#define CID_ERR_BITMASK     ( 0xFFF << CID_ERR_BITSHIFT  ) // Component error AND bitmask
#define CID_TYPE_BITMASK    ( 0x00F << CID_TYPE_BITSHIFT ) // Type AND bitmask

#define SP_ERR_GET_ERROR(err) ((err & CID_ERR_BITMASK  ) >> CID_ERR_BITSHIFT  )
#define SP_ERR_GET_CLASS(err) ((err & CID_CLASS_BITMASK) >> CID_CLASS_BITSHIFT)
#define SP_ERR_GET_TYPE(err)  ((err & CID_TYPE_BITMASK ) >> CID_TYPE_BITSHIFT )
#define SP_ERR_GET_LAYER(err) ((err & CID_LAYER_BITMASK) >> CID_LAYER_BITSHIFT)

// Component Class definitions (bits 31:28, 4 bits)
#define CID_CLASS_NONE      (0x1 << CID_CLASS_BITSHIFT) // No class information
#define CID_CLASS_INFRASTR  (0x2 << CID_CLASS_BITSHIFT)

// Component Type definitions (bits 27:24, 4 bits)
#define CID_TYPE_NONE       (0x0 << CID_TYPE_BITSHIFT)  // No data connections

// Component Tag definitions (bits 23:16, 8 bits)
// NOTE: Component tags are defined in groups, dependent on the class and type.
#define CID_TAG_NONE        (0x00 << CID_TAG_BITSHIFT)  // No tag information

#define TAG(num)            ((num)<< CID_TAG_BITSHIFT) // Create tag from num

// Component Layer definitions (bits 15:12, 4 bits)
#define CID_LAYER_NONE      (0x0 << CID_LAYER_BITSHIFT)  // No layer info
#define CID_LAYER_BOOT      (0x1 << CID_LAYER_BITSHIFT)  // Boot loader layer
#define CID_LAYER_HWAPI     (0x2 << CID_LAYER_BITSHIFT)  // Hardware API layer
#define CID_LAYER_IMA		(0x3 << CID_LAYER_BITSHIFT)  // iMA interface layer

// STANDARD ERRORS
#define SP_ERR_COMPATIBILITY            0x001 // SW Interface compatibility
#define SP_ERR_MAJOR_VERSION            0x002 // SW Major Version error
#define SP_ERR_COMP_VERSION             0x003 // SW component version error
#define SP_ERR_BAD_MODULE_ID            0x004 // SW - HW module ID error
#define SP_ERR_BAD_UNIT_NUMBER          0x005 // Invalid device unit number
#define SP_ERR_BAD_INSTANCE             0x006 // Bad input instance value
#define SP_ERR_BAD_HANDLE               0x007 // Bad input handle
#define SP_ERR_BAD_INDEX                0x008 // Bad input index
#define SP_ERR_BAD_PARAMETER            0x009 // Invalid input parameter
#define SP_ERR_NO_INSTANCES             0x00A // No instances available
#define SP_ERR_NO_COMPONENT             0x00B // Component is not present
#define SP_ERR_NO_RESOURCES             0x00C // Resource is not available
#define SP_ERR_INSTANCE_IN_USE          0x00D // Instance is already in use
#define SP_ERR_RESOURCE_OWNED           0x00E // Resource is already in use
#define SP_ERR_RESOURCE_NOT_OWNED       0x00F // Caller does not own resource
#define SP_ERR_INCONSISTENT_PARAMS      0x010 // Inconsistent input params
#define SP_ERR_NOT_INITIALIZED          0x011 // Component is not initialized
#define SP_ERR_NOT_ENABLED              0x012 // Component is not enabled
#define SP_ERR_NOT_SUPPORTED            0x013 // Function is not supported
#define SP_ERR_INIT_FAILED              0x014 // Initialization failed
#define SP_ERR_BUSY                     0x015 // Component is busy
#define SP_ERR_NOT_BUSY                 0x016 // Component is not busy
#define SP_ERR_READ                     0x017 // Read error
#define SP_ERR_WRITE                    0x018 // Write error
#define SP_ERR_ERASE                    0x019 // Erase error
#define SP_ERR_LOCK                     0x01A // Lock error
#define SP_ERR_UNLOCK                   0x01B // Unlock error
#define SP_ERR_OUT_OF_MEMORY            0x01C // Memory allocation failed
#define SP_ERR_BAD_VIRT_ADDRESS         0x01D // Bad virtual address
#define SP_ERR_BAD_PHYS_ADDRESS         0x01E // Bad physical address
#define SP_ERR_TIMEOUT                  0x01F // Timeout error
#define SP_ERR_OVERFLOW                 0x020 // Data overflow/overrun error
#define SP_ERR_FULL                     0x021 // Queue (etc.) is full
#define SP_ERR_EMPTY                    0x022 // Queue (etc.) is empty
#define SP_ERR_NOT_STARTED              0x023 // Component stream not started
#define SP_ERR_ALREADY_STARTED          0x024 // Comp. stream already started
#define SP_ERR_NOT_STOPPED              0x025 // Component stream not stopped
#define SP_ERR_ALREADY_STOPPED          0x026 // Comp. stream already stopped
#define SP_ERR_ALREADY_SETUP            0x027 // Component already setup
#define SP_ERR_NULL_PARAMETER           0x028 // Null input parameter
#define SP_ERR_NULL_DATAINFUNC          0x029 // Null data input function
#define SP_ERR_NULL_DATAOUTFUNC         0x02A // Null data output function
#define SP_ERR_NULL_CONTROLFUNC         0x02B // Null control function
#define SP_ERR_NULL_COMPLETIONFUNC      0x02C // Null completion function
#define SP_ERR_NULL_PROGRESSFUNC        0x02D // Null progress function
#define SP_ERR_NULL_ERRORFUNC           0x02E // Null error handler function
#define SP_ERR_NULL_MEMALLOCFUNC        0x02F // Null memory alloc function
#define SP_ERR_NULL_MEMFREEFUNC         0x030 // Null memory free  function
#define SP_ERR_NULL_CONFIGFUNC          0x031 // Null configuration function
#define SP_ERR_NULL_PARENT              0x032 // Null parent data
#define SP_ERR_NULL_IODESC              0x033 // Null in/out descriptor
#define SP_ERR_NULL_CTRLDESC            0x034 // Null control descriptor
#define SP_ERR_UNSUPPORTED_DATACLASS    0x035 // Unsupported data class
#define SP_ERR_UNSUPPORTED_DATATYPE     0x036 // Unsupported data type
#define SP_ERR_UNSUPPORTED_DATASUBTYPE  0x037 // Unsupported data subtype
#define SP_ERR_FORMAT                   0x038 // Invalid/unsupported format
#define SP_ERR_INPUT_DESC_FLAGS         0x039 // Bad input  descriptor flags
#define SP_ERR_OUTPUT_DESC_FLAGS        0x03A // Bad output descriptor flags
#define SP_ERR_CAP_REQUIRED             0x03B // Capabilities required ???
#define SP_ERR_BAD_TMALFUNC_TABLE       0x03C // Bad TMAL function table
#define SP_ERR_INVALID_CHANNEL_ID       0x03D // Invalid channel identifier
#define SP_ERR_INVALID_COMMAND          0x03E // Invalid command/request
#define SP_ERR_STREAM_MODE_CONFUSION    0x03F // Stream mode config conflict
#define SP_ERR_UNDERRUN                 0x040 // Data underflow/underrun
#define SP_ERR_EMPTY_PACKET_RECVD       0x041 // Empty data packet received
#define SP_ERR_OTHER_DATAINOUT_ERR      0x042 // Other data input/output err
#define SP_ERR_STOP_REQUESTED           0x043 // Stop in progress
#define SP_ERR_PIN_NOT_STARTED          0x044 // Pin not started
#define SP_ERR_PIN_ALREADY_STARTED      0x045 // Pin already started
#define SP_ERR_PIN_NOT_STOPPED          0x046 // Pin not stopped
#define SP_ERR_PIN_ALREADY_STOPPED      0x047 // Pin already stopped
#define SP_ERR_STOP_PIN_REQUESTED       0x048 // Stop of a single pin is in progress
#define SP_ERR_ASSERTION                0x049 // Assertion failure
#define SP_ERR_HIGHWAY_BANDWIDTH        0x04A // Highway bandwidth bus error
#define SP_ERR_HW_RESET_FAILED          0x04B // Hardware reset failed
#define SP_ERR_MEDIA_MISSING            0x04C // Media Missing
#define SP_ERR_DEVICE_ABORT             0x04D // Device Abort
#define SP_ERR_MEDIA_CHANGE             0x04E // Media Change
#define SP_ERR_MEDIA_WRITE_PROTECTED    0x04F // Media Write Protected
#define SP_ERR_HW_ERR_PENDING           0x050 // 
#define SP_ERR_HW_ERR                   0x051 // Generic Hw Error
#define SP_ERR_NOT_FOUND                0x052 // item not found
#define SP_ERR_DEVICE_NOT_FOUND			0x053 // device not found
#define SP_ERR_ITEMLIST_FULL			0x054 // no room for item in catalog
#define SP_ERR_FNF_ERR					0x055 // file not found
#define	SP_ERR_NOPARTITION				0x056 // partition not found
#define	SP_ERR_CHECKSUM_FAILED			0x057 // checksum do not match
#define	SP_ERR_ATA_SELECT_ERROR			0x058 // IDE error selecting device
#define	SP_ERR_ATA_COMMAND_ERROR		0x059 // IDE command failed
#define	SP_ERR_ATA_SEQUENCE_ERROR		0x05a // IDE command setup sequence failed
#define	SP_ERR_ATA_COMMUNICATIONS_ERROR	0x05b // IDE communications error
#define SP_ERR_IIC_NOACK                0x060 // IIC no ack error

// Add new standard error/progress status codes here

#define SP_ERR_COMP_UNIQUE_START        0x800 // 0x800-0xDFF: Component unique

// SP Standard assert error code start offset
// NOTE: This define should be added to the component's base error value and
//       standard error code(s) to define assert error codes.  
//
#define SP_ERR_ASSERT_START             0xE00 // 0xE00-0xEFF: Assert failures
#define SP_ERR_ASSERT_LAST              0xEFF // Last assert error range value
#define SP_IS_ASSERT_ERROR(err)     \
    ((SP_GET_ERROR(err) >= SP_ERR_ASSERT_START) && \
     (SP_GET_ERROR(err) <= SP_ERR_ASSERT_LAST))

// SP Standard fatal error code start offset
// NOTE: This define should be added to the component's base error value and
//       standard error code(s) to define fatal error codes.  For example:
#define SP_ERR_FATAL_START              0xF00 // 0xF00-0xFFF: Fatal failures
#define SP_ERR_FATAL_LAST               0xFFF // Last fatal error range value
#define SP_IS_FATAL_ERROR(err)          \
    ((SP_GET_ERROR(err) >= SP_ERR_FATAL_START) && \
     (SP_GET_ERROR(err) <= SP_ERR_FATAL_LAST))

//-----------------------------------------------------------------------------
// SMART Self Test error codes
//

// return values from ATA disk SMART test
#define	SP_SMART_OK							0x00 // no error
#define	SP_SMART_ABORTED					0x01 // aborted by host
#define	SP_SMART_RESET						0x02 // reset by host
#define	SP_SMART_FATAL_ERROR				0x03 // fatal or unknown error
#define	SP_SMART_UNKNOWN_ERROR				0x04 // unknown error
#define	SP_SMART_ELECTRICAL_ERROR			0x05 // electrical error
#define	SP_SMART_SERVO_ERROR				0x06 // servo error
#define	SP_SMART_READ_ERROR					0x07 // read error
#define	SP_SMART_RESERVED_8					0x08 // reserved
#define	SP_SMART_RESERVED_E					0x0E // reserved
// 8 - 14 reserved, assume they indicate error
#define	SP_SMART_IN_PROGRESS				0x0F // test in progress
#define SP_SMART_CANCELLED_BY_USER			0x10 // user cancelled test

// return values from test routine, simplified version of above errors
// if everything is ok, we return SP_SMART_OK
#define SP_SMART_RESCAN						0x01 // incomplete scan, try again
#define SP_SMART_DEADBEEF					0x02 // dead as a doornail

//-----------------------------------------------------------------------------
// Version Structure
//
typedef UInt32	spVersion_t, *pspVersion_t;

#define SP_VERSION_MAJOR_NUM_MASK		0xFF000000
#define SP_VERSION_MAJOR_NUM_SHIFT		24

#define SP_VERSION_MINOR_NUM_MASK		0x00FF0000
#define SP_VERSION_MINOR_NUM_SHIFT		16

#define SP_VERSION_COMPATIBILITY_MASK	0x0000FFFF
#define SP_VERSION_COMPATIBILITY_SHIFT	0

#define SP_VERSION_GET_MAJOR_NUM(v)		(((v) & SP_VERSION_MAJOR_NUM_MASK	  ) >> \
												SP_VERSION_MAJOR_NUM_SHIFT    )
#define SP_VERSION_GET_MINOR_NUM(v)		(((v) & SP_VERSION_MINOR_NUM_MASK	  ) >> \
												SP_VERSION_MINOR_NUM_SHIFT    )    
#define SP_VERSION_GET_COMPATIBILITY(v)	(((v) & SP_VERSION_COMPATIBILITY_MASK ) >> \
												SP_VERSION_COMPATIBILITY_SHIFT)

#define SP_VERSION(mj, mn, c )			( ( ( (mj ) << SP_VERSION_MAJOR_NUM_SHIFT 	  ) & \
													   SP_VERSION_MAJOR_NUM_MASK 	  ) | \
										  ( ( (mn ) << SP_VERSION_MINOR_NUM_SHIFT 	  ) & \
										  			   SP_VERSION_MINOR_NUM_MASK 	  ) | \
										  ( ( (c )  << SP_VERSION_COMPATIBILITY_SHIFT ) & \
										  			   SP_VERSION_COMPATIBILITY_MASK  ) )
										  			     
//-----------------------------------------------------------------------------
// Memory heap
//

typedef struct tagspboardMemHeap_t
{
    pVoid   pvHeapBase;
    UInt32  cbHeapSize;

}spboardMemHeap_t,*pspboardMemHeap_t;


//-----------------------------------------------------------------------------
// HW Unit Selection
//
typedef enum tagspUnitSelect_t
{
    spUnitNone  = -1,
    spUnit0     =  0,
    spUnit1     =  1,
    spUnit2     =  2,
    spUnit3     =  3,
    spUnit4     =  4

}   spUnitSelect_t, *pspUnitSelect_t;

//-----------------------------------------------------------------------------
// Boot Modes
//
typedef enum
{
    spBootModeRetail    =   1,
    spBootModeDebug     =   2,
    spBootModeUiShell   =   3,
    spBootModeDiag      =   4,
    spBootModeDisk      =   5,
    spBootModeScan      =   6,
    spBootModeSleepTest =	7,
    spBootModeReset     =   8
} spBootMode_t, *pspBootMode_t;

typedef enum
{
    CPU_13_5MHz = 13500,
    CPU_24MHz = 24000,
    CPU_36MHz = 36000,
    CPU_48MHz = 48000,
    CPU_64MHz = 64000,
    CPU_80MHz = 80000,
    CPU_90MHz = 90000
} spCpuMHz_t, *pspCpuMHz_t;

//-----------------------------------------------------------------------------
// Instance and handle
//
typedef UInt32 spInstance_t, *pspInstance_t;
typedef UInt32 spHandle_t, *pspHandle_t;

#define  SP_INVALID_HANDLE  ((spHandle_t)-1)

//-----------------------------------------------------------------------------
// Device flags
//
#define SP_HW_DEVICE_RAM        0x00000001
#define SP_HW_DEVICE_SDRAM      0x00000002
#define SP_HW_DEVICE_FLASH      0x00000004
#define SP_HW_DEVICE_IDE        0x00000008
#define SP_HW_DEVICE_UART1      0x00000010
#define SP_HW_DEVICE_UART2      0x00000020
#define SP_HW_DEVICE_LCD        0x00000040
#define SP_HW_DEVICE_FIREWIRE   0x00000080
#define SP_HW_DEVICE_GPIO       0x00000100
#define SP_HW_DEVICE_LEDS       0x00000200
#define SP_HW_DEVICE_PIEZO      0x00000400
#define SP_HW_DEVICE_CODEC      0x00000800
#define SP_HW_DEVICE_A2D		0x00001000

#define SP_HW_DEVICE_ALL        0xFFFFFFFF

//-----------------------------------------------------------------------------
// System config structure location.
// 

#define SP_SYSCONFIG_START_FLASH_ADDRESS_GRPA	0x2000
#define SP_SYSCONFIG_END_FLASH_ADDRESS_GRPA		0x4000
#define SP_SYSCONFIG_START_FLASH_ADDRESS_GRPB	0x4000
#define SP_SYSCONFIG_END_FLASH_ADDRESS_GRPB		0x6000

//-----------------------------------------------------------------------------
// System Information Structure
//

#define SP_FLASH_VERSION_OFFSET	0x4040
#define SP_FLASH_VERSION_MAGIC	'srev'

#define SP_SYSINFO_MAGIC	'SysI'

typedef struct tagspSysInfoAddr_t
{
	UInt32	magic;
	UInt32	addr;
	
}spSysInfoAddr_t,*pspSysInfoAddr_t;

#define SP_SYSINFO_BOARDHWNAME_SIZE		16		// SCSI INQ depends on this!
#define SP_SYSINFO_SERIALNUM_SIZE		32
#define SP_SYSINFO_FIREWIREGUID_SIZE	16
#define SP_SYSINFO_UNITCHARACTER_SIZE	16
#define SP_SYSINFO_MODELNUM_SIZE		16
#define SP_SYSINFO_HWOPTIONS1_SIZE		16
#define	SP_SYSINFO_CONTRAST_SIZE		16
#define	SP_SYSINFO_BACKLIGHT_SIZE		16
#define	SP_SYSINFO_DRMCONFIG_SIZE		16

#define SP_SYSINFO_MAX_MEM_RANGES		 8
#define SP_SYSINFO_MAX_BATTERY_CAL		 4

typedef enum
{
	kSysInfoAddrRangeNone  	 	= 0,
	kSysInfoAddrRangeFlash 		= 'Flsh',
	kSysInfoAddrRangeSdram 		= 'Sdrm',
	kSysInfoAddrRangeFirewire   = 'Frwr',
	kSysInfoAddrRangeIram  		= 'Iram',
	kSysInfoAddrRangeHdd0  		= 'hdd0',
	kSysInfoAddrRangeHdd1  		= 'hdd1'

}spSysInfoAddrRangeType_t;

typedef struct tagspBattA2DCal_t
{
	UInt32	voltage;	// Fix point format 16bit.16bit
	UInt32	a2d;
	
}spBattA2DCal_t,*pspBattA2DCal_t;

#define SP_SYSCFG_BATTERY_CAL_VERSION	1

typedef struct tagrtcAdjustKeyValueFormat_t
{
	UInt32	version;
	UInt32	secPerDay; // Signed fix point format 16bit.16bit
	
}rtcAdjustKeyValueFormat_t, *prtcAdjustKeyValueFormat_t;

#define SP_SYSCFG_RTC_ADJUST_VERSION	1

typedef struct tagbatteryA2DCalKeyValueFormat_t
{
	UInt32	version;
	UInt32	voltage; // Signed fix point format 16bit.16bit
	UInt32	a2d;

}batteryA2DCalKeyValueFormat_t, *pbatteryA2DCalKeyValueFormat_t;

//
// Region codes and policy flags
//
#define SP_SYSCFG_REGION_VERSION		1
#define SP_SYSCFG_REGION_POLICY_BYTES	2
#define SP_SYSCFG_REGION_VENDOR_MASK	0xC0

// NOTE: HP adds the high bit '0x80' to a given region code to avoid adding a string
//       to the model number, but still allowing for region differentiation
enum
{
	kSyscfgRegionVendor_Apple		= 0x00,
	kSyscfgRegionVendor_HP			= 0x80
};

// Check Wiki for most up-to-date list (v1.5, 3/10/06)
//	http://spgserver.apple.com/Ipodwiki/wiki?Region_Codes
enum // Needs to fit in 16bit!
{
                                           // Regions (Apple - 0x00)                            // Regions (HP - 0x80)
	kSyscfgRegion_World				= 0x00,// Unassigned                                        // Unassigned
		
	kSyscfgRegion_NorthAmerica		= 0x01,// LL = US/Canada                                    // ABA = US/Canada
	kSyscfgRegion_Japan				= 0x02,// J  = Japanese Only
	kSyscfgRegion_FrenchGerman		= 0x03,// FD = French (F), German (D), Swiss, Austria
	kSyscfgRegion_ItalianDutch		= 0x04,// TN = Italian (T), Dutch (N), Belgium
	kSyscfgRegion_SpanishBrazilian	= 0x05,// Y  = Spanish (Y), Brazilian Portuguese (BR)
	kSyscfgRegion_SwedishNorwegian	= 0x06,// S  = Swedish (S), Norwegian (H)
	kSyscfgRegion_DanishFinnish		= 0x07,// DK = Danish (DK), Finnish (K)
	kSyscfgRegion_ChineseHongKong	= 0x08,// CH = Chinese (CH), Hong Kong
	kSyscfgRegion_Korean			= 0x09,// KH = Korean (KH)
	
	kSyscfgRegion_Canada			= 0x0A,// C  = Canada                                       // ABC = Canada
	kSyscfgRegion_LatinAmerica		= 0x0B,// E  = Latin America                                // ABM = Latin America
	kSyscfgRegion_UnitedKingdom		= 0x0C,// B  = United Kingdom                               // ABU = United Kingdom
	kSyscfgRegion_Europe			= 0x0D,// Z  = Rest of Europe
	kSyscfgRegion_Australia			= 0x0E,// X  = Australia, New Zealand	
	kSyscfgRegion_HongKongSingapore	= 0x0F,// FE = HongKong, Singapore	
	kSyscfgRegion_Taiwan			= 0x10,// TA = Taiwan	
	kSyscfgRegion_ZV				= 0x11,// ZV = Europe (British, Swedish, Spanish, Danish)
	kSyscfgRegion_ZR				= 0x12,// ZR = Europe (French, German, Dutch, Italian)      // B19 = Europe (French, German, Dutch, Italian)
	kSyscfgRegion_SouthAsia			= 0x13,// ZP = Singapore, South Asia
//	kSyscfgRegion_Spanish_Q98		= 0x13,// Y  = Spanish (Y) [Q98 ONLY]
	kSyscfgRegion_HongKong			= 0x14,// HK = Hong Kong (HK)
//	kSyscfgRegion_KoreaTaiwan_Q98	= 0x14,// PA = Korean, Taiwanise [Q98 ONLY]
	kSyscfgRegion_Spanish			= 0x15,// Y  = Spanish (Y)                                  // ABE = Spanish (Y)
	kSyscfgRegion_Chinese			= 0x16,// CH = Chinese (CH)
	kSyscfgRegion_KoreanTaiwanese	= 0x17,// PA = Korean, Taiwanese                            // AB2 = Korean, Taiwanese
    kSyscfgRegion_French            = 0x18,// F  = French                                       // ABF = French
    kSyscfgRegion_German            = 0x19,// D  = German                                       // ABD = German
    kSyscfgRegion_SouthAfrica       = 0x1A,// FB = CEMEA, South Africa
    kSyscfgRegion_DutchBelgium      = 0x1B,// NF = Dutch (N), Belgium
    kSyscfgRegion_SpanishItalian    = 0x1C,// TY = Spanish, (Y) Italian (T)
    kSyscfgRegion_Brazil            = 0x1D,// YP = Brazil
    kSyscfgRegion_Portugal          = 0x1E,// PZ = Portugal

	kSyscfgRegion_LimitValue
};

enum // Needs to fit in 16bit!
{
	kSyscfgRegionPolicyFlag_None			= 0x0000,
	kSyscfgRegionPolicyFlag_EU				= 0x0001, // "French" volume problem
    kSyscfgRegionPolicyFlag_TVOut           = 0x0006  // Two bits, PAL = 00, NTSC = 01
};

enum
{
    kSyscfgRegionPolicyFlag_TVOut_PAL       = (0 << 1),
    kSyscfgRegionPolicyFlag_TVOut_NTSC      = (1 << 1)
};

typedef struct tagregionKeyValueFormat_t
{
	UInt16	version;		 // SP_SYSCFG_REGION_VERSION
	UInt8	reserve;
	UInt8	PolicySizeBytes; // Currently 2
	
	UInt16	regionCode;
	UInt16	policyFlags;
	
}regionKeyValueFormat_t,*pregionKeyValueFormat_t;

typedef struct tagunitCharacteristicsFormat_t
{
	// this structure maps on to data provided in flash
	UInt8	revision;
	UInt8	bytesOfValidData; 
	
	UInt16	oemVendorId;  // little endian 
	UInt32	color;		  // little endian
	UInt8   reserved[8];
	
}unitCharacteristicsFormat_t,*punitCharacteristicsFormat_t;

// Add stuff at end to maximize compatibility with old bootloaders
typedef struct tagspSysInfo_t
{
	UInt32	magic;
	UInt32	size;
	
	UInt8	pszBoardHwName [ SP_SYSINFO_BOARDHWNAME_SIZE  ];
	UInt8	pszSerialNumber[ SP_SYSINFO_SERIALNUM_SIZE    ];
	UInt8	pu8FirewireGuid64[ SP_SYSINFO_FIREWIREGUID_SIZE ];
	
	// Board Version
	spVersion_t				boardHwRev;
	
	// SW Versions
	spVersion_t				bootLoaderImageRev;
	spVersion_t				diskModeImageRev;
	spVersion_t				diagImageRev;
	spVersion_t				osImageRev;
	
	//
	// Number of Voltage / Analog-to-Digital calibration
	// points.
	//
	UInt32			ccBattA2DCal;
	spBattA2DCal_t	battA2DCal[SP_SYSINFO_MAX_BATTERY_CAL];

	//
	// Currection to RTC drift due to constant PLL error. 
	// Seconds per day.
	//
	Int32			RtcAdjustSecondsPerDay;		// Signed fix point 
												// format 16bit.16bit

	// Board Sw Interface Revision
	spVersion_t				boardHwSwInterfaceRev;
	
	// HDD Firmware version string
	UInt8	pszHddFirmwareVersion[ 9 ];

	// Region and Policy flags
	UInt16	regionCode;
	UInt32	policyFlags;			
	
	UInt8	pszModelNumStr [ SP_SYSINFO_MODELNUM_SIZE  ];

	UInt8	hardwareOptions1[SP_SYSINFO_HWOPTIONS1_SIZE];

	Int8	contrastAdjBLOff;
	Int8	contrastAdjBLOn;
	Int8	backlightAdjCenter;
	Int8	backlightAdjSwing;
	unitCharacteristicsFormat_t   unitCharacteristics;
    // Drive Low Power Cut Off
    UInt16  hddVoltageCutoff;

	UInt8	drmConfig[SP_SYSINFO_DRMCONFIG_SIZE];

    // Info on memory sizes, properly determined
    UInt32 SDRAMSize;
    UInt32 SDRAMBase;
    UInt32 IRAMSize;
    UInt32 IRAMBase;
    UInt32 FlashSize;
    UInt32 FlashBase;
    UInt32 NANDSizeTag;         // Tag to help Diagnostics to find NAND size info
    UInt32 NANDTotalLBA;
    Int32  NANDBlockSize;       // negtive number to indicate error

}spSystemInformation_t,*pspSystemInformation_t;

#define SYSINFO_BOARDHWREV				1
#define SYSINFO_BOOTLOADERIMAGEREV		2
#define SYSINFO_DISKMODEIMAGEREF		3
#define SYSINFO_DIAGIMAGEREV			4
#define SYSINFO_OSIMAGEREV				5


#define SP_SYSCFG_KEY_SERIALNUM		'SrNm'
#define SP_SYSCFG_KEY_FIREWIREGUID	'FwId'
#define SP_SYSCFG_KEY_HWID			'HwId'
#define	SP_SYSCFG_KEY_HWIFVERSION	'HwVr'
#define SP_SYSCFG_KEY_HWNAME		'HwNm'
#define SP_SYSCFG_KEY_BATTERY_CAL	'Btry'
#define SP_SYSCFG_KEY_RTC_ADJUST	'RtcA'
#define SP_SYSCFG_KEY_REGION		'Regn'
#define SP_SYSCFG_KEY_MODELNUM		'Mod#'
#define SP_SYSCFG_KEY_HWOPTS1		'HwO1'
#define	SP_SYSCFG_KEY_CONTRAST		'Cont'
#define SP_SYSCFG_KEY_UNITCHARACTER	'UntC'
#define	SP_SYSCFG_KEY_BACKLIGHT		'BkLt'
#define	SP_SYSCFG_KEY_DRMCONFIG		'DrmV'

//-----------------------------------------------------------------------------
// Image Manager types
//-----------------------------------------------------------------------------
//

#define BT_IMAGE_TYPE_DIAG  		'diag'
#define BT_IMAGE_TYPE_OS    		'osos'
#define BT_IMAGE_TYPE_DISKMODE		'disk'
#define	BT_IMAGE_TYPE_SCANMODE		'scan'
#define BT_IMAGE_TYPE_UPDATE		'aupd'
#define BT_IMAGE_TYPE_BOOTLOGO		'logo'
#define BT_IMAGE_VIDEOCORE_BOOT		'vmcs'
#define BT_IMAGE_TYPE_HIBERNATE		'hibe'
#define BT_IMAGE_TYPE_RESOURCE      'rsrc'
#define BT_IMAGE_TYPE_GRAPE 		'grpe'

//-----------------------------------------------------------------------------
// Image Manager types
//-----------------------------------------------------------------------------
//
#define BT_IMAGE_LOGO_MAGIC			'LoGo'


//-----------------------------------------------------------------------------
// Cpu Profile Interval type
//-----------------------------------------------------------------------------
//

#define SP_PROFILE_EVENT_CPU_0	0
#define SP_PROFILE_EVENT_COP_0	1

typedef struct tagpspProfileContext_t
{
	UInt32	id;
	UInt32	maskEnable;
	Bool	bEnable;
	
	UInt32	triggerPin;
	UInt32	signalPin;
	
	UInt32	signalPinAddr;
	UInt32	signalPinMask;

	UInt32	start;
	UInt32	end;
	
	Bool	bHigh;
	
	UInt32 	HiTime;
	UInt32 	LoTime;
		
}spProfileContext_t, *pspProfileContext_t;



//-----------------------------------------------------------------------------
// The target image is passed this 32bit flags word from the Bootloader
// 
#define SP_IMAGE_FLAGS_ADDR_PTR_OFFSET			0x10 // - 0x17

#define SP_IMAGE_NONE_FLAG						0x00000000
#define SP_IMAGE_REBOOT_INTO_DISKMODE_FLAG		0x00000001
#define SP_IMAGE_REBOOT_INTO_RETAILMODE_FLAG	0x00000002
#define SP_IMAGE_USER_KEY_PRESS_FLAG			0x00000004
#define SP_IMAGE_HIBERNATE_FLAG					0x00000008
#define SP_IMAGE_BANGFOLDER						0x00000010

//-----------------------------------------------------------------------------
// System info structure pointer has been moved up into the special space now.
// 

#define SP_SYSINFO_ADDR_PTR_OFFSET			0x18 // - 0x1F

//-----------------------------------------------------------------------------
// Storage area for RTC reference time across boots.
// 

#define SP_RTC_REF_TIME_STRUCT_OFFSET		0x20 // - 0x2B

//-----------------------------------------------------------------------------
// COP publishes its exception status in this area.
// Reserved for see t_watchdog.h.
// #define SP_COP_EXCEPTION_STATUS_OFFSET		0x2C // - 0x2F

//-----------------------------------------------------------------------------
// Storage area for Locale information to communicate to DiskMode.
// 

#define SP_LOCALE_TYPE_PTR_OFFSET			0x30 // - 0x33

#ifdef __cplusplus
}
#endif

#endif // __SPTYPES_H__ //------------------

