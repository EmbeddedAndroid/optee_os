// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * D-RTM measurement endpoint: the Arm DRTM (DEN0113) Dynamic Configuration
 * Environment in EL3 records the measurements of a dynamic launch in the
 * TPM's D-RTM PCRs through this logical partition. The ABI is in
 * <tpm2_drtm_lsp.h>.
 *
 * Requests run in a thread that cannot RPC, with foreign interrupts
 * masked. They take the chip with tpm2_chip_claim() and answer
 * TPM2_DRTM_RES_BUSY rather than wait for another thread that holds it
 * or for a transport lock a suspended thread holds.
 */

#include <drivers/tpm2_chip.h>
#include <ffa.h>
#include <initcall.h>
#include <inttypes.h>
#include <kernel/thread_spmc.h>
#include <string.h>
#include <tpm2.h>
#include <tpm2_drtm_lsp.h>
#include <trace.h>
#include <util.h>

#define TPM2_DRTM_SHA384_HI_SIZE \
	(TPM2_SHA384_DIGEST_SIZE - TPM2_DRTM_PAYLOAD_SIZE)

/* First half of a SHA-384 digest, until the request with the second half */
static uint8_t drtm_sha384[TPM2_SHA384_DIGEST_SIZE];
static uint8_t drtm_sha384_pcr;
/* Banks declared by the last DL_EVENT, 0 before the first launch */
static uint32_t drtm_banks;

static enum tpm2_result drtm_extend(uint8_t pcr, uint16_t alg, uint8_t *digest,
				    uint32_t len)
{
	if (!(drtm_banks & tpm2_alg_to_tcg_mask(alg)))
		return TPM2_ERR_INVALID_ARG;

	return tpm2_pcr_extend_locality(TPM2_DRTM_LOCALITY, pcr, alg, digest,
					len);
}

static enum tpm2_result drtm_request(uint8_t cmd, uint8_t arg,
				     uint32_t banks, uint8_t *payload)
{
	struct tpm2_caps caps = { };
	enum tpm2_result ret = TPM2_OK;

	if (cmd == TPM2_DRTM_CMD_DL_EVENT) {
		/*
		 * An active bank EL3 does not measure in would keep its reset
		 * value whatever was launched: refuse the launch.
		 */
		if (arg > TPM2_DRTM_PAYLOAD_SIZE ||
		    tpm2_chip_get_caps(&caps) || !banks ||
		    caps.active_mask != banks)
			return TPM2_ERR_INVALID_ARG;
		drtm_sha384_pcr = 0;
	} else if (cmd >= TPM2_DRTM_CMD_EXTEND &&
		   cmd <= TPM2_DRTM_CMD_SHA384_HI) {
		if (banks || arg < TPM2_DRTM_PCR_FIRST ||
		    arg > TPM2_DRTM_PCR_LAST)
			return TPM2_ERR_INVALID_ARG;
	} else {
		return TPM2_ERR_INVALID_ARG;
	}

	if (cmd == TPM2_DRTM_CMD_SHA384_LO) {
		memcpy(drtm_sha384, payload, TPM2_DRTM_PAYLOAD_SIZE);
		drtm_sha384_pcr = arg;
		return TPM2_OK;
	}
	if (cmd == TPM2_DRTM_CMD_SHA384_HI) {
		if (drtm_sha384_pcr != arg)
			return TPM2_ERR_INVALID_ARG;
		drtm_sha384_pcr = 0;
		memcpy(drtm_sha384 + TPM2_DRTM_PAYLOAD_SIZE, payload,
		       TPM2_DRTM_SHA384_HI_SIZE);
	}

	ret = tpm2_chip_claim();
	if (ret)
		return ret;

	if (cmd == TPM2_DRTM_CMD_DL_EVENT) {
		drtm_banks = 0;
		ret = tpm2_chip_hash_sequence(payload, arg);
		if (!ret)
			drtm_banks = banks;
	} else if (cmd == TPM2_DRTM_CMD_EXTEND) {
		ret = drtm_extend(arg, TPM2_ALG_SHA256, payload,
				  TPM2_SHA256_DIGEST_SIZE);
	} else {
		ret = drtm_extend(arg, TPM2_ALG_SHA384, drtm_sha384,
				  TPM2_SHA384_DIGEST_SIZE);
	}

	tpm2_chip_release();

	return ret;
}

