/*
 * Copyright (c) 2025 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_DRV_QTIMER_H
#define QTI_DRV_QTIMER_H

#include <stdint.h>

uint64_t qti_qtimer_get_usec(void);
uint64_t qti_qtimer_get_raw(void);
void qti_qtimer_init(void);

#endif /* QTI_DRV_QTIMER_H */
