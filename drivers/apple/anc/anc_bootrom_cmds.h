/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _ANC_CMDS_H_
#define _ANC_CMDS_H_

#define LINK_CMDQ_SIZE          128
#define DMA_CMDQ_SIZE           64 // 64b words, not 32b
#define LINK_PIO_READ_FIFO_SIZE 64

////////////////////////////////////////////////////////////////////////////////
// DMA Command Definitions
#define DMA_COMMAND__OPCODE(n)               (((n) & 0x7FULL) << 60)
#define DMA_COMMAND__OPCODE__CONFIG          (0)
#define DMA_COMMAND__OPCODE__BUFDESC         (1)
#define DMA_COMMAND__OPCODE__AES_KEY_IV      (2)
#define DMA_COMMAND__OPCODE__AES_UID1_IV     (3)
#define DMA_COMMAND__OPCODE__AES_IV          (4)
#define DMA_COMMAND__OPCODE__FLAG            (5)

#define DMA_COMMAND_CONFIG__DMA_DIRECTION(n)           (((n) & 0x1ULL) << 53)
#define     DMA_DIRECTION_M2N    (0)
#define     DMA_DIRECTION_N2M    (1)
#define DMA_COMMAND_CONFIG__PROC_ENABLE_AES(n)         (((n) & 0x1ULL) << 54)

#define DMA_COMMAND_BUFDESC__LENGTH(n)                 (((n) & 0x1FFFULL) << 40)
#define DMA_COMMAND_BUFDESC__ADDRESS(n)                (((n) & 0xFFFFFFFFFFULL) << 0)

#define DMA_COMMAND_AESKEY__UNWRAP(n)                  (((n) & 0x1ULL) << 2)
#define DMA_COMMAND_AESKEY__KEY_LEN(n)                 (((n) & 0x3ULL) << 0)
#define     DMA_COMMAND_AESKEY_128_BITS   (0)
#define     DMA_COMMAND_AESKEY_192_BITS   (1)
#define     DMA_COMMAND_AESKEY_256_BITS   (2)

#define DMA_COMMAND_FLAG__PAUSE(n)                     (((n) & 0x1ULL) << 16)
#define DMA_COMMAND_FLAG__CODE(n)                      (((n) & 0xFFFFULL) << 0)

#define DMA_COMMAND_CONFIG(dir,aes)                          \
    (DMA_COMMAND__OPCODE(DMA_COMMAND__OPCODE__CONFIG) |                 \
     DMA_COMMAND_CONFIG__DMA_DIRECTION(dir) |                           \
     DMA_COMMAND_CONFIG__PROC_ENABLE_AES(aes))
#define DMA_COMMAND_BUFDESC(data_bytes,address)          \
    (DMA_COMMAND__OPCODE(DMA_COMMAND__OPCODE__BUFDESC) |            \
     DMA_COMMAND_BUFDESC__LENGTH(data_bytes) |              \
     DMA_COMMAND_BUFDESC__ADDRESS(address))
#define DMA_COMMAND_AES_KEY_IV(key_len, unwrap_en)      \
    (DMA_COMMAND__OPCODE(DMA_COMMAND__OPCODE__AES_KEY_IV) |          \
     DMA_COMMAND_AESKEY__KEY_LEN(key_len) | \
     DMA_COMMAND_AESKEY__UNWRAP(unwrap_en))
#define DMA_COMMAND_FLAG(code,pause)                    \
    (DMA_COMMAND__OPCODE(DMA_COMMAND__OPCODE__FLAG) |   \
     DMA_COMMAND_FLAG__PAUSE(pause) |                   \
     DMA_COMMAND_FLAG__CODE(code))

////////////////////////////////////////////////////////////////////////////////
// Link Command Definitions
#define LINK_COMMAND__YIELD                    (1UL << 31)
#define LINK_COMMAND__OPCODE(n)                (((n) & 0x3FUL) << 24)

