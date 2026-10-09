// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk.h>
#include <drivers/clk_qcom.h>
#include <inttypes.h>
#include <io.h>
#include <kernel/delay.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <stdint.h>
#include <trace.h>

#include "clock_group.h"

register_phys_mem(MEM_AREA_IO_NSEC, AOSS_CC_BASE, AOSS_CC_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, MPM_SSCAON_BASE, MPM_SSCAON_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, RPMH_PDC_GLOBAL_BASE, RPMH_PDC_GLOBAL_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, TCSR_MUTEX_BASE, TCSR_MUTEX_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, TCSR_REGS_BASE, TCSR_REGS_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, LPASS_BASE, LPASS_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC, TURING_BASE, TURING_SIZE);

/* A QDSP6 in a bad state may never acknowledge a halt request. */
#define HALT_ACK_TIMEOUT_US			1000000

static vaddr_t map(paddr_t pa, size_t size)
{
	struct io_pa_va io = { .pa = pa };

	return io_pa_or_va(&io, size);
}

/*
 * Request a halt of a bus master port and wait for its acknowledgment;
 * @halt is the TCSR halt block of the port.
 */
static void halt_port(vaddr_t halt)
{
	uint64_t timeout = 0;

	io_setbits32(halt + TCSR_HALTREQ, TCSR_HALT_BIT);

	timeout = timeout_init_us(HALT_ACK_TIMEOUT_US);
	while (!(io_read32(halt + TCSR_HALTACK) & TCSR_HALT_BIT)) {
		if (timeout_elapsed(timeout)) {
			EMSG("Bus master halt not acknowledged");
			return;
		}
		udelay(5);
	}
}

static TEE_Result lpass_enable(void)
{
	vaddr_t gcc = map(GCC_BASE, GCC_SIZE);
	vaddr_t lpass = map(LPASS_BASE, LPASS_SIZE);
	vaddr_t mpm = map(MPM_SSCAON_BASE, MPM_SSCAON_SIZE);
	TEE_Result res = TEE_SUCCESS;

	if (!gcc || !lpass || !mpm)
		return TEE_ERROR_GENERIC;

	/* Release the clamp on the SSC power-domain handshakes */
	io_clrbits32(mpm + MPM_SSCAON_CONFIG1, MPM_SSCAON_CONFIG1_CLAMP);

	io_setbits32(gcc + GCC_SSC_RESET, GCC_SSC_RESET_SSC_ARES);
	io_clrbits32(gcc + GCC_SSC_RESET, GCC_SSC_RESET_SSC_ARES);

	/* SSC interface clocks sourced by GCC */
	res = qcom_clock_enable_cbc(gcc + GCC_LPASS_Q6_AXI_CBCR);
	if (res)
		return res;
	res = qcom_clock_enable_cbc(gcc + GCC_LPASS_CORE_AXIM_CBCR);
	if (res)
		return res;
	res = qcom_clock_enable_cbc(gcc + GCC_LPASS_SWAY_CBCR);
	if (res)
		return res;
	res = qcom_clock_enable_cbc(gcc + GCC_SSC_CNOC_MPU_CBCR);
	if (res)
		return res;

	/* QDSP6 bus and debug clocks */
	res = qcom_clock_enable_cbc(lpass + SSC_SCC_Q6_AHBS_CBCR);
	if (res)
		return res;
	res = qcom_clock_enable_cbc(lpass + SSC_SCC_Q6_AHBM_CBCR);
	if (res)
		return res;
	res = qcom_clock_enable_cbc(lpass + SSC_SCC_AT_CBCR);
	if (res)
		return res;
	res = qcom_clock_enable_cbc(lpass + SSC_SCC_PCLKDBG_CBCR);
	if (res)
		return res;

	io_setbits32(mpm + MPM_SSCAON_CONFIG1,
		     MPM_SSCAON_CONFIG1_RETAIN_FF_ENABLE);

	return TEE_SUCCESS;
}

