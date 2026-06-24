/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/common/platform.h>

#include <bl31qtilib_spd_agnostic.h>

void *bl31qtilib_spd_share_object(uint32_t object_id, void *ptr, size_t size)
{
	(void)object_id;
	(void)ptr;
	(void)size;

	return NULL;
}

__dead2 void bl31qtilib_spd_plat_error_handler(int error)
{
	(void)error;

	panic();
}