#define LINK_CMD_OP_DEBUG_DLL                             0x00
#define LINK_CMD_OP_CMD0                                  0x01
#define LINK_CMD_OP_CMD1                                  0x02
#define LINK_CMD_OP_CMD1_OP                               0x03
#define LINK_CMD_OP_CMD2                                  0x04
#define LINK_CMD_OP_CMD2_OP                               0x05
#define LINK_CMD_OP_CMD3                                  0x06
#define LINK_CMD_OP_CMD3_OP                               0x07
#define LINK_CMD_OP_CMD4                                  0x08
#define LINK_CMD_OP_CMD4_OP                               0x09
#define LINK_CMD_OP_CMD5                                  0x0A
#define LINK_CMD_OP_CMD5_OP                               0x0B
#define LINK_CMD_OP_CMD6                                  0x0C
#define LINK_CMD_OP_CMD6_OP                               0x0D
#define LINK_CMD_OP_CMD7                                  0x0E
#define LINK_CMD_OP_CMD7_OP                               0x0F
#define LINK_CMD_OP_CMD8_OP                               0x10
#define LINK_CMD_OP_ADDR1                                 0x12
#define LINK_CMD_OP_ADDR1_OP                              0x13
#define LINK_CMD_OP_ADDR2                                 0x14
#define LINK_CMD_OP_ADDR2_OP                              0x15
#define LINK_CMD_OP_ADDR3                                 0x16
#define LINK_CMD_OP_ADDR3_OP                              0x17
#define LINK_CMD_OP_ADDR4                                 0x18
#define LINK_CMD_OP_ADDR4_OP                              0x19
#define LINK_CMD_OP_ADDR5                                 0x1A
#define LINK_CMD_OP_ADDR5_OP                              0x1B
#define LINK_CMD_OP_ADDR6                                 0x1C
#define LINK_CMD_OP_ADDR6_OP                              0x1D
#define LINK_CMD_OP_ADDR7                                 0x1E
#define LINK_CMD_OP_ADDR7_OP                              0x1F
#define LINK_CMD_OP_ADDR8_OP                              0x20
#define LINK_CMD_OP_READ_RELEASE                          0x21
#define LINK_CMD_OP_CE                                    0x22
#define LINK_CMD_OP_CE_OP                                 0x23
#define LINK_CMD_OP_READ_DMA                              0x24
#define LINK_CMD_OP_READ_DMA_OP                           0x25
#define LINK_CMD_OP_READ_PIO                              0x26
#define LINK_CMD_OP_READ_PIO_OP                           0x27
#define LINK_CMD_OP_READ_NUL                              0x28
#define LINK_CMD_OP_READ_NUL_OP                           0x29
#define LINK_CMD_OP_READ_STATUS                           0x2A
#define LINK_CMD_OP_READ_STATUS_ABORT_ENABLED             0x2B
#define LINK_CMD_OP_WRITE_DMA                             0x2C
#define LINK_CMD_OP_WRITE_DMA_OP                          0x2D
#define LINK_CMD_OP_WRITE_PIO                             0x2E
#define LINK_CMD_OP_WRITE_PIO_OP                          0x2F
#define LINK_CMD_OP_WRITE_PIO_IMM                         0x30
#define LINK_CMD_OP_WRITE_PAD                             0x32
#define LINK_CMD_OP_WRITE_PAD_OP                          0x33
#define LINK_CMD_OP_READ_REGISTER                         0x34
#define LINK_CMD_OP_WRITE_REGISTER                        0x35
#define LINK_CMD_OP_POLL_REGISTER                         0x36
#define LINK_CMD_OP_POLL_REGISTER_ABORT_ENABLED           0x37
#define LINK_CMD_OP_WAIT_TIME                             0x38
#define LINK_CMD_OP_WAIT_TIME_ABORT_ENABLED               0x39
#define LINK_CMD_OP_WAIT_FOR_INTERRUPT                    0x3A
#define LINK_CMD_OP_WAIT_FOR_INTERRUPT_ABORT_ENABLED      0x3B
#define LINK_CMD_OP_SEND_INTERRUPT                        0x3C
#define LINK_CMD_OP_MACRO                                 0x3D
#define LINK_CMD_OP_DPI_OUT                               0x3E
#define LINK_CMD_OP_DPI_IN                                0x3F

#define LINK_COMMAND__CE__CE(ce)     (((ce) & 0xFFUL) << 0)
#define LINK_COMMAND__CE(ce)  (LINK_COMMAND__OPCODE(LINK_CMD_OP_CE) | LINK_COMMAND__CE__CE(ce))

#define LINK_COMMAND__CMD__CMD2(n)          (((n) & 0xFFUL) << 16)
#define LINK_COMMAND__CMD__CMD1(n)          (((n) & 0xFFUL) << 8)
#define LINK_COMMAND__CMD__CMD0(n)          (((n) & 0xFFUL) << 0)

