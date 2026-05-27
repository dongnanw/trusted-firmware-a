/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31QTILIB_INTERFACE_H
#define BL31QTILIB_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bl31qtilib_defs.h"

extern const char bl31qtilib_build_variant[];

/**
 * bl31qtilib_delay_timer_init
 *
 * Initializes the delay timer for the BL31 QTI library.
 *
 * @return None.
 */
void bl31qtilib_delay_timer_init(void);

/**
 * bl31qtilib_is_cold_boot_done
 *
 * Checks whether the cold boot sequence has been completed.
 *
 * @return true if cold boot is done, false otherwise.
 */
bool bl31qtilib_is_cold_boot_done(void);

/**
 * bl31qtilib_set_cold_boot_done
 *
 * Marks the cold boot sequence as completed.
 *
 * @return None.
 */
void bl31qtilib_set_cold_boot_done(void);

/**
 * bl31qtilib_get_boot_cpu_num
 *
 * Returns boot CPU number. If the API is called before the first CPU is
 * booted, it returns the running CPU core number.
 *
 * @return Boot CPU number.
 */
uint32_t bl31qtilib_get_boot_cpu_num(void);

/**
 * bl31qtilib_set_boot_cpu_num
 *
 * Sets the boot CPU number.
 *
 * @param[in] boot_cpu  Boot CPU number.
 *
 * @return None.
 */
void bl31qtilib_set_boot_cpu_num(uint32_t boot_cpu);

/**
 * bl31qtilib_is_quick_boot
 *
 * Checks whether the system is performing a quick boot.
 *
 * @return true if quick boot is enabled, false otherwise.
 */
bool bl31qtilib_is_quick_boot(void);

/**
 * bl31qtilib_bl31_platform_early_setup
 *
 * This interface is provided for driver modules to invoke their respective
 * early initialization methods. It is called early on in the
 * bl31_platform_setup() function.
 *
 * @return None.
 */
void bl31qtilib_bl31_platform_early_setup(void);

/**
 * bl31qtilib_bl31_platform_setup
 *
 * This interface is provided for driver modules to invoke their respective
 * initialization methods. It is called at the end of the
 * bl31_platform_setup() function.
 *
 * @return None.
 */
void bl31qtilib_bl31_platform_setup(void);

/**
 * bl31qtilib_bl31_platform_post_coldboot_setup
 *
 * This interface is provided to perform any platform-specific runtime setup
 * prior to cold boot exit from BL31 (post bl32 coldboot if exists).
 * It is called from the bl31_plat_runtime_setup() function.
 *
 * @return None.
 */
void bl31qtilib_bl31_platform_post_coldboot_setup(void);

/* API's that need to be exposed to TFA */

/**
 * PSCI
 *
 * The PSCI functions are those required by the open-source TF-A platform
 * for complete functionality. These primarily deal with power - such as
 * turning on cores, putting the application processors to sleep, and the
 * like. In addition, there are individual functions which need to be handled
 * on a target-specific level, such as identifying the deepest sleep mode.
 */

/**
 * bl31qtilib_psci_init
 *
 * This function initializes the PSCI library, including setting the location
 * for secondary cores to come up.
 *
 * @param[in] warmboot_entry  Pointer to secondary core function
 * @param[in] mpidr           MPIDR of the current, boot up core
 *
 * @return 0 upon success, negative number to indicate any error
 */
int bl31qtilib_psci_init(uintptr_t warmboot_entry, uint32_t mpidr);

/**
 * bl31qtilib_psci_power_domain_on
 *
 * When PSCI is called to turn on a secondary core, the final
 * platform-specific steps are taken care of with this function. This also
 * ends up making the direct call to any additional subsystems to physically
 * power on the call.
 *
 * @param[in] mpidr   Core which is being powered on
 *
 * @return PSCI_E_SUCCESS on success, error otherwise
 */
int32_t bl31qtilib_psci_power_domain_on(uint32_t mpidr);

/**
 * bl31qtilib_psci_power_domain_on_finish
 *
 * This function is called as part of the power up path for any secondary
 * core, setting any platform-specific registers and ensuring that all
 * subsystems are aligned and ready for this core to be up.
 *
 * @param[in] mpidr       Core which is being powered on
 * @param[in] pwr_states  The low-power modes to unconfigure after turning on
 */
