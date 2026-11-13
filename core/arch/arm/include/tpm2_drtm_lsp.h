/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#ifndef __TPM2_DRTM_LSP_H
#define __TPM2_DRTM_LSP_H

#include <util.h>

/*
 * ABI of the TPM D-RTM measurement endpoint (CFG_TPM2_DRTM_LSP): an FF-A
 * logical partition with endpoint ID CFG_TPM2_DRTM_LSP_ID and UUID
 * 3c0a5bd2-5b8f-4d0e-9b6a-9e1f6d2a7c41, called by the Arm DRTM (DEN0113)
 * service in EL3 and by the DLME, the normal world software the dynamic
 * launch started. The callers are built outside OP-TEE, so every value
 * below is fixed.
 *
 * Request: FFA_MSG_SEND_DIRECT_REQ_64 with x2 = 0 and
 * x3 bits [7:0]	command, TPM2_DRTM_CMD_*
 *    bits [15:8]	argument of the command
 *    bits [23:16]	for TPM2_DRTM_CMD_DL_EVENT, the PCR banks EL3
 *			measures in (EFI_TCG2_BOOT_HASH_ALG_* bits), else 0
 *    bits [63:24]	0
 * x4-x7		32 bytes of payload in memory order
 *
 * Response: FFA_MSG_SEND_DIRECT_RESP_64 with x3 = TPM2_DRTM_RES_*, or
 * FFA_ERROR with FFA_DENIED for a source the request is not accepted from
 * and FFA_NOT_SUPPORTED for a malformed request.
 *
 * Only source ID CFG_TPM2_DRTM_EL3_ID, an SPMD logical partition, may send
 * the commands up to TPM2_DRTM_CMD_CLOSE_LOCALITY; the SPMD refuses secure
 * source IDs from the normal world. Only source ID TPM2_DRTM_DLME_ID (a
 * hypervisor gives its guests other IDs) may send TPM2_DRTM_CMD_DLME_EVENT,
 * and only while locality 2 is open. The DLME sends its events, and closes
 * locality 2, before it starts other OP-TEE clients: an event that finds
 * the TPM in use gets TPM2_DRTM_RES_BUSY.
 */

#define TPM2_DRTM_X3_CMD(x3)		((x3) & 0xff)
#define TPM2_DRTM_X3_ARG(x3)		(((x3) >> 8) & 0xff)
#define TPM2_DRTM_X3_BANKS(x3)		(((x3) >> 16) & 0xff)
#define TPM2_DRTM_X3_MBZ		GENMASK_64(63, 24)

#define TPM2_DRTM_PAYLOAD_SIZE		U(32)
#define TPM2_DRTM_LOCALITY		U(4)
#define TPM2_DRTM_PCR_FIRST		U(17)
#define TPM2_DRTM_PCR_LAST		U(18)
#define TPM2_DRTM_DLME_ID		U(0)
#define TPM2_DRTM_DLME_LOCALITY		U(2)
#define TPM2_DRTM_DLME_PCR_FIRST	U(19)
#define TPM2_DRTM_DLME_PCR_LAST		U(22)

/*
 * Argument: payload length (up to 32). Refused unless the TPM's active PCR
 * banks are exactly the ones EL3 declares. Runs the locality 4
 * _TPM_Hash_Start/_TPM_Hash_Data/_TPM_Hash_End sequence over the payload:
 * PCRs 17 to 22 are reset and PCR 17 is extended with the digest of the
 * payload in every active bank.
 */
#define TPM2_DRTM_CMD_DL_EVENT		U(1)
/*
 * Argument: PCR index (17 or 18). Extends the SHA-256 bank of that PCR
 * with the payload, a SHA-256 digest, at locality 4.
 */
#define TPM2_DRTM_CMD_EXTEND		U(2)
/*
 * Argument: PCR index (17 or 18). SHA384_LO carries the first 32 bytes of
 * a SHA-384 digest, the following SHA384_HI the last 16 and extends the
 * SHA-384 bank of that PCR at locality 4.
 */
#define TPM2_DRTM_CMD_SHA384_LO		U(3)
#define TPM2_DRTM_CMD_SHA384_HI		U(4)
/*
 * Argument: TPM2_DRTM_DLME_LOCALITY. OPEN_LOCALITY, sent once the launch
 * succeeded, lets the DLME send DLME_EVENT until the matching
 * CLOSE_LOCALITY, sent on the DLME's DRTM_CLOSE_LOCALITY, or the next
 * DL_EVENT.
 */
#define TPM2_DRTM_CMD_OPEN_LOCALITY	U(5)
#define TPM2_DRTM_CMD_CLOSE_LOCALITY	U(6)
/*
 * From the DLME. Argument: PCR index (19 to 22). TPM2_PCR_Event at
 * locality 2 with the 32 payload bytes as event data: every active bank of
 * the PCR is extended with the payload's digest in that bank's algorithm.
 */
#define TPM2_DRTM_CMD_DLME_EVENT	U(7)

/*
 * Response x3. An extend for a bank that is not active or was not
 * declared by the last DL_EVENT, or before a DL_EVENT, fails with
 * TPM2_DRTM_RES_INVALID_ARG. TPM2_DRTM_RES_BUSY: another thread holds the
 * TPM, or a suspended OP-TEE thread holds any mutex (OP-TEE cannot tell
 * which the TPM transport needs); retry after the normal world ran.
 */
#define TPM2_DRTM_RES_OK			U(0)
#define TPM2_DRTM_RES_GENERIC			U(1)
#define TPM2_DRTM_RES_INVALID_ARG		U(2)
#define TPM2_DRTM_RES_ARG_LIST_TOO_LONG		U(3)
#define TPM2_DRTM_RES_BUSY			U(4)
#define TPM2_DRTM_RES_TIMEOUT			U(5)
#define TPM2_DRTM_RES_IO			U(6)
#define TPM2_DRTM_RES_NODEV			U(7)
#define TPM2_DRTM_RES_NO_ACTIVE_LOCALITY	U(8)
#define TPM2_DRTM_RES_SHORT_BUFFER		U(9)
#define TPM2_DRTM_RES_CMD			U(10)

#endif /* __TPM2_DRTM_LSP_H */
