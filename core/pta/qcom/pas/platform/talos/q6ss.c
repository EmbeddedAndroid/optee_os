// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <kernel/delay.h>
#include <trace.h>
#include <util.h>

#include "q6ss.h"

#define QDSP6SS_CORE_CBCR	0x20
#define QDSP6SS_XO_CBCR		0x38
#define QDSP6SS_SLEEP_CBCR	0x3c
#define QDSP6SS_BOOT_CORE_START	0x400
#define QDSP6SS_BOOT_CMD	0x404
#define QDSP6SS_BOOT_STATUS	0x408

#define QDSP6SS_CBCR_CLKEN	BIT(0)

#define BOOT_FSM_TIMEOUT_US	1000000

TEE_Result q6ss_boot(vaddr_t q6ss)
{
	uint64_t timeout = 0;

	io_setbits32(q6ss + QDSP6SS_XO_CBCR, QDSP6SS_CBCR_CLKEN);
	io_setbits32(q6ss + QDSP6SS_SLEEP_CBCR, QDSP6SS_CBCR_CLKEN);
	io_setbits32(q6ss + QDSP6SS_CORE_CBCR, QDSP6SS_CBCR_CLKEN);
	dsb();

	io_setbits32(q6ss + QDSP6SS_BOOT_CORE_START, BIT(0));
	udelay(5);
	io_write32(q6ss + QDSP6SS_BOOT_CMD, BIT(0));

	timeout = timeout_init_us(BOOT_FSM_TIMEOUT_US);
	while (!(io_read32(q6ss + QDSP6SS_BOOT_STATUS) & BIT(0))) {
		if (timeout_elapsed(timeout)) {
			EMSG("QDSP6 boot FSM timed out");
			return TEE_ERROR_TIMEOUT;
		}
		udelay(5);
	}

	return TEE_SUCCESS;
}
