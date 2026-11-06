/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef PAS_FW_SMMU_H
#define PAS_FW_SMMU_H

#include <stddef.h>
#include <stdint.h>
#include <tee_api_types.h>
#include <types_ext.h>

/*
 * Stage 2 of the remote processors' own SMMU streams, programmed by OP-TEE
 * on an Arm MMU-500 without the Security Extensions: OP-TEE owns the top
 * SMRs, context banks and VMIDs of the instance, the hypervisor keeps the
 * normal world out of them (OPTEE_FFA_FW_SMMU_INFO tells it which).
 */

struct pas_fw_smmu_range {
	paddr_t pa;
	size_t size;
};

/*
 * struct pas_fw_smmu_stream : the firmware streams of one remote processor.
 *
 * @pas_id:	PAS ID whose AUTH_AND_RESET starts the core.
 * @smr:	SMR values (mask << 16 | stream ID), 0-terminated.
 * @carveouts:	static carveouts the firmware may use, identity mapped
 *		(the host's MEM_SETUP region of @pas_id and of @dtb_pas_id
 *		must lie inside one of them), size 0 terminated.
 * @dtb_pas_id:	PAS ID of the DTB image loaded before @pas_id, or 0.
 * @fw_iova:	when not ~0, the firmware of @pas_id is mapped at this IOVA
 *		(whole carveout) instead of identity.
 * @use_rsc:	identity map the devmem entries of the subsystem's
 *		resource table.
 * @keep_after_stop: leave the bank live after SHUTDOWN, for a core the
 *		host halts only after the call.
 * @pool_pages:	pages of stage 2 tables reserved for this stream.
 */
struct pas_fw_smmu_stream {
	uint32_t pas_id;
	uint32_t smr[4];
	struct pas_fw_smmu_range carveouts[3];
	uint32_t dtb_pas_id;
	uint64_t fw_iova;
	bool use_rsc;
	bool keep_after_stop;
	unsigned int pool_pages;
};

struct pas_fw_smmu_instance {
	paddr_t base;
	uint32_t smr_first;
	uint32_t smr_count;
	uint32_t cb_first;
	uint32_t vmid_first;
	const struct pas_fw_smmu_stream *streams;
	size_t nr_streams;
};

/* Platform data, platform/<flavor>/fw_smmu.c */
const struct pas_fw_smmu_instance *pas_fw_smmu_platform(size_t *count);

bool pas_fw_smmu_owns(uint32_t pas_id);
TEE_Result pas_fw_smmu_start(uint32_t pas_id);
void pas_fw_smmu_stop(uint32_t pas_id);
/* Disable the bank after a failed start, whatever @keep_after_stop says */
void pas_fw_smmu_abort(uint32_t pas_id);

#endif /* PAS_FW_SMMU_H */
