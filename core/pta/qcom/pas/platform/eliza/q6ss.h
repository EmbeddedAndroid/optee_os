/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _Q6SS_H_
#define _Q6SS_H_

#include <stdbool.h>
#include <tee_api_types.h>
#include <types_ext.h>

#include "pas_subsys.h"

/*
 * The QDSP6 subsystems of Eliza boot in two PAS stages: their device tree
 * image (DTB ID), then the DSP image, which depends on it.
 *
 * q6ss_dtb_ops: the DTB stage, for every DSP. Start hands the DTB, the
 * chip and the platform to the QDSP6SS boot parameters; shutdown resets
 * the subsystem unless the image shutdown or a failed start already did.
 */
extern const struct qcom_pas_ops q6ss_dtb_ops;

/* VA of the QDSP6SS block of the subsystem, 0 if unmapped */
vaddr_t q6ss_base(struct qcom_pas_data *data);

/* Image stage shutdown: reset the subsystem through its clock group */
TEE_Result q6ss_shutdown(struct qcom_pas_data *data);

/*
 * Run the boot FSM from the programmed EVB with the retention resets
 * disarmed; @start_core releases the core before the FSM runs.
 */
TEE_Result q6ss_boot(vaddr_t q6ss, bool start_core);

#endif /* _Q6SS_H_ */
