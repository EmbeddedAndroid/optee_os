// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/clk_qcom.h>
#include <io.h>
#include <pta_qcom_pas.h>
#include <resource_table.h>
#include <stdint.h>
#include <string.h>

#include "cdsp.h"
#include "q6ss.h"

/* Relative to TURING_BASE */
#define TURING_QDSP6SS			0x2300000

/*
 * Peripherals the CDSP reaches through its SMMU stream, identity mapped: the
 * SM7750 "CDSP Q6 ELF" stage-2 aperture, plus SMEM and the global sync
 * memory Linux assigns to it.
 */
static const struct fw_rsc_devmem turing_mem_res[] = {
	{ .name = "gcc_pll", .flags = IOMMU_READ,
		.da = 0x00100000, .pa = 0x00100000, .len = 0x0000a000, },
	{ .name = "gcc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00117000, .pa = 0x00117000, .len = 0x00002000, },
	{ .name = "gcc1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0011e000, .pa = 0x0011e000, .len = 0x00001000, },
	{ .name = "gcc2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00123000, .pa = 0x00123000, .len = 0x00001000, },
	{ .name = "gcc3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00129000, .pa = 0x00129000, .len = 0x00001000, },
	{ .name = "gcc4", .flags = IOMMU_READ,
		.da = 0x00138000, .pa = 0x00138000, .len = 0x00001000, },
	{ .name = "gcc5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00145000, .pa = 0x00145000, .len = 0x00001000, },
	{ .name = "gcc6", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0015b000, .pa = 0x0015b000, .len = 0x00001000, },
	{ .name = "gcc7", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00162000, .pa = 0x00162000, .len = 0x00001000, },
	{ .name = "ipc_core", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00405000, .pa = 0x00405000, .len = 0x00001000, },
	{ .name = "ipc_core1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00442000, .pa = 0x00442000, .len = 0x00001000, },
	{ .name = "ipc_core2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00482000, .pa = 0x00482000, .len = 0x00001000, },
	{ .name = "ipc_core3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x004c2000, .pa = 0x004c2000, .len = 0x00001000, },
	{ .name = "ipc_core4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00502000, .pa = 0x00502000, .len = 0x00001000, },
	{ .name = "ipc_config", .flags = IOMMU_READ,
		.da = 0x005fc000, .pa = 0x005fc000, .len = 0x00001000, },
	{ .name = "nsp_cx_cpr3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0063c000, .pa = 0x0063c000, .len = 0x00004000, },
	{ .name = "rng_ee10_ee10_prng_sub", .flags = IOMMU_READ,
		.da = 0x010ca000, .pa = 0x010ca000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01f40000, .pa = 0x01f40000, .len = 0x00080000, },
	{ .name = "tcsr_tcsr_regs1", .flags = IOMMU_READ,
		.da = 0x01fc0000, .pa = 0x01fc0000, .len = 0x00010000, },
	{ .name = "tcsr_tcsr_regs2", .flags = IOMMU_READ,
		.da = 0x01fd1000, .pa = 0x01fd1000, .len = 0x0001a000, },
	{ .name = "tcsr_tcsr_regs3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01feb000, .pa = 0x01feb000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs4", .flags = IOMMU_READ,
		.da = 0x01fec000, .pa = 0x01fec000, .len = 0x00004000, },
	{ .name = "rpmh_pdc_compute_compute", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0b2b0000, .pa = 0x0b2b0000, .len = 0x00010000, },
	{ .name = "rpmh_pdc_grp0_grp0_pdc_s", .flags = IOMMU_READ,
		.da = 0x0b5f4000, .pa = 0x0b5f4000, .len = 0x00001000, },
	{ .name = "rpmh_bcm_bcm_status", .flags = IOMMU_READ,
		.da = 0x0bbf0000, .pa = 0x0bbf0000, .len = 0x00002000, },
	{ .name = "rpmh_cprf_cprf", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c20a000, .pa = 0x0c20a000, .len = 0x00002000, },
	{ .name = "aoss_peripherals", .flags = IOMMU_READ,
		.da = 0x0c210000, .pa = 0x0c210000, .len = 0x00080000, },
	{ .name = "aop_ss_msg_ram_drv3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c330000, .pa = 0x0c330000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv7", .flags = IOMMU_READ,
		.da = 0x0c370000, .pa = 0x0c370000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv8", .flags = IOMMU_READ,
		.da = 0x0c380000, .pa = 0x0c380000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv9", .flags = IOMMU_READ,
		.da = 0x0c390000, .pa = 0x0c390000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv10", .flags = IOMMU_READ,
		.da = 0x0c3a0000, .pa = 0x0c3a0000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv11", .flags = IOMMU_READ,
		.da = 0x0c3b0000, .pa = 0x0c3b0000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv12", .flags = IOMMU_READ,
		.da = 0x0c3c0000, .pa = 0x0c3c0000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv13", .flags = IOMMU_READ,
		.da = 0x0c3d0000, .pa = 0x0c3d0000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv14", .flags = IOMMU_READ,
		.da = 0x0c3e0000, .pa = 0x0c3e0000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv15", .flags = IOMMU_READ,
		.da = 0x0c3f0000, .pa = 0x0c3f0000, .len = 0x00001000, },
	{ .name = "pmic_arb_core", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c400000, .pa = 0x0c400000, .len = 0x00003000, },
	{ .name = "pmic_arb_mgpi", .flags = IOMMU_READ,
		.da = 0x0c406000, .pa = 0x0c406000, .len = 0x00011000, },
	{ .name = "pmic_arb_core_registers_", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c440000, .pa = 0x0c440000, .len = 0x00080000, },
	{ .name = "tlmm", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0f000000, .pa = 0x0f000000, .len = 0x01000000, },
	{ .name = "qdss_csr", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10001000, .pa = 0x10001000, .len = 0x00002000, },
	{ .name = "qdss_in_fun0_in_fun0_cxa", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10041000, .pa = 0x10041000, .len = 0x00001000, },
	{ .name = "qdss_repl64_repl64_cxatb", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10046000, .pa = 0x10046000, .len = 0x00001000, },
	{ .name = "qdss_etr_etr_cxtmc_r64w3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10048000, .pa = 0x10048000, .len = 0x00001000, },
	{ .name = "qdss_turing_apb_turing_t", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10980000, .pa = 0x10980000, .len = 0x00002000, },
	{ .name = "system_imem", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x14680000, .pa = 0x14680000, .len = 0x00001000, },
	{ .name = "tme_cpulayer_csr", .flags = IOMMU_READ,
		.da = 0x20c21000, .pa = 0x20c21000, .len = 0x00001000, },
	{ .name = "qfprom_corr", .flags = IOMMU_READ,
		.da = 0x221c2000, .pa = 0x221c2000, .len = 0x00001000, },
	{ .name = "fuse_controller_sw_range", .flags = IOMMU_READ,
		.da = 0x221c8000, .pa = 0x221c8000, .len = 0x00001000, },
	{ .name = "ddr_reg_ddr_ss_regs", .flags = IOMMU_READ,
		.da = 0x24080000, .pa = 0x24080000, .len = 0x00001000, },
	{ .name = "cdsp_bwmon_throttle_bwmo", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x240b9000, .pa = 0x240b9000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x2480f000, .pa = 0x2480f000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24822000, .pa = 0x24822000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp2", .flags = IOMMU_READ,
		.da = 0x24841000, .pa = 0x24841000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_common", .flags = IOMMU_READ,
		.da = 0x24864000, .pa = 0x24864000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c0f000, .pa = 0x24c0f000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c22000, .pa = 0x24c22000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp2", .flags = IOMMU_READ,
		.da = 0x24c41000, .pa = 0x24c41000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x2680f000, .pa = 0x2680f000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26822000, .pa = 0x26822000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp2", .flags = IOMMU_READ,
		.da = 0x26841000, .pa = 0x26841000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c0f000, .pa = 0x26c0f000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c22000, .pa = 0x26c22000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp2", .flags = IOMMU_READ,
		.da = 0x26c41000, .pa = 0x26c41000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_common", .flags = IOMMU_READ,
		.da = 0x26c64000, .pa = 0x26c64000, .len = 0x00001000, },
	{ .name = "turing_nsp_noc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x320c9000, .pa = 0x320c9000, .len = 0x00003000, },
	{ .name = "qdss_stm", .flags = IOMMU_WRITE,
		.da = 0x37000000, .pa = 0x37000000, .len = 0x01000000, },
	{ .name = "cmd_db", .flags = IOMMU_READ,
		.da = 0x81c60000, .pa = 0x81c60000, .len = 0x00020000, },
	{ .name = "smem", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x81d00000, .pa = 0x81d00000, .len = 0x00200000, },
	{ .name = "global_sync", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x82600000, .pa = 0x82600000, .len = 0x00100000, },
};

DEFINE_RESOURCE_TABLE(TURING, ARRAY_SIZE(turing_mem_res));

/* Clocks set up or core started, and the subsystem not reset since */
static bool turing_needs_reset;

static vaddr_t turing_base(struct qcom_pas_data *data)
{
	return io_pa_or_va(&data->base, data->size);
}

static TEE_Result cdsp_fw_shutdown(struct qcom_pas_data *data)
{
	TEE_Result res = qcom_clock_pas_reset(data->clk_group);

	if (!res)
		turing_needs_reset = false;

	return res;
}

static TEE_Result cdsp_dtb_start(struct qcom_pas_data *data)
{
	vaddr_t base = turing_base(data);
	TEE_Result res = TEE_SUCCESS;

	if (!base)
		return TEE_ERROR_GENERIC;

	turing_needs_reset = true;

	res = q6ss_set_dtb_params(base + TURING_QDSP6SS, data->fw_base,
				  data->fw_size);
	/* Linux sends no shutdown when auth-and-reset fails */
	if (res)
		cdsp_fw_shutdown(data);

	return res;
}

static TEE_Result cdsp_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = turing_base(data);
	TEE_Result res = TEE_SUCCESS;

	if (!base)
		return TEE_ERROR_GENERIC;

	res = qcom_clock_enable(data->clk_group);
	if (!res) {
		q6ss_set_boot_addr(base + TURING_QDSP6SS, data->fw_base);
		res = q6ss_boot(base + TURING_QDSP6SS, false);
	}
	/* The NSP Q6 PLL, core clock and core start follow the boot FSM */
	if (!res)
		res = qcom_clock_enable_pas_processor(data->clk_group);
	if (res) {
		/* Linux sends no shutdown when auth-and-reset fails */
		cdsp_fw_shutdown(data);
		return res;
	}

	return TEE_SUCCESS;
}

/*
 * The core runs this only once the image is down (depends_on). The image
 * shutdown and a failed start reset the subsystem; reset here only if
 * none did since the DTB stage.
 */
static TEE_Result cdsp_dtb_shutdown(struct qcom_pas_data *data)
{
	if (!turing_needs_reset)
		return TEE_SUCCESS;

	return cdsp_fw_shutdown(data);
}

static TEE_Result cdsp_get_resource_table(struct resource_table *rt,
					  size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = TURING_NUM_MEM_RESOURCES,
		.offset[TURING_NUM_MEM_RESOURCES - 1] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header, turing_mem_res,
			   TURING_RESOURCE_TABLE_HEADER_SIZE,
			   TURING_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops cdsp_dtb_ops = {
	.fw_start = cdsp_dtb_start,
	.fw_shutdown = cdsp_dtb_shutdown,
};

const struct qcom_pas_ops cdsp_ops = {
	.fw_start = cdsp_fw_start,
	.fw_shutdown = cdsp_fw_shutdown,
	.get_resource_table = cdsp_get_resource_table,
};
