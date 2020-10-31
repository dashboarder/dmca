/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _USB_REGISTERS_H_
#define _USB_REGISTERS_H_

#include <platform/soc/hwregbase.h>

#define USBREG(offset)		(USBOTG_BASE_ADDR + (offset))

// Register definitions for the Synopsis DWC USB OTG core as
// implemented in the Samsung S5L8900X
//
// Constants were obtained from the header file "_s5l8701.h"
// supplied with the H1 diagnostics and may be subject to
// other licensing conditions.
//
//
// All registers are 32 bits wide.

/*******************************************************************************
 S5L8701 USB Core Global Registers
 *******************************************************************************/

#define DWCUSB_GOTGCTL              USBREG(0x00000000)  /* OTG Control and Status Register */
#define DWCUSB_GOTGINT              USBREG(0x00000004)  /* OTG Interrupt Register */
#define DWCUSB_GAHBCFG              USBREG(0x00000008)  /* Core AHB Configuration Register */
#define DWCUSB_GUSBCFG              USBREG(0x0000000C)  /* Core USB Configuration Register */
#define DWCUSB_GRSTCTL              USBREG(0x00000010)  /* Core Reset Register */
#define DWCUSB_GINTSTS              USBREG(0x00000014)  /* Core Interrupt Status Register */
#define DWCUSB_GINTMSK              USBREG(0x00000018)  /* Core Interrupt Mask Register */
#define DWCUSB_GRXSTSR              USBREG(0x0000001C)  /* Receive Status Debug Read Register */
#define DWCUSB_GRXSTSP              USBREG(0x00000020)  /* Receive Status Read/Pop Register */
#define DWCUSB_GRXFSIZ              USBREG(0x00000024)  /* Receive FIFO Size Register */
#define DWCUSB_GNPTXFSIZ            USBREG(0x00000028)  /* Non-Periodic Transmit FIFO Size Register */
#define DWCUSB_GNPTXSTS             USBREG(0x0000002C)  /* Non-Periodic Transmit FIFO/Queue Status Register */
#define DWCUSB_GI2CCTL              USBREG(0x00000030)  /* I2C Access Register */
#define DWCUSB_GPVNDCTL             USBREG(0x00000034)  /* PHY Vendor Control Register */
#define DWCUSB_GGPIO                USBREG(0x00000038)  /* General Purpose I/O Register */
#define DWCUSB_GUID                 USBREG(0x0000003C)  /* User ID Register */
#define DWCUSB_GSNPSID              USBREG(0x00000040)  /* Synopsys ID Register */
#define DWCUSB_GHWCFG1              USBREG(0x00000044)  /* User HW Config1 Register */
#define DWCUSB_GHWCFG2              USBREG(0x00000048)  /* User HW Config2 Register */
#define DWCUSB_GHWCFG3              USBREG(0x0000004C)  /* User HW Config3 Register */
#define DWCUSB_GHWCFG4              USBREG(0x00000050)  /* User HW Config4 Register */
#define DWCUSB_HPTXFSIZ             USBREG(0x00000100)  /* Host Periodic Transmit FIFO Size Register */
#define DWCUSB_DPTXFSIZ(_n)         USBREG(0x00000104 + ((_n) - 1) * 4) /* Device Periodic Transmit FIFO Size Register */
#define DWCUSB_DIEPTXF(_n)          USBREG(0x00000104 + ((_n) - 1) * 4) /* Device IN EP Transmit FIFO Size Register */

/* OTG Control and Status Register Bit Definitions */

#define DWCUSB_GOTGCTL_BSESVLD      0x00080000          /* B-Session Valid */
#define DWCUSB_GOTGCTL_ASESVLD      0x00040000          /* A-Session Valid */
#define DWCUSB_GOTGCTL_DBNCTIME     0x00020000          /* Long/Short Debounce Time, 0:Long, 1:Short */
#define DWCUSB_GOTGCTL_CONIDSTS_B   0x00010000          /* Connector ID Status, 0:A-Device, 1:B-Device */
#define DWCUSB_GOTGCTL_DEVHNPEN     0x00000800          /* Device HNP Enabled */
#define DWCUSB_GOTGCTL_HSTSETHNPEN  0x00000400          /* Host Set HNP Enable */
#define DWCUSB_GOTGCTL_HNPREQ       0x00000200          /* HNP Request */
#define DWCUSB_GOTGCTL_HSTNEGSCS    0x00000100          /* Host Negotiation Success */
#define DWCUSB_GOTGCTL_SESREQ       0x00000002          /* Session Request */
#define DWCUSB_GOTGCTL_SESREQSCS    0x00000001          /* Session Request Success */

/* OTG Interrupt Register Bit Definitions */

#define DWCUSB_GOTGINT_DBNCEDONE        0x00080000      /* Debounce Done */
#define DWCUSB_GOTGINT_ADEVTOUTCHG      0x00040000      /* A-Device Timeout Change */
#define DWCUSB_GOTGINT_HSTNEGDET        0x00020000      /* Host Negotiation Detected */
#define DWCUSB_GOTGINT_HSTNEGSUCSTSCHG  0x00000200      /* Host Negotiation Success Status Change */
#define DWCUSB_GOTGINT_SESREQSUCSTSCHG  0x00000100      /* Session Request Success Status Change */
#define DWCUSB_GOTGINT_SESENDDET        0x00000004      /* Session End Detect */

/* Core AHB Configuration Register Bit Definitions */

#define DWCUSB_GAHBCFG_PTXFEMPLVL_HALF  0x00000000      /* Periodic TxFIFO Empty Level, Half Empty */
#define DWCUSB_GAHBCFG_PTXFEMPLVL_FULL  0x00000100      /* Periodic TxFIFO Empty Level, Completely Empty */
#define DWCUSB_GAHBCFG_NPTXFEMPLVL_HALF 0x00000000      /* Non-Periodic TxFIFO Empty Level, Half Empty */
#define DWCUSB_GAHBCFG_NPTXFEMPLVL_FULL 0x00000080      /* Non-Periodic TxFIFO Empty Level, Completely Empty */
#define DWCUSB_GAHBCFG_DMAEN            0x00000020      /* DMA Enable */
#define DWCUSB_GAHBCFG_HBST_MASK        0x0000001E      /* Burst Length/Type */
#define   DWCUSB_GAHBCFG_HBST_1         0x00000000      /*   1 word */
#define   DWCUSB_GAHBCFG_HBST_4         0x00000002      /*   4 words */
#define   DWCUSB_GAHBCFG_HBST_8         0x00000004      /*   8 words */
#define   DWCUSB_GAHBCFG_HBST_16        0x00000006      /*  16 words */
#define   DWCUSB_GAHBCFG_HBST_32        0x00000008      /*  32 words */
#define   DWCUSB_GAHBCFG_HBST_64        0x0000000A      /*  64 words */
#define   DWCUSB_GAHBCFG_HBST_128       0x0000000C      /* 128 words */
#define   DWCUSB_GAHBCFG_HBST_256       0x0000000E      /* 256 words */
#define   DWCUSB_GAHBCFG_HBST_SINGLE    0x00000000      /* Single */
#define   DWCUSB_GAHBCFG_HBST_INCR      0x00000002      /* INCR */
#define   DWCUSB_GAHBCFG_HBST_INCR4     0x00000006      /* INCR4 */
#define   DWCUSB_GAHBCFG_HBST_INCR8     0x0000000A      /* INCR8 */
#define   DWCUSB_GAHBCFG_HBST_INCR16    0x0000000E      /* INCR16 */
#define DWCUSB_GAHBCFG_GLBLINTRMSK      0x00000001      /* Global Interrupt Mask */

/* Core USB Configuration Register Bit Definitions */

