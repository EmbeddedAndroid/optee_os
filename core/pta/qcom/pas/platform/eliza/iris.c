// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <stdint.h>
#include <trace.h>
#include <util.h>

#include "iris.h"
#include "pas_video.h"

/* Registers, as offsets from IRIS_BASE */
#define WRAPPER_TZ_XTSS_SW_RESET		0x000c1000
#define WRAPPER_XTSS_SW_RESET_BIT		BIT(0)
#define WRAPPER_TZ_SEC_CPA_START_ADDR		0x000c1020
#define WRAPPER_TZ_SEC_CPA_END_ADDR		0x000c1024
#define WRAPPER_TZ_SEC_FW_START_ADDR		0x000c1028
#define WRAPPER_TZ_SEC_FW_END_ADDR		0x000c102c
#define WRAPPER_TZ_SEC_NONPIX_START_ADDR	0x000c1030
#define WRAPPER_TZ_SEC_NONPIX_END_ADDR		0x000c1034

#define SS_TZ_SEC_THRESHOLD_HEVC		0x000c2004
#define SS_TZ_SEC_THRESHOLD_H264		0x000c2008
#define SS_TZ_SEC_THRESHOLD_MP2			0x000c200c
#define SS_TZ_SEC_THRESHOLD_NON_VCL_HEVC	0x000c2018
#define SS_TZ_SEC_THRESHOLD_NON_VCL_H264	0x000c201c
#define SS_TZ_SEC_THRESHOLD_NON_VCL_MP2		0x000c2020
#define SS_TZ_SEC_DS_THRESHOLD			0x000c2024
#define SS_TZ_SEC_THRESHOLD_AV1			0x000c202c

#define SS_TZ_SEC_SID_0_SECURE_OVERRIDE		0x000c2040
#define SS_TZ_SEC_SID_1_SECURE_OVERRIDE		0x000c2044
#define SS_TZ_SEC_SID_2_SECURE_OVERRIDE		0x000c2048
#define SS_TZ_SEC_SID_3_SECURE_OVERRIDE		0x000c204c
#define SS_TZ_SEC_SID_4_SECURE_OVERRIDE		0x000c2050
#define SS_TZ_SEC_SID_5_SECURE_OVERRIDE		0x000c2054
#define SS_TZ_SEC_SID_6_SECURE_OVERRIDE		0x000c2058
#define SS_TZ_SEC_SID_7_SECURE_OVERRIDE		0x000c205c
#define SS_TZ_SEC_SID_8_SECURE_OVERRIDE		0x000c2060
#define SS_TZ_SEC_SID_9_SECURE_OVERRIDE		0x000c2064
#define SS_TZ_SEC_SID_A_SECURE_OVERRIDE		0x000c2068
#define SS_TZ_SEC_SID_B_SECURE_OVERRIDE		0x000c206c
#define SS_TZ_SEC_SID_C_SECURE_OVERRIDE		0x000c2070
#define SS_TZ_SEC_SID_D_SECURE_OVERRIDE		0x000c2074
#define SS_TZ_SEC_SID_E_SECURE_OVERRIDE		0x000c2078
#define SS_TZ_SEC_SID_F_SECURE_OVERRIDE		0x000c207c
#define SS_TZ_SEC_SID_10_SECURE_OVERRIDE	0x000c2080

#define SS_TZ_CP_OVERRIDE			0x000c2090
#define SS_TZ_CP_VP8_SECURE_OVERRIDE_DEC	0x000c2098
#define SS_TZ_CP_VP9_SECURE_OVERRIDE_DEC	0x000c209c

/*
 * Content protection (CP) windows in the device address space of the core,
 * end exclusive: the CP window starts at 0 and holds the firmware, the CP
 * non-pixel window lies inside it, above the firmware. This is the layout
 * Linux passes to the stock firmware for Eliza.
 */
