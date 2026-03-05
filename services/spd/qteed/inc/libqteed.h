/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef LIBQTEED_H
#define LIBQTEED_H

#define LIBQTEED_INIT_SUCCESS		(0U)
#define LIBQTEED_INIT_ERROR		(1U)
#define LIBQTEED_INIT_COMPAT_FAIL	(2U)

#define LIBQTEED_INIT_PARM_VERSION      (1U)

/*
 * Assume that the version doesn't get longer than v<MMM>.<mmm>.<ppp>\0
 * in length (13 bytes) and round up to nearest power of 2 => 16
 */
#define LIBQTEED_MAX_VERSION_SIZE	(16UL)

struct libqteed_init_param {
	/* Version of this structure */
	uint32_t  param_version;

	/* Whether to enforce that TF-A version match */
	bool enforce_tfa_version;

	/* BL31 QTEED TF-A version */
	const char qteed_tfa_version[LIBQTEED_MAX_VERSION_SIZE];
	const size_t qteed_tfa_version_size;

	/*
	 * Always enforced struct sizes
	 * A mismatch indicates a geniune binary incompatibility
	 */
	const size_t cpu_context_size;
	const size_t entry_point_info_size;
	const size_t spd_pm_ops_size;
	const size_t spm_core_manifest_sect_attribute_size;
};

/**
 * Common SMC handler for QTEE
 *
 *
 * @param[in] 	smc_fid 	SMC Function ID
 * @param[in] 	x1 		x1 register
 * @param[in] 	x2 		x2 register
 * @param[in] 	x3 		x3 register
 * @param[in] 	x4 		x4 register
 * @param[in] 	cookie 		Reserved for future use
 * @param[in] 	handle 		Pointer to the cpu_context_t for the SMC call
 * @param[in] 	flags 		TF-A defined flags including caller security
 * 				state and SME liveness
 * @return 			SMC_OK if SMC was handled successfully
 */
uintptr_t qteed_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			    u_register_t x3, u_register_t x4, void *cookie,
			    void *handle, u_register_t flags);

/**
 * Initial setup function for libqteed.
 *
 * This includes cross checking the TF-A version of BL31 and libqteed,
 * along with sizes of key TF-A structures to prevent binary runtime
 * incompatibility issues if BL31 is linked against an incorrect
 * version of libqteed.
 *
 * @param[in] init_params               Pointer to libqteed_init_param struct
 *                                      containing binary compatibility info
 * @param[in] init_params_size	        Size of init_params in bytes
 */
int32_t libqteed_setup(struct libqteed_init_param *init_params,
		       size_t init_params_size);

/**
 * qteed_plat_error_handler is used by TF-A plat_error_handler to
 * allow EL3 to capture the current CPU state and forward that
 * to QTEE, along with an error code in order to prepare for crashdump.
 *
 * @param[in]	error	        QTEE error code to report
 *
 * This function does not return
 */
__dead2 void qteed_plat_error_handler(int error);

/**
 * qteed_panic_handler is used by plat_panic_handler to
 * allow EL3 to capture the current CPU state and forward that
 * to QTEE in order to prepare for crashdump.
 *
 * This function does not return
 */
__dead2 void qteed_panic_handler(void);

/**
 * qteed_error_handler is used by QTI drivers to
 * allow EL3 to capture the current CPU state and forward that
 * to QTEE, along with an error code in order to prepare for crashdump.
 *
 * @param[in]	error	        QTEE error code to report
 *
 * This function does not return
 */
__dead2 void qteed_error_handler(int error);

/**
 * Register a new ISR handler in QTEE which will then call the provided function
 * whenever the corresponding interrupt is triggered.
 *
 * @param [in] intnum - Interrupt number
 * @param [in] int_desc - Interrupt description
 * @param [in] fn - ISR
 * @param [in] ctx - additional context
 * @param [in] flags - flags
 * @param [in] enable - enable the interrupt
 *
 * @return E_SUCCESS if successful, Error code otherwise.
 */
int qteed_register_qtee_isr(uint32_t intnum, const char *int_desc,
    void *(*fn)(void *), void *ctx, uint32_t flags, bool enable);

/**
 * Disable the current interrupt for a fatal error registered with QTEE.
 *
 * @param [in] intnum - Interrupt number not used
 * @return E_SUCCESS if successful, Error code otherwise.
 */
int qteed_disable_qtee_int(uint32_t intnum);

/**
 * This function will store the error in a global variable so that we can pass
 * it down to QTEE and act on it.
 *
 * @param[in]  err   The category of error requiring the reset
*/
void qteed_set_err_fatal_qtee(uint32_t err);

/**
 * This function will store the error and the condition in global variables so
 * that we can pass them down to QTEE and act on them.
 *
 * @param [in]  err   The category of error requiring the reset
 * @param [in]  flag  flag indicating if execution should return to HLOS
 * @returns None
*/
void qteed_set_err_fatal_with_cond_qtee(uint32_t err,
    bool return_to_hlos);

/**
 * Create a shallow copy of the given object, identified by a unique ID, a ptr
 * and a size, to the shared data structure between TFA bl31 and QTEE.
 * This function will return a ptr to the copy which will be read-only for QTEE.
 * Altough this copy can be updated, there is no guaranteed that any update will
 * be consumed by QTEE. This memory is statically allocated and cannot be
 * freed, thus remaining available throughout TFA and QTEE execution.
 *
 * @param[in] objectID 	unique objectID for the data to be shared, to be defined
 *                       in the common header file qtee_tfa_bl31_sh_mem.h
 *
 * @param[in] ptr 	pointer to the data to be copied. It will be shallow
 *			copied, reason for which it should not contain pointers!
 *
 * @param[in] size	size of the data pointed by 'ptr', to be copied
 *
 * @return  a pointer to the new shared data in memory. This pointer can be used
 *			to update the data.
 */
void *qteed_copy_obj_to_shared_data(uint32_t objectID, void *ptr, size_t size);


#endif /* LIBQTEED_H */
