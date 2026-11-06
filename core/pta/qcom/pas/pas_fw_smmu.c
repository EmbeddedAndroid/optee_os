// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <initcall.h>
#include <io.h>
#include <kernel/cache_helpers.h>
#include <kernel/delay.h>
#include <kernel/thread_spmc.h>
#include <malloc.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <mm/page_alloc.h>
#include <optee_ffa.h>
#include <string.h>
#include <trace.h>
#include <util.h>

#include "pas_fw_smmu.h"
#include "pas_subsys.h"

#define SMMU_SIZE			0x100000

#define GR0_SCR0			0x0
#define  SCR0_CLIENTPD			BIT32(0)
#define GR0_IDR0			0x20
#define  IDR0_SES			BIT32(31)
#define  IDR0_NUMSMRG			GENMASK_32(7, 0)
#define GR0_IDR1			0x24
#define  IDR1_PAGESIZE			BIT32(31)
#define  IDR1_NUMPAGENDXB_SHIFT		28
#define  IDR1_NUMPAGENDXB_MASK		0x7
#define  IDR1_NUMCB			GENMASK_32(7, 0)
#define GR0_TLBIVMID			0x64
#define GR0_TLBGSYNC			0x70
#define GR0_TLBGSTATUS			0x74
#define  TLBGSTATUS_GSACTIVE		BIT32(0)
#define GR0_SMR(n)			(0x800 + (n) * 4)
#define  SMR_VALID			BIT32(31)
#define GR0_S2CR(n)			(0xc00 + (n) * 4)
#define  S2CR_TYPE_FAULT		SHIFT_U32(2, 16)
#define GR1_CBAR(n)			((n) * 4)
#define GR1_CBA2R(n)			(0x800 + (n) * 4)
#define  CBA2R_VA64			BIT32(0)
#define  CBA2R_VMID16_SHIFT		16
#define CB_SCTLR			0x0
#define  CB_SCTLR_M			BIT32(0)
#define  CB_SCTLR_TRE			BIT32(1)
#define  CB_SCTLR_AFE			BIT32(2)
#define  CB_SCTLR_CFRE			BIT32(5)
#define CB_TTBR0			0x20
#define CB_TCR				0x30
#define CB_FSR				0x58

/* VTCR format: 36-bit IPA and PA, 4 KiB granule, walk from level 1 */
#define S2_VTCR		(BIT32(31) | SHIFT_U32(1, 16) | SHIFT_U32(3, 12) | \
			 SHIFT_U32(1, 10) | SHIFT_U32(1, 8) | SHIFT_U32(1, 6) | 28)
#define S2_IPA_BITS	36

/* Stage 2 descriptors, MemAttr in the encoding without FWB */
#define S2_TABLE	3ULL
#define S2_BLOCK	1ULL
#define S2_PAGE		3ULL
#define S2_AF		BIT64(10)
#define S2_SH_IS	SHIFT_U64(3, 8)
#define S2_AP_R		BIT64(6)
#define S2_AP_W		BIT64(7)
#define S2_NORMAL_WB	SHIFT_U64(0xf, 2)
#define S2_DEVICE_NGNRE	SHIFT_U64(0x1, 2)
#define S2_OA_MASK	GENMASK_64(S2_IPA_BITS - 1, 12)

/* Peripherals below 2 GiB, DDR above */
#define DEVICE_LIMIT	0x80000000UL

#define TLB_SYNC_US	100000

struct bank {
	vaddr_t pool;
	paddr_t pool_pa;
	unsigned int pool_pages;
	unsigned int used;
	unsigned int smr0;
	unsigned int nr_smr;
	bool live;
};

struct inst {
	vaddr_t gr0;
	vaddr_t gr1;
	vaddr_t cb0;
	bool ok;
	struct bank *banks;
};

static const struct pas_fw_smmu_instance *instances;
static size_t nr_instances;
static struct inst *insts;

static vaddr_t cb_va(struct inst *in, unsigned int cb)
{
	return in->cb0 + cb * SMALL_PAGE_SIZE;
}

