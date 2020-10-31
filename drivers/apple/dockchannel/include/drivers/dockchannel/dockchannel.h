/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DOCKCHANNEL_H
#define __DOCKCHANNEL_H

int32_t dockchannel_uart_init();
int32_t dockchannel_uart_putc(char c);
int32_t dockchannel_uart_getc(bool wait);

void	dockchannel_enable_clock_gating(uint32_t num);
void	dockchannel_enable_top_clock_gating();
void	dockchannel_access_enable(uint32_t enable_flags);

#endif /* __DOCKCHANNEL_H */