#define DWCUSB_GUSBCFG_OTGI2CSEL        0x00010000      /* UTMIFS I2C Interface Select */
#define DWCUSB_GUSBCFG_PHYLPWRCLKSEL    0x00008000      /* PHY Low-Power Clock Select */
#define DWCUSB_GUSBCFG_USBTRDTIM_MASK   0x00003C00      /* USB Turnaround Time */
#define DWCUSB_GUSBCFG_USBTRDTIM_SHIFT  10
#define DWCUSB_GUSBCFG_HNPCAP           0x00000200      /* HNP-Capable */
#define DWCUSB_GUSBCFG_SRPCAP           0x00000100      /* SRP-Capable */
#define DWCUSB_GUSBCFG_ULPIIF_SDR       0x00000000      /* Single Data Rate ULPI Interface */
#define DWCUSB_GUSBCFG_ULPIIF_DDR       0x00000080      /* Double Data Rate ULPI Interface */
#define DWCUSB_GUSBCFG_PHYSEL_HS        0x00000000      /* USB 2.0 High-Speed UTMI+ or ULPI PHY */
#define DWCUSB_GUSBCFG_PHYSEL_FS        0x00000040      /* USB 1.1 Full-Speed Serial Transceiver */
#define DWCUSB_GUSBCFG_FSIF_UNI         0x00000000      /* Full-Speed Serial Interface Select, Unidirectional */
#define DWCUSB_GUSBCFG_FSIF_BI          0x00000020      /* Full-Speed Serial Interface Select, Bidirectional */
#define DWCUSB_GUSBCFG_UTMI             0x00000000      /* UTMI+ Interface */
#define DWCUSB_GUSBCFG_ULPI             0x00000010      /* ULPI Interface */
#define DWCUSB_GUSBCFG_PHYIF_8          0x00000000      /* PHY Interface, 8 bits */
#define DWCUSB_GUSBCFG_PHYIF_16         0x00000008      /* PHY Interface, 16 bits */
#define DWCUSB_GUSBCFG_TOUTCAL_MASK     0x00000007      /* HS/FS Timeout Calibration */
#define DWCUSB_GUSBCFG_TOUTCAL_SHIFT	0

/* Core Reset Register Bit Definitions */

#define DWCUSB_GRSTCTL_AHBIDLE          0x80000000      /* AHB Master Idle */
#define DWCUSB_GRSTCTL_DMAREQ           0x40000000      /* DMA Request Signal */
#define DWCUSB_GRSTCTL_TXFNUM_MASK      0x000007C0      /* TxFIFO Number */
#define DWCUSB_GRSTCTL_TXFNUM_SHIFT	6
#define   DWCUSB_GRSTCTL_TXFNUM_NP      0x00000000      /* Flush Non-Periodic TxFIFO */
#define   DWCUSB_GRSTCTL_TXFNUM_P(_n)   ((_n) << 6)     /* Flush Periodic TxFIFO 1 ~ 15 */
#define   DWCUSB_GRSTCTL_TXFNUM_ALL     0x00000400      /* Flush All TxFIFO */
#define DWCUSB_GRSTCTL_TXFFLSH          0x00000020      /* TxFIFO Flush */
#define DWCUSB_GRSTCTL_RXFFLSH          0x00000010      /* RxFIFO Flush */
#define DWCUSB_GRSTCTL_INTKNQFLSH       0x00000008      /* IN TOken Sequence Learning Queue Flush */
#define DWCUSB_GRSTCTL_FRMCNTRRST       0x00000004      /* Host Frame Counter Reset */
#define DWCUSB_GRSTCTL_HSFTRST          0x00000002      /* HClk Soft Reset */
#define DWCUSB_GRSTCTL_CSFTRST          0x00000001      /* Core Soft Reset */

/* Core Interrupt Status/Mask Register Bit Definitions */

#define DWCUSB_GINT_WKUPINT         0x80000000          /* Resume/Remote Wakeup Detected Interrupt */
#define DWCUSB_GINT_SESSREQINT      0x40000000          /* Session Request/New Session Detected Interrupt */
#define DWCUSB_GINT_DISCONNINT      0x20000000          /* Disconnect Detected Interrupt */
#define DWCUSB_GINT_CONIDSTSCHG     0x10000000          /* Connector ID Status Change */
#define DWCUSB_GINT_PTXFEMP         0x04000000          /* Periodic TxFIFO Empty */
#define DWCUSB_GINT_HCHINT          0x02000000          /* Host Channels Interrupt */
#define DWCUSB_GINT_PRTINT          0x01000000          /* Host Port Interrupt */
#define DWCUSB_GINT_FETSUSP         0x00400000          /* Data Fetch Suspended */
#define DWCUSB_GINT_INCOMPLP        0x00200000          /* Incomplete Periodic Transfer */
#define DWCUSB_GINT_INCOMPLISOOUT   0x00200000          /* Incomplete Isochronous OUT Transfer */
#define DWCUSB_GINT_INCOMPLISOIN    0x00100000          /* Incomplete Isochronous IN Transfer */
#define DWCUSB_GINT_OEPINT          0x00080000          /* OUT Endpoints Interrupt */
#define DWCUSB_GINT_IEPINT          0x00040000          /* IN Endpoints Interrupt */
#define DWCUSB_GINT_EPMIS           0x00020000          /* Endpoint Mismatch Interrupt */
#define DWCUSB_GINT_EOPF            0x00008000          /* End of Periodic Frame Interrupt */
#define DWCUSB_GINT_ISOOUTDROP      0x00004000          /* Isochronous OUT Packet Dropped Interrupt */
#define DWCUSB_GINT_ENUMDONE        0x00002000          /* Enumeration Done */
#define DWCUSB_GINT_USBRST          0x00001000          /* USB Reset */
#define DWCUSB_GINT_USBSUSP         0x00000800          /* USB Suspend */
#define DWCUSB_GINT_ERLYSUSP        0x00000400          /* Early Suspend */
#define DWCUSB_GINT_I2CINT          0x00000200          /* I2C Interrupt */
#define DWCUSB_GINT_I2CCKINT        0x00000100          /* I2C CarKit Interrupt */
#define DWCUSB_GINT_GOUTNAKEFF      0x00000080          /* Global OUT NAK Effective */
#define DWCUSB_GINT_GINNAKEFF       0x00000040          /* Global IN NAK Effective */
#define DWCUSB_GINT_NPTXFEMP        0x00000020          /* Non-Periodic TxFIFO Empty */
#define DWCUSB_GINT_RXFLVL          0x00000010          /* RxFIFO Non-Empty */
#define DWCUSB_GINT_SOF             0x00000008          /* Start of (Micro)Frame */
#define DWCUSB_GINT_OTGINT          0x00000004          /* OTG Interrupt */
#define DWCUSB_GINT_MODEMIS         0x00000002          /* Mode Mismatch Interrupt */
#define DWCUSB_GINT_CURMOD_HOST     0x00000001          /* Current Mode of Operation, 0:Device, 1:Host */

#define DWCUSB_GINT_OTG_MASK        0x70000006          /* OTG Interrupt */
#define DWCUSB_GINT_HOST_MASK       0x87200338          /* Device Interrupt */
#define DWCUSB_GINT_DEVICE_MASK     0x807EFFF8          /* Host Interrupt */

/* Host Mode Receive Status Debug Read/Status Read and Pop Register Bit Definitions */

#define DWCUSB_HRXSTS_PKTSTS_MASK       0x001E0000      /* Packet Status */
#define   DWCUSB_HRXSTS_PKTSTS_INDATA   0x00040000      /* IN Data Packet Received */
#define   DWCUSB_HRXSTS_PKTSTS_IN       0x00060000      /* IN Transfer Completed */
#define   DWCUSB_HRXSTS_PKTSTS_DTERR    0x000A0000      /* Data Toggle Error */
#define   DWCUSB_HRXSTS_PKTSTS_CHHALT   0x000E0000      /* Channel Halted */
#define DWCUSB_HRXSTS_DPID_MASK         0x00018000      /* Data PID */
#define   DWCUSB_HRXSTS_DPID_DATA0      0x00000000      /* DATA0 */
#define   DWCUSB_HRXSTS_DPID_DATA1      0x00008000      /* DATA1 */
#define   DWCUSB_HRXSTS_DPID_DATA2      0x00010000      /* DATA2 */
#define   DWCUSB_HRXSTS_DPID_DATAM      0x00018000      /* MDATA */
#define DWCUSB_HRXSTS_BCNT_MASK         0x00007FF0      /* Byte Count */
#define DWCUSB_HRXSTS_BCNT_SHIFT        4
#define DWCUSB_HRXSTS_CHNUM_MASK        0x0000000F      /* Channel Number */

/* Device Mode Receive Status Debug Read/Status Read and Pop Register Bit Definitions */

