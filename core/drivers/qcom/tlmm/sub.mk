# SPDX-License-Identifier: BSD-2-Clause
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

srcs-$(CFG_QCOM_TLMM) += gpio.c
srcs-$(CFG_QCOM_TLMM) += pinctrl.c
# A SoC sharing another's TLMM sets QCOM_TLMM_SOC to that SoC.
QCOM_TLMM_SOC ?= $(PLATFORM_FLAVOR)
srcs-$(CFG_QCOM_TLMM) += $(QCOM_TLMM_SOC)/tlmm_soc_data.c