static bool find_stream(uint32_t pas_id, size_t *ii, size_t *si)
{
	size_t i = 0;
	size_t j = 0;

	for (i = 0; i < nr_instances; i++)
		for (j = 0; j < instances[i].nr_streams; j++)
			if (instances[i].streams[j].pas_id == pas_id) {
				*ii = i;
				*si = j;
				return true;
			}

	return false;
}

bool pas_fw_smmu_owns(uint32_t pas_id)
{
	size_t i = 0;
	size_t j = 0;

	return find_stream(pas_id, &i, &j);
}

static uint64_t *table_alloc(struct bank *b, paddr_t *pa)
{
	vaddr_t va = 0;

	if (b->used == b->pool_pages)
		return NULL;

	va = b->pool + b->used * SMALL_PAGE_SIZE;
	*pa = b->pool_pa + b->used * SMALL_PAGE_SIZE;
	b->used++;
	memset((void *)va, 0, SMALL_PAGE_SIZE);

	return (uint64_t *)va;
}

static uint64_t *table_next(struct bank *b, uint64_t *tbl, size_t idx)
{
	paddr_t pa = 0;
	uint64_t *next = NULL;

	if (tbl[idx]) {
		if ((tbl[idx] & 3) != S2_TABLE)
			return NULL;
		pa = tbl[idx] & S2_OA_MASK;
		return (uint64_t *)(b->pool + (pa - b->pool_pa));
	}

	next = table_alloc(b, &pa);
	if (next)
		tbl[idx] = pa | S2_TABLE;

	return next;
}

static TEE_Result s2_map(struct bank *b, uint64_t iova, paddr_t pa,
			 size_t size, uint64_t attr)
{
	uint64_t *l1 = (uint64_t *)b->pool;
	uint64_t *l2 = NULL;
	uint64_t *l3 = NULL;
	uint64_t end = iova + size;

	if ((iova | pa | size) & SMALL_PAGE_MASK || !size ||
	    end > BIT64(S2_IPA_BITS) || pa + size > BIT64(S2_IPA_BITS))
		return TEE_ERROR_BAD_PARAMETERS;

	while (iova < end) {
		l2 = table_next(b, l1, iova >> 30);
		if (!l2)
			return TEE_ERROR_OUT_OF_MEMORY;

		if (!((iova | pa) & (CORE_MMU_PGDIR_SIZE - 1)) &&
		    end - iova >= CORE_MMU_PGDIR_SIZE &&
		    !l2[(iova >> 21) & 511]) {
			l2[(iova >> 21) & 511] = pa | attr | S2_BLOCK;
			iova += CORE_MMU_PGDIR_SIZE;
			pa += CORE_MMU_PGDIR_SIZE;
			continue;
		}

		l3 = table_next(b, l2, (iova >> 21) & 511);
		if (!l3)
			return TEE_ERROR_OUT_OF_MEMORY;
		if (l3[(iova >> 12) & 511])
			return TEE_ERROR_BAD_PARAMETERS;
		l3[(iova >> 12) & 511] = pa | attr | S2_PAGE;
		iova += SMALL_PAGE_SIZE;
		pa += SMALL_PAGE_SIZE;
	}

	return TEE_SUCCESS;
}

static bool in_carveout(const struct pas_fw_smmu_stream *s, paddr_t pa,
			size_t size)
{
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(s->carveouts) && s->carveouts[i].size;
	     i++)
		if (pa >= s->carveouts[i].pa && size &&
		    pa + size <= s->carveouts[i].pa + s->carveouts[i].size)
			return true;

	return false;
}