#define DWCUSB_DRXSTS_FN_MASK               0x01E00000  /* Frame Number */
#define DWCUSB_DRXSTS_PKTSTS_MASK           0x001E0000  /* Packet Status */
#define   DWCUSB_DRXSTS_PKTSTS_GOUTNAK      0x00020000  /* Global OUT NAK */
#define   DWCUSB_DRXSTS_PKTSTS_OUTDATA      0x00040000  /* OUT Data Packet Received */
#define   DWCUSB_DRXSTS_PKTSTS_OUT          0x00060000  /* OUT Transfer Completed */
#define   DWCUSB_DRXSTS_PKTSTS_SETUP        0x00080000  /* SETUP Transaction Completed */
#define   DWCUSB_DRXSTS_PKTSTS_SETUPDATA    0x000C0000  /* SETUP Data Packet Received */
#define DWCUSB_DRXSTS_DPID_MASK             0x00018000  /* Data PID */
#define   DWCUSB_DRXSTS_DPID_DATA0          0x00000000  /* DATA0 */
#define   DWCUSB_DRXSTS_DPID_DATA1          0x00008000  /* DATA1 */
#define   DWCUSB_DRXSTS_DPID_DATA2          0x00010000  /* DATA2 */
#define   DWCUSB_DRXSTS_DPID_DATAM          0x00018000  /* MDATA */
#define DWCUSB_DRXSTS_BCNT_MASK             0x00007FF0  /* Byte Count */
#define DWCUSB_DRXSTS_BCNT_SHIFT            4
#define DWCUSB_DRXSTS_CHNUM_MASK            0x0000000F  /* Channel Number */

/* Receive FIFO Size Register Bit Definitions */

#define DWCUSB_GRXFSIZ_RXFDEP_MASK     0x0000FFFF      /* RxFIFO Depth, 16~32768 */

/* Non-Periodic Transmit FIFO Size Register Bit Definitions */

#define DWCUSB_GNPTXFSIZ_NPTXFDEP_MASK      0xFFFF0000  /* Non-Periodic TxFIFO Depth, 16~32768 */
#define DWCUSB_GNPTXFSIZ_NPTXFDEP_SHIFT     16
#define DWCUSB_GNPTXFSIZ_NPTXFSTADDR_MASK   0x0000FFFF  /* Non-Periodic Transmit RAM Start Address */
#define DWCUSB_GNPTXFSIZ_NPTXFSTADDR_SHIFT  0

/* Non-Periodic Transmit FIFO/Queue Status Register Bit Definitions */

#define DWCUSB_GNPTXSTS_NPTXQTOP_MASK      0x7F000000  /* Top of the Non-Periodic Transmit Request Queue */
#define DWCUSB_GNPTXSTS_NPTXQTOP_SHIFT     24
#define DWCUSB_GNPTXSTS_NPTXQAVAIL_MASK    0x00FF0000  /* Non-Periodic Transmit Request Queue Space Available */
#define DWCUSB_GNPTXSTS_NPTXQAVAIL_SHIFT   16
#define DWCUSB_GNPTXSTS_NPTXFAVAIL_MASK    0x0000FFFF  /* Non-Periodic TxFIFO Space Available */
#define DWCUSB_GNPTXSTS_NPTXFAVAIL_SHIFT   0

/* I2C Access Register Bit Definitions */

#define DWCUSB_GI2CCTL_BSYDONE          0x80000000      /* I2C Busy/Done */
#define DWCUSB_GI2CCTL_READ             0x00000000      /* Read */
#define DWCUSB_GI2CCTL_WRITE            0x40000000      /* Write */
#define DWCUSB_GI2CCTL_VPVM             0x00000000      /* VP-VM USB Mode */
#define DWCUSB_GI2CCTL_DATSE0           0x10000000      /* DAT_SE0 USB Mode */
#define DWCUSB_GI2CCTL_DEVADDR_MASK     0x0C000000      /* I2C Device Address */
#define   DWCUSB_GI2CCTL_DEVADDR_2C     0x00000000      /* 2C */
#define   DWCUSB_GI2CCTL_DEVADDR_2D     0x04000000      /* 2D */
#define   DWCUSB_GI2CCTL_DEVADDR_2E     0x08000000      /* 2E */
#define   DWCUSB_GI2CCTL_DEVADDR_2F     0x0C000000      /* 2F */
#define DWCUSB_GI2CCTL_SUSPCTL          0x02000000      /* I2C Suspend Control */
#define DWCUSB_GI2CCTL_ACK              0x01000000      /* I2C ACK */
#define DWCUSB_GI2CCTL_EN               0x00800000      /* I2C Enable */
#define DWCUSB_GI2CCTL_ADDR_MASK        0x007F0000      /* I2C Address */
#define DWCUSB_GI2CCTL_ADDR_SHIFT       16
#define DWCUSB_GI2CCTL_REGADDR_MASK     0x0000FF00      /* I2C Register Address */
#define DWCUSB_GI2CCTL_REGADDR_SHIFT    8
#define DWCUSB_GI2CCTL_RWDATA_MASK      0x000000FF      /* I2C Read/Write Data */
#define DWCUSB_GI2CCTL_RWDATA_SHIFT     0

/* PHY Vendor Control Register Bit Definitions */

#define DWCUSB_GPVNDCTL_VSTSDONE        0x08000000      /* VStatus Done */
#define DWCUSB_GPVNDCTL_VSTSBSY         0x04000000      /* VStatus Busy */
#define DWCUSB_GPVNDCTL_NEWREGREQ       0x02000000      /* New Register Request */
#define DWCUSB_GPVNDCTL_REGWR           0x00400000      /* Register Write */
#define DWCUSB_GPVNDCTL_REGADDR_MASK    0x003F0000      /* Register Address */
#define DWCUSB_GPVNDCTL_REGADDR_SHIFT   16
#define DWCUSB_GPVNDCTL_VCTRL_MASK      0x0000FF00      /* UTMI+ Vendor Control Register Address */
#define DWCUSB_GPVNDCTL_VCTRL_SHIFT     8
#define DWCUSB_GPVNDCTL_REGDATA_MASK    0x000000FF      /* Register Data */
#define DWCUSB_GPVNDCTL_REGDATA_SHIFT   0

/* General Purpose I/O Register Bit Definitions */

#define DWCUSB_GGPIO_GPO_MASK       0xFFFF0000          /* General Purpose Output */
#define DWCUSB_GGPIO_GPO_SHIFT      16
#define DWCUSB_GGPIO_GPI_MASK       0x0000FFFF          /* General Purpose Input */
#define DWCUSB_GGPIO_GPI_SHIFT      0

/* User HW Config1 Register Bit Definitions */

#define DWCUSB_GHWCFG1_BIDIR        0x00000000          /* BIDIR Endpoint */
#define DWCUSB_GHWCFG1_IN           0x00000001          /* IN Endpoint */
#define DWCUSB_GHWCFG1_OUT          0x00000002          /* OUT Endpoint */
#define DWCUSB_GHWCFG1_EP_MASK      0x00000003
#define DWCUSB_GHWCFG1_EP_SHIFT     2

/* User HW Config2 Register Bit Definitions */