static TEE_Result turing_enable(void)
{
	vaddr_t gcc = map(GCC_BASE, GCC_SIZE);
	vaddr_t turing = map(TURING_BASE, TURING_SIZE);
	TEE_Result res = TEE_SUCCESS;

	if (!gcc || !turing)
		return TEE_ERROR_GENERIC;

	/* Config NoC slave port, disconnected by the last shutdown */
	res = qcom_clock_enable_cbc(gcc + GCC_TURING_CFG_AHB_CBCR);
	if (res)
		return res;

	res = qcom_clock_enable_cbc(turing + TURING_WRAPPER_AON_CBCR);
	if (res)
		return res;

	/* AHB-AON clock root from XO */
	io_clrbits32(turing + TURING_AON_CFG_RCGR, RCGR_SRC_SEL_MASK);

	res = qcom_clock_enable_cbc(turing + TURING_Q6SS_AHBS_AON_CBCR);
	if (res)
		return res;

	return qcom_clock_enable_cbc(turing + TURING_Q6SS_AHBM_AON_CBCR);
}

/*
 * Put the SSC, which holds the ADSP, through a subsystem restart, so that a
 * stopped or crashed ADSP boots again from reset.
 */
static TEE_Result lpass_reset_processor(void)
{
	vaddr_t gcc = map(GCC_BASE, GCC_SIZE);
	vaddr_t lpass = map(LPASS_BASE, LPASS_SIZE);
	vaddr_t aoss_cc = map(AOSS_CC_BASE, AOSS_CC_SIZE);
	vaddr_t pdc = map(RPMH_PDC_GLOBAL_BASE, RPMH_PDC_GLOBAL_SIZE);
	vaddr_t tcsr = map(TCSR_MUTEX_BASE, TCSR_MUTEX_SIZE);
	TEE_Result res = TEE_SUCCESS;

	if (!gcc || !lpass || !aoss_cc || !pdc || !tcsr)
		return TEE_ERROR_GENERIC;

	/* The QDSP6SS registers are reachable only with this branch on */
	res = qcom_clock_enable_cbc(gcc + GCC_LPASS_SWAY_CBCR);
	if (res)
		return res;

	/* Reset the retention flops with the core */
	io_setbits32(lpass + LPASS_QDSP6SS_OFFSET + QDSP6SS_RET_CFG,
		     QDSP6SS_RET_CFG_RET_ARES_ENA);

	if ((io_read32(tcsr + TCSR_SSC_PWR_ON) & TCSR_HALT_BIT) &&
	    !(io_read32(tcsr + TCSR_LPASS_HALT + TCSR_MASTER_IDLE) &
	      TCSR_HALT_BIT))
		halt_port(tcsr + TCSR_LPASS_HALT);

	halt_port(tcsr + TCSR_LPASS_CORE_AXIM_HALT);

	io_setbits32(pdc + RPMH_PDC_SYNC_RESET,
		     PDC_SYNC_RESET_AUDIO_SENSORS_BIT);
	io_setbits32(aoss_cc + AOSS_CC_SENSORS_RESTART, AOSS_CC_SS_RESTART_BIT);
	dsb();
	udelay(200);

	io_clrbits32(pdc + RPMH_PDC_SYNC_RESET,
		     PDC_SYNC_RESET_AUDIO_SENSORS_BIT);
	io_clrbits32(aoss_cc + AOSS_CC_SENSORS_RESTART, AOSS_CC_SS_RESTART_BIT);
	dsb();
	udelay(200);

	io_clrbits32(tcsr + TCSR_LPASS_HALT + TCSR_HALTREQ, TCSR_HALT_BIT);
	io_clrbits32(tcsr + TCSR_LPASS_CORE_AXIM_HALT + TCSR_HALTREQ,
		     TCSR_HALT_BIT);

	return TEE_SUCCESS;
}

/*
 * Put Turing, which holds the CDSP, through a subsystem restart and drop the
 * CX iPeak limits management vote the CDSP may have left behind.
 */