static TEE_Result check_fw(const struct pas_fw_smmu_stream *s, uint32_t id)
{
	paddr_t base = 0;
	size_t size = 0;
	TEE_Result res = qcom_pas_get_fw(id, &base, &size);

	/* The core image is not marked loaded before its own start */
	if (res == TEE_ERROR_BAD_STATE && id == s->pas_id) {
		struct qcom_pas_subsys *ss = qcom_pas_lookup(id);

		if (!ss)
			return TEE_ERROR_NOT_SUPPORTED;
		base = ss->data.fw_base;
		size = ss->data.fw_size;
		res = TEE_SUCCESS;
	}
	if (res)
		return res;

	if (!in_carveout(s, base, size)) {
		EMSG("fw-smmu: PAS %#"PRIx32" firmware %#"PRIxPA" + %#zx outside its carveouts",
		     id, base, size);
		return TEE_ERROR_ACCESS_DENIED;
	}

	return TEE_SUCCESS;
}

static TEE_Result map_rsc(struct bank *b, uint32_t pas_id)
{
	struct qcom_pas_subsys *ss = qcom_pas_lookup(pas_id);
	struct resource_table *rt = NULL;
	TEE_Result res = TEE_SUCCESS;
	size_t size = 0;
	size_t i = 0;

	if (!ss || !ss->ops->get_resource_table)
		return TEE_ERROR_NOT_SUPPORTED;

	res = ss->ops->get_resource_table(NULL, &size);
	if (res)
		return res;
	rt = malloc(size);
	if (!rt)
		return TEE_ERROR_OUT_OF_MEMORY;
	res = ss->ops->get_resource_table(rt, &size);

	for (i = 0; !res && i < rt->num; i++) {
		struct fw_rsc_hdr *h = (void *)((uint8_t *)rt + rt->offset[i]);
		struct fw_rsc_devmem *d = (void *)h->data;
		uint64_t attr = S2_AF;

		if (h->type != RSC_DEVMEM)
			continue;
		if (d->da != d->pa) {
			res = TEE_ERROR_BAD_PARAMETERS;
			break;
		}
		if (d->flags & IOMMU_READ)
			attr |= S2_AP_R;
		if (d->flags & IOMMU_WRITE)
			attr |= S2_AP_W;
		if (d->pa < DEVICE_LIMIT)
			attr |= S2_DEVICE_NGNRE;
		else
			attr |= S2_NORMAL_WB | S2_SH_IS;
		res = s2_map(b, d->da, d->pa, d->len, attr);
		if (res)
			EMSG("fw-smmu: window %s %#"PRIx32" + %#"PRIx32": %#"PRIx32,
			     d->name, d->pa, d->len, res);
	}

	free(rt);
	return res;
}

static TEE_Result build_s2(const struct pas_fw_smmu_stream *s, struct bank *b)
{
	const uint64_t mem = S2_AF | S2_AP_R | S2_AP_W | S2_NORMAL_WB |
			     S2_SH_IS;
	paddr_t pa = 0;
	TEE_Result res = TEE_SUCCESS;
	size_t i = 0;

	b->used = 0;
	if (!table_alloc(b, &pa))
		return TEE_ERROR_OUT_OF_MEMORY;

	res = check_fw(s, s->pas_id);
	if (!res && s->dtb_pas_id)
		res = check_fw(s, s->dtb_pas_id);
	if (res)
		return res;

	if (s->fw_iova != UINT64_MAX)
		return s2_map(b, s->fw_iova, s->carveouts[0].pa,
			      s->carveouts[0].size, mem);

	for (i = 0; !res && i < ARRAY_SIZE(s->carveouts) &&
	     s->carveouts[i].size; i++)
		res = s2_map(b, s->carveouts[i].pa, s->carveouts[i].pa,
			     s->carveouts[i].size, mem);

	if (!res && s->use_rsc)
		res = map_rsc(b, s->pas_id);

	return res;
}

static TEE_Result tlb_inv_vmid(struct inst *in, uint32_t vmid)
{
	uint64_t t = timeout_init_us(TLB_SYNC_US);

	io_write32(in->gr0 + GR0_TLBIVMID, vmid);
	io_write32(in->gr0 + GR0_TLBGSYNC, 0);
	while (io_read32(in->gr0 + GR0_TLBGSTATUS) & TLBGSTATUS_GSACTIVE)
		if (timeout_elapsed(t))
			return TEE_ERROR_BUSY;

	return TEE_SUCCESS;
}