#define DWCUSB_GHWCFG2_TKNQDEPTH_MASK   0x3C000000      /* Device Mode IN Token Sequence Learning Queue Depth, 0~30 */
#define DWCUSB_GHWCFG2_TKNQDEPTH_SHIFT  26
#define DWCUSB_GHWCFG2_PTXQDEPTH_MASK   0x03000000      /* Host Mode Periodic Request Queue Depth */
#define   DWCUSB_GHWCFG2_PTXQDEPTH_2    0x00000000
#define   DWCUSB_GHWCFG2_PTXQDEPTH_4    0x01000000
#define   DWCUSB_GHWCFG2_PTXQDEPTH_8    0x02000000
#define DWCUSB_GHWCFG2_NPTXQDEPTH_MASK  0x00C00000      /* Non-Periodic Request Queue Depth */
#define   DWCUSB_GHWCFG2_NPTXQDEPTH_2   0x00000000
#define   DWCUSB_GHWCFG2_NPTXQDEPTH_4   0x00400000
#define   DWCUSB_GHWCFG2_NPTXQDEPTH_8   0x00800000
#define DWCUSB_GHWCFG2_DYNFIFOSIZING    0x00080000      /* Dynamic FIFO Sizing Enabled */
#define DWCUSB_GHWCFG2_POUTSUPPORT      0x00040000      /* Periodic OUT Channels Supported in Host Mode */
#define DWCUSB_GHWCFG2_NUMHSTCHNL_MASK  0x0003C000      /* Number of Host Channels, 0~15 */
#define DWCUSB_GHWCFG2_NUMHSTCHNL_SHIFT 14
#define DWCUSB_GHWCFG2_NUMDEVEPS_MASK   0x00003C00      /* Number of Device Endpoints, 0~15 */
#define DWCUSB_GHWCFG2_NUMDEVEPS_SHIFT  10
#define DWCUSB_GHWCFG2_FSPHYTYPE_MASK   0x00000300      /* Full-Speed PHY Interface Type */
#define   DWCUSB_GHWCFG2_FSPHYTYPE_NOFS 0x00000000      /* Full-Speed Interface Not supported */
#define   DWCUSB_GHWCFG2_FSPHYTYPE_FS   0x00000100      /* Dedicated Full-Speed Interface */
#define   DWCUSB_GHWCFG2_FSPHYTYPE_UTMI 0x00000200      /* FS Pins Share with UTMI+ Pins */
#define   DWCUSB_GHWCFG2_FSPHYTYPE_ULPI 0x00000300      /* FS Pins Share with ULPI Pins */
#define DWCUSB_GHWCFG2_HSPHYTYPE_MASK   0x000000C0      /* High-Speed PHY Interface Type */
#define   DWCUSB_GHWCFG2_HSPHYTYPE_NOHS 0x00000000      /* High-Speed Interface Not supported */
#define   DWCUSB_GHWCFG2_HSPHYTYPE_UTMI 0x00000040      /* UTMI+ */
#define   DWCUSB_GHWCFG2_HSPHYTYPE_ULPI 0x00000080      /* ULPI */
#define   DWCUSB_GHWCFG2_HSPHYTYPE_ANY  0x000000C0      /* UTMI+ and ULPI */
#define DWCUSB_GHWCFG2_MULTIPOINT       0x00000000      /* Multi-Point */
#define DWCUSB_GHWCFG2_SINGLEPOINT      0x00000020      /* Single-Point */
#define DWCUSB_GHWCFG2_ARCH_MASK        0x00000018      /* Architecture */
#define   DWCUSB_GHWCFG2_ARCH_SLAVE     0x00000000      /* Slave-Only */
#define   DWCUSB_GHWCFG2_ARCH_EXTDMA    0x00000008      /* External DMA */
#define   DWCUSB_GHWCFG2_ARCH_INTDMA    0x00000010      /* Internal DMA */
#define DWCUSB_GHWCFG2_MODE_MASK        0x00000007      /* Mode of Operation */
#define   DWCUSB_GHWCFG2_MODE_FULLOTG   0x00000000      /* HNP & SRP Capable OTG */
#define   DWCUSB_GHWCFG2_MODE_SRPOTG    0x00000001      /* SRP Capable OTG */
#define   DWCUSB_GHWCFG2_MODE_MINOTG    0x00000002      /* Non-HNP & Non-SRP Capable OTG */
#define   DWCUSB_GHWCFG2_MODE_SRPDEV    0x00000003      /* SRP Capable Device */
#define   DWCUSB_GHWCFG2_MODE_DEV       0x00000004      /* Non-OTG Device */
#define   DWCUSB_GHWCFG2_MODE_SRPHST    0x00000005      /* SRP Capable Host */
#define   DWCUSB_GHWCFG2_MODE_HOST      0x00000006      /* Non-OTG Host */

/* User HW Config3 Register Bit Definitions */

#define DWCUSB_GHWCFG3_DFIFODEPTH_MASK      0xFFFF0000  /* DFIFO Depth, 32~32768 */
#define DWCUSB_GHWCFG3_DFIFODEPTH_SHIFT     16
#define DWCUSB_GHWCFG3_AHBPHYSYNC           0x00001000  /* AHB and PHY Synchronous */
#define DWCUSB_GHWCFG3_RSTTYPE_ASYNC        0x00000000  /* Asynchronous Reset */
#define DWCUSB_GHWCFG3_RSTTYPE_SYNC         0x00000800  /* Synchronous Reset */
#define DWCUSB_GHWCFG3_OPTFEATURE           0x00000400  /* Optional Features Removed */
#define DWCUSB_GHWCFG3_VCTRLIF              0x00000200  /* Vendor Control Interface is Available */
#define DWCUSB_GHWCFG3_I2CIF                0x00000100  /* I2C Interface is Available */
#define DWCUSB_GHWCFG3_OTGEN                0x00000080  /* OTG Function Enabled */
#define DWCUSB_GHWCFG3_PKTSIZEWIDTH_MASK    0x00000070  /* Width of Packet Size Counters */
#define DWCUSB_GHWCFG3_PKTSIZEWIDTH_SHIFT   4
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_4     0x00000000  /* 4 bits */
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_5     0x00000010  /* 5 bits */
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_6     0x00000020  /* 6 bits */
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_7     0x00000030  /* 7 bits */
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_8     0x00000040  /* 8 bits */
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_9     0x00000050  /* 9 bits */
#define   DWCUSB_GHWCFG3_PKTSIZEWIDTH_10    0x00000060  /* 10 bits */
#define DWCUSB_GHWCFG3_XFERSIZEWIDTH_MASK   0x0000000F  /* Width of Transfer Size Counters */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_11   0x00000000  /* 11 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_12   0x00000001  /* 12 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_13   0x00000002  /* 13 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_14   0x00000003  /* 14 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_15   0x00000004  /* 15 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_16   0x00000005  /* 16 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_17   0x00000006  /* 17 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_18   0x00000007  /* 18 bits */
#define   DWCUSB_GHWCFG3_XFERSIZEWIDTH_19   0x00000008  /* 19 bits */

/* User HW Config4 Register Bit Definitions */

#define DWCUSB_GHWCFG4_DEDFIFOMODE	    0x02000000  /* Dedicated FIFO Enabled */
#define DWCUSB_GHWCFG4_SESSENDFLTR          0x01000000  /* "session_end" Filter Enabled */
#define DWCUSB_GHWCFG4_BVALIDFLTR           0x00800000  /* "b_valid" Filter Enabled */
#define DWCUSB_GHWCFG4_AVALIDFLTR           0x00400000  /* "a_valid" Filter Enabled */
#define DWCUSB_GHWCFG4_VBUSVALIDFLTR        0x00200000  /* "vbus_valid" Filter Enabled */
#define DWCUSB_GHWCFG4_IDDGFLTR             0x00100000  /* "iddig" Filter Enabled */
#define DWCUSB_GHWCFG4_NUMCTLEPS_MASK       0x000F0000  /* Number of Device Mode Control Endpoints in Addition to Endpoint 0, 0~15 */
#define DWCUSB_GHWCFG4_NUMCTLEPS_SHIFT      16
#define DWCUSB_GHWCFG4_HSPHYDWIDTH_MASK     0x0000C000  /* HS PHY Data Width */
#define   DWCUSB_GHWCFG4_HSPHYDWIDTH_8      0x00000000  /* 8 bits */
#define   DWCUSB_GHWCFG4_HSPHYDWIDTH_16     0x00004000  /* 16 bits */
#define   DWCUSB_GHWCFG4_HSPHYDWIDTH_8_16   0x00008000  /* 8/16 bits, S/W Selectable */
#define DWCUSB_GHWCFG4_AHBFREQ              0x00000020  /* Minimum AHB Frequency Less Than 60 MHz */
#define DWCUSB_GHWCFG4_ENABLEPWROPT         0x00000010  /* Enable Power Optimization */
#define DWCUSB_GHWCFG4_NUMDEVPIEPS_MASK     0x0000000F  /* Number of Device Mode Periodic IN Endpoints */
#define DWCUSB_GHWCFG4_NUMDEVPIEPS_SHIFT    0

/* Host Periodic Transmit FIFO Size Register Bit Definitions */

#define DWCUSB_HPTXFSIZ_PTXFDEP_MASK        0xFFFF0000  /* Host Periodic TxFIFO Depth, 16~32768 */
#define DWCUSB_HPTXFSIZ_PTXFDEP_SHIFT       16
#define DWCUSB_HPTXFSIZ_PTXFSTADDR_MASK     0x0000FFFF  /* Host Periodic TxFIFO Start Address */
#define DWCUSB_HPTXFSIZ_PTXFSTADDR_SHIFT    0

/* Device Periodic Transmit FIFO Size Register Bit Definitions */

#define DWCUSB_DPTXFSIZ_PTXFDEP_MASK        0xFFFF0000  /* Host Periodic TxFIFO Depth, 4~768 */
#define DWCUSB_DPTXFSIZ_PTXFDEP_SHIFT       16
#define DWCUSB_DPTXFSIZ_PTXFSTADDR_MASK     0x0000FFFF  /* Host Periodic TxFIFO Start Address */
#define DWCUSB_DPTXFSIZ_PTXFSTADDR_SHIFT    0

