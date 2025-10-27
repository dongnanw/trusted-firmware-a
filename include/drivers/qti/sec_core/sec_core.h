/*
 * Copyright (c) 2025 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_DRV_SEC_CORE_H
#define QTI_DRV_SEC_CORE_H

#include <stdint.h>

void qti_sec_core_init(void);
void qti_sec_core_remap(uintptr_t entrypoint);

#endif /* QTI_DRV_SEC_CORE_H */
