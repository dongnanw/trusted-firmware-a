/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>

#include "libqteed.h"

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/psci/psci_lib.h>
#include <lib/smccc.h>
#include <services/spm_core_manifest.h>

/* Version string with build variant, for printing and post-build checks */
#ifdef QTI_BL31_WITH_TEST
static const char qteed_version[] = "qteed_with_test:v" BUILD_MESSAGE_VERSION;
#else
static const char qteed_version[] = "qteed:v" BUILD_MESSAGE_VERSION;
#endif

/* Setup function invoked by TF-A framework */
int32_t qteed_setup(void)
{
	INFO("qteed_setup: %s\n", qteed_version);

	/* Setup libqteed_setup params for binary compatiblity checks */
	struct libqteed_init_param init_params = {
		.param_version = LIBQTEED_INIT_PARM_VERSION,

		/* TF-A version checking not strictly enforced */
		.enforce_tfa_version = false,
		.qteed_tfa_version = BUILD_MESSAGE_VERSION,
		.qteed_tfa_version_size = sizeof(BUILD_MESSAGE_VERSION),

		/* Structure sizes will be enforced */
		.cpu_context_size = sizeof(struct cpu_context),
		.entry_point_info_size = sizeof(struct entry_point_info),
		.spd_pm_ops_size = sizeof(struct spd_pm_ops),
		.spm_core_manifest_sect_attribute_size =
			sizeof(struct spm_core_manifest_sect_attribute)
	};

	return libqteed_setup(&init_params, sizeof(init_params));
}

#ifndef BUILD_OFFTARGET_UNITTEST
/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(qteed_fast, OEN_TOS_START, OEN_TOS_END, SMC_TYPE_FAST,
	       qteed_setup, qteed_smc_handler);

/* Define a SPD runtime service descriptor for Yielding SMC Calls */
DECLARE_RT_SVC(qteed_std, OEN_TOS_START, OEN_TOS_END, SMC_TYPE_YIELD, NULL,
	       qteed_smc_handler);

/* Define a SPD runtime service descriptor for fast TA SMC calls */
DECLARE_RT_SVC(qteed_tap_fast, OEN_TAP_START, OEN_TAP_END, SMC_TYPE_FAST, NULL,
	       qteed_smc_handler);

/* Define a SPD runtime service descriptor for Yielding TA SMC Calls */
DECLARE_RT_SVC(qteed_tap_std, OEN_TAP_START, OEN_TAP_END, SMC_TYPE_YIELD, NULL,
	       qteed_smc_handler);
/**
 * Register qteed to handle ARM yielding calls
 *
 * This is currently required so that when HLOS resumes an interrupted SMC call
 * using SMC_FID = SMC_INTERRUPTED = 0x1, this request is forwarded to QTEE and
 * not rejected as SMC_UNK by TF-A
 */
DECLARE_RT_SVC(qteed_std_arm, OEN_ARM_START, OEN_ARM_END, SMC_TYPE_YIELD, NULL,
	       qteed_smc_handler);

#endif /* BUILD_OFFTARGET_UNITTEST */
