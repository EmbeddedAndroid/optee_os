# TZDRAM is the 6 MiB DDR carve-out the Linux DT reserves for the hypervisor,
# which is free with Linux at EL2. It has to match TF-A's BL32_BASE.
CFG_TZDRAM_START ?= 0x45700000
CFG_TEE_RAM_VA_SIZE ?= 0x200000
CFG_TA_RAM_VA_SIZE ?= 0x400000

CFG_DRIVERS_CLK ?= y
CFG_DRIVERS_QCOM_CLK ?= y

CFG_QCOM_PAS_PTA ?= y

ifeq ($(CFG_QCOM_PAS_PTA),y)
# PAS subsystems map their controller windows at runtime from the reserved VA
# pool (never released), in 2 MiB blocks.
CFG_RESERVED_VASPACE_SIZE ?= (48 * 1024 * 1024)
CFG_IN_TREE_EARLY_TAS += qcom_pas/cff7d191-7ca0-4784-af13-48223b9a4fbe
endif
