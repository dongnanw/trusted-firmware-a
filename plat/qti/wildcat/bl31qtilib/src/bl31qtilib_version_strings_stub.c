/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Fallback/default version strings
 *
 * These are weak symbols that will be used if the version_builder.py
 * script does not generate the actual version strings. This allows
 * the build to succeed even without the generated version files.
 *
 * If version_builder.py generates qc_version.c and oem_version.c,
 * those strong symbols will override these weak defaults.
 */

__attribute__((weak, used, section(".rodata")))
char QC_IMAGE_VERSION_STRING_AUTO_UPDATED[] =
	"QC_IMAGE_VERSION_STRING=UNKNOWN";

__attribute__((weak, used, section(".rodata")))
char IMAGE_VARIANT_STRING_AUTO_UPDATED[] =
	"IMAGE_VARIANT_STRING=UNKNOWN";

__attribute__((weak, used, section(".rodata")))
char OEM_IMAGE_VERSION_STRING_AUTO_UPDATED[] =
	"OEM_IMAGE_VERSION_STRING=UNKNOWN";

__attribute__((weak, used, section(".rodata")))
char OEM_IMAGE_UUID_STRING_AUTO_UPDATED[] =
	"OEM_IMAGE_UUID_STRING=UNKNOWN";

__attribute__((weak, used, section(".rodata")))
char OEM_HOST_TIMESTAMP_STRING_AUTO_UPDATED[] =
	"OEM_HOST_TIMESTAMP_STRING=UNKNOWN";