#define IRIS_CP_END			0x25800000
#define IRIS_CP_NONPIX_START		0x01000000
#define IRIS_CP_NONPIX_END		0x25800000

#define IRIS_FW_ALIGN			0x1000

static struct pas_video_seed iris_seed;

/* Raised non-VCL thresholds for H.264 and HEVC */
static void iris_program_core_thresholds(vaddr_t base)
{
	io_write32(base + SS_TZ_SEC_DS_THRESHOLD, 3);
	io_write32(base + SS_TZ_SEC_THRESHOLD_HEVC, 1344);
	io_write32(base + SS_TZ_SEC_THRESHOLD_H264, 270);
	io_write32(base + SS_TZ_SEC_THRESHOLD_MP2, 46);
	io_write32(base + SS_TZ_SEC_THRESHOLD_AV1, 1280);
	io_write32(base + SS_TZ_SEC_THRESHOLD_NON_VCL_HEVC, 335872);
	io_write32(base + SS_TZ_SEC_THRESHOLD_NON_VCL_H264, 335872);
	io_write32(base + SS_TZ_SEC_THRESHOLD_NON_VCL_MP2, 1600);
}

static void iris_clear_sid_overrides(vaddr_t base)
{
	io_write32(base + SS_TZ_CP_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_0_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_1_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_2_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_3_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_4_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_5_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_6_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_7_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_8_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_9_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_A_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_B_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_C_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_D_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_E_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_F_SECURE_OVERRIDE, 0);
	io_write32(base + SS_TZ_SEC_SID_10_SECURE_OVERRIDE, 0);
}

/* Read back the overrides the core relies on */
static TEE_Result iris_set_checked(vaddr_t reg, const char *name)
{
	uint32_t val = 0;

	io_write32(reg, 1);
	val = io_read32(reg);
	if (val != 1) {
		EMSG("Iris: %s reads %#"PRIx32", expected 1", name, val);
		return TEE_ERROR_GENERIC;
	}

	return TEE_SUCCESS;
}

static TEE_Result iris_program_sec_sid_registers(vaddr_t base)
{
	TEE_Result res = TEE_ERROR_GENERIC;

	iris_clear_sid_overrides(base);
	iris_program_core_thresholds(base);

	io_write32(base + SS_TZ_CP_VP8_SECURE_OVERRIDE_DEC, 0);
	io_write32(base + SS_TZ_CP_VP9_SECURE_OVERRIDE_DEC, 0);

	io_write32(base + SS_TZ_SEC_SID_F_SECURE_OVERRIDE, 1);

	res = iris_set_checked(base + SS_TZ_SEC_SID_1_SECURE_OVERRIDE,
			       "SID_1 secure override");
	if (res)
		return res;

	res = iris_set_checked(base + SS_TZ_SEC_SID_7_SECURE_OVERRIDE,
			       "SID_7 secure override");
	if (res)
		return res;

	io_write32(base + SS_TZ_SEC_SID_C_SECURE_OVERRIDE, 1);
	io_write32(base + SS_TZ_SEC_SID_D_SECURE_OVERRIDE, 1);
	io_write32(base + SS_TZ_SEC_SID_E_SECURE_OVERRIDE, 1);

	return TEE_SUCCESS;
}

