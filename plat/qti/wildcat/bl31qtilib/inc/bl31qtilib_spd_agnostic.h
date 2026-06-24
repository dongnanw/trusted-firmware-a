/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31QTILIB_SPD_AGNOSTIC_H
#define BL31QTILIB_SPD_AGNOSTIC_H

#include <stddef.h>
#include <stdint.h>

#include <arch.h>
#include <cdefs.h>

#define BOOT_QSEE_INTERFACE	(1U)

void *bl31qtilib_spd_share_object(uint32_t object_id, void *ptr, size_t size);

__dead2 void bl31qtilib_spd_plat_error_handler(int error);

#endif /* BL31QTILIB_SPD_AGNOSTIC_H */
