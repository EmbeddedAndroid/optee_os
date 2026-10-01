# BRUIN architecture configuration

include core/arch/arm/cpu/cortex-armv8-0.mk
$(call force,CFG_TEE_CORE_NB_CORE,4)
$(call force,CFG_ARM_GICV3,y)

CFG_TZDRAM_SIZE ?= (CFG_TEE_RAM_VA_SIZE + CFG_TA_RAM_VA_SIZE)
CFG_NUM_THREADS ?= 4
