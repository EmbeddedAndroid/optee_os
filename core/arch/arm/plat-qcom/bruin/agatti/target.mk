# TZDRAM is the 6 MiB DDR carve-out the Linux DT reserves for the hypervisor,
# which is free with Linux at EL2. It has to match TF-A's BL32_BASE.
CFG_TZDRAM_START ?= 0x45700000
CFG_TEE_RAM_VA_SIZE ?= 0x200000
CFG_TA_RAM_VA_SIZE ?= 0x400000