/* Device IN EP Transmit FIFO Size Register Bit Definitions */

#define DWCUSB_DIEPTXF_INEPTXFDEP_MASK        0xFFFF0000  /* Host Periodic TxFIFO Depth, 4~768 */
#define DWCUSB_DIEPTXF_INEPTXFDEP_SHIFT       16
#define DWCUSB_DIEPTXF_INEPTXFSTADDR_MASK     0x0000FFFF  /* Host Periodic TxFIFO Start Address */
#define DWCUSB_DIEPTXF_INEPTXFSTADDR_SHIFT    0

/*******************************************************************************
 S5L8701 USB Core Host Registers
 *******************************************************************************/

#define DWCUSB_HCFG                 USBREG(0x00000400)  /* Host Configuration Register */
#define DWCUSB_HFIR                 USBREG(0x00000404)  /* Host Frame Interval Register */
#define DWCUSB_HNUM                 USBREG(0x00000408)  /* Host Frame Number/Time Remaining Register */
#define DWCUSB_HPTXSTS              USBREG(0x00000410)  /* Host Periodic Transmit FIFO/Queue Status Register */
#define DWCUSB_HAINT                USBREG(0x00000414)  /* Host All Channels Interrupt Register */
#define DWCUSB_HAINTMSK             USBREG(0x00000418)  /* Host All Channels Interrupt Mask Register */
#define DWCUSB_HPRT                 USBREG(0x00000440)  /* Host Port Control and Status Register */
#define DWCUSB_HCCHAR(_n)           USBREG(0x00000500 + (_n) * 0x20)    /* Host Channel Characteristics Register */
#define DWCUSB_HCSPLT(_n)           USBREG(0x00000504 + (_n) * 0x20)    /* Host Channel Split Control Register */
#define DWCUSB_HCINT(_n)            USBREG(0x00000508 + (_n) * 0x20)    /* Host Channel Interrupt Status Register */
#define DWCUSB_HCINTMSK(_n)         USBREG(0x0000050C + (_n) * 0x20)    /* Host Channel Interrupt Mask Register */
#define DWCUSB_HCTSIZ(_n)           USBREG(0x00000510 + (_n) * 0x20)    /* Host Channel Transfer Size Register */
#define DWCUSB_HCDMA(_n)            USBREG(0x00000514 + (_n) * 0x20)    /* Host Channel DMA Address Register */

/* Host Configuration Register Bit Definitions */

#define DWCUSB_HCFG_HSFSLSSUPP          0x00000000      /* HS/FS/LS Support */
#define DWCUSB_HCFG_FSLSSUPP            0x00000004      /* FS/LS Only Support */
#define DWCUSB_HCFG_FSPCLKSEL_MASK      0x00000003      /* FS PHY Clock */
#define   DWCUSB_HCFG_FSPCLKSEL_30_60   0x00000000      /* FS PHY Clock is 30/60 MHz */
#define   DWCUSB_HCFG_FSPCLKSEL_48      0x00000001      /* FS PHY Clock is 48 MHz */
#define DWCUSB_HCFG_LSPCLKSEL_MASK      0x00000003      /* LS PHY Clock */
#define   DWCUSB_HCFG_LSPCLKSEL_30_60   0x00000000      /* LS PHY Clock is 30/60 MHz */
#define   DWCUSB_HCFG_LSPCLKSEL_48      0x00000001      /* LS PHY Clock is 48 MHz */
#define   DWCUSB_HCFG_LSPCLKSEL_6       0x00000002      /* LS PHY Clock is 48 MHz */

/* Host Frame Interval Register Bit Definitions */

#define DWCUSB_HFIR_FRINT_MASK      0x0000FFFF          /* Frame Interval */

/* Host Frame Number/Time Remaining Register Bit Definitions */

#define DWCUSB_HNUM_FRREM_MASK      0xFFFF0000          /* Frame Interval */
#define DWCUSB_HNUM_FRREM_SHIFT     16
#define DWCUSB_HNUM_FRNUM_MASK      0x0000FFFF          /* Frame Number */
#define DWCUSB_HNUM_FRNUM_SHIFT     0

/* Host Periodic Transmit FIFO/Queue Status Register Bit Definitions */

#define DWCUSB_HPTXSTS_PTXQTOP_ODDFRM       0x80000000  /* Send in Odd (Micro)Frame */
#define DWCUSB_HPTXSTS_PTXQTOP_EPNUM_MASK   0x78000000  /* Channel/Endpoint Number */
#define DWCUSB_HPTXSTS_PTXQTOP_EPNUM_SHIFT  27
#define DWCUSB_HPTXSTS_PTXQTOP_TYPE_MASK    0x06000000
#define   DWCUSB_HPTXSTS_PTXQTOP_INOUT      0x00000000  /* IN/OUT */
#define   DWCUSB_HPTXSTS_PTXQTOP_ZLENPKT    0x02000000  /* Zero Length Pzcket */
#define   DWCUSB_HPTXSTS_PTXQTOP_CSPLIT     0x04000000  /* CSPLIT */
#define   DWCUSB_HPTXSTS_PTXQTOP_DISCHCMD   0x06000000  /* Disable Channel Command */
#define DWCUSB_HPTXSTS_PTXQSPCAVAIL_MASK    0x00FF0000  /* Periodic Transmit Request Queue Space Available */
#define DWCUSB_HPTXSTS_PTXQSPCAVAIL_SHIFT   16
#define DWCUSB_HPTXSTS_PTXFSPCAVAIL_MASK    0x0000FFFF  /* Periodic Transmit Data FIFO Space Available */
#define DWCUSB_HPTXSTS_PTXFSPCAVAIL_SHIFT   0

/* Host Port Control and Status Register Bit Definitions */

#define DWCUSB_HPRT_PRTSPD_MASK         0x00060000      /* Port Speed */
#define   DWCUSB_HPRT_PRTSPD_HS         0x00000000      /* HS */
#define   DWCUSB_HPRT_PRTSPD_FS         0x00020000      /* FS */
#define   DWCUSB_HPRT_PRTSPD_LS         0x00040000      /* LS */
#define DWCUSB_HPRT_PRTTSTCTL_MASK      0x0001E000      /* Port Test Control */
#define DWCUSB_DCTL_TSTCTL_SHIFT	4
#define   DWCUSB_HPRT_PRTTSTCTL_NOTEST  0x00000000      /* Test Mode Disabled */
#define   DWCUSB_HPRT_PRTTSTCTL_TJ      0x00000000      /* Test_J Mode */
#define   DWCUSB_HPRT_PRTTSTCTL_TK      0x00000000      /* Test_K Mode */
#define   DWCUSB_HPRT_PRTTSTCTL_TSN     0x00000000      /* Test_SE0_NAK Mode */
#define   DWCUSB_HPRT_PRTTSTCTL_TP      0x00000000      /* Test_Packet Mode */
#define   DWCUSB_HPRT_PRTTSTCTL_TEN     0x00000000      /* Test_Force_Enable */
#define DWCUSB_HPRT_PRTPWR              0x00001000      /* Port Power, 0:OFF, 1:ON */
#define DWCUSB_HPRT_PRTLNSTS_MASK       0x00000C00      /* Port Line Status */
#define   DWCUSB_HPRT_PRTLNSTS_DM       0x00000800      /* D- */
#define   DWCUSB_HPRT_PRTLNSTS_DP       0x00000C00      /* D+ */
#define DWCUSB_HPRT_PRTRST              0x00000100      /* Port Reset */
#define DWCUSB_HPRT_PRTSUSP             0x00000080      /* Port Suspend */
#define DWCUSB_HPRT_PRTRES              0x00000040      /* Port Resume */
#define DWCUSB_HPRT_PRTOVRCURRCHG       0x00000020      /* Port Overcurrent Change */
#define DWCUSB_HPRT_PRTOVRCURRACT       0x00000010      /* Port Overcurrent Active */
#define DWCUSB_HPRT_PRTENCHG            0x00000008      /* Port Enable/Disable Change */
#define DWCUSB_HPRT_PRTEN               0x00000004      /* Port Enable */
#define DWCUSB_HPRT_PRTCONNDET          0x00000002      /* Port Connect Detected */
#define DWCUSB_HPRT_PRTCONNSTS          0x00000001      /* Port Connect Status, 0:Detach, 1:Attach */

/* Host Channel Characteristics Register Bit Definitions */

