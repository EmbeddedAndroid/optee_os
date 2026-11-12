/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef SECA_CDSP_MEM_RES_H
#define SECA_CDSP_MEM_RES_H

/*
 * Register windows the SecaAU CDSP0 and CDSP1 firmware use beyond the Nord
 * CDSP0 and CDSP1 resource tables. Pages a Nord entry already maps are
 * left out: an overlapping devmem entry fails the IOMMU mapping in Linux.
 */

#define CDSP0_TARGET_MEM_RES \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00128000, .pa = 0x00128000, .len = 0x00001000, }, \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00147000, .pa = 0x00147000, .len = 0x00001000, }, \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0019f000, .pa = 0x0019f000, .len = 0x00001000, }, \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x001b9000, .pa = 0x001b9000, .len = 0x00004000, }, \
	{ .name = "edp_dp2_phy_0_cm_edp_dp2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00fc1000, .pa = 0x00fc1000, .len = 0x00003000, }, \
	{ .name = "edp_dp2_phy_1_cm_edp_dp2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00fc4000, .pa = 0x00fc4000, .len = 0x00003000, }, \
	{ .name = "edp_dp2_phy_2_cm_edp_dp2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00fc7000, .pa = 0x00fc7000, .len = 0x00003000, }, \
	{ .name = "rng_ee4_ee4_soc_qrng_sub", \
		.flags = IOMMU_READ, \
		.da = 0x010c4000, .pa = 0x010c4000, .len = 0x00001000, }, \
	{ .name = "usb2phy_2_cm_dwc_usb2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088e0000, .pa = 0x088e0000, .len = 0x00001000, }, \
	{ .name = "eud_mode_manager2", \
		.flags = IOMMU_READ, \
		.da = 0x088e9000, .pa = 0x088e9000, .len = 0x00001000, }, \
	{ .name = "usb2phy_0_cm_dwc_usb2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088ea000, .pa = 0x088ea000, .len = 0x00001000, }, \
	{ .name = "usb2phy_1_cm_dwc_usb2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088eb000, .pa = 0x088eb000, .len = 0x00001000, }, \
	{ .name = "ne_gcc_ne_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08959000, .pa = 0x08959000, .len = 0x00001000, }, \
	{ .name = "se_gcc_gpll7_cm_pll_lucid_ole", \
		.flags = IOMMU_READ, \
		.da = 0x08a07000, .pa = 0x08a07000, .len = 0x00001000, }, \
	{ .name = "se_gcc_se_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08a35000, .pa = 0x08a35000, .len = 0x00001000, }, \
	{ .name = "se_gcc_se_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08a42000, .pa = 0x08a42000, .len = 0x00001000, }, \
	{ .name = "se_gcc_se_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08a90000, .pa = 0x08a90000, .len = 0x00001000, }, \
	{ .name = "nw_gcc_nw_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08b56000, .pa = 0x08b56000, .len = 0x00003000, }, \
	{ .name = "nw_gcc_nw_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08b78000, .pa = 0x08b78000, .len = 0x00001000, }, \
	{ .name = "mdss_1_mdss_hw", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08c00000, .pa = 0x08c00000, .len = 0x00001000, }, \
	{ .name = "mdss_1_mdp", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08c01000, .pa = 0x08c01000, .len = 0x00093000, }, \
	{ .name = "mdss_1_dptx_0", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08d54000, .pa = 0x08d54000, .len = 0x00008000, }, \
	{ .name = "mdss_1_dptx_1", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08d5c000, .pa = 0x08d5c000, .len = 0x00008000, }, \
	{ .name = "mdss_1_dptx_2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08d64000, .pa = 0x08d64000, .len = 0x00008000, }, \
	{ .name = "mdss_1_dptx_3", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08d6c000, .pa = 0x08d6c000, .len = 0x00008000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09002000, .pa = 0x09002000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0900b000, .pa = 0x0900b000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x090c0000, .pa = 0x090c0000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x090d0000, .pa = 0x090d0000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09180000, .pa = 0x09180000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09190000, .pa = 0x09190000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09240000, .pa = 0x09240000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0924b000, .pa = 0x0924b000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09300000, .pa = 0x09300000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0931d000, .pa = 0x0931d000, .len = 0x00002000, }, \
	{ .name = "mdss_0_mdss_hw", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0ae00000, .pa = 0x0ae00000, .len = 0x00001000, }, \
	{ .name = "mdss_0_mdp", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0ae01000, .pa = 0x0ae01000, .len = 0x00093000, }, \
	{ .name = "mdss_0_dptx_0", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0af54000, .pa = 0x0af54000, .len = 0x00008000, }, \
	{ .name = "mdss_0_dptx_1", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0af5c000, .pa = 0x0af5c000, .len = 0x00008000, }, \
	{ .name = "mdss_0_dptx_2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0af64000, .pa = 0x0af64000, .len = 0x00008000, }, \
	{ .name = "mdss_0_dptx_3", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0af6c000, .pa = 0x0af6c000, .len = 0x00008000, }, \
	{ .name = "qdss_apb", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x10000000, .pa = 0x10000000, .len = 0x00080000, }, \
	{ .name = "qdss_hpass_qdsp6_0q6v81dgb_1_ap", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x11988000, .pa = 0x11988000, .len = 0x00002000, },

