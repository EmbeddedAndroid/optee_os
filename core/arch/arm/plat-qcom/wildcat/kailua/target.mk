# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# Qualcomm Kailua (SM8550) platform configuration: one DynamIQ cluster of
# eight cores, MPIDR Aff1 0..7 (core_pos = cluster * 6 + core = Aff1).

$(call force,CFG_TEE_CORE_NB_CORE,8)

# OP-TEE and its TA RAM take the first 30 MB of the TA area of the TZ
# carve-out (ta-region in the SM8550 device tree); TF-A loads BL32 there
# (kailua_hdk BL32_BASE, BL32_SIZE). Its protection from the normal world is
# not verified, so only insecure builds are allowed.
CFG_TZDRAM_START ?= 0xd8800000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x01c00000

ifneq ($(CFG_INSECURE),y)
$(error kailua: TZDRAM protection is not verified)
endif

# Seed the PRNG from the TZ PRNG instead of the system counter.
$(call force,CFG_QCOM_CSRNG,y)
$(call force,CFG_WITH_SOFTWARE_PRNG,n)
