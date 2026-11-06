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

# Late mappings (core_mmu_add_mapping()) take whole 2 MiB blocks of the
# reserved VA space, which only gives back the most recent one: the RNG,
# SMEM, the iris registers and the transient iris firmware seed need
# 10 MiB, and with the S-EL1 SPMC the manifest DT block is lost as well.
CFG_RESERVED_VASPACE_SIZE ?= (32 * 1024 * 1024)

# Seed the PRNG from the TZ PRNG instead of the system counter.
$(call force,CFG_QCOM_CSRNG,y)
$(call force,CFG_WITH_SOFTWARE_PRNG,n)

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_PAS_PTA ?= y

# The ADSP, the CDSP and the iris video codec can be left out of the PAS
# pseudo-TA one by one.
CFG_QCOM_PAS_ADSP ?= y
CFG_QCOM_PAS_CDSP ?= y
CFG_QCOM_PAS_IRIS ?= y

# OP-TEE programs the stage 2 of the ADSP, CDSP and iris firmware streams
# on apps_smmu itself (the host DT then carries none of their iommus).
CFG_QCOM_PAS_FW_SMMU ?= n

ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
