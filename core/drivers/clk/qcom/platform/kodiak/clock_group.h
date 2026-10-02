/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CLOCK_GROUP_QCOM_H_
#define _CLOCK_GROUP_QCOM_H_

#define GCC_SEC_CTRL_CFG_RCGR			0x3d030
#define GCC_SEC_CTRL_CMD_RCGR			0x3d02c
#define QFPROM_CLOCK_DIVIDE			0x7

#define GCC_WPSS_AHB_CLK			0x9d154
#define GCC_WPSS_AHB_BDG_MST_CLK		0x9d158
#define GCC_WPSS_RSCP_CLK			0x9d16c
#define GCC_TURING_CFG_AHB_CLK			0x45028
#define GCC_CFG_NOC_LPASS_CLK			0x47020

#define TURING_CC_OFFSET			0x00800000
#define TURING_CC_SIZE				0x00023000
#define TURING_Q6SS_Q6_AXIM_CLK			0xb000
#define TURING_CENG_CLK				0x4030
#define TURING_NSPNOC_CLK			0x4040
#define TURING_Q6SS_AHBS_AON_CLK		0x10000
#define TURING_VAPSS_GDSCR			0x16000
#define TURING_VAPSS_CFG_GDSCR			0x16004
#define TURING_VAPSS_GDS_HW_CTRL		0x1600c

#define VAPSS_GDSCR_SW_COLLAPSE_MASK		0x1
#define VAPSS_GDSCR_PWR_ON_MASK			0x80000000
#define VAPSS_GDSCR_RETAIN_FF_ENABLE_MASK	0x800
#define VAPSS_CFG_GDSCR_PWR_UP_COMPLETE_MASK	0x10000
#define VAPSS_GDS_HW_STATE_MASK			0x1E
#define VAPSS_GDS_HW_STATE_SHIFT		1

#define LPASS_GDSC_OFFSET			0x01000000
#define LPASS_GDSC_SIZE				0x0000c000
#define TOP_CC_LPI_Q6_AXIM_HS_CLK		0x4000
#define TOP_CC_AGGNOC_MPU_LS_CLK		0x7000

/* QDSP6SS block, relative to LPASS_BASE. */
#define LPASS_QDSP6SS_OFFSET			0x00400000
#define QDSP6SS_RET_CFG				0x1c
#define QDSP6SS_RET_CFG_RET_ARES_ENA_BIT	BIT(0)

/* Subsystem restarts, within the AOSS_CC window. */
#define AOSS_CC_LPASS_RESTART			0x30000
#define AOSS_CC_SS_RESTART_BIT			BIT(0)

/* PDC sync resets, within the RPMH_PDC_GLOBAL window. */
#define RPMH_PDC_SYNC_RESET			0x1000
#define PDC_SYNC_RESET_AUDIO_BIT		BIT(2)

/*
 * QDSP6 bus master halt blocks, within the TCSR mutex window, and the
 * registers of each block.
 */
#define TCSR_LPASS_HALT				0x22000
#define TCSR_LPASS_PWR_ON			0x10
#define TCSR_HALTREQ				0x0
#define TCSR_HALTACK				0x4
#define TCSR_MASTER_IDLE			0x8
#define TCSR_HALT_BIT				BIT(0)

#endif /* _CLOCK_GROUP_QCOM_H_ */