/*
 * Without a hypervisor fence the normal world can rewrite the SMRs and S2CRs
 * of a live bank: report whether they still hold what was programmed.
 */
static void bank_disable(const struct pas_fw_smmu_instance *pi,
			 struct inst *in, size_t si)
{
	const struct pas_fw_smmu_stream *s = pi->streams + si;
	struct bank *b = &in->banks[si];
	unsigned int cb = pi->cb_first + si;
	vaddr_t cbv = cb_va(in, cb);
	unsigned int intact = 0;
	unsigned int smr = 0;
	unsigned int n = 0;
	uint32_t smr_val = 0;
	uint32_t s2cr = 0;

	for (n = 0; n < b->nr_smr; n++) {
		smr = b->smr0 + n;
		smr_val = io_read32(in->gr0 + GR0_SMR(smr));
		s2cr = io_read32(in->gr0 + GR0_S2CR(smr));
		if (smr_val == (SMR_VALID | s->smr[n]) && s2cr == cb)
			intact++;
		else
			EMSG("fw-smmu: PAS %#"PRIx32": SMR %u %#"PRIx32" S2CR %#"PRIx32" changed",
			     s->pas_id, smr, smr_val, s2cr);
		io_write32(in->gr0 + GR0_SMR(smr), 0);
		io_write32(in->gr0 + GR0_S2CR(smr), S2CR_TYPE_FAULT);
	}
	io_write32(cbv + CB_SCTLR, 0);
	if (tlb_inv_vmid(in, pi->vmid_first + si))
		EMSG("fw-smmu: CB %u TLB sync timed out", cb);
	b->live = false;
	IMSG("fw-smmu: PAS %#"PRIx32": CB %u disabled, %u/%u SMRs as programmed",
	     s->pas_id, cb, intact, b->nr_smr);
}

TEE_Result pas_fw_smmu_start(uint32_t pas_id)
{
	const struct pas_fw_smmu_instance *pi = NULL;
	const struct pas_fw_smmu_stream *s = NULL;
	struct inst *in = NULL;
	struct bank *b = NULL;
	TEE_Result res = TEE_SUCCESS;
	unsigned int cb = 0;
	unsigned int n = 0;
	size_t ii = 0;
	size_t si = 0;
	vaddr_t cbv = 0;

	if (!find_stream(pas_id, &ii, &si))
		return TEE_SUCCESS;

	pi = instances + ii;
	in = insts + ii;
	s = pi->streams + si;
	b = &in->banks[si];
	cb = pi->cb_first + si;
	cbv = cb_va(in, cb);

	if (!in->ok)
		return TEE_ERROR_BAD_STATE;
	if (io_read32(in->gr0 + GR0_SCR0) & SCR0_CLIENTPD) {
		EMSG("fw-smmu: SMMU %#"PRIxPA" in global bypass", pi->base);
		return TEE_ERROR_BAD_STATE;
	}
	if (b->live)
		bank_disable(pi, in, si);

	res = build_s2(s, b);
	if (res) {
		EMSG("fw-smmu: PAS %#"PRIx32" stage 2: %#"PRIx32, pas_id, res);
		return res;
	}
	cache_op_inner(DCACHE_AREA_CLEAN, (void *)b->pool,
		       b->used * SMALL_PAGE_SIZE);

	io_write32(cbv + CB_SCTLR, 0);
	/* The VMID lives in CBA2R when the host sets sCR0.VMID16EN */
	io_write32(in->gr1 + GR1_CBA2R(cb),
		   CBA2R_VA64 |
		   SHIFT_U32(pi->vmid_first + si, CBA2R_VMID16_SHIFT));
	/* CBAR.TYPE 0: stage 2 only */
	io_write32(in->gr1 + GR1_CBAR(cb), pi->vmid_first + si);
	io_write64(cbv + CB_TTBR0, b->pool_pa);
	io_write32(cbv + CB_TCR, S2_VTCR);
	io_write32(cbv + CB_FSR, UINT32_MAX);
	res = tlb_inv_vmid(in, pi->vmid_first + si);
	if (res)
		return res;
	io_write32(cbv + CB_SCTLR, CB_SCTLR_CFRE | CB_SCTLR_AFE |
		   CB_SCTLR_TRE | CB_SCTLR_M);

	for (n = 0; n < b->nr_smr; n++) {
		io_write32(in->gr0 + GR0_S2CR(b->smr0 + n), cb);
		io_write32(in->gr0 + GR0_SMR(b->smr0 + n),
			   SMR_VALID | s->smr[n]);
	}
	b->live = true;

	IMSG("fw-smmu: PAS %#"PRIx32": SMR %u..%u -> CB %u VMID %#"PRIx32", %u table pages",
	     pas_id, b->smr0, b->smr0 + b->nr_smr - 1, cb,
	     (uint32_t)(pi->vmid_first + si), b->used);

	return TEE_SUCCESS;
}