#define DWCUSB_HCCHAR_CHENA             0x80000000      /* Channel Enable */
#define DWCUSB_HCCHAR_CHDIS             0x40000000      /* Channel Disable */
#define DWCUSB_HCCHAR_ODDFRM            0x20000000      /* Odd Frame */
#define DWCUSB_HCCHAR_DEVADDR_MASK      0x1FC00000      /* Device Address */
#define DWCUSB_HCCHAR_DEVADDR_SHIFT     22
#define DWCUSB_HCCHAR_MC_EC_MASK        0x00300000      /* Multi Count / Error Count */
#define DWCUSB_HCCHAR_MC_EC_SHIFT       20
#define DWCUSB_HCCHAR_EPTYPE_MASK       0x000C0000      /* Endpoint Type */
#define   DWCUSB_HCCHAR_EPTYPE_CTL      0x00000000      /* Control */
#define   DWCUSB_HCCHAR_EPTYPE_ISO      0x00040000      /* Isochronous */
#define   DWCUSB_HCCHAR_EPTYPE_BULK     0x00080000      /* Bulk */
#define   DWCUSB_HCCHAR_EPTYPE_INT      0x000C0000      /* Interrupt */
#define DWCUSB_HCCHAR_LSPDDEV           0x00020000      /* Low Speed Device */
#define DWCUSB_HCCHAR_EPDIR_OUT         0x00000000      /* Endpoint Direction, OUT */
#define DWCUSB_HCCHAR_EPDIR_IN          0x00008000      /* Endpoint Direction, IN */
#define DWCUSB_HCCHAR_EPNUM_MASK        0x00007800      /* Endpoint Number */
#define DWCUSB_HCCHAR_EPNUM_SHIFT       11
#define DWCUSB_HCCHAR_MPS_MASK          0x000007FF      /* Maximum Packet Size */

/* Host Channel Split Control Register Bit Definitions */

#define DWCUSB_HCSPLT_SPLTENA           0x80000000      /* Split Enable */
#define DWCUSB_HCSPLT_COMPSPLT          0x00010000      /* Do Complete Split */
#define DWCUSB_HCSPLT_XACTPOS_MASK      0x0000C000      /* Transaction Position */
#define   DWCUSB_HCSPLT_XACTPOS_ALL     0x0000C000      /* All */
#define   DWCUSB_HCSPLT_XACTPOS_BEGIN   0x00008000      /* Begin */
#define   DWCUSB_HCSPLT_XACTPOS_MID     0x00000000      /* Middle */
#define   DWCUSB_HCSPLT_XACTPOS_END     0x00004000      /* End */
#define DWCUSB_HCSPLT_HUBADDR_MASK      0x00003F80      /* Hub Address */
#define DWCUSB_HCSPLT_HUBADDR_SHIFT     7
#define DWCUSB_HCSPLT_PRTADDR_MASK      0x0000007F      /* Port Address */
#define DWCUSB_HCSPLT_PRTADDR_SHIFT     0

/* Host Channel Interrupt Status/Mask Register Bit Definitions */

#define DWCUSB_HCINT_DATATGLERR         0x00000400      /* Data Toggle Error */
#define DWCUSB_HCINT_FRMOVRUN           0x00000200      /* Frame Overrun */
#define DWCUSB_HCINT_BBLERR             0x00000100      /* Babble Error */
#define DWCUSB_HCINT_XACTERR            0x00000080      /* Transction Error */
#define DWCUSB_HCINT_NYET               0x00000040      /* NYET Response Received */
#define DWCUSB_HCINT_ACK                0x00000020      /* ACK Response Received */
#define DWCUSB_HCINT_NAK                0x00000010      /* NAK Response Received */
#define DWCUSB_HCINT_STALL              0x00000008      /* STALL Response Received */
#define DWCUSB_HCINT_AHBERROR           0x00000004      /* AHB Error */
#define DWCUSB_HCINT_CHHLTD             0x00000002      /* Channel Halted */
#define DWCUSB_HCINT_XFERCOMPL          0x00000001      /* Transfer Completed */

/* Host Channel Transfer Size Register Bit Definitions */

#define DWCUSB_HCTSIZ_DOPNG             0x80000000      /* Do Ping */
#define DWCUSB_HCTSIZ_PID_MASK          0x60000000      /* PID */
#define   DWCUSB_HCTSIZ_PID_DATA0       0x00000000      /* DATA0 */
#define   DWCUSB_HCTSIZ_PID_DATA1       0x20000000      /* DATA1 */
#define   DWCUSB_HCTSIZ_PID_DATA2       0x40000000      /* DATA2 */
#define   DWCUSB_HCTSIZ_PID_MDATA       0x60000000      /* MDATA/SETUP */
#define DWCUSB_HCTSIZ_PKTCNT_MASK       0x1FF80000      /* Packet Count */
#define DWCUSB_HCTSIZ_PKTCNT_SHIFT      19
#define DWCUSB_HCTSIZ_XFERSIZE_MASK     0x0007FFFF      /* Transfer Size */
#define DWCUSB_HCTSIZ_XFERSIZE_SHIFT    0


/*******************************************************************************
 S5L8701 USB Core Device Registers
 *******************************************************************************/

#define DWCUSB_DCFG                 USBREG(0x00000800)  /* Device Configuration Register */
#define DWCUSB_DCTL                 USBREG(0x00000804)  /* Device Control Register */
#define DWCUSB_DSTS                 USBREG(0x00000808)  /* Device Status Register */
#define DWCUSB_DIEPMSK              USBREG(0x00000810)  /* Device IN Endpoint Common Interrupt Mask Register */
#define DWCUSB_DOEPMSK              USBREG(0x00000814)  /* Device OUT Endpoint Common Interrupt Masl Register */
#define DWCUSB_DAINT                USBREG(0x00000818)  /* Device All Endpoint Common Interrupt Register */
#define DWCUSB_DAINTMSK             USBREG(0x0000081C)  /* Device All Endpoint Common Interrupt Mask Register */
#define DWCUSB_DTKNQR1              USBREG(0x00000820)  /* Device IN Token Sequence Learning Queue Read Register 1 */
#define DWCUSB_DTKNQR2              USBREG(0x00000824)  /* Device IN Token Sequence Learning Queue Read Register 2 */
#define DWCUSB_DTKNQR3              USBREG(0x00000830)  /* Device IN Token Sequence Learning Queue Read Register 3 */
#define DWCUSB_DTKNQR4              USBREG(0x00000834)  /* Device IN Token Sequence Learning Queue Read Register 4 */
#define DWCUSB_DVBUSDIS             USBREG(0x00000828)  /* Device VBUS Discharge Time Register */
#define DWCUSB_DVBUSPULSE           USBREG(0x0000082C)  /* Device VBUS Pulsing Time Register */
#define DWCUSB_DIEPCTL(_n)          USBREG(0x00000900 + (_n) * 0x20)    /* Device IN Endpoint Control Register */
#define DWCUSB_DIEPINT(_n)          USBREG(0x00000908 + (_n) * 0x20)    /* Device IN Endpoint Interrupt Register */
#define DWCUSB_DIEPTSIZ(_n)         USBREG(0x00000910 + (_n) * 0x20)    /* Device IN Endpoint Transfer Size Register */
#define DWCUSB_DIEPDMA(_n)          USBREG(0x00000914 + (_n) * 0x20)    /* Device IN Endpoint DMA Address Register */
#define DWCUSB_DOEPCTL(_n)          USBREG(0x00000B00 + (_n) * 0x20)    /* Device OUT Endpoint Control Register */
#define DWCUSB_DOEPINT(_n)          USBREG(0x00000B08 + (_n) * 0x20)    /* Device OUT Endpoint Interrupt Register */
#define DWCUSB_DOEPTSIZ(_n)         USBREG(0x00000B10 + (_n) * 0x20)    /* Device OUT Endpoint Transfer Size Register */
#define DWCUSB_DOEPDMA(_n)          USBREG(0x00000B14 + (_n) * 0x20)    /* Device OUT Endpoint DMA Address Register */

/* Device Configuration Register Bit Definitions */

