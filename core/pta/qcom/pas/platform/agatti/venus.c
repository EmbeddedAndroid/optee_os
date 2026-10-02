// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <crypto/crypto.h>
#include <elf32.h>
#include <io.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <trace.h>
#include <util.h>

#include "venus.h"

/* Relative to VENUS_BASE */
#define WRAPPER_SEC_SID_SECURE_0	0xb1000
#define WRAPPER_SEC_SID_SECURE_8	0xb1040
#define WRAPPER_SEC_SID_COUNT		16

#define WRAPPER_TZ_XTSS_SW_RESET	0xc1000
#define WRAPPER_TZ_SEC_CPA_START	0xc1020
#define WRAPPER_TZ_SEC_CPA_END		0xc1024
#define WRAPPER_TZ_SEC_FW_START		0xc1028
#define WRAPPER_TZ_SEC_FW_END		0xc102c
#define WRAPPER_TZ_SEC_NONPIX_START	0xc1030
#define WRAPPER_TZ_SEC_NONPIX_END	0xc1034
#define WRAPPER_TZ_CP_OVERRIDE		0xc1104
#define WRAPPER_TZ_SEC_SID_E_OVERRIDE	0xc1148
#define WRAPPER_TZ_SEC_THRESHOLD_HEVC	0xc1150
#define WRAPPER_TZ_SEC_THRESHOLD_H264	0xc1154
#define WRAPPER_TZ_SEC_THRESHOLD_NON_VCL_HEVC	0xc116c
#define WRAPPER_TZ_SEC_THRESHOLD_NON_VCL_H264	0xc1170
#define WRAPPER_TZ_SEC_DS_THRESHOLD	0xc1184
#define WRAPPER_TZ_CP_VP8_SECURE_OVERRIDE_DEC	0xc1188
#define WRAPPER_TZ_CP_VP9_SECURE_OVERRIDE_DEC	0xc118c

#define XTSS_SW_RESET			BIT(0)

/*
 * Content protection (CP) layout in the device address space of the core,
 * end-exclusive: the CP range starts at 0 and holds the firmware, the CP
 * non-pixel range lies inside it, above the firmware. These are the ranges
 * Linux sets for QCM2290 on the QTI TZ.
 */
#define VENUS_CP_END			0x70800000
#define VENUS_CP_NONPIX_START		0x01000000
#define VENUS_CP_NONPIX_END		0x25800000

/* Qualcomm ELF program header flags: segment type */
#define SEG_TYPE_MASK			GENMASK_32(26, 24)
#define SEG_TYPE_HASH			SHIFT_U32(2, 24)

/* Largest firmware seed segment accepted */
#define SEED_MAX_SIZE			0x1000

/* End-exclusive device address range */
struct venus_range {
	uint32_t start;
	uint32_t end;
};

static struct venus_state {
	bool loaded;
	bool running;
	uint32_t fw_end;
	struct venus_range seed;
} venus;

static bool venus_is_halted(vaddr_t base)
{
	return io_read32(base + WRAPPER_TZ_XTSS_SW_RESET) & XTSS_SW_RESET;
}

static void venus_halt(vaddr_t base)
{
	io_setbits32(base + WRAPPER_TZ_XTSS_SW_RESET, XTSS_SW_RESET);
}

static vaddr_t venus_sec_sid_reg(vaddr_t base, unsigned int n)
{
	if (n < 8)
		return base + WRAPPER_SEC_SID_SECURE_0 + 4 * n;

	return base + WRAPPER_SEC_SID_SECURE_8 + 4 * (n - 8);
}

static void venus_program_ranges(vaddr_t base)
{
	unsigned int n = 0;

	io_write32(base + WRAPPER_TZ_SEC_FW_START, 0);
	io_write32(base + WRAPPER_TZ_SEC_FW_END, venus.fw_end);
	io_write32(base + WRAPPER_TZ_SEC_CPA_START, 0);
	io_write32(base + WRAPPER_TZ_SEC_CPA_END, VENUS_CP_END);
	io_write32(base + WRAPPER_TZ_SEC_NONPIX_START, VENUS_CP_NONPIX_START);
	io_write32(base + WRAPPER_TZ_SEC_NONPIX_END, VENUS_CP_NONPIX_END);

	io_write32(base + WRAPPER_TZ_CP_OVERRIDE, 0);
	io_write32(base + WRAPPER_TZ_SEC_SID_E_OVERRIDE, 1);
	for (n = 0; n < WRAPPER_SEC_SID_COUNT; n++)
		io_write32(venus_sec_sid_reg(base, n), 0);
}

/* Secure session thresholds and overrides, values from the QTI reference */
static void venus_program_sec(vaddr_t base)
{
	io_write32(base + WRAPPER_TZ_SEC_DS_THRESHOLD, 3);
	io_write32(base + WRAPPER_TZ_SEC_THRESHOLD_HEVC, 1344);
	io_write32(base + WRAPPER_TZ_SEC_THRESHOLD_H264, 270);
	io_write32(base + WRAPPER_TZ_SEC_THRESHOLD_NON_VCL_HEVC, 3800);
	io_write32(base + WRAPPER_TZ_SEC_THRESHOLD_NON_VCL_H264, 12600);
	io_write32(base + WRAPPER_TZ_CP_VP8_SECURE_OVERRIDE_DEC, 0);
	io_write32(base + WRAPPER_TZ_CP_VP9_SECURE_OVERRIDE_DEC, 0);
}

