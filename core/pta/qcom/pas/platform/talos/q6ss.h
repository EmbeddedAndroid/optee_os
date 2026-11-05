/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _Q6SS_H_
#define _Q6SS_H_

#include <types_ext.h>

#include "pas_data.h"

/*
 * Release the QDSP6 core and run the QDSP6SS boot FSM. The clock driver
 * has already turned on the QDSP6SS clocks and the boot address must be
 * programmed; @q6ss is the QDSP6SS block. On failure the core may be left
 * running: the caller resets the subsystem.
 */
TEE_Result q6ss_boot(vaddr_t q6ss);

#endif /* _Q6SS_H_ */
