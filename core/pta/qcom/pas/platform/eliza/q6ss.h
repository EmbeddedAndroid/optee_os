/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _Q6SS_H_
#define _Q6SS_H_

#include <types_ext.h>

#include "pas_data.h"

/* Program the boot address of the QDSP6SS block @q6ss */
void q6ss_set_boot_addr(vaddr_t q6ss, paddr_t addr);

/*
 * Hand the DSP its device tree image (@dtb, @dtb_size) and the chip and
 * platform it runs on through the QDSP6SS boot parameters.
 */
TEE_Result q6ss_set_dtb_params(vaddr_t q6ss, paddr_t dtb, size_t dtb_size);

/* Run the boot FSM; @start_core also releases the core */
TEE_Result q6ss_boot(vaddr_t q6ss, bool start_core);

#endif /* _Q6SS_H_ */