static void stop_bank(uint32_t pas_id, bool force)
{
	const struct pas_fw_smmu_instance *pi = NULL;
	struct inst *in = NULL;
	size_t ii = 0;
	size_t si = 0;

	if (!find_stream(pas_id, &ii, &si))
		return;

	pi = instances + ii;
	in = insts + ii;
	if (!in->ok || !in->banks[si].live ||
	    (pi->streams[si].keep_after_stop && !force))
		return;

	bank_disable(pi, in, si);
}

void pas_fw_smmu_stop(uint32_t pas_id)
{
	stop_bank(pas_id, false);
}

void pas_fw_smmu_abort(uint32_t pas_id)
{
	stop_bank(pas_id, true);
}

/*
 * OPTEE_FFA_FW_SMMU_INFO, entry 0: base and SMR/CB slice, entry 1: VMIDs
 * and stream count, entry 2 + k: stream k (one SMR each).
 */
TEE_Result optee_ffa_fw_smmu_info(uint32_t version, uint32_t smmu,
				  uint32_t entry, uint32_t out[4])
{
	const struct pas_fw_smmu_instance *pi = NULL;
	size_t nr = 0;
	size_t i = 0;
	size_t k = 0;
	size_t j = 0;

	if (version != OPTEE_FFA_FW_SMMU_INFO_V1)
		return TEE_ERROR_NOT_SUPPORTED;
	if (smmu >= nr_instances || !insts[smmu].ok)
		return TEE_ERROR_ITEM_NOT_FOUND;

	pi = instances + smmu;
	for (i = 0; i < pi->nr_streams; i++)
		for (j = 0; j < ARRAY_SIZE(pi->streams[i].smr) &&
		     pi->streams[i].smr[j]; j++)
			nr++;

	memset(out, 0, 4 * sizeof(uint32_t));
	if (entry == 0) {
		out[0] = pi->base;
		out[1] = (uint64_t)pi->base >> 32;
		out[2] = pi->smr_first | SHIFT_U32(pi->smr_count, 16);
		out[3] = pi->cb_first | SHIFT_U32(pi->nr_streams, 16);
		return TEE_SUCCESS;
	}
	if (entry == 1) {
		out[0] = pi->vmid_first | SHIFT_U32(pi->nr_streams, 16);
		out[1] = nr;
		return TEE_SUCCESS;
	}

	k = entry - 2;
	for (i = 0; i < pi->nr_streams; i++)
		for (j = 0; j < ARRAY_SIZE(pi->streams[i].smr) &&
		     pi->streams[i].smr[j]; j++, k--)
			if (!k) {
				out[0] = pi->streams[i].smr[j];
				out[1] = pi->cb_first + i;
				out[2] = pi->streams[i].pas_id;
				return TEE_SUCCESS;
			}

	return TEE_ERROR_ITEM_NOT_FOUND;
}