void bl31qtilib_psci_power_domain_on_finish(uint32_t mpidr,
					    const uint8_t *pwr_states);

/**
 * bl31qtilib_psci_suspend
 *
 * This function configures the different platform-specific settings required
 * to put any portion of the application processor subsystem into sleep.
 *
 * @param[in] mpidr       Core which is being put to sleep
 * @param[in] pwr_states  Low-power modes at each level to enter
 *
 * @return PSCI_E_SUCCESS on success, error otherwise
 */
int32_t bl31qtilib_psci_suspend(uint32_t mpidr, const uint8_t *pwr_states);

/**
 * bl31qtilib_psci_resume
 *
 * This function configures the different platform-specific settings that
 * need to be cleared after waking up from a suspend state.
 *
 * @param[in] mpidr       Core which is waking up
 * @param[in] pwr_states  Low-power modes at each level to clear
 *
 * @return PSCI_E_SUCCESS on success, error otherwise
 */
int32_t bl31qtilib_psci_resume(uint32_t mpidr, const uint8_t *pwr_states);

/**
 * bl31qtilib_psci_get_suspend_state
 *
 * This function gets the settings for entering the deepest possible suspend
 * state within the application processor subsystem.
 *
 * @return Bitfield containing the states for each level of subsystem sleep.
 */
uint64_t bl31qtilib_psci_get_suspend_state(void);

/**
 * bl31qtilib_psci_validate_power_state
 *
 * This validates and splits up the power state into the individual LPMs.
 *
 * @param[in]  power_state  Parameter passed in containing all power state info
 * @param[out] req_state    Set of LPMs for each level
 *
 * @return PSCI_E_SUCCESS on success, error otherwise
 */
int32_t bl31qtilib_psci_validate_power_state(unsigned int power_state,
					     uint8_t *req_state);

/**
 * bl31qtilib_psci_get_target_pwr_state
 *
 * Ensure that the requested power state can properly be entered.
 *
 * @param[in] lvl     Current level of sleep
 * @param[in] states  Get the set of sleep states for each node
 * @param[in] ncpu    Number of children nodes at the current level
 * @param[in] mpidr   Current host CPU running aggregation
 *
 * @return The deepest available sleep state based on the request and the
 *         children
 */
uint8_t bl31qtilib_psci_get_target_pwr_state(unsigned int lvl,
					     const uint8_t *states,
					     unsigned int ncpu,
					     uint32_t mpidr);

/**
 * bl31qtilib_psci_cpu_pwr_down
 *
 * Turns off the selected core. This only has functionality related to CPU
 * power down, and not anything related to suspend.
 *
 * @param[in] mpidr   CPU to be powered down
 */
void bl31qtilib_psci_cpu_pwr_down(uint32_t mpidr);

/**
 * bl31qtilib_psci_system_off
 *
 * Powers off the system according to the PSCI spec. Expected to be the
 * last call before powering off.
 */
void bl31qtilib_psci_system_off(void);

/**
 * bl31qtilib_psci_system_reset
 *
 * Resets the system according to the PSCI spec.
 */
void bl31qtilib_psci_system_reset(void);

/**
 * bl31qtilib_psci_system_reset2
 *
 * This resets the chip according to SYSTEM_RESET2 and vendor standards.
 *
 * @param[in] reset_type    Type of reset to enter
 * @param[in] cookie        Cookie to be parsed for reset
 */
void bl31qtilib_psci_system_reset2(int reset_type, uint64_t cookie);

/**
 * bl31qtilib_psci_warm_reset
 *
 * Warm-resets the system based on the feature supported, i.e. warm reset
 * to enter EDL vs warm reset to reboot the device.
 *
 * @return Does not return on success, a negative value on failure.
 */
int bl31qtilib_psci_warm_reset(void);
/* END PSCI */

/**
 * bl31qtilib_get_chip_id
 *
 * This function returns the chip ID associated with the part number read
 * from HW. Will return 0 if no associated chip ID could be found, or if
 * called before ChipInfo is initialized.
 *
 * @return The chip ID or 0
 */
