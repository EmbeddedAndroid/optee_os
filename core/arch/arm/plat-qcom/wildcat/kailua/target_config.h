/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

/* SM8550 (Kailua): debug UART uart7 (QUPV3_1 SE7), GIC-700 */
#define GENI_UART_REG_BASE		UL(0xa9c000)

#define GICD_BASE			UL(0x17100000)
#define GICR_BASE			UL(0x17180000)

#define QCOM_RNG_REG_BASE		UL(0x010c1000)

/* 12 GB on the HDK */
#define DRAM0_BASE			UL(0x80000000)
#define DRAM0_SIZE			UL(0x80000000)
#define DRAM1_BASE			ULL(0x880000000)
#define DRAM1_SIZE			ULL(0x280000000)

#endif /* TARGET_CONFIG_H */