/* The firmware window ends on a page, below the CP non-pixel window */
static TEE_Result iris_fw_end(const struct qcom_pas_data *data,
			      uint32_t *fw_end)
{
	if (!data->fw_size || data->fw_size > IRIS_CP_NONPIX_START) {
		EMSG("Iris: firmware size %#zx outside the CP layout",
		     data->fw_size);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	*fw_end = ROUNDUP(data->fw_size, IRIS_FW_ALIGN);

	return TEE_SUCCESS;
}

static void iris_program_mem_regions(vaddr_t base, uint32_t fw_end)
{
	io_write32(base + WRAPPER_TZ_SEC_FW_START_ADDR, 0);
	io_write32(base + WRAPPER_TZ_SEC_FW_END_ADDR, fw_end);
	io_write32(base + WRAPPER_TZ_SEC_CPA_START_ADDR, 0);
	io_write32(base + WRAPPER_TZ_SEC_CPA_END_ADDR, IRIS_CP_END);
	io_write32(base + WRAPPER_TZ_SEC_NONPIX_START_ADDR,
		   IRIS_CP_NONPIX_START);
	io_write32(base + WRAPPER_TZ_SEC_NONPIX_END_ADDR, IRIS_CP_NONPIX_END);
}

static bool iris_is_halted(vaddr_t base)
{
	return io_read32(base + WRAPPER_TZ_XTSS_SW_RESET) &
	       WRAPPER_XTSS_SW_RESET_BIT;
}

static TEE_Result iris_reset_cpu(vaddr_t base, uint32_t fw_end)
{
	TEE_Result res = TEE_ERROR_GENERIC;

	iris_program_mem_regions(base, fw_end);

	res = iris_program_sec_sid_registers(base);
	if (res)
		return res;

	io_write32(base + WRAPPER_TZ_XTSS_SW_RESET, 0);

	return TEE_SUCCESS;
}

static TEE_Result iris_fw_init_image(struct qcom_pas_data *data __unused,
				     const uint8_t *metadata,
				     const struct elf_info *e_info)
{
	return pas_video_find_seed(metadata, e_info, &iris_seed);
}

static TEE_Result iris_fw_start(struct qcom_pas_data *data)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t fw_end = 0;
	vaddr_t base = 0;

	res = iris_fw_end(data, &fw_end);
	if (res) {
		pas_video_clear_seed(&iris_seed);
		return res;
	}

	base = io_pa_or_va(&data->base, data->size);
	if (!base) {
		pas_video_clear_seed(&iris_seed);
		return TEE_ERROR_GENERIC;
	}

	/* Never write a seed under a running core */
	if (!iris_is_halted(base)) {
		pas_video_clear_seed(&iris_seed);
		EMSG("Iris: core out of reset, cannot start a new image");
		return TEE_ERROR_BAD_STATE;
	}

	res = pas_video_fill_seed(data, &iris_seed);
	if (res) {
		EMSG("Iris: cannot fill the firmware seed: %#"PRIx32, res);
		return res;
	}

	return iris_reset_cpu(base, fw_end);
}

/*
 * The core is put back in reset by Linux, through the video controller
 * reset and power collapse that follow a shutdown or a suspend: the reset
 * control is not retained and comes back asserted. Writing it here instead
 * would race with the controller clocks Linux may already have stopped.
 */
static TEE_Result iris_fw_shutdown(struct qcom_pas_data *data __unused)
{
	return TEE_SUCCESS;
}

static TEE_Result iris_fw_resume(struct qcom_pas_data *data)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t fw_end = 0;
	vaddr_t base = 0;

	if (!data->loaded) {
		EMSG("Iris: no firmware to resume");
		return TEE_ERROR_BAD_STATE;
	}

	res = iris_fw_end(data, &fw_end);
	if (res)
		return res;

	base = io_pa_or_va(&data->base, data->size);
	if (!base)
		return TEE_ERROR_GENERIC;

	/* A resume of a running core is not an error */
	if (!iris_is_halted(base))
		return TEE_SUCCESS;

	return iris_reset_cpu(base, fw_end);
}

static TEE_Result iris_fw_set_state(struct qcom_pas_data *data, bool power_on)
{
	if (power_on)
		return iris_fw_resume(data);

	return iris_fw_shutdown(data);
}

const struct qcom_pas_ops iris_ops = {
	.fw_init_image = iris_fw_init_image,
	.fw_start = iris_fw_start,
	.fw_shutdown = iris_fw_shutdown,
	.fw_set_state = iris_fw_set_state,
};