#define LINK_COMMAND__CMD1(cmd1)      (LINK_COMMAND__OPCODE(LINK_CMD_OP_CMD1) | LINK_COMMAND__CMD__CMD0(cmd1))
#define LINK_COMMAND__CMD2(cmd1,cmd2) (LINK_COMMAND__OPCODE(LINK_CMD_OP_CMD2) | \
                                       LINK_COMMAND__CMD__CMD0(cmd1) |  \
                                       LINK_COMMAND__CMD__CMD1(cmd2))
#define LINK_COMMAND__CMD3(cmd1,cmd2,cmd3) (LINK_COMMAND__OPCODE(LINK_CMD_OP_CMD3) | \
                                            LINK_COMMAND__CMD__CMD0(cmd1) | \
                                            LINK_COMMAND__CMD__CMD1(cmd2) | \
                                            LINK_COMMAND__CMD__CMD2(cmd3))


#define LINK_COMMAND__ADDR__ADDR2(n)        (((n) & 0xFFUL) << 16)
#define LINK_COMMAND__ADDR__ADDR1(n)        (((n) & 0xFFUL) << 8)
#define LINK_COMMAND__ADDR__ADDR0(n)        (((n) & 0xFFUL) << 0)
#define LINK_COMMAND__ADDR1(addr0)       (LINK_COMMAND__OPCODE(LINK_CMD_OP_ADDR1) | \
                                          LINK_COMMAND__ADDR__ADDR0(addr0))
#define LINK_COMMAND__ADDR2(addr0,addr1) (LINK_COMMAND__OPCODE(LINK_CMD_OP_ADDR2) | \
                                          LINK_COMMAND__ADDR__ADDR0(addr0) | \
                                          LINK_COMMAND__ADDR__ADDR1(addr1))
#define LINK_COMMAND__ADDR3(addr0,addr1,addr2) (LINK_COMMAND__OPCODE(LINK_CMD_OP_ADDR3) | \
                                                LINK_COMMAND__ADDR__ADDR0(addr0) | \
                                                LINK_COMMAND__ADDR__ADDR1(addr1) | \
                                                LINK_COMMAND__ADDR__ADDR2(addr2))

#define LINK_COMMAND__WRITE_DMA__LEN(n)       (((n) & 0x7FFFUL) << 0)
#define LINK_COMMAND__WRITE_DMA__CRC_START(n) (((n) & 0x1UL) << 22)
#define LINK_COMMAND__WRITE_DMA__CRC_STOP(n)  (((n) & 0x1UL) << 23)
#define LINK_COMMAND__WRITE_DMA(len, crc_start, crc_stop) (LINK_COMMAND__OPCODE(LINK_CMD_OP_WRITE_DMA) | \
                                                           LINK_COMMAND__WRITE_DMA__LEN(len) | \
                                                           LINK_COMMAND__WRITE_DMA__CRC_START(crc_start) | \
                                                           LINK_COMMAND__WRITE_DMA__CRC_STOP(crc_stop))

#define LINK_COMMAND__WRITE_PIO__LEN(n)       (((n) & 0x7FFFUL) << 0)
#define LINK_COMMAND__WRITE_PIO__CRC_START(n) (((n) & 0x1UL) << 22)
#define LINK_COMMAND__WRITE_PIO__CRC_STOP(n)  (((n) & 0x1UL) << 23)
#define LINK_COMMAND__WRITE_PIO(len, crc_start, crc_stop) (LINK_COMMAND__OPCODE(LINK_CMD_OP_WRITE_PIO) | \
                                                           LINK_COMMAND__WRITE_PIO__LEN(len) | \
                                                           LINK_COMMAND__WRITE_PIO__CRC_START(crc_start) | \
                                                           LINK_COMMAND__WRITE_PIO__CRC_STOP(crc_stop))

#define LINK_COMMAND__READ_DMA__LEN(n)       (((n) & 0x7FFFUL) << 0)
#define LINK_COMMAND__READ_DMA__CRC_START(n) (((n) & 0x1UL) << 22)
#define LINK_COMMAND__READ_DMA__CRC_STOP(n)  (((n) & 0x1UL) << 23)
#define LINK_COMMAND__READ_DMA(len, crc_start, crc_stop) (LINK_COMMAND__OPCODE(LINK_CMD_OP_READ_DMA) | \
                                                          LINK_COMMAND__READ_DMA__LEN(len) | \
                                                          LINK_COMMAND__READ_DMA__CRC_START(crc_start) | \
                                                          LINK_COMMAND__READ_DMA__CRC_STOP(crc_stop))

