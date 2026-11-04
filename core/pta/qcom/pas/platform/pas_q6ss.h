/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PAS_Q6SS_H
#define PAS_Q6SS_H

#include <stdbool.h>
#include <tee_api_types.h>
#include <types_ext.h>

/*
 * Run the boot FSM of the QDSP6SS block mapped at @q6ss, releasing the core
 * first if @start_core. The boot address and the QDSP6SS clocks must be set
 * up. On a timeout the core may be running: the caller resets the
 * subsystem.
 */
TEE_Result pas_q6ss_boot(vaddr_t q6ss, bool start_core);

#endif /* PAS_Q6SS_H */
