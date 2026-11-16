# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

# SecaAU (QAM8787P): a Nord with two of its three Oryon clusters.

$(call force,CFG_TEE_CORE_NB_CORE,12)

CFG_TZDRAM_START ?= 0xBC280000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000
CFG_TA_RAM_VA_SIZE ?= 0x07B80000

CFG_QCOM_RPMH_CLIENT ?= y
ifeq ($(CFG_QCOM_RPMH_CLIENT),y)
$(call force,CFG_QCOM_CMD_DB,y)
endif

CFG_QCOM_TLMM ?= y
QCOM_TLMM_SOC := nord

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y
QCOM_CLK_SOC := nord

CFG_QCOM_PAS_PTA ?= y
QCOM_PAS_SOC := nord

ifeq ($(CFG_QCOM_PAS_PTA),y)
# One metadata slot per PAS image in use: four DSPs, their four DTB
# images and the iris firmware
CFG_PAS_MD_SLOTS ?= 9
CFG_RESERVED_VASPACE_SIZE ?= (64 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
