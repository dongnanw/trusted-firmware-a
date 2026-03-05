/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <stddef.h>
#include <stdint.h>

#include "libqteed.h"

#include <common/debug.h>

int qteed_register_qtee_isr(uint32_t intnum, const char *int_desc,
							void *(*fn)(void *), void *ctx, uint32_t flags, bool enable)
{
    return 0;
}

int qteed_disable_qtee_int(uint32_t intnum)
{
    return 0;
}

void *qteed_copy_obj_to_shared_data(uint32_t objectID, void *ptr, size_t size)
{
    return NULL;
}

__dead2 void qteed_error_handler(int error)
{
    panic();
}

void qteed_set_err_fatal_qtee(uint32_t err)
{

}

void qteed_set_err_fatal_with_cond_qtee(uint32_t err, bool return_to_hlos)
{

}

uintptr_t qteed_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
							u_register_t x3, u_register_t x4, void *cookie,
							void *handle, u_register_t flags)
{
    return 0;
}

int32_t libqteed_setup(struct libqteed_init_param *init_params,
						size_t init_params_size)
{
    ERROR("Please use QTEEDLIB_PATH while building TF-A\n");
    return 0;
}

uint64_t qteed_spmd_ffa_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
									uint64_t x3, uint64_t x4, void *cookie,
									void *handle, uint64_t flags)
{
    return 0;
}

__dead2 void qteed_panic_handler(void)
{
    panic();
}

