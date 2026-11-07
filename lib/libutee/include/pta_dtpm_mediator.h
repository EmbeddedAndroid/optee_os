/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */
#ifndef __PTA_DTPM_MEDIATOR_H
#define __PTA_DTPM_MEDIATOR_H

#define PTA_DTPM_MEDIATOR_UUID { 0x50568cd9, 0x370a, 0x46b8, \
		{ 0x9f, 0x2c, 0x6f, 0xd9, 0x06, 0x64, 0xd2, 0xee } }

/*
 * Submit a raw TPM command to the discrete TPM owned by OP-TEE and return
 * its response. The command/response wire format is plain TPM2 bytes,
 * identical in shape to the fTPM TA's SUBMIT_COMMAND but backed by the
 * in-core discrete-TPM (SPI TIS) driver. The secure world mediates this
 * access (see the PTA source) rather than merely bridging it.
 *
 * [in]     memref[0]        TPM command buffer
 * [in/out] memref[1]        TPM response buffer (in: capacity, out: length)
 */
#define PTA_DTPM_MEDIATOR_SUBMIT_COMMAND	0

/*
 * Same as PTA_DTPM_MEDIATOR_SUBMIT_COMMAND at a given TIS locality. A
 * locality the platform does not grant the normal world (never locality 4,
 * which is reserved for the secure world) fails with TEE_ERROR_ACCESS_DENIED
 * and nothing reaches the TPM.
 *
 * [in]     memref[0]        TPM command buffer
 * [in/out] memref[1]        TPM response buffer (in: capacity, out: length)
 * [in]     value[2].a       TIS locality (0 to 4)
 */
#define PTA_DTPM_MEDIATOR_SUBMIT_COMMAND_LOCALITY	1

#endif /* __PTA_DTPM_MEDIATOR_H */
