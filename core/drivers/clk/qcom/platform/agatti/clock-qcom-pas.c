// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk_qcom.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <stdint.h>
#include <trace.h>

#include "clock_group_qcom.h"

register_phys_mem(MEM_AREA_IO_NSEC, TCSR_BASE, TCSR_SIZE);

#define GDSC_TIMEOUT_US		10000
#define HALT_ACK_TIMEOUT_US	1000000
#define NTS_ACK_TIMEOUT_US	1000

static vaddr_t gcc_va(void)
{
	struct io_pa_va io = { .pa = GCC_BASE };

	return io_pa_or_va(&io, GCC_SIZE);
}

static vaddr_t lpass_va(void)
{
	struct io_pa_va io = { .pa = LPASS_BASE };

	return io_pa_or_va(&io, LPASS_SIZE);
}

static vaddr_t tcsr_va(void)
{
	struct io_pa_va io = { .pa = TCSR_BASE };

	return io_pa_or_va(&io, TCSR_SIZE);
}

static TEE_Result poll_set(vaddr_t reg, uint32_t mask, uint32_t timeout_us)
{
	uint32_t val = 0;

	if (IO_READ32_POLL_TIMEOUT(reg, val, (val & mask) == mask, 1,
				   timeout_us))
		return TEE_ERROR_TIMEOUT;

	return TEE_SUCCESS;
}

static TEE_Result poll_clear(vaddr_t reg, uint32_t mask, uint32_t timeout_us)
{
	uint32_t val = 0;

	if (IO_READ32_POLL_TIMEOUT(reg, val, !(val & mask), 1, timeout_us))
		return TEE_ERROR_TIMEOUT;

	return TEE_SUCCESS;
}

/* Clocks and bus handshakes the ADSP needs before its boot FSM runs */
static TEE_Result lpass_setup(void)
{
	vaddr_t gcc = gcc_va();
	vaddr_t lpass = lpass_va();
	TEE_Result res = TEE_SUCCESS;
	uint32_t dbg_cfg = 0;

	if (!gcc || !lpass)
		return TEE_ERROR_GENERIC;

	res = qcom_clock_enable_cbc(gcc + GCC_LPASS_SWAY_CBCR);
	if (res)
		return res;

	res = qcom_clock_enable_cbc(lpass + LPASS_Q6_AHBM_CBCR);
	if (res)
		return res;

	res = qcom_clock_enable_cbc(lpass + LPASS_Q6_AHBS_CBCR);
	if (res)
		return res;

	/* Preserve a debugger's QDSP6SS_DBG_CFG across the clock setup */
	dbg_cfg = io_read32(lpass + LPASS_QDSP6SS_DBG_CFG);

	io_setbits32(lpass + LPASS_PDC_HM_GDSCR, GDSCR_CLK_DIS_WAIT_MASK);

	res = qcom_clock_enable_cbc(gcc + GCC_LPASS_CORE_AXIM_CBCR);
	if (res)
		return res;

	/* The core clock turns on when the boot FSM completes */
	io_setbits32(lpass + LPASS_QDSP6SS_XO_CBCR, CBCR_BRANCH_ENABLE_BIT);
	io_setbits32(lpass + LPASS_QDSP6SS_SLEEP_CBCR, CBCR_BRANCH_ENABLE_BIT);
	io_setbits32(lpass + LPASS_QDSP6SS_CORE_CBCR, CBCR_BRANCH_ENABLE_BIT);

	io_write32(lpass + LPASS_QDSP6SS_DBG_CFG, dbg_cfg);
	if (dbg_cfg) {
		io_setbits32(lpass + LPASS_AT_CBCR, CBCR_BRANCH_ENABLE_BIT);
		io_setbits32(lpass + LPASS_PCLKDBG_CBCR,
			     CBCR_BRANCH_ENABLE_BIT);
	}

	io_setbits32(lpass + LPASS_APB_LOW_POWER_HANDSHAKE,
		     LOW_POWER_HANDSHAKE_REQUEST);
	io_setbits32(lpass + LPASS_NTS_LOW_POWER_HANDSHAKE,
		     LOW_POWER_HANDSHAKE_REQUEST);
	if (poll_set(lpass + LPASS_NTS_LOW_POWER_HANDSHAKE,
		     LOW_POWER_HANDSHAKE_ACK, NTS_ACK_TIMEOUT_US))
		DMSG("LPASS NTS handshake not acknowledged");

	return TEE_SUCCESS;
}

