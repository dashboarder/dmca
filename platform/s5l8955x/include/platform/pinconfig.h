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
#ifndef __PLATFORM_PINCONFIG_H
#define __PLATFORM_PINCONFIG_H

/* Default S5L8955X SoC Pin Configuration */

#define FMI_DRIVE_STR   (DRIVE_X2 | SLOW_SLEW)

static const u_int32_t gpio_default_cfg[GPIO_GROUP_COUNT * GPIOPADPINS] = {

/*
 *      config                                                             idx  pad                        connection
 *      ========                                                           ===  ========                   ================
 */

/* Port  0 */
        CFG_IN,                                                         //   0: GPIO0                   -> Menu Button (DFU)
        CFG_IN,                                                         //   1: GPIO1                   -> Hoid Button (DFU)
        CFG_DISABLED,                                                   //   2: GPIO2                   -> Vol+
        CFG_DISABLED,                                                   //   3: GPIO3                   -> Vol-
        CFG_DISABLED,                                                   //   4: GPIO4                   -> Ringer
        CFG_DISABLED,                                                   //   5: GPIO5                   -> WLAN wake
        CFG_DISABLED,                                                   //   6: GPIO6                   -> WLAN reset_L
        CFG_DISABLED,                                                   //   7: GPIO7                   -> BT wake

/* Port  1 */
        CFG_DISABLED,                                                   //   8: GPIO10                  -> BB reset
        CFG_DISABLED,                                                   //   9: GPIO11                  -> BB reset detect
        CFG_DISABLED,                                                   //  10: GPIO12                  -> Baseband IPC SRDY
        CFG_DISABLED,                                                   //  11: GPIO13                  -> Codec IRQ
        CFG_DISABLED,                                                   //  12: GPIO14                  -> Codec reset
        CFG_DISABLED,                                                   //  13: GPIO15                  -> Diff mic sel_L
        CFG_DISABLED,                                                   //  14: (reserved)
        CFG_DISABLED,                                                   //  15: (reserved)

/* Port  2 */
        CFG_DISABLED,                                                   //  16: (reserved)
        CFG_DISABLED,                                                   //  17: (reserved)
        CFG_DISABLED,                                                   //  18: (reserved)
        CFG_DISABLED,                                                   //  19: (reserved)
        CFG_DISABLED,                                                   //  20: (reserved)
        CFG_DISABLED,                                                   //  21: (reserved)
        CFG_DISABLED,                                                   //  22: (reserved)
        CFG_DISABLED,                                                   //  23: (reserved)

/* Port  3 */
        CFG_DISABLED,                                                   //  24: (reserved)
        CFG_DISABLED,                                                   //  25: (reserved)
        CFG_DISABLED,                                                   //  26: (reserved)
        CFG_DISABLED,                                                   //  27: (reserved)
        CFG_DISABLED,                                                   //  28: (reserved)
        CFG_DISABLED,                                                   //  29: (reserved)
        CFG_DISABLED,                                                   //  30: (reserved)
        CFG_DISABLED,                                                   //  31: (reserved)

/* Port  4 */
        CFG_DISABLED,                                                   //  32: GPIO8                   -> BT reset_L
        CFG_DISABLED,                                                   //  33: GPIO9                   -> RADIO_ON
        CFG_DISABLED,                                                   //  34: EHCI_PORT_PWR0          ->
        CFG_DISABLED,                                                   //  35: EHCI_PORT_PWR1          ->
        CFG_DISABLED,                                                   //  36: EHCI_PORT_PWR2          ->
        CFG_DISABLED,                                                   //  37: EHCI_PORT_PWR3          ->
        CFG_DISABLED,                                                   //  38: GPIO16                  -> Board ID 3
        CFG_DISABLED,                                                   //  39: GPIO17                  -> MICEY_PRESENT

/* Port  5 */
        CFG_DISABLED,                                                   //  40: GPIO18                  -> PMU IRQ_L/Boot Config Bit 0
        CFG_DISABLED,                                                   //  41: GPIO19                  -> PMU Keepact
        CFG_DISABLED,                                                   //  42: UART1_TXD               -> BB_USART0 TXD
        CFG_DISABLED,                                                   //  43: UART1_RXD               -> BB_USART0 RXD
        CFG_DISABLED,                                                   //  44: UART1_RTSN              -> BB_USART0 RTSN
        CFG_DISABLED,                                                   //  45: UART1_CTSN              -> BB_USART0 CTSN
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  46: FMI0_CEN3               -> NAND Ch. 0
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  47: FMI0_CEN2               -> "

/* Port  6 */
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  48: FMI0_CEN1               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  49: FMI0_CEN0               -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  50: FMI0_CLE                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  51: FMI0_ALE                -> "
        CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,                         //  52: FMI0_REN                -> "
        CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,                         //  53: FMI0_WEN                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  54: FMI0_WENN               -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  55: FMI0_IO7                -> "

/* Port  7 */
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  56: FMI0_IO6                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  57: FMI0_IO5                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  58: FMI0_IO4                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  59: FMI0_DQS                -> "
        CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,                         //  60: FMI0_DQSN               -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  61: FMI0_IO3                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  62: FMI0_IO2                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  63: FMI0_IO1                -> "

/* Port  8 */
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  64: FMI0_IO0                -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  65: FMI0_CEN7               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  66: FMI0_CEN6               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  67: FMI0_CEN5               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  68: FMI0_CEN4               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  69: FMI1_CEN3               -> NAND Ch. 1
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  70: FMI1_CEN2               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  71: FMI1_CEN1               -> "

/* Port  9 */
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  72: FMI1_CEN0               -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  73: FMI1_CLE                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  74: FMI1_ALE                -> "
        CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,                         //  75: FMI1_REN                -> "
        CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,                         //  76: FMI1_WEN                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  77: FMI1_WENN               -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  78: FMI1_IO7                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  79: FMI1_IO6                -> "

/* Port 10 */
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  80: FMI1_IO5                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  81: FMI1_IO4                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  82: FMI1_DQS                -> "
        CFG_DISABLED | PULL_UP | FMI_DRIVE_STR,                         //  83: FMI1_DQSN               -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  84: FMI1_IO3                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  85: FMI1_IO2                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  86: FMI1_IO1                -> "
        CFG_DISABLED | PULL_DOWN | FMI_DRIVE_STR,                       //  87: FMI1_IO0                -> "

/* Port 11 */
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  88: FMI1_CEN7               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  89: FMI1_CEN6               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  90: FMI1_CEN5               -> "
        CFG_DISABLED | FMI_DRIVE_STR,                                   //  91: FMI1_CEN4               -> "
        CFG_DISABLED,                                                   //  92: UART2_TXD               -> UMTS debug TXD
        CFG_DISABLED,                                                   //  93: UART2_RXD               -> UMTS debug RXD
        CFG_DISABLED,                                                   //  94: UART2_RTSN              -> UMTS debug RTSN
        CFG_DISABLED,                                                   //  95: UART2_CTSN              -> UMTS debug CTSN

/* Port 12 */
        CFG_DISABLED,                                                   //  96: UART3_TXD               -> Bluetooth TXD
        CFG_DISABLED,                                                   //  97: UART3_RXD               -> Bluetooth RXD
        CFG_DISABLED,                                                   //  98: UART3_RTSN              -> Bluetooth RTSN
        CFG_DISABLED,                                                   //  99: UART3_CTSN              -> Bluetooth CTSN
        CFG_DISABLED,                                                   // 100: UART4_TXD               -> GPS? TXD
        CFG_DISABLED,                                                   // 101: UART4_RXD               -> GPS? RXD
        CFG_DISABLED,                                                   // 102: UART4_RTSN              -> GPS? RTSN
        CFG_DISABLED,                                                   // 103: UART4_CTSN              -> GPS? CTSN

/* Port 13 */
        CFG_DISABLED,                                                   // 104: UART6_TXD               -> Spare
        CFG_DISABLED,                                                   // 105: UART6_RXD               -> Spare
        CFG_DISABLED,                                                   // 106: UART6_RTSN              -> Spare
        CFG_DISABLED,                                                   // 107: UART6_CTSN              -> Spare
        CFG_DISABLED,                                                   // 108: SPI1_SCLK               -> Touch Panel (Grape) CLK
        CFG_DISABLED,                                                   // 109: SPI1_MOSI               -> Touch Panel (Grape) MOSI
        CFG_DISABLED,                                                   // 110: SPI1_MISO               -> Touch Panel (Grape) MISO
        CFG_DISABLED,                                                   // 111: SPI1_SSIN               -> Touch Panel (Grape) SSIN

/* Port 14 */
        CFG_DISABLED,                                                   // 112: SPI0_SCLK               -> Serial ROM CLK/Board ID Bit 0
        CFG_DISABLED,                                                   // 113: SPI0_MOSI               -> Serial ROM MOSI/Board ID Bit 1
        CFG_DISABLED,                                                   // 114: SPI0_MISO               -> Serial ROM MISO/Board ID Bit 2
        CFG_DISABLED | PULL_UP,                                         // 115: SPI0_SSIN               -> Serial ROM SSIN
        CFG_DISABLED,                                                   // 116: SPI2_SCLK               -> Baseband IPC CLK
        CFG_DISABLED,                                                   // 117: SPI2_MOSI               -> Baseband IPC MOSI
        CFG_DISABLED,                                                   // 118: SPI2_MISO               -> Baseband IPC MISO
        CFG_DISABLED,                                                   // 119: SPI2_SSIN               -> Baseband IPC SSIN

/* Port 15 */
        CFG_DISABLED,                                                   // 120: I2C0_SDA                -> PMU, Codec, etc    SDA
        CFG_DISABLED,                                                   // 121: I2C0_SCL                -> PMU, Codec, etc   SCL
        CFG_DISABLED,                                                   // 122: I2C1_SDA                -> ALS, etc    SDA
        CFG_DISABLED,                                                   // 123: I2C1_SCL                -> ALS, etc   SCL
        CFG_DISABLED,                                                   // 124: ISP0_SDA                ->
        CFG_DISABLED,                                                   // 125: ISP0_SCL                ->
        CFG_DISABLED,                                                   // 126: ISP1_SDA                ->
        CFG_DISABLED,                                                   // 127: ISP1_SCL                ->

/* Port 16 */
        CFG_DISABLED,                                                   // 128: MIPI_VSYNC              ->
        CFG_DISABLED,                                                   // 129: TMR32_PWM0              -> Highland park 6MHz clock
        CFG_DISABLED,                                                   // 130: TMR32_PWM1              -> Vibrator
        CFG_DISABLED,                                                   // 131: TMR32_PWM2              ->
        CFG_DISABLED,                                                   // 132: SWI_DATA                -> Backlight control
        CFG_DISABLED,                                                   // 133: DWI_DI                  -> Backlight control
        CFG_DISABLED,                                                   // 134: DWI_DO                  -> Backlight control
        CFG_DISABLED,                                                   // 135: DWI_CLK                 -> Backlight control

/* Port 17 */
        CFG_DISABLED,                                                   // 136: SENSOR0_RST             -> Sensor reset
        CFG_DISABLED,                                                   // 137: SENSOR0_CLK             -> Sensor clk
        CFG_DISABLED,                                                   // 138: SENSOR1_RST             -> Sensor reset
        CFG_DISABLED,                                                   // 139: SENSOR1_CLK             -> Sensor clk
        CFG_DISABLED,                                                   // 140: (reserved)
        CFG_DISABLED,                                                   // 141: (reserved)
        CFG_DISABLED,                                                   // 142: (reserved)
        CFG_DISABLED,                                                   // 143: (reserved)

/* Port 18 */
        CFG_DISABLED,                                                   // 144: (reserved)
        CFG_DISABLED,                                                   // 145: (reserved)
        CFG_DISABLED,                                                   // 146: (reserved)
        CFG_DISABLED,                                                   // 147: (reserved)
        CFG_DISABLED,                                                   // 148: (reserved)
        CFG_DISABLED,                                                   // 149: (reserved)
        CFG_DISABLED,                                                   // 150: (reserved)
        CFG_DISABLED,                                                   // 151: (reserved)

/* Port 19 */
        CFG_DISABLED,                                                   // 152: (reserved)
        CFG_DISABLED,                                                   // 153: (reserved)
        CFG_DISABLED,                                                   // 154: (reserved)
        CFG_DISABLED,                                                   // 155: (reserved)
        CFG_DISABLED,                                                   // 156: (reserved)
        CFG_DISABLED,                                                   // 157: (reserved)
        CFG_DISABLED,                                                   // 158: (reserved)
        CFG_DISABLED,                                                   // 159: (reserved)

/* Port 20 */
        CFG_DISABLED,                                                   // 160: CPU0_SWITCH             -> Sensor clk (XXX Suspected to be incorrect. Follow up.)
        CFG_DISABLED,                                                   // 161: CPU1_SWITCH             -> Sensor clk
        CFG_DISABLED,                                                   // 162: ISP0_PRE_FLASH          -> "
        CFG_DISABLED,                                                   // 163: ISP0_FLASH              -> "
        CFG_DISABLED,                                                   // 164: ISP1_PRE_FLASH          -> "
        CFG_DISABLED,                                                   // 165: ISP1_FLASH              -> "
        CFG_DISABLED,                                                   // 166: I2S0_MCK                -> Audio Codec
        CFG_DISABLED,                                                   // 167: I2S0_LRCK               -> "

/* Port 21 */
        CFG_DISABLED,                                                   // 168: I2S0_BCLK               -> "
        CFG_DISABLED,                                                   // 169: I2S0_DOUT               -> "
        CFG_DISABLED,                                                   // 170: I2S0_DIN                -> "
        CFG_DISABLED,                                                   // 171: I2S1_MCK                -> Bluetooth
        CFG_DISABLED,                                                   // 172: I2S1_LRCK               -> "
        CFG_DISABLED,                                                   // 173: I2S1_BCLK               -> "
        CFG_DISABLED,                                                   // 174: I2S1_DOUT               -> "
        CFG_DISABLED,                                                   // 175: I2S1_DIN                -> "

/* Port 22 */
        CFG_DISABLED,                                                   // 176: I2S2_MCK                -> BB audio
        CFG_DISABLED,                                                   // 177: I2S2_LRCK               -> "
        CFG_DISABLED,                                                   // 178: I2S2_BCLK               -> "
        CFG_DISABLED,                                                   // 179: I2S2_DOUT               -> "
        CFG_DISABLED,                                                   // 180: I2S2_DIN                -> "
        CFG_DISABLED,                                                   // 181: I2S3_MCK                ->
        CFG_DISABLED,                                                   // 182: I2S3_LRCK               ->
        CFG_DISABLED,                                                   // 183: I2S3_BCLK               ->

/* Port 23 */
        CFG_DISABLED,                                                   // 184: I2S3_DOUT               ->
        CFG_DISABLED,                                                   // 185: I2S3_DIN                ->
        CFG_DISABLED,                                                   // 186: I2S4_MCK                -> BB audio
        CFG_DISABLED,                                                   // 187: I2S4_LRCK               -> "
        CFG_DISABLED,                                                   // 188: I2S4_BCLK               -> "
        CFG_DISABLED,                                                   // 189: I2S4_DOUT               -> "
        CFG_DISABLED,                                                   // 190: I2S4_DIN                -> "
        CFG_DISABLED,                                                   // 191: SPDIF                   -> HDMI Transmitter

/* Port 24 */
        CFG_DISABLED,                                                   // 192: GPIO20                  -> Grape reset_L
        CFG_DISABLED,                                                   // 193: GPIO21                  -> Grape IRQ_L
        CFG_DISABLED,                                                   // 194: GPIO22                  -> LCD reset_L
        CFG_DISABLED,                                                   // 195: GPIO23                  -> LCD checksum
        CFG_DISABLED,                                                   // 196: GPIO24                  -> Compass reset_L
        CFG_DISABLED,                                                   // 197: GPIO25                  -> Compass IRQ_L/Boot Config Bit 1
        CFG_IN | PULL_DOWN,                                             // 198: GPIO26                  -> DFU force
        CFG_DISABLED | PULL_DOWN,                                       // 199: GPIO27                  -> DFU status

/* Port 25 */
        CFG_DISABLED,                                                   // 200: GPIO28                  -> Accelerometer IRQ_L/Boot Config Bit 2
        CFG_DISABLED,                                                   // 201: GPIO29                  -> ALS IRQ_L/Boot Config Bit 3
        CFG_DISABLED,                                                   // 202: GPIO30                  -> GPS reset_ap_L
        CFG_DISABLED,                                                   // 203: GPIO31                  -> GPS standby_ap_L
        CFG_DISABLED,                                                   // 204: GPIO32                  -> GPS_intr_L
        CFG_DISABLED,                                                   // 205: GPIO33                  -> GSM_txburst_int
        CFG_DISABLED,                                                   // 206: GPIO34                  -> FM mux select
        CFG_DISABLED,                                                   // 207: GPIO35                  -> Board reliability test NE

/* Port 26 */
        CFG_DISABLED,                                                   // 208: GPIO36                  -> Board reliability test SE
        CFG_DISABLED,                                                   // 209: GPIO37                  -> Board reliability test SW
        CFG_DISABLED,                                                   // 210: GPIO38                  -> Board reliability test NW
        CFG_DISABLED,                                                   // 211: GPIO39                  -> 
        CFG_DISABLED,                                                   // 212: (reserved)
        CFG_DISABLED,                                                   // 213: (reserved)
        CFG_DISABLED,                                                   // 214: (reserved)
        CFG_DISABLED,                                                   // 215: (reserved)

/* Port 27 */
        CFG_DISABLED,                                                   // 216: (reserved)
        CFG_DISABLED,                                                   // 217: (reserved)
        CFG_DISABLED,                                                   // 218: (reserved)
        CFG_DISABLED,                                                   // 219: (reserved)
        CFG_DISABLED,                                                   // 220: (reserved)
        CFG_DISABLED,                                                   // 221: (reserved)
        CFG_DISABLED,                                                   // 222: (reserved)
        CFG_DISABLED,                                                   // 223: (reserved)

/* Port 28 */
        CFG_DISABLED,                                                   // 224: SPI3_MOSI               -> MOSI
        CFG_DISABLED,                                                   // 225: SPI3_MISO               -> MISO
        CFG_DISABLED,                                                   // 226: SPI3_SCLK               -> CLK
        CFG_DISABLED,                                                   // 227: SPI3_SSIN               -> SSIN
        CFG_DISABLED,                                                   // 228: I2C2_SDA                -> ALS/prox
        CFG_DISABLED,                                                   // 229: I2C2_SCL                -> "
        CFG_DISABLED,                                                   // 230: GPIO_3V0                -> Video amp enable
        CFG_DISABLED,                                                   // 231: GPIO_3V1                -> SEL_SECURE_BOOT

/* Port 29 */
        CFG_DISABLED,                                                   // 232: DP_HPD                  -> Hot Plug Detect
        CFG_DISABLED,                                                   // 233: EDP_HPD                 -> Hot Plug Detect
        CFG_DISABLED,                                                   // 234: UART0_TXD               -> Dock, UART TXD
        CFG_DISABLED,                                                   // 235: UART0_RXD               -> Dock, UART RXD
        CFG_DISABLED,                                                   // 236: UART5_RXD               -> Battery gas gauge RXD
        CFG_DISABLED,                                                   // 237: UART5_TXD               -> Battery gas gauge TXD
        CFG_DISABLED,                                                   // 238: TST_CLKOUT              ->
        CFG_DISABLED,                                                   // 239: TST_STPCLK              ->

/* Port 30 */
        CFG_DISABLED,                                                   // 240: WDOG                    -> 
        CFG_DISABLED,                                                   // 241: (reserved)
        CFG_DISABLED,                                                   // 242: (reserved)
        CFG_DISABLED,                                                   // 243: (reserved)
        CFG_DISABLED,                                                   // 244: (reserved)
        CFG_DISABLED,                                                   // 245: (reserved)
        CFG_DISABLED,                                                   // 246: (reserved)
        CFG_DISABLED,                                                   // 247: (reserved)
};

#endif /* ! __PLATFORM_PINCONFIG_H */
