# A SoC sharing another's clock data sets QCOM_CLK_SOC to that SoC.
QCOM_CLK_SOC ?= $(PLATFORM_FLAVOR)

global-incdirs-y += .
global-incdirs-y += platform/$(QCOM_CLK_SOC)

srcs-y += clk.c
srcs-$(CFG_QCOM_PAS_PTA) += platform/$(QCOM_CLK_SOC)/clock-pas.c
incdirs-$(CFG_QCOM_CLK_HOYA_PAS) += hoya
subdirs-$(CFG_QCOM_CLK_HOYA_PAS) += hoya

ifeq ($(CFG_QCOM_CLK_CFG),y)
srcs-y += platform/$(QCOM_CLK_SOC)/clk_cfg.c
srcs-y += clk_ops.c
srcs-$(CFG_QCOM_RPMH_CLIENT) += rail_rpmh.c
endif

incdirs-y += .
incdirs-$(CFG_QCOM_PAS_PTA) += platform/$(QCOM_CLK_SOC)
incdirs-$(CFG_QCOM_CLK_CFG) += platform/$(QCOM_CLK_SOC)