#define DWCUSB_DCFG_EPMISCNT_MASK   0x007C0000          /* IN Endpoint Mismatch Count */
#define DWCUSB_DCFG_EPMISCNT_SHIFT  18
#define DWCUSB_DCFG_PERFRINT_MASK   0x00001800          /* Periodic Frame Interval */
#define   DWCUSB_DCFG_PERFRINT_80   0x00000000          /* 80% of the (Micro)Frame Interval */
#define   DWCUSB_DCFG_PERFRINT_85   0x00000800          /* 80% of the (Micro)Frame Interval */
#define   DWCUSB_DCFG_PERFRINT_90   0x00001000          /* 80% of the (Micro)Frame Interval */
#define   DWCUSB_DCFG_PERFRINT_95   0x00001800          /* 80% of the (Micro)Frame Interval */
#define DWCUSB_DCFG_DEVADDR_MASK    0x000007F0          /* Device Address */
#define DWCUSB_DCFG_DEVADDR_SHIFT   4
#define DWCUSB_DCFG_NZSTSOUTHSHK    0x00000004          /* Non-Zero-Length Status OUT Handshake */
#define DWCUSB_DCFG_DEVSPD_MASK     0x00000003          /* Device Speed */
#define   DWCUSB_DCFG_DEVSPD_HS     0x00000000          /* High Speed, USB 2.0 PHY Clock is 30MHz or 60MHz */
#define   DWCUSB_DCFG_DEVSPD_FS     0x00000001          /* Full Speed, USB 2.0 PHY Clock is 30MHz or 60MHz */
#define   DWCUSB_DCFG_DEVSPD_LS     0x00000002          /* Low Speed, USB 1.1 Transceiver Clock is 6MHz */
#define   DWCUSB_DCFG_DEVSPD_FS_48  0x00000003          /* Full Speed, USB 1.1 Transceiver Clock is 48MHz */

/* Device Control Register Bit Definitions */

#define DWCUSB_DCTL_PWRONPRGDONE    0x00000800          /* Power-On Programming Done */
#define DWCUSB_DCTL_CGOUTNAK        0x00000400          /* Clear Global OUT NAK */
#define DWCUSB_DCTL_SGOUTNAK        0x00000200          /* Set Global OUT NAK */
#define DWCUSB_DCTL_CGNPINNAK       0x00000100          /* Clear Global Non-Periodic IN NAK */
#define DWCUSB_DCTL_SGNPINNAK       0x00000080          /* Set Global Non-Periodic IN NAK */
#define DWCUSB_DCTL_TSTCTL_MASK     0x00000070          /* Test Control */
#define   DWCUSB_DCTL_TSTCTL_NOTEST 0x00000000          /* Test Mode Disabled */
#define   DWCUSB_DCTL_TSTCTL_TJ     0x00000010          /* Test_J Mode */
#define   DWCUSB_DCTL_TSTCTL_TK     0x00000020          /* Test_K Mode */
#define   DWCUSB_DCTL_TSTCTL_TSN    0x00000030          /* Test_SE0_NAK Mode */
#define   DWCUSB_DCTL_TSTCTL_TP     0x00000040          /* Test_Packet Mode */
#define   DWCUSB_DCTL_TSTCTL_TEN    0x00000050          /* Test_Force_Enable */
#define DWCUSB_DCTL_GOUTNAKSTS      0x00000008          /* Global OUT NAK Status */
#define DWCUSB_DCTL_GNPINNAKSTS     0x00000004          /* Global Non-Periodic IN NAK Status */
#define DWCUSB_DCTL_SFTDISCON       0x00000002          /* Soft Disconnect */
#define DWCUSB_DCTL_RMTWKUPSIG      0x00000001          /* Remote Wakeup Signalling */

/* Device Status Register Bit Definitions */

#define DWCUSB_DSTS_SOFFN_MASK      0x003FFF00          /* (Micro)Frame Number of the Received SOF */
#define DWCUSB_DSTS_SOFFN_SHIFT     8
#define DWCUSB_DSTS_ERRTICERR       0x00000008          /* Erratic Error */
#define DWCUSB_DSTS_ENUMSPD_MASK    0x00000006          /* Enumerated Speed */
#define   DWCUSB_DSTS_ENUMSPD_HS    0x00000000          /* High Speed(PHY Clock is 30MHz or 60MHz) */
#define   DWCUSB_DSTS_ENUMSPD_FS    0x00000002          /* Full Speed(PHY Clock is 30MHz or 60MHz) */
#define   DWCUSB_DSTS_ENUMSPD_LS    0x00000004          /* Low Speed(PHY Clock is 6MHz) */
#define   DWCUSB_DSTS_ENUMSPD_FS_48 0x00000006          /* Full Speed(PHY Clock is 48MHz) */
#define DWCUSB_DSTS_SUSPSTS         0x00000001          /* Suspend Status */

/* Device IN Endpoint Interrupt Status/Mask Register Bit Definitions */

#define DWCUSB_DIEPINT_IEPNAKEFF    0x00000040          /* IN Endpoint NAK Effective */
#define DWCUSB_DIEPINT_INTKNEPMIS   0x00000020          /* IN Token Received with EP Mismatch */
#define DWCUSB_DIEPINT_INTKNTXFEMP  0x00000010          /* IN Token Received when TxFIFO Empty */
#define DWCUSB_DIEPINT_TIMEOUT      0x00000008          /* Timeout Condition(Non-Isochronous Endpoints) */
#define DWCUSB_DIEPINT_AHBERR       0x00000004          /* AHB Error */
#define DWCUSB_DIEPINT_EPDISBLD     0x00000002          /* Endpoint Disabled Interrupt */
#define DWCUSB_DIEPINT_XFERCOMPL    0x00000001          /* Transfer Completed Interrupt */

/* Device OUT Endpoint Interrupt Status/Mask Register Bit Definitions */

#define DWCUSB_DOEPINT_STUPKTRCVD   0x00008000          /* Setup Packet Received */
#define DWCUSB_DOEPINT_OUTTKNEPDIS  0x00000010          /* OUT Token Received when EP Disabled */
#define DWCUSB_DOEPINT_SETUP        0x00000008          /* Setup Phase Done */
#define DWCUSB_DOEPINT_AHBERR       0x00000004          /* AHB Error */
#define DWCUSB_DOEPINT_EPDISBLD     0x00000002          /* Endpoint Disabled Interrupt */
#define DWCUSB_DOEPINT_XFERCOMPL    0x00000001          /* Transfer Completed Interrupt */

/* Device All Endpoint Interrupt Status/Mask Register Bit Definitions */

#define DWCUSB_DAINT_OEP_MASK       0xFFFF0000          /* OUT Endpoint Interrupt */
#define DWCUSB_DAINT_OEP_SHIFT      16
#define DWCUSB_DAINT_IEP_MASK       0x0000FFFF          /* IN Endpoint Interrupt */
#define DWCUSB_DAINT_IEP_SHIFT      0

/* Device Endpoint 0 Control Register Bit Definitions */

#define DWCUSB_DEP0CTL_EPENA        0x80000000          /* Endpoint Enable */
#define DWCUSB_DEP0CTL_EPDIS        0x40000000          /* Endpoint Disable */
#define DWCUSB_DEP0CTL_SNAK         0x08000000          /* Set NAK */
#define DWCUSB_DEP0CTL_CNAK         0x04000000          /* Clear NAK */
#define DWCUSB_DEP0CTL_TXFNUM_MASK  0x03C00000          /* TxFIFO Number, Always 0 */
#define DWCUSB_DEP0CTL_TXFNUM_SHIFT 22
#define DWCUSB_DEP0CTL_STALL        0x00200000          /* STALL Handshake */
#define DWCUSB_DEP0CTL_SNP          0x00100000          /* Snoop Mode */
#define DWCUSB_DEP0CTL_EPTYPE_MASK  0x000C0000          /* Endpoint Type */
#define   DWCUSB_DEP0CTL_EPTYPE_CTL 0x00000000          /* Control */
#define DWCUSB_DEP0CTL_NAKSTS       0x00020000          /* NAK Status */
#define DWCUSB_DEP0CTL_USBACTEP     0x00008000          /* USB Active Endpoint, Always 1 */
#define DWCUSB_DEP0CTL_NEXTEP_MASK  0x00007800          /* Next Endpoint */
#define DWCUSB_DEP0CTL_NEXTEP_SHIFT 11
#define DWCUSB_DEP0CTL_MPS_MASK     0x00000003          /* Maximum Packet Size */
#define   DWCUSB_DEP0CTL_MPS_64     0x00000000          /* 64 bytes */
#define   DWCUSB_DEP0CTL_MPS_32     0x00000001          /* 32 bytes */
#define   DWCUSB_DEP0CTL_MPS_16     0x00000002          /* 16 bytes */
#define   DWCUSB_DEP0CTL_MPS_8      0x00000003          /*  8 bytes */

