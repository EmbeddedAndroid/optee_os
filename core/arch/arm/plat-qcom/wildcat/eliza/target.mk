# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# Qualcomm Eliza (SM7750) platform configuration: one DynamIQ cluster of
# eight cores, MPIDR Aff1 0..7 (core_pos = cluster * 6 + core = Aff1).

$(call force,CFG_TEE_CORE_NB_CORE,8)

# OP-TEE and its TA RAM run from the DDR region the SM7750 device tree
# reserves for the Gunyah OEM VM (oem-vm, 0xf7c00000 + 0x4c00000), which
# is unused when Linux runs at EL2 without Gunyah. TF-A loads BL32 there.
CFG_TZDRAM_START ?= 0xf7c00000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x01c00000

# No access control protects TZDRAM on Eliza yet (neither OP-TEE nor
# TF-A programs one), so the normal world can read and write it.
ifneq ($(CFG_INSECURE),y)
$(error Eliza: TZDRAM is not protected, CFG_INSECURE=n is not supported)
endif

# Random numbers and the core ASLR seed come from the TZ PRNG.
$(call force,CFG_QCOM_CSRNG,y)
$(call force,CFG_WITH_SOFTWARE_PRNG,n)
