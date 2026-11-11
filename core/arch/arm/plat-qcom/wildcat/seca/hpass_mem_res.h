/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef SECA_HPASS_MEM_RES_H
#define SECA_HPASS_MEM_RES_H

/*
 * Register windows the SecaAU ADSP0 and ADSP1 firmware use beyond the Nord
 * HPASS0 and HPASS1 resource tables. Pages a Nord entry already maps are
 * left out: an overlapping devmem entry fails the IOMMU mapping in Linux.
 */

#define HPASS0_TARGET_MEM_RES \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0019f000, .pa = 0x0019f000, .len = 0x00001000, }, \
	{ .name = "usb3_dp_phy_0_cm_usb3_dp_con", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088e1000, .pa = 0x088e1000, .len = 0x00003000, }, \
	{ .name = "usb3_dp_phy_1_cm_usb3_dp_con", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088e4000, .pa = 0x088e4000, .len = 0x00003000, }, \
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
	{ .name = "usb20s", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0a200000, .pa = 0x0a200000, .len = 0x00200000, }, \
	{ .name = "usb3_1_usb31_gen2_subsystem", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0a40e000, .pa = 0x0a40e000, .len = 0x001f2000, }, \
	{ .name = "usb3_0_usb31_gen2_subsystem", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0a60e000, .pa = 0x0a60e000, .len = 0x001f2000, }, \
	{ .name = "tlmm_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0f1dd000, .pa = 0x0f1dd000, .len = 0x00003000, }, \
	{ .name = "tlmm_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0f1f8000, .pa = 0x0f1f8000, .len = 0x00001000, },

#define HPASS1_TARGET_MEM_RES \
	{ .name = "gcc_clk_ctl_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x001a0000, .pa = 0x001a0000, .len = 0x00001000, }, \
	{ .name = "usb3_dp_phy_0_cm_usb3_dp_con", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088e1000, .pa = 0x088e1000, .len = 0x00003000, }, \
	{ .name = "usb3_dp_phy_1_cm_usb3_dp_con", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x088e4000, .pa = 0x088e4000, .len = 0x00003000, }, \
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
	{ .name = "usb20s", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0a200000, .pa = 0x0a200000, .len = 0x00200000, }, \
	{ .name = "usb3_1_usb31_gen2_subsystem", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0a40e000, .pa = 0x0a40e000, .len = 0x001f2000, }, \
	{ .name = "usb3_0_usb31_gen2_subsystem", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0a60e000, .pa = 0x0a60e000, .len = 0x001f2000, }, \
	{ .name = "tlmm_reg", \
		.flags = IOMMU_READ | IOMMU_WRITE, \
		.da = 0x0f1f9000, .pa = 0x0f1f9000, .len = 0x00001000, },

#endif /* SECA_HPASS_MEM_RES_H */
