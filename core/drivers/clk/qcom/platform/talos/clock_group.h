/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CLOCK_GROUP_QCOM_H_
#define _CLOCK_GROUP_QCOM_H_

/* GCC, relative to GCC_BASE */
#define GCC_TURING_CFG_AHB_CBCR			0x45018
#define GCC_LPASS_Q6_AXI_CBCR			0x47004
#define GCC_LPASS_CORE_AXIM_CBCR		0x47008
#define GCC_LPASS_SWAY_CBCR			0x47010
#define GCC_SSC_CNOC_MPU_CBCR			0x4704c
#define GCC_SSC_RESET				0x63020

#define GCC_SSC_RESET_SSC_ARES			BIT(0)

/* LPASS (SSC), relative to LPASS_BASE */
#define LPASS_QDSP6SS_OFFSET			0x400000
#define SSC_SCC_Q6_AHBM_CBCR			0xb1901c
#define SSC_SCC_Q6_AHBS_CBCR			0xb19020
#define SSC_SCC_AT_CBCR				0xb1a008
#define SSC_SCC_PCLKDBG_CBCR			0xb1a010

/* Turing, relative to TURING_BASE */
#define TURING_QDSP6SS_OFFSET			0x300000
#define TURING_AON_CFG_RCGR			0x5004
#define TURING_WRAPPER_AON_CBCR			0x5098
#define TURING_Q6SS_AHBM_AON_CBCR		0x9000
#define TURING_Q6SS_AHBS_AON_CBCR		0x10000

#define RCGR_SRC_SEL_MASK			GENMASK_32(10, 8)

/* QDSP6SS, relative to the QDSP6SS block */
#define QDSP6SS_RET_CFG				0x1c

#define QDSP6SS_RET_CFG_RET_ARES_ENA		BIT(0)

/* MPM, relative to MPM_SSCAON_BASE */
#define MPM_SSCAON_CONFIG1			0x8

/* Clamps the SSC power-domain handshakes (few_ack, rest_ack) */
#define MPM_SSCAON_CONFIG1_CLAMP		BIT(31)
#define MPM_SSCAON_CONFIG1_RETAIN_FF_ENABLE	BIT(2)

/* AOSS_CC, relative to AOSS_CC_BASE */
#define AOSS_CC_SENSORS_RESTART			0x1000
#define AOSS_CC_COMPUTESS_RESTART		0x10000

#define AOSS_CC_SS_RESTART_BIT			BIT(0)

/* RPMh PDC, relative to RPMH_PDC_GLOBAL_BASE */
#define RPMH_PDC_SYNC_RESET			0x100

#define PDC_SYNC_RESET_AUDIO_SENSORS_BIT	BIT(1)
#define PDC_SYNC_RESET_COMPUTE_BIT		BIT(6)

/* TCSR halt blocks, relative to TCSR_MUTEX_BASE */
#define TCSR_LPASS_HALT				0x22000
#define TCSR_LPASS_CORE_AXIM_HALT		0x2f000
#define TCSR_TURING_HALT			0x34000

#define TCSR_HALTREQ				0x0
#define TCSR_HALTACK				0x4
#define TCSR_MASTER_IDLE			0x8
#define TCSR_TURING_PWR_ON			0xc

/* TCSR_SSC_PWR_ON, relative to TCSR_MUTEX_BASE */
#define TCSR_SSC_PWR_ON				0x26010

#define TCSR_HALT_BIT				BIT(0)

/* TCSR CX iPeak limits management, relative to TCSR_REGS_BASE */
#define TCSR_CXIP_LM_FNT_CLEAR			0x2d018
#define TCSR_CXIP_LM_FNT_STATUS			0x2d01c

#define TCSR_CXIP_LM_FNT_CLEAR_ALL		(BIT(1) | BIT(0))

#endif /* _CLOCK_GROUP_QCOM_H_ */
