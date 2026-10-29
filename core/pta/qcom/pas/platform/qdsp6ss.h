/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef QDSP6SS_H
#define QDSP6SS_H

#include <tee_api_types.h>
#include <types_ext.h>

/* QDSP6SS boot registers, relative to the QDSP6SS block of a subsystem */
#define QDSP6SS_RST_EVB			0x010
#define QDSP6SS_BOOT_CORE_START		0x400
#define QDSP6SS_BOOT_CMD		0x404
#define QDSP6SS_BOOT_STATUS		0x408

/* Release the QDSP6 core from its stop-core state */
void qdsp6ss_core_start(vaddr_t qdsp6ss);

/*
 * qdsp6ss_boot_fsm() : run the QDSP6SS boot FSM and wait for it to finish.
 *
 * The entry address must already be programmed. The core may be released
 * before or after this call, as the subsystem requires.
 *
 * @qdsp6ss:    VA of the QDSP6SS block.
 * @timeout_us: Time allowed for BOOT_STATUS to report completion.
 * Returns TEE_ERROR_TIMEOUT if the FSM does not finish in time.
 */
TEE_Result qdsp6ss_boot_fsm(vaddr_t qdsp6ss, uint32_t timeout_us);

#endif /* QDSP6SS_H */
