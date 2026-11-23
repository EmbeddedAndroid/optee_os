/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PAS_CAMERA_H
#define PAS_CAMERA_H

#include <stdint.h>

#include "pas_subsys.h"

/*
 * struct pas_camera_data : subsystem_data of a camera ICP subsystem.
 *
 * @qos: ICP_SYS_ACCESS value, the qos-val of the ICP node in the camera DT
 */
struct pas_camera_data {
	uint32_t qos;
};

extern const struct qcom_pas_ops pas_camera_icp_ops;

#endif /* PAS_CAMERA_H */