static uint64_t drtm_result(enum tpm2_result res)
{
	switch (res) {
	case TPM2_OK:
		return TPM2_DRTM_RES_OK;
	case TPM2_ERR_INVALID_ARG:
		return TPM2_DRTM_RES_INVALID_ARG;
	case TPM2_ERR_ARG_LIST_TOO_LONG:
		return TPM2_DRTM_RES_ARG_LIST_TOO_LONG;
	case TPM2_ERR_BUSY:
		return TPM2_DRTM_RES_BUSY;
	case TPM2_ERR_TIMEOUT:
		return TPM2_DRTM_RES_TIMEOUT;
	case TPM2_ERR_IO:
		return TPM2_DRTM_RES_IO;
	case TPM2_ERR_NODEV:
		return TPM2_DRTM_RES_NODEV;
	case TPM2_ERR_NO_ACTIVE_LOCALITY:
		return TPM2_DRTM_RES_NO_ACTIVE_LOCALITY;
	case TPM2_ERR_SHORT_BUFFER:
		return TPM2_DRTM_RES_SHORT_BUFFER;
	case TPM2_ERR_CMD:
		return TPM2_DRTM_RES_CMD;
	default:
		return TPM2_DRTM_RES_GENERIC;
	}
}

static void drtm_direct_req(struct thread_smc_1_2_regs *args,
			    struct sp_session *caller_sp)
{
	uint8_t payload[TPM2_DRTM_PAYLOAD_SIZE] = { };
	uint32_t src = FFA_SRC(args->a1);
	uint32_t dst = FFA_DST(args->a1);
	enum tpm2_result ret = TPM2_OK;

	if (caller_sp || src != CFG_TPM2_DRTM_EL3_ID) {
		spmc_set_args(args, FFA_ERROR, 0, FFA_DENIED, 0, 0, 0);
		return;
	}
	if (args->a0 != FFA_MSG_SEND_DIRECT_REQ_64 || args->a2 ||
	    (args->a3 & TPM2_DRTM_X3_MBZ)) {
		spmc_set_args(args, FFA_ERROR, 0, FFA_NOT_SUPPORTED, 0, 0, 0);
		return;
	}

	memcpy(payload, &args->a[4], sizeof(payload));
	ret = drtm_request(TPM2_DRTM_X3_CMD(args->a3),
			   TPM2_DRTM_X3_ARG(args->a3),
			   TPM2_DRTM_X3_BANKS(args->a3), payload);
	if (ret)
		EMSG("D-RTM request %#"PRIx64" failed: %d", args->a3, ret);

	spmc_set_args(args, FFA_MSG_SEND_DIRECT_RESP_64,
		      SHIFT_U32(dst, 16) | src, 0, drtm_result(ret), 0, 0);
}

static struct spmc_lsp_desc drtm_lsp __nex_data = {
	.name = "TPM2 D-RTM",
	.direct_req = drtm_direct_req,
	.in_thread = true,
	.sp_id = CFG_TPM2_DRTM_LSP_ID,
	.properties = FFA_PART_PROP_DIRECT_REQ_RECV |
		      FFA_PART_PROP_AARCH64_STATE,
	/* UUID 3c0a5bd2-5b8f-4d0e-9b6a-9e1f6d2a7c41 */
	.uuid_words = { 0xd25b0a3c, 0x0e4d8f5b, 0x1f9e6a9b, 0x417c2a6d },
};

static TEE_Result drtm_lsp_init(void)
{
	return spmc_register_lsp(&drtm_lsp);
}

nex_service_init_late(drtm_lsp_init);