static TEE_Result init_instance(const struct pas_fw_smmu_instance *pi,
				struct inst *in)
{
	unsigned int smr = pi->smr_first;
	uint32_t idr0 = 0;
	uint32_t idr1 = 0;
	unsigned int numpage = 0;
	size_t i = 0;
	vaddr_t base = 0;

	base = (vaddr_t)phys_to_virt(pi->base, MEM_AREA_IO_NSEC, SMMU_SIZE);
	if (!base)
		return TEE_ERROR_GENERIC;

	idr0 = io_read32(base + GR0_IDR0);
	idr1 = io_read32(base + GR0_IDR1);
	IMSG("fw-smmu: SMMU %#"PRIxPA" IDR0 %#"PRIx32" (SES %u) IDR1 %#"PRIx32,
	     pi->base, idr0, !!(idr0 & IDR0_SES), idr1);

	if (idr1 & IDR1_PAGESIZE)
		return TEE_ERROR_NOT_SUPPORTED;
	numpage = 1 << (((idr1 >> IDR1_NUMPAGENDXB_SHIFT) &
			 IDR1_NUMPAGENDXB_MASK) + 1);
	if (pi->smr_first + pi->smr_count != (idr0 & IDR0_NUMSMRG) ||
	    pi->cb_first + pi->nr_streams != (idr1 & IDR1_NUMCB) ||
	    pi->vmid_first + pi->nr_streams > 256 ||
	    2 * numpage * SMALL_PAGE_SIZE > SMMU_SIZE)
		return TEE_ERROR_BAD_PARAMETERS;

	in->gr0 = base;
	in->gr1 = base + SMALL_PAGE_SIZE;
	in->cb0 = base + numpage * SMALL_PAGE_SIZE;
	in->banks = calloc(pi->nr_streams, sizeof(*in->banks));
	if (!in->banks)
		return TEE_ERROR_OUT_OF_MEMORY;

	for (i = 0; i < pi->smr_count; i++) {
		uint32_t v = io_read32(base + GR0_SMR(pi->smr_first + i));

		if (v & SMR_VALID) {
			EMSG("fw-smmu: SMR %zu already valid (%#"PRIx32")",
			     pi->smr_first + i, v);
			return TEE_ERROR_BAD_STATE;
		}
	}

	for (i = 0; i < pi->nr_streams; i++) {
		const struct pas_fw_smmu_stream *s = pi->streams + i;
		struct bank *b = &in->banks[i];

		b->smr0 = smr;
		while (b->nr_smr < ARRAY_SIZE(s->smr) && s->smr[b->nr_smr])
			b->nr_smr++;
		smr += b->nr_smr;
		if (smr > pi->smr_first + pi->smr_count)
			return TEE_ERROR_BAD_PARAMETERS;

		b->pool_pages = s->pool_pages;
		b->pool = virt_page_alloc(b->pool_pages, MAF_ZERO_INIT);
		if (!b->pool)
			return TEE_ERROR_OUT_OF_MEMORY;
		b->pool_pa = virt_to_phys((void *)b->pool);
		if (!b->pool_pa)
			return TEE_ERROR_GENERIC;
	}

	in->ok = true;
	return TEE_SUCCESS;
}

static TEE_Result pas_fw_smmu_init(void)
{
	const struct pas_fw_smmu_instance *pis = NULL;
	TEE_Result res = TEE_SUCCESS;
	size_t nr = 0;
	size_t i = 0;

	pis = pas_fw_smmu_platform(&nr);
	insts = calloc(nr, sizeof(*insts));
	if (!insts)
		return TEE_ERROR_OUT_OF_MEMORY;
	instances = pis;
	nr_instances = nr;

	for (i = 0; i < nr_instances; i++) {
		res = init_instance(instances + i, insts + i);
		if (res)
			EMSG("fw-smmu: SMMU %#"PRIxPA" not usable: %#"PRIx32,
			     instances[i].base, res);
	}

	return TEE_SUCCESS;
}
driver_init(pas_fw_smmu_init);
