/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	bl31qtilib_spd_panic_handler
	.extern	plat_set_my_stack
	.extern	bl31qtilib_panic

func bl31qtilib_spd_panic_handler
	msr	spsel, #0
	bl	plat_set_my_stack
	no_ret	bl31qtilib_panic
endfunc bl31qtilib_spd_panic_handler
