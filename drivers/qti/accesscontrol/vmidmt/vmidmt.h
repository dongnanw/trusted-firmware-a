/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VMIDMT_H_
#define _VMIDMT_H_

#include <stdbool.h>

#define MSM_ERR_VMIDMT_CFG_FAIL    1

#define MSM_VMID_NOACCESS          0
#define MSM_VMID_VMID_0            0
#define MSM_VMID_RPM               1
#define MSM_VMID_TZ                2
#define MSM_VMID_AP                3
#define MSM_VMID_MSS               4
#define MSM_VMID_LPASS             5
#define MSM_VMID_CP                6
#define MSM_VMID_VIDEO             7
#define MSM_VMID_ZAP_SHADER        8
#define MSM_VMID_MDSS              9
/* Space for VMID 10 */
#define MSM_VMID_SSC               11

/* IPA VMIDs (used to generate SIDs) */
#define MSM_VMID_IPA_AP            (0b1100)  /* 12 */
#define MSM_VMID_IPA_UC_PIPE       (0b1101)  /* 13 */
#define MSM_VMID_IPA_WIFI          MSM_VMID_IPA_UC_PIPE
#define MSM_VMID_IPA_UC            (0b1110)  /* 14 */
#define MSM_VMID_IPA_GP            (0b0000)  /* 0  */
#define MSM_VMID_IPA_FW            (0b0010)  /* 2  */
#define MSM_VMID_IPA_PERIPH_1      (0b0001)  /* 1  */
#define MSM_VMID_IPA_PERIPH_2      (0b0011)  /* 3  */
#define MSM_VMID_IPA_PERIPH_3      (0b0100)  /* 4  */
#define MSM_VMID_IPA_PERIPH_4      (0b0101)  /* 5  */

/*
 * 9x35/9x45 IPA VMID behavior:
 * IPA uC handles power collapse save/restore. Without XPU/VMIDMT retention,
 * uC firmware may reprogram VMIDMT. Security requires IPA HW to force VMID
 * bit 4 high so it cannot use regular system VMIDs.
 */
#define MSM_VMID_IPA_AP_MDM        0x13      /* BIT 4 SET | MSM_VMID_AP  */
#define MSM_VMID_IPA_UC_PIPE_MDM   0x14      /* BIT 4 SET | MSM_VMID_MSS */
#define MSM_VMID_IPA_UC_MDM        0x15      /* BIT 4 SET & new VMID     */

#define MSM_VMID_WLAN              15

/*
 */
#define MSM_VMID_AP_GSI            0x16
#define MSM_VMID_QUP_TZ            0x0

#define MSM_VMID_NOACCESS_BIT      (1U << MSM_VMID_NOACCESS)
#define MSM_VMID_VMID_0_BIT        (1U << MSM_VMID_VMID_0)
#define MSM_VMID_TZ_BIT            (1U << MSM_VMID_TZ)
#define MSM_VMID_RPM_BIT           (1U << MSM_VMID_RPM)
#define MSM_VMID_LPASS_BIT         (1U << MSM_VMID_LPASS)
#define MSM_VMID_MSS_BIT           (1U << MSM_VMID_MSS)
#define MSM_VMID_AP_BIT            (1U << MSM_VMID_AP)
#define MSM_VMID_CP_BIT            (1U << MSM_VMID_CP)
#define MSM_VMID_VIDEO_BIT         (1U << MSM_VMID_VIDEO)
#define MSM_VMID_MDSS_BIT          (1U << MSM_VMID_MDSS)
#define MSM_VMID_SSC_BIT           (1U << MSM_VMID_SSC)

#define MSM_VMID_IPA_AP_BIT        (1U << MSM_VMID_IPA_AP)
#define MSM_VMID_IPA_UC_PIPE_BIT   (1U << MSM_VMID_IPA_UC_PIPE)
#define MSM_VMID_IPA_UC_BIT        (1U << MSM_VMID_IPA_UC)

#define MSM_VMID_IPA_AP_MDM_BIT        (1U << MSM_VMID_IPA_AP_MDM)
#define MSM_VMID_IPA_UC_PIPE_MDM_BIT   (1U << MSM_VMID_IPA_UC_PIPE_MDM)
#define MSM_VMID_IPA_UC_MDM_BIT        (1U << MSM_VMID_IPA_UC_MDM)

#define MSM_VMID_WLAN_BIT          (1U << MSM_VMID_WLAN)
#define MSM_VMID_ZAP_SHADER_BIT    (1U << MSM_VMID_ZAP_SHADER)
#define MSM_VMID_IPA_WIFI_BIT      (1U << MSM_VMID_IPA_WIFI)

/* All VMIDs except NOACCESS, with domain bits masked out */
#define MSM_ALL_VMID               ((~MSM_VMID_NOACCESS) & (~MSM_DOMAIN_MASK))

/* Domain definitions. Start from 31 downward to avoid VMID conflicts. */
#define MSM_DOMAIN_MASK            0xFF000000  /* Reserve 8 bits for domains */
#define MSM_DOMAIN_MSA             31
#define MSM_DOMAIN_HYP             30
#define MSM_DOMAIN_SP              29
#define MSM_DOMAIN_TZ              28

/* Domain bit masks */
#define MSM_DOMAIN_MSA_BIT         (1U << MSM_DOMAIN_MSA)
#define MSM_DOMAIN_HYP_BIT         (1U << MSM_DOMAIN_HYP)
#define MSM_DOMAIN_SP_BIT          (1U << MSM_DOMAIN_SP)
#define MSM_DOMAIN_TZ_BIT          (1U << MSM_DOMAIN_TZ)

int vmidmt_configure(void);

#endif /* _VMIDMT_H_ */

