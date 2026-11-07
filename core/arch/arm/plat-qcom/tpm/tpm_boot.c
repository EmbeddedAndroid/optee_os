// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <drivers/qcom_geni_spi.h>
#include <drivers/tpm2_chip.h>
#include <drivers/tpm2_spi.h>
#include <initcall.h>
#include <inttypes.h>
#include <spi.h>
#include <stdio.h>
#include <string.h>
#include <tpm2.h>
#include <trace.h>

static struct qup_spi_data tpm2_spi_qs;

static TEE_Result init_tpm2_spi(void)
{
	TEE_Result res = TEE_SUCCESS;

	res = qup_spi_init(&tpm2_spi_qs, CFG_TPM2_SPI_SE_ID);
	if (res) {
		EMSG("TPM2: qup_spi_init(%u) failed: %#" PRIx32,
		     (unsigned int)CFG_TPM2_SPI_SE_ID, res);
		return res;
	}

	tpm2_spi_qs.speed_hz = CFG_TPM2_SPI_SPEED_HZ;
	tpm2_spi_qs.bits_per_word = 8;
	tpm2_spi_qs.mode = SPI_MODE0;
	tpm2_spi_qs.cs = CFG_TPM2_SPI_CS;
	tpm2_spi_qs.cs_high = false;
	tpm2_spi_qs.loopback = false;
	tpm2_spi_qs.chip.ops->configure(&tpm2_spi_qs.chip);

	if (tpm2_spi_init(&tpm2_spi_qs.chip)) {
		EMSG("TPM2: chip registration failed");
		return TEE_ERROR_GENERIC;
	}

	DMSG("TPM2 SPI chip initialized");

	if (IS_ENABLED(CFG_QCOM_TPM2_LOC4_TEST)) {
		/*
		 * Test hook: a locality 4 hash sequence over a fixed string, so
		 * that the normal world can check PCR 17 =
		 * SHA256(0^32 || SHA256(msg)), a value only locality 4 can set.
		 */
		static const char msg[] = "OP-TEE TPM locality 4 test";
		uint8_t pcr[32] = { };
		uint32_t pcr_len = sizeof(pcr);
		char hex[2 * sizeof(pcr) + 1] = { };
		unsigned int i = 0;

		if (tpm2_chip_hash_sequence((const uint8_t *)msg,
					    strlen(msg))) {
			EMSG("TPM2: locality 4 hash sequence failed");
		} else if (tpm2_pcr_read(17, TPM2_ALG_SHA256, pcr, &pcr_len)) {
			EMSG("TPM2: PCR 17 read failed");
		} else {
			for (i = 0; i < pcr_len && i < sizeof(pcr); i++)
				snprintf(hex + 2 * i, 3, "%02x", pcr[i]);
			IMSG("TPM2: locality 4 hash done, PCR 17 sha256 %s",
			     hex);
		}
	}

	return TEE_SUCCESS;
}

/*
 * driver_init_late: the SPI transfer path polls for the TPM's wait-state
 * with mdelay(), which needs the tick/delay facilities up, so this must run
 * after the early driver init pass.
 */
driver_init_late(init_tpm2_spi);