/* Device Endpoint-n Control Register Bit Definitions */

#define DWCUSB_DEPCTL_EPENA         0x80000000          /* Endpoint Enable */
#define DWCUSB_DEPCTL_EPDIS         0x40000000          /* Endpoint Disable */
#define DWCUSB_DEPCTL_SETD1PID      0x20000000          /* Set DATA1 PID, Interrupt/Bulk */
#define DWCUSB_DEPCTL_SETODDFR      0x20000000          /* Set Odd (Micro)Frame, Isochronous */
#define DWCUSB_DEPCTL_SETD0PID      0x10000000          /* Set DATA0 PID, Interrupt/Bulk */
#define DWCUSB_DEPCTL_SETEVENFR     0x10000000          /* Set Even (Micro)Frame, Isochronous */
#define DWCUSB_DEPCTL_SNAK          0x08000000          /* Set NAK */
#define DWCUSB_DEPCTL_CNAK          0x04000000          /* Clear NAK */
#define DWCUSB_DEPCTL_TXFNUM_MASK   0x03C00000          /* TxFIFO Number, 0~15 */
#define DWCUSB_DEPCTL_TXFNUM_SHIFT  22
#define DWCUSB_DEPCTL_STALL         0x00200000          /* STALL Handshake */
#define DWCUSB_DEPCTL_SNP           0x00100000          /* Snoop Mode */
#define DWCUSB_DEPCTL_EPTYPE_MASK   0x000C0000          /* Endpoint Type */
#define DWCUSB_DEPCTL_EPTYPE_SHIFT  18
#define   DWCUSB_DEPCTL_EPTYPE_CTL  0x00000000          /* Control */
#define   DWCUSB_DEPCTL_EPTYPE_ISO  0x00040000          /* Isochronous */
#define   DWCUSB_DEPCTL_EPTYPE_BULK 0x00080000          /* Bulk */
#define   DWCUSB_DEPCTL_EPTYPE_INT  0x000C0000          /* Interrupt */
#define DWCUSB_DEPCTL_NAKSTS        0x00020000          /* NAK Status */
#define DWCUSB_DEPCTL_DPID_DATA0    0x00000000          /* DATA0 */
#define DWCUSB_DEPCTL_DPID_DATA1    0x00010000          /* DATA1 */
#define DWCUSB_DEPCTL_EOFRNUM_MASK  0x00010000
#define DWCUSB_DEPCTL_FR_EVEN       0x00000000          /* Even (Micro)Frame */
#define DWCUSB_DEPCTL_FR_ODD        0x00010000          /* Odd (Micro)Frame */
#define DWCUSB_DEPCTL_USBACTEP      0x00008000          /* USB Active Endpoint */
#define DWCUSB_DEPCTL_NEXTEP_MASK   0x00007800          /* Next Endpoint */
#define DWCUSB_DEPCTL_NEXTEP_SHIFT  11
#define DWCUSB_DEPCTL_MPS_MASK      0x000007FF          /* Maximum Packet Size */
#define DWCUSB_DEPCTL_MPS_SHIFT     0

/* Device Endpoint 0 Transfer Size Register Bit Definitions */

#define DWCUSB_DEP0TSIZ_SUPCNT_MASK     0x60000000      /* SETUP Packet Count */
#define DWCUSB_DEP0TSIZ_SUPCNT_SHIFT    29
#define DWCUSB_DEP0TSIZ_PKTCNT          0x00080000      /* Packet Count, Always 1 */
#define DWCUSB_DEP0TSIZ_PKTCNT_MASK	0x00080000
#define DWCUSB_DEP0TSIZ_XFERSIZE_MASK   0x0000007F      /* Transfer Size */
#define DWCUSB_DEP0TSIZ_XFERSIZE_SHIFT  0

/* Device Endpoint-n Transfer Size Register Bit Definitions */

#define DWCUSB_DEPTSIZ_MC_MASK          0x60000000      /* Multi Count */
#define DWCUSB_DEPTSIZ_MC_SHIFT         29
#define DWCUSB_DEPTSIZ_RXDPID_MASK      0x60000000      /* Received Data PID */
#define   DWCUSB_DEPTSIZ_RXDPID_DATA0   0x00000000      /* DATA0 */
#define   DWCUSB_DEPTSIZ_RXDPID_DATA1   0x20000000      /* DATA1 */
#define   DWCUSB_DEPTSIZ_RXDPID_DATA2   0x40000000      /* DATA2 */
#define   DWCUSB_DEPTSIZ_RXDPID_MDATA   0x60000000      /* MDATA */
#define DWCUSB_DEPTSIZ_SUPCNT_MASK      0x60000000      /* SETUP Packet Count */
#define DWCUSB_DEPTSIZ_SUPCNT_SHIFT     29
#define DWCUSB_DEPTSIZ_PKTCNT_MASK      0x1FF80000      /* Packet Count */
#define DWCUSB_DEPTSIZ_PKTCNT_SHIFT     19
#define DWCUSB_DEPTSIZ_XFERSIZE_MASK    0x0007FFFF      /* Transfer Size */
#define DWCUSB_DEPTSIZ_XFERSIZE_SHIFT   0

/* Device  IN Token Sequence Learning Queue Read Register 1 (DTKNQR1) Bits Definitions */
#define DWCUSB_DTKNQR1_WRAP_MASK	0x00000080
#define DWCUSB_DTKNQR1_WRAP_SHIFT	7
#define DWCUSB_DTKNQR1_INTKNWPTR_MASK   0x0000001f



/*******************************************************************************
 S5L8701 USB Power and Clock Gating Registers
 *******************************************************************************/

#define DWCUSB_PCGCR                USBREG(0x00000E00)  /* Power and Clock Gating Control Register */

/* Power and Clock Gating Control Register Bit Definitions */

#define DWCUSB_PCGCR_PHYSUSPEND     0x00000010          /* PHY Suspend */
#define DWCUSB_PCGCR_RSTPDWNMODULE  0x00000008          /* Reset Power-Down Module */
#define DWCUSB_PCGCR_PWRCLMP        0x00000004          /* Power Clamp */
#define DWCUSB_PCGCR_GATEHCLK       0x00000002          /* Gate Hclk */
#define DWCUSB_PCGCR_STOPPCLK       0x00000001          /* Stop Pclk */


/*******************************************************************************
 S5L8701 USB Data FIFO Access Registers
 *******************************************************************************/

#define DWCUSB_FIFO(_n)             USBREG(0x00001000 + (_n) * 0x1000)  /* Data FIFO Access Register */


/*******************************************************************************
 S5L8701 OTG PHY Registers
 *******************************************************************************/

#define DWCUSB_PHYPWR                      USBPHYREG(0x00000000)  /* USB OTG PHY Power Control Register */
#define DWCUSB_PHYCON                      USBPHYREG(0x00000004)  /* USB OTG PHY Control Register */
#define DWCUSB_URSTCON                     USBPHYREG(0x00000008)  /* USB Reset Control Register */
#define DWCUSB_UCONDET                     USBPHYREG(0x00000028)  /* USB Connection Detect Register*/

#define DWCUSB_PHYPWR_ANLGPWRDWN	   0x00000008
#define DWCUSB_PHYPWR_ANLGPWRUP            0x00000000
#define DWCUSB_PHYPWR_OTGPWRDWN            0x00000008
#define DWCUSB_PHYPWR_OTGPWRUP             0x00000000
#define DWCUSB_PHYPWR_REFCLK_CRYSTAL       0x00000002
#define DWCUSB_PHYPWR_REFCLK_OSCI          0x00000000
#define DWCUSB_PHYPWR_FRCSUSPNDDIS         0x00000000
#define DWCUSB_PHYPWR_FRCSUSPNDEN          0x00000001

#define DWCUSB_PHYCON_CLK_12MHZ            0x00000000
#define DWCUSB_PHYCON_CLK_24MHZ            0x00000001
#define DWCUSB_PHYCON_CLK_48MHZ            0x00000002

#define DWCUSB_URSTCON_PHYSWRST            0x00000001

#define DWCUSB_UCONDET_VBUS_LEVEL_MASK     0x00000001          /* USB Connection Detect Mask */
#define DWCUSB_UCONTDET_HOSTDEVICE_MASK    0x00000010
#define		DWCUSB_UCONTDET_HOST	   0x00000000
#define 	DWCUSB_UCONTDET_DEVICE	   0x00000010

#endif // _SYNOPSYS_OTG_REGISTERS_H_
