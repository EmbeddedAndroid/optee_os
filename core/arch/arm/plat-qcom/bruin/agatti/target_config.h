/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

/* 2 GiB at 0x40000000, plus 2 GiB at 0xc0000000 on 4 GiB boards. */
#define DRAM0_BASE			UL(0x40000000)
#define DRAM0_SIZE			UL(0x80000000)
#define DRAM1_BASE			UL(0xc0000000)
#define DRAM1_SIZE			UL(0x80000000)

#define GENI_UART_REG_BASE		UL(0x04a90000)

#define GCC_BASE			UL(0x01400000)
#define GCC_SIZE			UL(0x001f0000)

#define TCSR_BASE			UL(0x00300000)
#define TCSR_SIZE			UL(0x00100000)

#define LPASS_BASE			UL(0x0a000000)
#define LPASS_SIZE			UL(0x01000000)

#define GPU_BASE			UL(0x05900000)
#define GPU_SIZE			UL(0x00040000)

#define GICR_BASE			UL(0x0f300000)

#endif /* TARGET_CONFIG_H */
