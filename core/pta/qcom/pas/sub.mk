srcs-y += pta_qcom_pas.c
srcs-y += pas_core.c
srcs-y += pas_elf.c
srcs-y += pas_iris.c
srcs-$(CFG_QCOM_PAS_AUTH) += pas_auth.c
incdirs-y += .
incdirs-y += platform/
# A SoC sharing another's PAS data sets QCOM_PAS_SOC to that SoC.
QCOM_PAS_SOC ?= $(PLATFORM_FLAVOR)
subdirs-y += platform/$(QCOM_PAS_SOC)
