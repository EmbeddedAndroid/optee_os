/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef SECA_TARGET_CONFIG_H
#define SECA_TARGET_CONFIG_H

/* SecaAU keeps the Nord address map for everything OP-TEE uses. */
#include "../nord/target_config.h"

/*
 * The GIC keeps all 18 Nord redistributor frames; the probe walks past
 * the frames of the 12 CPUs up to GICR_TYPER.Last, so map them all.
 */
#define GICR_COUNT			18

#endif /* SECA_TARGET_CONFIG_H */