#define CDSP1_TARGET_MEM_RES \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00128000, .pa = 0x00128000, .len = 0x00001000, }, \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x00147000, .pa = 0x00147000, .len = 0x00001000, }, \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x001a0000, .pa = 0x001a0000, .len = 0x00001000, }, \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x001b9000, .pa = 0x001b9000, .len = 0x00004000, }, \
	{ .name = "rng_ee4_ee4_soc_qrng_sub", \
		.flags = IOMMU_READ, \
		.da = 0x010c4000, .pa = 0x010c4000, .len = 0x00001000, }, \
	{ .name = "usb2phy_2_cm_dwc_usb2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088e0000, .pa = 0x088e0000, .len = 0x00001000, }, \
	{ .name = "eud_mode_manager2", \
		.flags = IOMMU_READ, \
		.da = 0x088e9000, .pa = 0x088e9000, .len = 0x00001000, }, \
	{ .name = "usb2phy_0_cm_dwc_usb2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088ea000, .pa = 0x088ea000, .len = 0x00001000, }, \
	{ .name = "usb2phy_1_cm_dwc_usb2", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088eb000, .pa = 0x088eb000, .len = 0x00001000, }, \
	{ .name = "ne_gcc_ne_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0895a000, .pa = 0x0895a000, .len = 0x00001000, }, \
	{ .name = "se_gcc_gpll7_cm_pll_lucid_ole", \
		.flags = IOMMU_READ, \
		.da = 0x08a07000, .pa = 0x08a07000, .len = 0x00001000, }, \
	{ .name = "se_gcc_se_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08a36000, .pa = 0x08a36000, .len = 0x00001000, }, \
	{ .name = "se_gcc_se_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08a42000, .pa = 0x08a42000, .len = 0x00001000, }, \
	{ .name = "se_gcc_se_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08a90000, .pa = 0x08a90000, .len = 0x00001000, }, \
	{ .name = "nw_gcc_nw_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08b56000, .pa = 0x08b56000, .len = 0x00003000, }, \
	{ .name = "nw_gcc_nw_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x08b79000, .pa = 0x08b79000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09002000, .pa = 0x09002000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0900b000, .pa = 0x0900b000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x090c0000, .pa = 0x090c0000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x090d0000, .pa = 0x090d0000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09180000, .pa = 0x09180000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09190000, .pa = 0x09190000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09240000, .pa = 0x09240000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0924b000, .pa = 0x0924b000, .len = 0x00002000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x09300000, .pa = 0x09300000, .len = 0x00001000, }, \
	{ .name = "ipc_soc_ipc_core", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0931d000, .pa = 0x0931d000, .len = 0x00002000, }, \
	{ .name = "qdss_apb", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x10000000, .pa = 0x10000000, .len = 0x00080000, },

#endif /* SECA_CDSP_MEM_RES_H */
