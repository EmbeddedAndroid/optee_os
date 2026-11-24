// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <kernel/delay.h>
#include <platform_config.h>
#include <stdint.h>
#include <trace.h>
#include <util.h>

#include "pas_camera.h"

#define ICP_SYS_RESET			0x0000
#define ICP_SYS_RESET_FUNC_RESET	BIT(4)

#define ICP_SYS_CONTROL			0x0004
#define ICP_SYS_CONTROL_CPU_EN		BIT(9)

#define ICP_SYS_STATUS			0x000c
#define ICP_SYS_STATUS_LX7_STANDBYWFI	BIT(7)

#define ICP_SYS_ACCESS			0x0010

#define ICP_SYS_SID1_START_ADDR		0x0048
#define ICP_SYS_SID1_START_ADDR_EN	BIT(0)
#define ICP_SYS_SID_REGION_MASK		0xFFFFFFF0U

#define ICP_SYS_SID1_END_ADDR		0x004c

/* Domain ID masks the ICP firmware checks each frame request against */
#define ICP_DOM_0_CFG			0x0000
#define ICP_DOM_1_CFG			0x0020
#define ICP_DOM_0_CFG_MASK		0x00ff00ff
#define ICP_DOM_1_CFG_MASK		0xff00ff00

#define LX7_WFI_TIMEOUT_US		300000

static TEE_Result camera_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);
	uint32_t sid_end = 0;
	uint32_t reg = 0;

	if (!base)
		return TEE_ERROR_GENERIC;

	io_write32(base + ICP_SYS_RESET, ICP_SYS_RESET_FUNC_RESET);

	sid_end = (uint32_t)(data->fw_size) & ICP_SYS_SID_REGION_MASK;

	DMSG("camera: SID1 start=0x%08x end=0x%08x", 0, sid_end);

	io_write32(base + ICP_SYS_SID1_START_ADDR, 0);
	io_write32(base + ICP_SYS_SID1_END_ADDR, sid_end);

	io_write32(base + ICP_SYS_ACCESS, CAMERA_ICP_QOS);

	reg = io_read32(base + ICP_SYS_SID1_START_ADDR);
	io_write32(base + ICP_SYS_SID1_START_ADDR,
		   reg | ICP_SYS_SID1_START_ADDR_EN);

#ifdef CAMERA_ICP_DOM_MASK_OFFSET
	io_write32(base + CAMERA_ICP_DOM_MASK_OFFSET + ICP_DOM_0_CFG,
		   ICP_DOM_0_CFG_MASK);
	io_write32(base + CAMERA_ICP_DOM_MASK_OFFSET + ICP_DOM_1_CFG,
		   ICP_DOM_1_CFG_MASK);
#endif

	io_write32(base + ICP_SYS_CONTROL, ICP_SYS_CONTROL_CPU_EN);

	return TEE_SUCCESS;
}

static TEE_Result camera_fw_shutdown(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	if (!base)
		return TEE_ERROR_GENERIC;

	io_write32(base + ICP_SYS_CONTROL, 0);

	return TEE_SUCCESS;
}

static TEE_Result camera_fw_set_state(struct qcom_pas_data *data, bool power_on)
{
	uint64_t timeout = timeout_init_us(LX7_WFI_TIMEOUT_US);
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	if (!base)
		return TEE_ERROR_GENERIC;

	if (power_on) {
		if (io_read32(base + ICP_SYS_CONTROL) & ICP_SYS_CONTROL_CPU_EN)
			return TEE_ERROR_BAD_STATE;
		return camera_fw_start(data);
	}

	while (!timeout_elapsed(timeout)) {
		if (io_read32(base + ICP_SYS_STATUS) &
		    ICP_SYS_STATUS_LX7_STANDBYWFI)
			return camera_fw_shutdown(data);
		udelay(10);
	}

	DMSG("camera: WFI timeout, forcing halt");
	camera_fw_shutdown(data);
	return TEE_ERROR_TIMEOUT;
}

const struct qcom_pas_ops pas_camera_icp_ops = {
	.fw_start = camera_fw_start,
	.fw_shutdown = camera_fw_shutdown,
	.fw_set_state = camera_fw_set_state,
};