#define LINK_COMMAND__READ_PIO__LEN(n)       (((n) & 0x7FFFUL) << 0)
#define LINK_COMMAND__READ_PIO__CRC_START(n) (((n) & 0x1UL) << 22)
#define LINK_COMMAND__READ_PIO__CRC_STOP(n)  (((n) & 0x1UL) << 23)
#define LINK_COMMAND__READ_PIO(len, crc_start, crc_stop) (LINK_COMMAND__OPCODE(LINK_CMD_OP_READ_PIO) | \
                                                          LINK_COMMAND__READ_PIO__LEN(len) | \
                                                          LINK_COMMAND__READ_PIO__CRC_START(crc_start) | \
                                                          LINK_COMMAND__READ_PIO__CRC_STOP(crc_stop))

#define LINK_COMMAND__READ_STATUS__CODE(n)     (((n) & 0xFFUL) << 16)
#define LINK_COMMAND__READ_STATUS__MASK(n)     (((n) & 0xFFUL) << 8)
#define LINK_COMMAND__READ_STATUS__COND(n)     (((n) & 0xFFUL) << 0)
#define LINK_COMMAND__READ_STATUS(code,mask,cond) (LINK_COMMAND__OPCODE(LINK_CMD_OP_READ_STATUS) | \
                                                   LINK_COMMAND__READ_STATUS__CODE(code) | \
                                                   LINK_COMMAND__READ_STATUS__MASK(mask) | \
                                                   LINK_COMMAND__READ_STATUS__COND(cond))

#define LINK_COMMAND__SEND_INTERRUPT__PAUSE(n) (((n) & 0x1UL) << 23)
#define LINK_COMMAND__SEND_INTERRUPT__CODE(n)  (((n) & 0xFFFFUL) << 0)
#define LINK_COMMAND__SEND_INTERRUPT(pause,code)   (LINK_COMMAND__OPCODE(LINK_CMD_OP_SEND_INTERRUPT) | \
                                                    LINK_COMMAND__SEND_INTERRUPT__PAUSE(pause) | \
                                                    LINK_COMMAND__SEND_INTERRUPT__CODE(code))

#define LINK_COMMAND__READ_REGISTER__ADDR(n)        (((n) & 0xFFFFFFUL) << 0)
#define LINK_COMMAND__READ_REGISTER(addr) (LINK_COMMAND__OPCODE(LINK_CMD_OP_READ_REGISTER) | \
                                           LINK_COMMAND__READ_REGISTER__ADDR(addr))

#define LINK_COMMAND__WRITE_REGISTER__ADDR(n)       (((n) & 0xFFFFFFUL) << 0)
#define LINK_COMMAND__WRITE_REGISTER(addr) (LINK_COMMAND__OPCODE(LINK_CMD_OP_WRITE_REGISTER) | \
                                            LINK_COMMAND__WRITE_REGISTER__ADDR(addr))

#define LINK_COMMAND__WAIT_TIME__CYCLES(n)    (((n) & 0xFFFFUL) << 0)
#define LINK_COMMAND__WAIT_TIME(link_cycles)  (LINK_COMMAND__OPCODE(LINK_CMD_OP_WAIT_TIME) | \
					       LINK_COMMAND__WAIT_TIME__CYCLES((link_cycles)))

#define LINK_COMMAND__MACRO__LENGTH(n)      (((n) & 0xFFUL) << 0)
#define LINK_COMMAND__MACRO__ADDR(n)        (((n) & 0xFFUL) << 8)
#define LINK_COMMAND__MACRO__REPEAT(n)      (((n) & 0xFFUL) << 16)
#define LINK_COMMAND__MACRO(addr,length,repeat)  (LINK_COMMAND__OPCODE(LINK_CMD_OP_MACRO) | \
                                                  LINK_COMMAND__MACRO__LENGTH(length-1) | \
                                                  LINK_COMMAND__MACRO__ADDR(addr) | \
                                                  LINK_COMMAND__MACRO__REPEAT(repeat))

#endif // _ANC_CMDS_H_