/*
 * Put LPASS back into reset so that a stopped or crashed ADSP boots again
 * from a clean state: reset and collapse the audio head switch, make sure
 * the SSC domain is up, halt the Q6 bus port and restart the subsystem.
 */
static TEE_Result lpass_reset_processor(void)
{
	vaddr_t gcc = gcc_va();
	vaddr_t lpass = lpass_va();
	vaddr_t tcsr = tcsr_va();
	TEE_Result res = TEE_SUCCESS;

	if (!gcc || !lpass || !tcsr)
		return TEE_ERROR_GENERIC;

	io_setbits32(lpass + LPASS_QDSP6SS_RET_CFG,
		     QDSP6SS_RET_CFG_RET_ARES_ENA);

	io_clrbits32(lpass + LPASS_AUDIO_HM_GDSCR, GDSCR_SW_COLLAPSE);
	res = poll_set(lpass + LPASS_AUDIO_HM_GDSCR, GDSCR_PWR_ON,
		       GDSC_TIMEOUT_US);
	if (res)
		return res;

	io_setbits32(lpass + LPASS_AUDIO_HM_BCR, BCR_BLK_ARES);
	udelay(150);
	io_clrbits32(lpass + LPASS_AUDIO_HM_BCR, BCR_BLK_ARES);
	udelay(150);

	io_clrbits32(lpass + LPASS_AUDIO_CC_PLL_MODE,
		     PLL_MODE_OUTCTRL | PLL_MODE_BYPASSNL | PLL_MODE_RESET_N);
	io_clrbits32(lpass + LPASS_AUDIO_CC_DIG_PLL_MODE,
		     PLL_MODE_OUTCTRL | PLL_MODE_BYPASSNL | PLL_MODE_RESET_N);

	io_setbits32(lpass + LPASS_AUDIO_HM_GDSCR, GDSCR_SW_COLLAPSE);
	res = poll_clear(lpass + LPASS_AUDIO_HM_GDSCR, GDSCR_PWR_ON,
			 GDSC_TIMEOUT_US);
	if (res)
		return res;

	if (!(io_read32(lpass + LPASS_SSC_GDSCR) & GDSCR_PWR_ON)) {
		io_clrbits32(lpass + LPASS_SSC_GDSCR, GDSCR_SW_COLLAPSE);
		res = poll_set(lpass + LPASS_SSC_GDSCR, GDSCR_PWR_ON,
			       GDSC_TIMEOUT_US);
		if (!res)
			res = poll_set(lpass + LPASS_SSC_PWR_RDY_STATUS,
				       SSC_RDY_FOR_ACCESS, GDSC_TIMEOUT_US);
		if (!res)
			res = poll_clear(lpass + LPASS_SSC_SCC_CRIF_CBCR,
					 CBCR_BRANCH_OFF_BIT, GDSC_TIMEOUT_US);
		if (res)
			return res;
	}

	/* A Q6 in a bad state may never acknowledge the halt */
	io_setbits32(tcsr + TCSR_LPASS_HALTREQ, TCSR_HALT_BIT);
	if (poll_set(tcsr + TCSR_LPASS_HALTACK, TCSR_HALT_BIT,
		     HALT_ACK_TIMEOUT_US))
		DMSG("LPASS halt not acknowledged");

	io_setbits32(gcc + GCC_LPASS_RESTART, BIT(0));
	udelay(200);
	io_clrbits32(gcc + GCC_LPASS_RESTART, BIT(0));

	io_clrbits32(tcsr + TCSR_LPASS_HALTREQ, TCSR_HALT_BIT);
	udelay(100);

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_enable_pas_processor(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_LPASS:
		/* The boot FSM in fw_start released the core */
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

TEE_Result qcom_clock_pas_reset(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_LPASS:
		return lpass_reset_processor();
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

TEE_Result qcom_clock_enable_pas(enum qcom_clk_group group)
{
	TEE_Result res = TEE_ERROR_NOT_SUPPORTED;

	switch (group) {
	case QCOM_CLKS_LPASS:
		res = lpass_setup();
		break;
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}

	if (res)
		EMSG("Clock group %d setup failed: %#"PRIx32, group, res);

	return res;
}
