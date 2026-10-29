# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# Qualcomm Eliza (SM7750) platform configuration: one DynamIQ cluster of
# eight cores, MPIDR Aff1 0..7 (core_pos = cluster * 6 + core = Aff1).

$(call force,CFG_TEE_CORE_NB_CORE,8)

# OP-TEE and its TA RAM run from the DDR region the SM7750 device tree
# reserves for the Gunyah OEM VM (oem-vm, 0xf7c00000 + 0x4c00000), which
# is unused when Linux runs at EL2 without Gunyah; TF-A loads BL32 there
# (eliza_mtp BL32_BASE, BL32_SIZE).
CFG_TZDRAM_START ?= 0xf7c00000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x01c00000

# Seed the PRNG from the TZ PRNG instead of the system counter.
$(call force,CFG_QCOM_CSRNG,y)
$(call force,CFG_WITH_SOFTWARE_PRNG,n)

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_PAS_PTA ?= y

# The ADSP and the CDSP can be left out of the PAS pseudo-TA one by one.
CFG_QCOM_PAS_ADSP ?= y
CFG_QCOM_PAS_CDSP ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
