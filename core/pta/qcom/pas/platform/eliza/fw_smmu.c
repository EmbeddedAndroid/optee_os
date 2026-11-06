// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <mm/core_mmu.h>
#include <pas_fw_smmu.h>
#include <pta_qcom_pas.h>
#include <stdint.h>
#include <util.h>

#define APPS_SMMU_BASE		0x15000000

register_phys_mem(MEM_AREA_IO_NSEC, APPS_SMMU_BASE, 0x100000);

/* Carveouts as reserved by the SM7750 device tree */
static const struct pas_fw_smmu_stream apps_streams[] = {
#ifdef CFG_QCOM_PAS_ADSP
	{
		.pas_id = PAS_ID_QDSP6,
		.smr = { 0x1000, 0x100c },
		.carveouts = {
			{ .pa = 0x99e00000, .size = 0x2a00000 },
			{ .pa = 0x99d80000, .size = 0x80000 },
		},
		.dtb_pas_id = PAS_ID_QDSP6_DTB,
		.fw_iova = UINT64_MAX,
		.use_rsc = true,
		.pool_pages = 64,
	},
#endif
#ifdef CFG_QCOM_PAS_CDSP
	{
		.pas_id = PAS_ID_TURING,
		.smr = { 0x0c00 },
		.carveouts = {
			{ .pa = 0x98900000, .size = 0x1400000 },
			{ .pa = 0x99d00000, .size = 0x80000 },
		},
		.dtb_pas_id = PAS_ID_TURING_DTB,
		.fw_iova = UINT64_MAX,
		.use_rsc = true,
		.pool_pages = 48,
	},
#endif
#ifdef CFG_QCOM_PAS_IRIS
	/* The iris core runs its firmware from IOVA 0 */
	{
		.pas_id = PAS_ID_IRIS,
		.smr = { 0x1942 },
		.carveouts = {
			{ .pa = 0x95f00000, .size = 0x800000 },
		},
		.fw_iova = 0,
		.keep_after_stop = true,
		.pool_pages = 8,
	},
#endif
};

/* The top SMRs, context banks and VMIDs of apps_smmu (192 SMRs, 112 CBs) */
static const struct pas_fw_smmu_instance instances[] = {
	{
		.base = APPS_SMMU_BASE,
		.smr_first = 188,
		.smr_count = 4,
		.cb_first = 112 - ARRAY_SIZE(apps_streams),
		.vmid_first = 0xf0,
		.streams = apps_streams,
		.nr_streams = ARRAY_SIZE(apps_streams),
	},
};

const struct pas_fw_smmu_instance *pas_fw_smmu_platform(size_t *count)
{
	*count = ARRAY_SIZE(instances);
	return instances;
}
