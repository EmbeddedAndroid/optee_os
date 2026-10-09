/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _Q6SS_H_
#define _Q6SS_H_

#include <types_ext.h>

#include "pas_data.h"

/*
 * Run the QDSP6SS out-of-reset sequence: turn on the XO, sleep and core
 * clocks, release the core and start the boot FSM. The boot address must
 * already be programmed; @q6ss is the QDSP6SS block.
 */
TEE_Result q6ss_boot(vaddr_t q6ss);

#endif /* _Q6SS_H_ */
