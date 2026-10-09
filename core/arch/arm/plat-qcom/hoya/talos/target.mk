$(call force,CFG_QCOM_RAMBLUR_PIMEM_V3,n,Talos has RAMBLUR v2.3)

# OP-TEE and its TA RAM run from the DDR carve-out that the QCS615 memory
# map reserves for trusted applications.
CFG_TZDRAM_START ?= 0x87a00000
CFG_TA_RAM_VA_SIZE ?= 0x1a00000

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_PAS_PTA ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