uint32_t bl31qtilib_get_chip_id(void);

/**
 * bl31qtilib_get_soc_revision
 *
 * Returns the version of the chip in the bottom 16 bits as follows:
 *   [31:16] = Zero
 *   [15:8]  = Major Revision
 *   [7:0]   = Minor Revision
 *
 * @return Chip version if successful, 0 if called before ChipInfo is
 *         initialized
 */
uint32_t bl31qtilib_get_soc_revision(void);

/**
 * bl31qtilib_ncc_hwtrace_set_atid
 *
 * Sets the ATID for hardware trace on a given MPIDR.
 *
 * @param[in] mpidr  Multiprocessor Affinity Register value identifying
 *                   the core
 * @param[in] atid   ATID to set for hardware trace. Must be in range
 *                   [0x1, 0x6F]
 *
 * @return 0 on success
 * @return -ENODEV if hardware trace is not supported or core is invalid
 * @return -EINVAL if atid is out of valid range [0x1, 0x6F]
 */
int bl31qtilib_ncc_hwtrace_set_atid(u_register_t mpidr, u_register_t atid);

/**
 * bl31qtilib_ncc_hwtrace_set_enabled
 *
 * Enables or disables hardware trace for a given MPIDR.
 *
 * @param[in] mpidr    Multiprocessor Affinity Register value identifying
 *                     the core
 * @param[in] enabled  true to enable hardware trace, false to disable
 *
 * @return 0 on success
 * @return -ENODEV if hardware trace is not supported or core is invalid
 * @return -EINVAL if no trace options have been configured
 *         (rec_en_mask is zero)
 */
int bl31qtilib_ncc_hwtrace_set_enabled(u_register_t mpidr, bool enabled);

/**
 * bl31qtilib_ncc_hwtrace_set_options
 *
 * Sets hardware trace options for a given MPIDR using a bitmask.
 *
 * @param[in] mpidr    Multiprocessor Affinity Register value identifying
 *                     the core
 * @param[in] options  Bitmask of hardware trace options to enable
 *
 * @return 0 on success
 * @return -ENODEV if hardware trace is not supported or core is invalid
 * @return -EBUSY if hardware trace is already enabled (must be disabled
 *         before changing options)
 * @return -EINVAL if options contains invalid/unsupported bits or if
 *         PC_SRC is requested but not available
 */
int bl31qtilib_ncc_hwtrace_set_options(u_register_t mpidr,
				       u_register_t options);

/**
 * bl31qtilib_ncc_hwtrace_get_features
 *
 * Retrieves the bitmask of supported hardware trace features.
 *
 * @param[out] features  Pointer to store the bitmask of supported features
 *
 * @return 0 on success
 * @return -EFAULT if features pointer is NULL
 * @return -1 if hardware trace is not supported on this platform
 */
int bl31qtilib_ncc_hwtrace_get_features(u_register_t *features);

/**
 * bl31qtilib_config_hw_for_offline_ram_dump
 *
 * Configures hardware for offline RAM dump. This is used to disable RAM
 * dump mode (SDI) when requested.
 *
 * @param[in] disable_wd_dbg      Flag to indicate if watchdog debug should
 *                                be disabled.
 * @param[in] boot_partition_sel  Boot partition selection.
 *
 * @return 0 on success, negative value on failure.
 */
int bl31qtilib_config_hw_for_offline_ram_dump(uint32_t disable_wd_dbg,
					      uint32_t boot_partition_sel);

#if SDEI_SUPPORT
/**
 * Validate a NS address by checking whether it belongs to a Secure region
 * (in which case we return error).
 *
 * @param[in] address  The NS address.
 * @param[in] size     The size of the region to be validated. If it is a
 *                     function pointer, it should be at least 4 (smallest
 *                     size of an instruction)
 *
 * @return 0 on success, error code if the NS address is in a Secure region.
 */
int bl31qtilib_validate_ns_address(uintptr_t address, size_t size);
#endif /* SDEI_SUPPORT */

#endif /* BL31QTILIB_INTERFACE_H */
