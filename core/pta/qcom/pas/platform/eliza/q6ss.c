// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <io.h>
#include <kernel/delay.h>
#include <trace.h>
#include <util.h>

#include "q6ss.h"
#include "socinfo.h"

#define QDSP6SS_RST_EVB		0x10
#define QDSP6SS_RET_CFG		0x1c
#define QDSP6SS_BOOT_PARAMS(n)	(0x60 + 4 * (n))
#define QDSP6SS_BOOT_CORE_START	0x400
#define QDSP6SS_BOOT_CMD	0x404
#define QDSP6SS_BOOT_STATUS	0x408

#define RST_EVB_MASK		GENMASK_32(27, 4)
#define BOOT_FSM_TIMEOUT_US	1000000

/* ChipInfo family of the SM7750, as the DSP device tree images select on it */
#define CHIP_FAMILY_ELIZA	159

void q6ss_set_boot_addr(vaddr_t q6ss, paddr_t addr)
{
	/* EVB holds bits 31:8 of the address in bits 27:4 */
	io_write32(q6ss + QDSP6SS_RST_EVB, (addr >> 4) & RST_EVB_MASK);
	dsb();
}

TEE_Result q6ss_set_dtb_params(vaddr_t q6ss, paddr_t dtb, size_t dtb_size)
{
	struct qcom_socinfo info = { };
	TEE_Result res = TEE_SUCCESS;
	uint32_t chip_ver = 0;
	uint32_t plat_ver = 0;

	res = qcom_socinfo_read(&info);
	if (res)
		return res;

	chip_ver = SHIFT_U32((info.version >> 16) & 0xff, 8) |
		   (info.version & 0xff);
	plat_ver = SHIFT_U32((info.platform_version >> 16) & 0xff, 8) |
		   (info.platform_version & 0xff);

	io_write32(q6ss + QDSP6SS_BOOT_PARAMS(0), (uint32_t)dtb);
	io_write32(q6ss + QDSP6SS_BOOT_PARAMS(1), (uint64_t)dtb >> 32);
	io_write32(q6ss + QDSP6SS_BOOT_PARAMS(2),
		   SHIFT_U32(CHIP_FAMILY_ELIZA, 16) | (info.id & 0xffff));
	io_write32(q6ss + QDSP6SS_BOOT_PARAMS(3), chip_ver);
	io_write32(q6ss + QDSP6SS_BOOT_PARAMS(4),
		   SHIFT_U32(info.platform & 0xff, 24) |
		   SHIFT_U32(info.platform_subtype & 0xff, 16) | plat_ver);
	io_write32(q6ss + QDSP6SS_BOOT_PARAMS(5), dtb_size);
	dsb();

	DMSG("DSP DTB %#"PRIxPA" size %zu chip %"PRIu32" v%#"PRIx32
	     " platform %"PRIu32".%"PRIu32" v%#"PRIx32, dtb, dtb_size,
	     info.id, chip_ver, info.platform, info.platform_subtype,
	     plat_ver);

	return TEE_SUCCESS;
}

TEE_Result q6ss_boot(vaddr_t q6ss, bool start_core)
{
	uint64_t timeout = 0;

	io_write32(q6ss + QDSP6SS_RET_CFG, 0);
	if (start_core)
		io_setbits32(q6ss + QDSP6SS_BOOT_CORE_START, BIT(0));
	io_write32(q6ss + QDSP6SS_BOOT_CMD, BIT(0));

	timeout = timeout_init_us(BOOT_FSM_TIMEOUT_US);
	while (!(io_read32(q6ss + QDSP6SS_BOOT_STATUS) & BIT(0))) {
		if (timeout_elapsed(timeout)) {
			EMSG("QDSP6 boot FSM timed out");
			return TEE_ERROR_TIMEOUT;
		}
		udelay(5);
	}
	udelay(5);

	return TEE_SUCCESS;
}
