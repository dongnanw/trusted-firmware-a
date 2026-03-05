#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
#
QTEED_DIR     := services/spd/qteed
QTI_PLAT_PATH := plat/qti/v2

SPD_INCLUDES  := -I${QTEED_DIR}/inc \
                 -I${QTI_PLAT_PATH}/common/inc

SPD_SOURCES    := ${QTEED_DIR}/src/qteed_main.c

NEED_BL32 := yes

# QTEED will save floating point registers
CTX_INCLUDE_FPREGS := 1

# Enforce Runtime version check between BL31 and libqteed
# If the check fails, libqteed_init will cause bootup to fail
QTEED_ENFORCE_RUNTIME_VERSION_MATCH ?= 0
ifeq (${QTEED_ENFORCE_RUNTIME_VERSION_MATCH},1)
$(eval $(call add_defines, QTEED_ENFORCE_RUNTIME_VERSION_MATCH))
endif

ifeq (${QTI_BL31_WITH_TEST},1)
$(eval $(call add_defines, QTI_BL31_WITH_TEST))
endif

# Enable unshipped debugging
QTEED_DEBUG_PRIV ?= 1
ifeq (${QTEED_DEBUG_PRIV},1)
$(eval $(call add_define,QTEED_DEBUG_PRIV))
endif

# Compile QTEED with FF-A support
ifeq (${QTEED_WITH_FFA_SUPPORT},1)
$(eval $(call add_defines, QTEED_WITH_FFA_SUPPORT))
endif

# Command line flag to enable UART LOG from QTEED
ifeq (${QTEED_UART_LOG},1)
$(eval $(call add_defines, QTEED_UART_LOG))
endif

# Set to 1 to enable lazy saving of NS FP registers
QTEED_LAZY_VFP_ENABLED := 0
ifeq (${QTEED_LAZY_VFP_ENABLED},1)
$(eval $(call add_defines, QTEED_LAZY_VFP_ENABLED))
endif


# QTEEDLIB_PATH should point to either a library (libqteed.a) or
# a Makefile (libqteed.mk)
# First see if it points to a real file
ifeq ($(wildcard ${QTEEDLIB_PATH}),)
    $(info QTEEDLIB_PATH (${QTEEDLIB_PATH}) is not valid)
endif

# Look for a library or a Makefile
ifneq ($(findstring libqteed.a, ${QTEEDLIB_PATH}),)
    $(info Linking libqteed.a at ${QTEEDLIB_PATH})
    # Found a static library, so setup the linker flags
    LDFLAGS += -L $(dir $(QTEEDLIB_PATH))
    LDLIBS += -lqteed
else
    ifneq ($(findstring libqteed.mk, ${QTEEDLIB_PATH}),)
        # Found a Makefile, so include it here to build libqteed.a
        # libqteed.mk will take care of the linker flags
        $(info Including ${QTEEDLIB_PATH})
        include ${QTEEDLIB_PATH}
    else
        # Found neither libqteed.a or libqteed.mk, error
        $(info QTEEDLIB_PATH (${QTEEDLIB_PATH}) is not libqteed.mk or libqteed.a)
        SPD_SOURCES    := ${QTEED_DIR}/src/qteed_stub.c
    endif
endif
