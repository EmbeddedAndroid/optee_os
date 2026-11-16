/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef SECA_TARGET_CONFIG_H
#define SECA_TARGET_CONFIG_H

/* Two of the four Nord NSPs and two of the three HPASS DSPs remain. */
#define CDSP_COUNT			2
#define HPASS_Q6_COUNT			2

/* The iris video codec has one of the two Nord vcodec cores. */
#define IRIS_VCODEC_COUNT		1

#define DTB_CHIP_FAMILY_ID		0x00af02e6

/* SecaAU keeps the Nord address map for everything OP-TEE uses. */
#include "../nord/target_config.h"

/*
 * The GIC keeps all 18 Nord redistributor frames; the probe walks past
 * the frames of the 12 CPUs up to GICR_TYPER.Last, so map them all.
 */
#define GICR_COUNT			18

#include "cdsp_mem_res.h"
#include "hpass_mem_res.h"

#endif /* SECA_TARGET_CONFIG_H */