static bool phdr_loadable(const Elf32_Phdr *phdr)
{
	return phdr->p_type == PT_LOAD &&
	       (phdr->p_flags & SEG_TYPE_MASK) != SEG_TYPE_HASH &&
	       phdr->p_memsz;
}

/*
 * The last loadable segment of the video firmware is a seed that the secure
 * side fills with random data before the core starts. Record where it lands
 * in the firmware region.
 */
static TEE_Result venus_fw_init_image(struct qcom_pas_data *data __unused,
				      const void *metadata, size_t size)
{
	const Elf32_Ehdr *ehdr = metadata;
	const Elf32_Phdr *phdr = NULL;
	const Elf32_Phdr *last = NULL;
	uint32_t min = UINT32_MAX;
	uint32_t start = 0;
	uint32_t end = 0;
	size_t len = 0;
	size_t n = 0;

	venus.seed = (struct venus_range){ };

	if (!metadata || size < sizeof(*ehdr) ||
	    memcmp(ehdr->e_ident, ELFMAG, SELFMAG) ||
	    ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
	    ehdr->e_phentsize != sizeof(*phdr) ||
	    !IS_ALIGNED(ehdr->e_phoff, sizeof(uint32_t)) ||
	    MUL_OVERFLOW(ehdr->e_phnum, sizeof(*phdr), &len) ||
	    ADD_OVERFLOW(len, ehdr->e_phoff, &len) || len > size)
		return TEE_ERROR_BAD_FORMAT;

	phdr = (const void *)((const uint8_t *)metadata + ehdr->e_phoff);
	for (n = 0; n < ehdr->e_phnum; n++) {
		if (!phdr_loadable(phdr + n))
			continue;
		min = MIN(min, phdr[n].p_paddr);
		last = phdr + n;
	}

	if (!last || last->p_memsz > SEED_MAX_SIZE)
		return TEE_ERROR_BAD_FORMAT;

	start = last->p_paddr - min;
	if (ADD_OVERFLOW(start, last->p_memsz, &end))
		return TEE_ERROR_BAD_FORMAT;

	venus.seed.start = start;
	venus.seed.end = end;

	return TEE_SUCCESS;
}

static TEE_Result venus_fill_seed(struct qcom_pas_data *data)
{
	size_t len = venus.seed.end - venus.seed.start;
	TEE_Result res = TEE_ERROR_GENERIC;
	paddr_t pa = 0;
	void *va = NULL;

	if (!len)
		return TEE_SUCCESS;

	if (venus.seed.end > data->fw_size ||
	    ADD_OVERFLOW(data->fw_base, venus.seed.start, &pa) ||
	    !core_pbuf_is(CORE_MEM_NON_SEC, pa, len))
		return TEE_ERROR_SECURITY;

	va = core_mmu_add_mapping(MEM_AREA_RAM_NSEC, pa, len);
	if (!va)
		return TEE_ERROR_GENERIC;

	res = crypto_rng_read(va, len);

	if (core_mmu_remove_mapping(MEM_AREA_RAM_NSEC, va, len))
		EMSG("Venus: failed to unmap the firmware seed");

	return res;
}

/*
 * The wrapper loses these settings when the venus power domain collapses, so
 * they are programmed again on every resume. A core already out of reset is
 * left running.
 */
static void venus_release(vaddr_t base)
{
	if (!venus_is_halted(base)) {
		venus.running = true;
		return;
	}

	venus_program_ranges(base);
	venus_program_sec(base);
	io_clrbits32(base + WRAPPER_TZ_XTSS_SW_RESET, XTSS_SW_RESET);
	venus.running = true;
}

static TEE_Result venus_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);
	TEE_Result res = TEE_ERROR_GENERIC;

	if (!base)
		return TEE_ERROR_GENERIC;

	/* The firmware runs from 0 and must end below the non-pixel range */
	if (!data->fw_size || data->fw_size > VENUS_CP_NONPIX_START) {
		EMSG("Venus: firmware size %#zx outside the CP layout",
		     data->fw_size);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	res = venus_fill_seed(data);
	if (res) {
		EMSG("Venus: cannot seed the firmware: %#"PRIx32, res);
		return res;
	}

	venus.fw_end = data->fw_size;
	venus.loaded = true;
	venus_release(base);

	return TEE_SUCCESS;
}

static TEE_Result venus_fw_shutdown(struct qcom_pas_data *data)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	if (!base)
		return TEE_ERROR_GENERIC;

	venus_halt(base);
	venus.loaded = false;
	venus.running = false;

	return TEE_SUCCESS;
}

static TEE_Result venus_fw_set_state(struct qcom_pas_data *data, bool on)
{
	vaddr_t base = io_pa_or_va(&data->base, data->size);

	if (!base)
		return TEE_ERROR_GENERIC;

	/* Resume before a load, or suspend twice, is harmless */
	if (!venus.loaded)
		return TEE_SUCCESS;

	if (on) {
		venus_release(base);
	} else if (venus.running) {
		venus_halt(base);
		venus.running = false;
	}

	return TEE_SUCCESS;
}

const struct qcom_pas_ops venus_ops = {
	.fw_init_image = venus_fw_init_image,
	.fw_start = venus_fw_start,
	.fw_shutdown = venus_fw_shutdown,
	.fw_set_state = venus_fw_set_state,
};
