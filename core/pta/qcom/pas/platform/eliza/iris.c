// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <util.h>

#include "iris.h"

static const struct pas_iris_write after_thresholds[] = {
	PAS_IRIS_WRITE(SS_TZ_SEC_THRESHOLD_AV1, 1280),
};

static const struct pas_iris_write non_vcl[] = {
	PAS_IRIS_WRITE(SS_TZ_SEC_THRESHOLD_NON_VCL_HEVC, 335872),
	PAS_IRIS_WRITE(SS_TZ_SEC_THRESHOLD_NON_VCL_H264, 335872),
	PAS_IRIS_WRITE(SS_TZ_SEC_THRESHOLD_NON_VCL_MP2, 1600),
};

static const struct pas_iris_write before_checks[] = {
	PAS_IRIS_WRITE(SS_TZ_CP_VP8_SECURE_OVERRIDE_DEC, 0),
	PAS_IRIS_WRITE(SS_TZ_CP_VP9_SECURE_OVERRIDE_DEC, 0),
	PAS_IRIS_WRITE(SS_TZ_SEC_SID_F_SECURE_OVERRIDE, 1),
};

static const struct pas_iris_write after_checks[] = {
	PAS_IRIS_WRITE(SS_TZ_SEC_SID_C_SECURE_OVERRIDE, 1),
	PAS_IRIS_WRITE(SS_TZ_SEC_SID_D_SECURE_OVERRIDE, 1),
	PAS_IRIS_WRITE(SS_TZ_SEC_SID_E_SECURE_OVERRIDE, 1),
};

const struct pas_iris_cfg iris_cfg = {
	.after_thresholds = { after_thresholds, ARRAY_SIZE(after_thresholds) },
	.non_vcl = { non_vcl, ARRAY_SIZE(non_vcl) },
	.before_checks = { before_checks, ARRAY_SIZE(before_checks) },
	.after_checks = { after_checks, ARRAY_SIZE(after_checks) },
	.every_release = true,
};
