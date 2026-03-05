/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTEE_TFA_BL31_SH_MEM_H
#define QTEE_TFA_BL31_SH_MEM_H

#include <stdint.h>

#define TFA_BL31_MAX_SHARED_OBJECTS		    15
#define TFA_BL31_SHARED_MEMORY_MAGIC 	    0x5348415245444D59ULL
#define TFA_BL31_SHARED_MEMORY_VERSION 	    0x1

#define BOOT_QSEE_INTERFACE		0x1
#define FATAL_ISR_TABLE		    0x2
#define PLATFORM_INFO		    0x3
#define CHIP_INFO_VERSION		0x4
#define CHIP_INFO_FAMILY		0x5
#define CHIP_INFO_ID		    0x6

typedef struct {
    uint64_t magic;             // Used to check if shared data is in use
    uint64_t version;           // Version check to avoid mismatches
    uintptr_t start_address;    // Where the data starts
    uint64_t size;              // Size of the shared data
    uint32_t num_objs;          // Number of shared objects
} __attribute__((aligned(8))) SharedQTEEData_header_t;

typedef struct {
    uint32_t objectID;      // Unique ID defined in this header
    uint32_t size;          // Size of this object
    uint64_t offset;        // Offset of this object from the start_address
}  SharedQTEEData_object_t;

typedef struct {
	SharedQTEEData_header_t header;
	SharedQTEEData_object_t object_list[TFA_BL31_MAX_SHARED_OBJECTS];
} __attribute__((aligned(8))) SharedQTEEData_t;

#endif /* QTEE_TFA_BL31_SH_MEM_H */