static TEE_Result turing_reset_processor(void)
{
	vaddr_t gcc = map(GCC_BASE, GCC_SIZE);
	vaddr_t turing = map(TURING_BASE, TURING_SIZE);
	vaddr_t aoss_cc = map(AOSS_CC_BASE, AOSS_CC_SIZE);
	vaddr_t pdc = map(RPMH_PDC_GLOBAL_BASE, RPMH_PDC_GLOBAL_SIZE);
	vaddr_t tcsr = map(TCSR_MUTEX_BASE, TCSR_MUTEX_SIZE);
	vaddr_t tcsr_regs = map(TCSR_REGS_BASE, TCSR_REGS_SIZE);
	vaddr_t halt = tcsr + TCSR_TURING_HALT;
	TEE_Result res = TEE_SUCCESS;

	if (!gcc || !turing || !aoss_cc || !pdc || !tcsr || !tcsr_regs)
		return TEE_ERROR_GENERIC;

	/* The QDSP6SS registers are reachable only with this branch on */
	res = qcom_clock_enable_cbc(gcc + GCC_TURING_CFG_AHB_CBCR);
	if (res)
		return res;

	/* Reset the retention flops with the core */
	io_setbits32(turing + TURING_QDSP6SS_OFFSET + QDSP6SS_RET_CFG,
		     QDSP6SS_RET_CFG_RET_ARES_ENA);

	/* Halt config NoC traffic by disconnecting the slave port */
	io_clrbits32(gcc + GCC_TURING_CFG_AHB_CBCR, CBCR_BRANCH_ENABLE_BIT);

	if (io_read32(halt + TCSR_TURING_PWR_ON) & TCSR_HALT_BIT) {
		if (!(io_read32(halt + TCSR_MASTER_IDLE) & TCSR_HALT_BIT))
			halt_port(halt);

		io_setbits32(pdc + RPMH_PDC_SYNC_RESET,
			     PDC_SYNC_RESET_COMPUTE_BIT);
		io_setbits32(aoss_cc + AOSS_CC_COMPUTESS_RESTART,
			     AOSS_CC_SS_RESTART_BIT);
		dsb();
		udelay(200);

		io_clrbits32(halt + TCSR_HALTREQ, TCSR_HALT_BIT);
		io_clrbits32(pdc + RPMH_PDC_SYNC_RESET,
			     PDC_SYNC_RESET_COMPUTE_BIT);
		io_clrbits32(aoss_cc + AOSS_CC_COMPUTESS_RESTART,
			     AOSS_CC_SS_RESTART_BIT);
		dsb();
		udelay(200);
	}

	io_write32(tcsr_regs + TCSR_CXIP_LM_FNT_CLEAR,
		   TCSR_CXIP_LM_FNT_CLEAR_ALL);
	udelay(1);
	if (io_read32(tcsr_regs + TCSR_CXIP_LM_FNT_STATUS)) {
		EMSG("CX iPeak vote not cleared");
		return TEE_ERROR_GENERIC;
	}

	return TEE_SUCCESS;
}

TEE_Result qcom_clock_enable_pas_processor(enum qcom_clk_group group __unused)
{
	/* The DSP core is released entirely within the PTA fw_start path. */
	return TEE_SUCCESS;
}

TEE_Result qcom_clock_pas_reset(enum qcom_clk_group group)
{
	switch (group) {
	case QCOM_CLKS_LPASS:
		return lpass_reset_processor();
	case QCOM_CLKS_TURING:
		return turing_reset_processor();
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}

TEE_Result qcom_clock_enable_pas(enum qcom_clk_group group)
{
	TEE_Result res = TEE_ERROR_GENERIC;

	switch (group) {
	case QCOM_CLKS_LPASS:
		res = lpass_enable();
		break;
	case QCOM_CLKS_TURING:
		res = turing_enable();
		break;
	default:
		EMSG("Unsupported clock group %d", group);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (res)
		EMSG("Failed to enable clock group %d: %#"PRIx32, group, res);

	return res;
}
