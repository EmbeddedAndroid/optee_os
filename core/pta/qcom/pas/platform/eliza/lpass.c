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

#include "lpass.h"
#include "q6ss.h"

/* Relative to LPASS_BASE */
#define LPASS_QDSP6SS			0x800000
#define LPASS_ALT_RESET_Q6SS		0xeea000
#define LPASS_EFUSE_Q6SS_EVB_SEL	0xeeb000

/*
 * Peripherals the ADSP reaches through its SMMU stream, identity mapped: the
 * SM7750 "ADSP Q6 ELF" stage-2 aperture, plus SMEM and the LLCC LPI region
 * the ADSP island (uImage) runs from.
 */
static const struct fw_rsc_devmem lpass_mem_res[] = {
	{ .name = "gcc_pll", .flags = IOMMU_READ,
		.da = 0x00100000, .pa = 0x00100000, .len = 0x0000a000, },
	{ .name = "gcc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00110000, .pa = 0x00110000, .len = 0x00002000, },
	{ .name = "gcc1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00117000, .pa = 0x00117000, .len = 0x00002000, },
	{ .name = "gcc2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0011e000, .pa = 0x0011e000, .len = 0x00001000, },
	{ .name = "gcc3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00123000, .pa = 0x00123000, .len = 0x00001000, },
	{ .name = "gcc4", .flags = IOMMU_READ,
		.da = 0x00138000, .pa = 0x00138000, .len = 0x00001000, },
	{ .name = "gcc5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00145000, .pa = 0x00145000, .len = 0x00001000, },
	{ .name = "gcc6", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00147000, .pa = 0x00147000, .len = 0x00001000, },
	{ .name = "gcc7", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00155000, .pa = 0x00155000, .len = 0x00001000, },
	{ .name = "gcc8", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00162000, .pa = 0x00162000, .len = 0x00001000, },
	{ .name = "gcc9", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0016b000, .pa = 0x0016b000, .len = 0x00002000, },
	{ .name = "gcc10", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0016f000, .pa = 0x0016f000, .len = 0x00001000, },
	{ .name = "gcc11", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0018e000, .pa = 0x0018e000, .len = 0x00001000, },
	{ .name = "gcc12", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x001ac000, .pa = 0x001ac000, .len = 0x00002000, },
	{ .name = "ipc_core", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00403000, .pa = 0x00403000, .len = 0x00001000, },
	{ .name = "ipc_core1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00441000, .pa = 0x00441000, .len = 0x00001000, },
	{ .name = "ipc_core2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00481000, .pa = 0x00481000, .len = 0x00001000, },
	{ .name = "ipc_core3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x004c1000, .pa = 0x004c1000, .len = 0x00001000, },
	{ .name = "ipc_core4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x00501000, .pa = 0x00501000, .len = 0x00001000, },
	{ .name = "ipc_config", .flags = IOMMU_READ,
		.da = 0x005fc000, .pa = 0x005fc000, .len = 0x00001000, },
	{ .name = "rng_cm_cm_prng_cm", .flags = IOMMU_READ,
		.da = 0x010c0000, .pa = 0x010c0000, .len = 0x00001000, },
	{ .name = "rng_ee4_ee4_prng_sub", .flags = IOMMU_READ,
		.da = 0x010c4000, .pa = 0x010c4000, .len = 0x00001000, },
	{ .name = "pcie0_pcie_parf", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01c00000, .pa = 0x01c00000, .len = 0x00003000, },
	{ .name = "pcie_rscc_rscc_rscc_rsc", .flags = IOMMU_READ,
		.da = 0x01d00000, .pa = 0x01d00000, .len = 0x00002000, },
	{ .name = "pcie_rscc_crm_client", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01d02000, .pa = 0x01d02000, .len = 0x00001000, },
	{ .name = "pcie_rscc_crmb", .flags = IOMMU_READ,
		.da = 0x01d03000, .pa = 0x01d03000, .len = 0x00003000, },
	{ .name = "pcie_rscc_pcie_pcie_sm", .flags = IOMMU_READ,
		.da = 0x01d07000, .pa = 0x01d07000, .len = 0x00007000, },
	{ .name = "crypto0_crypto_bam", .flags = IOMMU_READ,
		.da = 0x01dc4000, .pa = 0x01dc4000, .len = 0x00001000, },
	{ .name = "crypto0_crypto_bam1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01dc5000, .pa = 0x01dc5000, .len = 0x00001000, },
	{ .name = "crypto0_crypto_bam2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01dc8000, .pa = 0x01dc8000, .len = 0x00001000, },
	{ .name = "crypto0_crypto", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01dfa000, .pa = 0x01dfa000, .len = 0x00001000, },
	{ .name = "crypto0_crypto1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01dfd000, .pa = 0x01dfd000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01f40000, .pa = 0x01f40000, .len = 0x00080000, },
	{ .name = "tcsr_tcsr_regs1", .flags = IOMMU_READ,
		.da = 0x01fc0000, .pa = 0x01fc0000, .len = 0x00010000, },
	{ .name = "tcsr_tcsr_regs2", .flags = IOMMU_READ,
		.da = 0x01fd1000, .pa = 0x01fd1000, .len = 0x00004000, },
	{ .name = "tcsr_tcsr_regs3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01fd5000, .pa = 0x01fd5000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs4", .flags = IOMMU_READ,
		.da = 0x01fd6000, .pa = 0x01fd6000, .len = 0x00013000, },
	{ .name = "tcsr_tcsr_regs5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x01fe9000, .pa = 0x01fe9000, .len = 0x00001000, },
	{ .name = "tcsr_tcsr_regs6", .flags = IOMMU_READ,
		.da = 0x01fea000, .pa = 0x01fea000, .len = 0x00006000, },
	{ .name = "lpass_audio_lpm", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x06c91000, .pa = 0x06c91000, .len = 0x00001000, },
	{ .name = "lpass_lpicx_noc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x07437000, .pa = 0x07437000, .len = 0x00007000, },
	{ .name = "lpass_ag_noc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x07e4d000, .pa = 0x07e4d000, .len = 0x00002000, },
	{ .name = "eud_eud", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x088e1000, .pa = 0x088e1000, .len = 0x00003000, },
	{ .name = "usb3_pri_dwc_usb3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0a600000, .pa = 0x0a600000, .len = 0x00003000, },
	{ .name = "mdss", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0ae00000, .pa = 0x0ae00000, .len = 0x000c0000, },
	{ .name = "mdss1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0af55000, .pa = 0x0af55000, .len = 0x00001000, },
	{ .name = "rpmh_pdc_audio_audio_pdc", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0b250000, .pa = 0x0b250000, .len = 0x00010000, },
	{ .name = "rpmh_pdc_grp1_grp1_pdc_s", .flags = IOMMU_READ,
		.da = 0x0b5f5000, .pa = 0x0b5f5000, .len = 0x00001000, },
	{ .name = "rpmh_bcm_bcm_status", .flags = IOMMU_READ,
		.da = 0x0bbf0000, .pa = 0x0bbf0000, .len = 0x00002000, },
	{ .name = "aoss_peripherals", .flags = IOMMU_READ,
		.da = 0x0c210000, .pa = 0x0c210000, .len = 0x00080000, },
	{ .name = "aop_ss_msg_ram_drv4", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c340000, .pa = 0x0c340000, .len = 0x00001000, },
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
	{ .name = "aop_ss_msg_ram_drv14", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c3e0000, .pa = 0x0c3e0000, .len = 0x00001000, },
	{ .name = "aop_ss_msg_ram_drv15", .flags = IOMMU_READ,
		.da = 0x0c3f0000, .pa = 0x0c3f0000, .len = 0x00001000, },
	{ .name = "pmic_arb_core", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c400000, .pa = 0x0c400000, .len = 0x00003000, },
	{ .name = "pmic_arb_mgpi", .flags = IOMMU_READ,
		.da = 0x0c406000, .pa = 0x0c406000, .len = 0x00011000, },
	{ .name = "spmi0_spmi_cfg_top", .flags = IOMMU_READ,
		.da = 0x0c42c000, .pa = 0x0c42c000, .len = 0x0000a000, },
	{ .name = "pmic_arb_core_registers_", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0c440000, .pa = 0x0c440000, .len = 0x00080000, },
	{ .name = "spmi0_spmi_pic_owner", .flags = IOMMU_READ,
		.da = 0x0c4c0000, .pa = 0x0c4c0000, .len = 0x00020000, },
	{ .name = "tlmm", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x0f000000, .pa = 0x0f000000, .len = 0x01000000, },
	{ .name = "qdss_csr", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10001000, .pa = 0x10001000, .len = 0x00002000, },
	{ .name = "qdss_in_fun0_in_fun0_cxa", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10041000, .pa = 0x10041000, .len = 0x00001000, },
	{ .name = "qdss_repl64_repl64_cxatb", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10046000, .pa = 0x10046000, .len = 0x00001000, },
	{ .name = "qdss_etr1_etr1_cxtmc_r64", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x1004f000, .pa = 0x1004f000, .len = 0x00001000, },
	{ .name = "qdss_aoss_apb_fun0_fun0_", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10b04000, .pa = 0x10b04000, .len = 0x00002000, },
	{ .name = "qdss_aoss_apb_swaocsr_sw", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10b11000, .pa = 0x10b11000, .len = 0x00001000, },
	{ .name = "qdss_ddrss_lpi_ddrss_lpi", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10b30000, .pa = 0x10b30000, .len = 0x00006000, },
	{ .name = "qdss_lpass_lpi_lpass_lpi", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x10b40000, .pa = 0x10b40000, .len = 0x00020000, },
	{ .name = "system_imem", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x14680000, .pa = 0x14680000, .len = 0x00001000, },
	{ .name = "tme_cpulayer_csr", .flags = IOMMU_READ,
		.da = 0x20c21000, .pa = 0x20c21000, .len = 0x00001000, },
	{ .name = "qfprom_corr", .flags = IOMMU_READ,
		.da = 0x221c2000, .pa = 0x221c2000, .len = 0x00002000, },
	{ .name = "fuse_controller_sw_range", .flags = IOMMU_READ,
		.da = 0x221c8000, .pa = 0x221c8000, .len = 0x00001000, },
	{ .name = "shrm2_rvss_periph_shrm2_", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24077000, .pa = 0x24077000, .len = 0x00001000, },
	{ .name = "ddr_reg_ddr_ss_regs", .flags = IOMMU_READ,
		.da = 0x24080000, .pa = 0x24080000, .len = 0x00001000, },
	{ .name = "lpicc_lpicc_reg", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24321000, .pa = 0x24321000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24803000, .pa = 0x24803000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x2480c000, .pa = 0x2480c000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24816000, .pa = 0x24816000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24823000, .pa = 0x24823000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp4", .flags = IOMMU_READ,
		.da = 0x24841000, .pa = 0x24841000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_trp5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24849000, .pa = 0x24849000, .len = 0x00001000, },
	{ .name = "mach9_0_llcc_common", .flags = IOMMU_READ,
		.da = 0x24864000, .pa = 0x24864000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c03000, .pa = 0x24c03000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c0c000, .pa = 0x24c0c000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c16000, .pa = 0x24c16000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c23000, .pa = 0x24c23000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp4", .flags = IOMMU_READ,
		.da = 0x24c41000, .pa = 0x24c41000, .len = 0x00001000, },
	{ .name = "mach9_2_llcc_trp5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x24c49000, .pa = 0x24c49000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26803000, .pa = 0x26803000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x2680c000, .pa = 0x2680c000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26816000, .pa = 0x26816000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26823000, .pa = 0x26823000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp4", .flags = IOMMU_READ,
		.da = 0x26841000, .pa = 0x26841000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp5", .flags = IOMMU_READ,
		.da = 0x26845000, .pa = 0x26845000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_trp6", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26849000, .pa = 0x26849000, .len = 0x00001000, },
	{ .name = "llcc_or_bcast_lcp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x268e6000, .pa = 0x268e6000, .len = 0x00002000, },
	{ .name = "llcc_and_bcast_trp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c03000, .pa = 0x26c03000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c0c000, .pa = 0x26c0c000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp2", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c16000, .pa = 0x26c16000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp3", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c23000, .pa = 0x26c23000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp4", .flags = IOMMU_READ,
		.da = 0x26c41000, .pa = 0x26c41000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_trp5", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26c49000, .pa = 0x26c49000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_common", .flags = IOMMU_READ,
		.da = 0x26c64000, .pa = 0x26c64000, .len = 0x00001000, },
	{ .name = "llcc_and_bcast_lcp", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x26ce6000, .pa = 0x26ce6000, .len = 0x00002000, },
	{ .name = "qdss_stm", .flags = IOMMU_WRITE,
		.da = 0x37000000, .pa = 0x37000000, .len = 0x01000000, },
	{ .name = "pcie_gen3x1_0_axi", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x40000000, .pa = 0x40000000, .len = 0x00001000, },
	{ .name = "pcie_gen3x1_0_axi1", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x40002000, .pa = 0x40002000, .len = 0x03ffe000, },
	{ .name = "cmd_db", .flags = IOMMU_READ,
		.da = 0x81c60000, .pa = 0x81c60000, .len = 0x00020000, },
	{ .name = "smem", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x81d00000, .pa = 0x81d00000, .len = 0x00200000, },
	{ .name = "adsp_mhi", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0x82380000, .pa = 0x82380000, .len = 0x00020000, },
	{ .name = "llcc_lpi", .flags = IOMMU_READ | IOMMU_WRITE,
		.da = 0xff800000, .pa = 0xff800000, .len = 0x00180000, },
};

DEFINE_RESOURCE_TABLE(LPASS, ARRAY_SIZE(lpass_mem_res));

/* Clocks set up or core started, and the subsystem not reset since */
static bool lpass_needs_reset;

static vaddr_t lpass_base(struct qcom_pas_data *data)
{
	return io_pa_or_va(&data->base, data->size);
}

static TEE_Result lpass_fw_shutdown(struct qcom_pas_data *data)
{
	TEE_Result res = qcom_clock_pas_reset(data->clk_group);

	if (!res)
		lpass_needs_reset = false;

	return res;
}

static TEE_Result lpass_dtb_start(struct qcom_pas_data *data)
{
	vaddr_t base = lpass_base(data);
	TEE_Result res = TEE_SUCCESS;

	if (!base)
		return TEE_ERROR_GENERIC;

	lpass_needs_reset = true;

	res = q6ss_set_dtb_params(base + LPASS_QDSP6SS, data->fw_base,
				  data->fw_size);
	/* Linux sends no shutdown when auth-and-reset fails */
	if (res)
		lpass_fw_shutdown(data);

	return res;
}

static TEE_Result lpass_fw_start(struct qcom_pas_data *data)
{
	vaddr_t base = lpass_base(data);
	TEE_Result res = TEE_SUCCESS;

	if (!base)
		return TEE_ERROR_GENERIC;

	res = qcom_clock_enable(data->clk_group);
	if (!res) {
		/* Boot from RST_EVB, not from the fused EVB */
		q6ss_set_boot_addr(base + LPASS_QDSP6SS, data->fw_base);
		io_clrbits32(base + LPASS_EFUSE_Q6SS_EVB_SEL, BIT(0));
		io_clrbits32(base + LPASS_ALT_RESET_Q6SS, BIT(0));
		dsb();
		res = q6ss_boot(base + LPASS_QDSP6SS, true);
	}
	if (res) {
		/* Linux sends no shutdown when auth-and-reset fails */
		lpass_fw_shutdown(data);
		return res;
	}

	return TEE_SUCCESS;
}

/*
 * The core runs this only once the image is down (depends_on). The image
 * shutdown and a failed start reset the subsystem; reset here only if
 * none did since the DTB stage.
 */
static TEE_Result lpass_dtb_shutdown(struct qcom_pas_data *data)
{
	if (!lpass_needs_reset)
		return TEE_SUCCESS;

	return lpass_fw_shutdown(data);
}

static TEE_Result lpass_get_resource_table(struct resource_table *rt,
					   size_t *rt_size)
{
	const struct fw_rsc_hdr header = {
		.type = RSC_DEVMEM,
	};
	static struct resource_table table = {
		.ver = 1,
		.num = LPASS_NUM_MEM_RESOURCES,
		.offset[LPASS_NUM_MEM_RESOURCES - 1] = 0,
	};

	return get_mem_rsc(rt, rt_size, &table, &header, lpass_mem_res,
			   LPASS_RESOURCE_TABLE_HEADER_SIZE,
			   LPASS_RESOURCE_TABLE_SIZE);
}

const struct qcom_pas_ops lpass_dtb_ops = {
	.fw_start = lpass_dtb_start,
	.fw_shutdown = lpass_dtb_shutdown,
};

const struct qcom_pas_ops lpass_ops = {
	.fw_start = lpass_fw_start,
	.fw_shutdown = lpass_fw_shutdown,
	.get_resource_table = lpass_get_resource_table,
};
