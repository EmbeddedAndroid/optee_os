// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Pseudo TA that exposes the discrete TPM owned by OP-TEE to the normal
 * world as a TPM transport. It advertises itself on the OP-TEE device bus
 * (TA_FLAG_DEVICE_ENUM) so the Linux TEE-backed TPM driver can bind to it
 * by UUID and pipe raw TPM2 commands through to the in-core driver (the
 * SPI TIS chip driver). This is the discrete-TPM analogue of the fTPM TA's
 * SUBMIT_COMMAND, using a distinct UUID rather than the fTPM one.
 */

#include <assert.h>
#include <drivers/tpm2_chip.h>
#include <inttypes.h>
#include <kernel/pseudo_ta.h>
#include <pta_tpm_passthrough.h>
#include <string.h>
#include <tee_api_defines.h>
#include <trace.h>

#define PTA_NAME "tpm_passthrough.pta"

/*
 * Localities the normal world may submit at, one bit per locality. The
 * secure world reserves locality 4 (D-RTM: _TPM_Hash_* and PCR 17 reset).
 */
#define TPM_NS_LOCALITIES	CFG_TPM_PASSTHROUGH_NS_LOCALITIES

static_assert(!(TPM_NS_LOCALITIES & BIT(TPM2_LOCALITY_MAX)),
	      "locality 4 is reserved for the secure world");
static_assert(TPM_NS_LOCALITIES & BIT(0), "the normal world needs locality 0");

static TEE_Result tpm_pta_submit(uint32_t ptypes,
				 TEE_Param params[TEE_NUM_PARAMS],
				 bool with_locality)
{
	uint32_t exp = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				       TEE_PARAM_TYPE_MEMREF_INOUT,
				       with_locality ?
				       TEE_PARAM_TYPE_VALUE_INPUT :
				       TEE_PARAM_TYPE_NONE,
				       TEE_PARAM_TYPE_NONE);
	uint8_t *cmd = params[0].memref.buffer;
	uint32_t cmd_len = params[0].memref.size;
	uint8_t *resp = params[1].memref.buffer;
	uint32_t resp_len = params[1].memref.size;
	uint32_t locality = 0;
	enum tpm2_result res = TPM2_OK;

	if (ptypes != exp)
		return TEE_ERROR_BAD_PARAMETERS;

	if (!cmd || !cmd_len || !resp || !resp_len)
		return TEE_ERROR_BAD_PARAMETERS;

	if (with_locality) {
		locality = params[2].value.a;
		if (locality > TPM2_LOCALITY_MAX)
			return TEE_ERROR_BAD_PARAMETERS;
		if (!(TPM_NS_LOCALITIES & BIT(locality))) {
			IMSG("TPM passthrough: locality %" PRIu32
			     " refused to the normal world", locality);
			return TEE_ERROR_ACCESS_DENIED;
		}
	}

	res = tpm2_chip_transceive(locality, cmd, cmd_len, resp, &resp_len,
				   TPM2_CMD_DURATION_DEFAULT);
	if (res == TPM2_ERR_SHORT_BUFFER) {
		/* resp_len now holds the required length */
		params[1].memref.size = resp_len;
		return TEE_ERROR_SHORT_BUFFER;
	}
	if (res) {
		EMSG("TPM passthrough: transceive at locality %" PRIu32
		     " failed (%d)", locality, res);
		return TEE_ERROR_COMMUNICATION;
	}

	params[1].memref.size = resp_len;

	return TEE_SUCCESS;
}

static TEE_Result invoke_command(void *session __unused, uint32_t cmd,
				 uint32_t ptypes,
				 TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case PTA_TPM_PASSTHROUGH_SUBMIT_COMMAND:
		return tpm_pta_submit(ptypes, params, false);
	case PTA_TPM_PASSTHROUGH_SUBMIT_COMMAND_LOCALITY:
		return tpm_pta_submit(ptypes, params, true);
	default:
		return TEE_ERROR_NOT_IMPLEMENTED;
	}
}

static TEE_Result open_session(uint32_t ptypes __unused,
			       TEE_Param par[TEE_NUM_PARAMS] __unused,
			       void **session __unused)
{
	struct ts_session *ts = ts_get_current_session();
	struct tee_ta_session *ta_session = to_ta_session(ts);

	/*
	 * The discrete TPM is a shared security resource: raw command access
	 * lets a caller reset PCRs or alter hierarchies and so defeat measured
	 * boot. The passthrough exists only to carry the normal-world kernel's
	 * TPM stack, so restrict it to the REE kernel and reject other TAs and
	 * REE user space.
	 */
	if (ta_session->clnt_id.login != TEE_LOGIN_REE_KERNEL)
		return TEE_ERROR_ACCESS_DENIED;

	return TEE_SUCCESS;
}

pseudo_ta_register(.uuid = PTA_TPM_PASSTHROUGH_UUID, .name = PTA_NAME,
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_DEVICE_ENUM,
		   .open_session_entry_point = open_session,
		   .invoke_command_entry_point = invoke_command);
