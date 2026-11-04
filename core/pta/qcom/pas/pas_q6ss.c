// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <kernel/delay.h>
#include <trace.h>
#include <util.h>

#include "pas_q6ss.h"

/* Relative to the QDSP6SS block */
#define QDSP6SS_BOOT_CORE_START		0x400
#define QDSP6SS_BOOT_CMD		0x404
#define QDSP6SS_BOOT_STATUS		0x408

#define BOOT_FSM_TIMEOUT_US		1000000

TEE_Result pas_q6ss_boot(vaddr_t q6ss, bool start_core)
{
	uint32_t val = 0;

	if (start_core)
		io_setbits32(q6ss + QDSP6SS_BOOT_CORE_START, BIT(0));
	io_write32(q6ss + QDSP6SS_BOOT_CMD, BIT(0));

	if (IO_READ32_POLL_TIMEOUT(q6ss + QDSP6SS_BOOT_STATUS, val,
				   val & BIT(0), 5, BOOT_FSM_TIMEOUT_US)) {
		EMSG("QDSP6 boot FSM timed out");
		return TEE_ERROR_TIMEOUT;
	}

	return TEE_SUCCESS;
}
