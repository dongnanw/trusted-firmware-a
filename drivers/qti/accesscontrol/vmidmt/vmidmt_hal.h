/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VMIDMT_HAL_H_
#define _VMIDMT_HAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/utils_def.h>

#define VMIDMT_INTERFACE_V1			0

#define HAL_VMIDMT_MAX_VMID			31
#define HAL_VMIDMT_SECURE_EXT_DEFAULT		0xFF
#define HAL_VMIDMT_ALL_ERROR_STATUS_BITS	0x7FFFFFFE
#define HAL_VMIDMT_VMID_ENABLE_BIT		1
#define HAL_VMIDMT_PERM_MASK			0x1
#define HAL_VMIDMT_PERM_WIDTH			1
#define HAL_VMIDMT_MASKS_PER_FIELD		32
#define HAL_VMIDMT_PERMS_PER_FIELD		32

#define HAL_VMIDMT_MASTER_MASK_WORDS \
	((HAL_VMIDMT_MAX_VMID / HAL_VMIDMT_MASKS_PER_FIELD) + 1)

#define HAL_VMIDMT_MASTER_PERM_WORDS \
	((HAL_VMIDMT_MAX_VMID / HAL_VMIDMT_PERMS_PER_FIELD) + 1)

typedef enum {
	HAL_VMIDMT_NO_ERROR,             /* successful operation */
	HAL_VMIDMT_INVALID_INSTANCE,     /* out of range VMIDMT instance */
	HAL_VMIDMT_UNSUPPORTED_INSTANCE_FOR_TARGET,
	/* unsupported VMIDMT instance for the current target */
	HAL_VMIDMT_UNSUPPORTED_HANDLER,  /* unsupported handler */
	HAL_VMIDMT_INVALID_BASE_ADDR,    /* invalid VMIDMT instance base address */
	HAL_VMIDMT_INVALID_PARAM,        /* invalid passed in parameter */
	HAL_VMIDMT_INVALID_HW_VALUE,     /* invalid HW generic value */
	HAL_VMIDMT_READ_WRITE_MISMATCH   /* read-after-write values not match */
} HAL_vmidmt_Status;

typedef enum {
	HAL_VMIDMT_CRYPTO0_AXI = 0,     /* CRYPTO AXI - 0 */
	HAL_VMIDMT_CRYPTO1_AXI,         /* CRYPTO AXI - 1 */
	HAL_VMIDMT_CRYPTO0_BAM,         /* CRYPTO BAM - 0 */
	HAL_VMIDMT_CRYPTO0_CRYPTO = HAL_VMIDMT_CRYPTO0_BAM,
	HAL_VMIDMT_CRYPTO1_BAM,         /* CRYPTO BAM - 1 */
	HAL_VMIDMT_DEHR,                /* DEHR */
	HAL_VMIDMT_DEHR_BIMC_WRAPPER = HAL_VMIDMT_DEHR,
	HAL_VMIDMT_LPASS_DM,            /* LPASS DM */
	HAL_VMIDMT_LPASS_LPAIF,         /* LPASS LPAIF */
	HAL_VMIDMT_LPASS_MIDI,          /* LPASS MIDI */
	HAL_VMIDMT_LPASS_Q6AHB,         /* LPASS Q6AHB */
	HAL_VMIDMT_LPASS_Q6AXI,         /* LPASS Q6AXI */
	HAL_VMIDMT_LPASS_RESAMPLER,     /* LPASS Resampler */
	HAL_VMIDMT_LPASS_SLIMBUS,       /* LPASS Slimbus */
	HAL_VMIDMT_CAMERA_SS,           /* CAMERA Subsystem */
	HAL_VMIDMT_CAMSS_VBIF_JPEG,     /* CAMERA VBIF JPEG */
	HAL_VMIDMT_CAMSS_VBIF_VFE,      /* CAMERA VBIF VFE */
	HAL_VMIDMT_MMSS_DDR,            /* MMSS DDR */
	HAL_VMIDMT_OCMEM,               /* OCMEM */
	HAL_VMIDMT_OXILI,               /* OXILI */
	HAL_VMIDMT_VENUS_CPUSS,         /* Venus CPU SS */
	HAL_VMIDMT_VENUS_VBIF,          /* Venus VBIF */
	HAL_VMIDMT_MSS_A2BAM,           /* MSS A2BAM */
	HAL_VMIDMT_MSS_NAV_CE,          /* MSS NAV CE */
	HAL_VMIDMT_MSS_Q6,              /* MSS Q6 */
	HAL_VMIDMT_BAM_DMA,             /* BAM DMA */
	HAL_VMIDMT_BAM_BLSP1_DMA,       /* BAM BLSP DMA - 1 */
	HAL_VMIDMT_BLSP1_BLSP_BAM = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_QUPV3_0 = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_QUPV3_WEST = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_BAM_BLSP2_DMA,       /* BAM BLSP DMA - 2 */
	HAL_VMIDMT_BLSP2_BLSP_BAM = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_QUPV3_1 = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_QUPV3_EAST1 = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_BAM_SDCC1,           /* BAM SDCC1 */
	HAL_VMIDMT_BAM_SDCC2,           /* BAM SDCC2 */
	HAL_VMIDMT_BAM_SDCC3,           /* BAM SDCC3 */
	HAL_VMIDMT_BAM_SDCC4,           /* BAM SDCC4 */
	HAL_VMIDMT_SDC_SDCC4_BAM = HAL_VMIDMT_BAM_SDCC4,
	HAL_VMIDMT_TSIF,                /* TSIF */
	HAL_VMIDMT_USB1_HS,             /* BAM USB OTG */
	HAL_VMIDMT_USB2_HSIC,           /* USB OTG */
	HAL_VMIDMT_QDSS_VMIDDAP,        /* QDSS VMIDDAP */
	HAL_VMIDMT_QDSS_VMIDETR,        /* QDSS VMIDETR */
	HAL_VMIDMT_RPM_MSGRAM,          /* RPM MSGRAM */
	HAL_VMIDMT_RPM = HAL_VMIDMT_RPM_MSGRAM,
	HAL_VMIDMT_AOP = HAL_VMIDMT_RPM_MSGRAM,
	HAL_VMIDMT_SPDM_WRAPPER,        /* SPDM TOP */
	HAL_VMIDMT_USB30,               /* USB30 */
	HAL_VMIDMT_PRONTO,              /* PRONTO */
	HAL_VMIDMT_QPIC_BAM,            /* QPIC BAM */
	HAL_VMIDMT_QPIC = HAL_VMIDMT_QPIC_BAM,
	HAL_VMIDMT_IPA,                 /* IPA */
	HAL_VMIDMT_IPA_0_IPA = HAL_VMIDMT_IPA,
	HAL_VMIDMT_IPA_WRAPPER = HAL_VMIDMT_IPA,
	HAL_VMIDMT_APCS,                /* APCS */
	HAL_VMIDMT_CRYPTO2_AXI,         /* CRYPTO AXI - 2 */
	HAL_VMIDMT_CRYPTO2_BAM,         /* CRYPTO BAM - 2 */
	HAL_VMIDMT_EMAC,
	HAL_VMIDMT_LPASS_HDMI,
	HAL_VMIDMT_LPASS_SPDIF,
	HAL_VMIDMT_VENUS_VBIF2,
	HAL_VMIDMT_MMSS_VPU_MAPLE,
	HAL_VMIDMT_USB_HS_SEC,
	HAL_VMIDMT_SATA,
	HAL_VMIDMT_PCIE20,
	HAL_VMIDMT_PCIE_0_PCIE20 = HAL_VMIDMT_PCIE20,
	HAL_VMIDMT_PCIE0,
	HAL_VMIDMT_PCIE1,
	HAL_VMIDMT_USB3_HSIC,
	HAL_VMIDMT_UFS,
	HAL_VMIDMT_LPASS_SB1,
	HAL_VMIDMT_SSC_BLSP_BAM,
	HAL_VMIDMT_SSC_SSC_BLSP_BAM = HAL_VMIDMT_SSC_BLSP_BAM,
	HAL_VMIDMT_SSC_QUPV3 = HAL_VMIDMT_SSC_BLSP_BAM,
	HAL_VMIDMT_LPASS_SENSOR_BLSP_BAM,
	HAL_VMIDMT_SSC_SDC,
	HAL_VMIDMT_LPASS_SSC_SDC,
	HAL_VMIDMT_SPMI,
	HAL_VMIDMT_SPMI_FETCHER = HAL_VMIDMT_SPMI,
	HAL_VMIDMT_BAM_BLSP3_DMA,
	HAL_VMIDMT_QUPV3_SOUTH = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_QUPV3_NORTH = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_QUPV3_2 = HAL_VMIDMT_BAM_BLSP3_DMA,
	HAL_VMIDMT_QUPV3_EAST = HAL_VMIDMT_BAM_BLSP3_DMA,
	HAL_VMIDMT_QSPI,
	HAL_VMIDMT_LPASS_RXTX,
	HAL_VMIDMT_LPASS_WSA,
	HAL_VMIDMT_LPASS_VA,
	HAL_VMIDMT_MSS_NAV,
	HAL_VMIDMT_PCNOC_SNOC1,
	HAL_VMIDMT_PCNOC_SNOC2,
	HAL_VMIDMT_PCNOC_SNOC3,
	HAL_VMIDMT_PCNOC_SNOC4,
	HAL_VMIDMT_DEHR_BIMC,
	HAL_VMIDMT_COUNT,
} HAL_vmidmt_InstanceType;

typedef struct {
	uint32_t auVMID[HAL_VMIDMT_MASTER_MASK_WORDS];      /* VMID to mask */
	uint32_t auVMIDPerm[HAL_VMIDMT_MASTER_PERM_WORDS];  /* VMID permissions */
} HAL_vmidmt_AccessConfigType;

typedef enum {
	HAL_VMIDMT_ERROR_O_SMCFCFG_EN = 0x1,  /* stream match conflict fault */
	HAL_VMIDMT_ERROR_O_USFCFG_EN  = 0x2,  /* unidentified stream fault */
	HAL_VMIDMT_ERROR_O_GCFGFIE    = 0x4,  /* interrupt on global config fault */
	HAL_VMIDMT_ERROR_O_GCFGFRE    = 0x8,  /* report fault to offending master */
	HAL_VMIDMT_ERROR_O_GFIE       = 0x10, /* interrupt on global client fault */
} HAL_vmidmt_ErrorOptionType;

/* Store error reporting options ORed in this */
typedef uint32_t HAL_vmidmt_ErrorOptionConfigType;

typedef struct {
	uint16_t uEntryCount;      /* Total number of table entries */
	uint16_t uNumVmid;         /* number of VMID supported by the core */
	uint8_t  bStreamMapSupport; /* Is stream mapping mode supported? */
	uint8_t  uNumSSDIndexBits; /* number of SSD index bits */
	uint8_t  uNumStreamIDBits; /* number of SID bits available */
} HAL_vmidmt_DeviceParamsType;

typedef enum {
	HAL_VMIDMT_ERROR_F_CLMULTI  = 0x1,  /* multiple error; 2nd is client */
	HAL_VMIDMT_ERROR_F_CFGMULTI = 0x2,  /* multiple error; 2nd is config */
	HAL_VMIDMT_ERROR_F_PF       = 0x4,  /* permission fault */
	HAL_VMIDMT_ERROR_F_CAF      = 0x8,  /* configuration access fault */
	HAL_VMIDMT_ERROR_F_SMCF     = 0x10, /* stream match conflict fault */
	HAL_VMIDMT_ERROR_F_USF      = 0x20, /* unidentified stream fault */
} HAL_vmidmt_ErrorFlagsType;

/* Store error flags ORed in this */
typedef uint32_t HAL_vmidmt_ErrorFlagsConfigType;

typedef enum {
	HAL_VMIDMT_BUS_F_ERROR_NSATTR  = 0x1, /* non-secure attribute */
	HAL_VMIDMT_BUS_F_ERROR_NSSTATE = 0x2, /* non-secure state */
	HAL_VMIDMT_BUS_F_ERROR_WNR     = 0x4, /* write not read */
} HAL_vmidmt_BusErrorFlagsType;

/* Store bus flags ORed in this */
typedef uint32_t HAL_vmidmt_BusErrorFlagsMaskType;

typedef struct {
	HAL_vmidmt_ErrorFlagsConfigType  uErrorFlags;  /* Error Flags */
	HAL_vmidmt_BusErrorFlagsMaskType uBusFlags;    /* Bus specific flags */
	uint32_t uPhysicalAddressLower32;              /* Lower 32 bits of PA */
	uint32_t uPhysicalAddressUpper32;              /* Upper 32 bits of PA */
	uint32_t uSSDIndex;                            /* SSD index */
	uint32_t uSID;                                 /* Stream ID */
	uint32_t uMasterId;                            /* Master ID */
	uint32_t uAVMID;                               /* Virtual master ID */
	uint32_t uATID;                                /* ATID */
	uint32_t uABID;                                /* ABID */
	uint32_t uAPID;                                /* APID */
} HAL_vmidmt_ErrorType;

typedef uint32_t HAL_vmidmt_SecureStatusDetType;

typedef enum {
	HAL_VMIDMT_NSCFG_XTRAN      = 0, /* use xPROTNS from transaction */
	HAL_VMIDMT_NSCFG_SECURE     = 2, /* secure */
	HAL_VMIDMT_NSCFG_NONSECURE  = 3, /* non-secure */
	HAL_VMIDMT_NSCFG_DEFAULT    = 4  /* use default */
} HAL_vmidmt_NonSecureAllocConfigType;

typedef enum {
	HAL_VMIDMT_WACFG_XTRAN   = 0, /* use attributes from transaction */
	HAL_VMIDMT_WACFG_ALLOC   = 2, /* allocate */
	HAL_VMIDMT_WACFG_NONALLOC = 3, /* non-allocate */
	HAL_VMIDMT_WACFG_DEFAULT = 4  /* use default */
} HAL_vmidmt_WriteAllocConfigType;

typedef enum {
	HAL_VMIDMT_RACFG_XTRAN   = 0, /* use attributes from transaction */
	HAL_VMIDMT_RACFG_ALLOC   = 2, /* allocate */
	HAL_VMIDMT_RACFG_NONALLOC = 3, /* non-allocate */
	HAL_VMIDMT_RACFG_DEFAULT = 4  /* use default */
} HAL_vmidmt_ReadAllocConfigType;

typedef enum {
	HAL_VMIDMT_SHCFG_XTRAN       = 0, /* use attributes from transaction */
	HAL_VMIDMT_SHCFG_OUTER_SHARE = 1, /* outer-shareable */
	HAL_VMIDMT_SHCFG_INNER_SHARE = 2, /* inner-shareable */
	HAL_VMIDMT_SHCFG_NON_SHARE   = 3, /* non-shareable */
	HAL_VMIDMT_SHCFG_DEFAULT     = 4  /* use default */
} HAL_vmidmt_SharedConfigType;

typedef enum {
	HAL_VMIDMT_MTCFG_XTRAN   = 0, /* use attributes from transaction */
	HAL_VMIDMT_MTCFG_MEMATTR = 1, /* use MemAttr field */
	HAL_VMIDMT_MTCFG_DEFAULT = 2  /* use default */
} HAL_vmidmt_MemTypeConfigType;

typedef enum {
	HAL_VMIDMT_TRANSIENTCFG_XTRAN          = 0, /* use transaction attr */
	HAL_VMIDMT_TRANSIENTCFG_NON_TRANSIENT  = 2, /* non-transient */
	HAL_VMIDMT_TRANSIENTCFG_TRANSIENT      = 3, /* transient */
	HAL_VMIDMT_TRANSIENTCFG_DEFAULT        = 4  /* use default */
} HAL_vmidmt_TransientConfigType;

typedef struct {
	HAL_vmidmt_NonSecureAllocConfigType eNSCFG; /* non-secure allocate */
	HAL_vmidmt_WriteAllocConfigType     eWACFG; /* write allocate */
	HAL_vmidmt_ReadAllocConfigType      eRACFG; /* read allocate */
	HAL_vmidmt_SharedConfigType         eSHCFG; /* shareable attribute */
	HAL_vmidmt_MemTypeConfigType        eMTCFG; /* memory type config */
	uint8_t                             uMemAttr; /* memory attributes (3b) */
	HAL_vmidmt_TransientConfigType      eTransientCfg; /* transient config */
} HAL_vmidmt_BusAttribType;

typedef enum {
	HAL_VMIDMT_RCNSH_DISABLE = 0, /* do not redirect */
	HAL_VMIDMT_RCNSH_ENABLE  = 1, /* enable redirect */
	HAL_VMIDMT_RCNSH_DEFAULT = 2  /* use default */
} HAL_vmidmt_RedirCacheNonShareableConfigType;

typedef enum {
	HAL_VMIDMT_RCISH_DISABLE = 0, /* do not redirect */
	HAL_VMIDMT_RCISH_ENABLE  = 1, /* enable redirect */
	HAL_VMIDMT_RCISH_DEFAULT = 2  /* use default */
} HAL_vmidmt_RedirCacheInnerShareableConfigType;

typedef enum {
	HAL_VMIDMT_RCOSH_DISABLE = 0, /* do not redirect */
	HAL_VMIDMT_RCOSH_ENABLE  = 1, /* enable redirect */
	HAL_VMIDMT_RCOSH_DEFAULT = 2  /* use default */
} HAL_vmidmt_RedirCacheOuterShareableConfigType;

typedef enum {
	HAL_VMIDMT_REQPRICFG_XTRAN     = 0, /* use transaction attr */
	HAL_VMIDMT_REQPRICFG_ACR_REQPRI = 1, /* use (S)ACR.REQPRIORITY override */
	HAL_VMIDMT_REQPRICFG_DEFAULT   = 2  /* use default */
} HAL_vmidmt_ReqPriorityConfigType;

typedef enum {
	HAL_VMIDMT_REQPRI_NORMAL  = 0, /* normal priority */
	HAL_VMIDMT_REQPRI_HIGH    = 1, /* high priority */
	HAL_VMIDMT_REQPRI_HIGHER  = 2, /* higher priority */
	HAL_VMIDMT_REQPRI_HIGHEST = 3, /* highest priority */
	HAL_VMIDMT_REQPRI_DEFAULT = 4  /* use default */
} HAL_vmidmt_ReqPriorityType;

typedef struct {
	HAL_vmidmt_RedirCacheNonShareableConfigType eRCNSH; /* redirect C-NSH */
	HAL_vmidmt_RedirCacheInnerShareableConfigType eRCISH; /* redirect C-ISH */
	HAL_vmidmt_RedirCacheOuterShareableConfigType eRCOSH; /* redirect C-OSH */
	HAL_vmidmt_ReqPriorityConfigType eReqPriorityCfg;     /* req pri cfg */
	HAL_vmidmt_ReqPriorityType       eReqPriority;        /* req priority */
} HAL_vmidmt_AuxConfigType;

typedef struct {
	HAL_vmidmt_BusAttribType    *pBypassBusAttrib;  /* bypass bus attributes */
	HAL_vmidmt_AuxConfigType    *pBypassAuxConfig;  /* bypass aux config */
	HAL_vmidmt_AccessConfigType *pAccessControl;    /* RPU access control */
	bool                         bVmidPrivateNamespaceEnable; /* private NS */
	uint8_t                      bypassVmid;        /* bypass VMID */
} HAL_vmidmt_DefaultVmidConfigType;

typedef struct {
	HAL_vmidmt_DefaultVmidConfigType *pDefaultSecureConfig; /* default cfg */
	bool                               bGlbAddrSpaceRestrictedAccEnable;
	/* global address space restricted access enable */
	uint8_t                            secureExtensions;
	/* non-secure number of stream mapping register groups override
	 * HAL_VMIDMT_SECURE_EXT_DEFAULT can be used for default setting
	 */
} HAL_vmidmt_DefaultSecureVmidConfigType;

typedef struct {
	HAL_vmidmt_BusAttribType *pBusAttrib;  /* config S2VRn */
	HAL_vmidmt_AuxConfigType *pAuxConfig;  /* config AS2VRn */
	uint8_t                   uVmid;       /* config VMID in S2VRn */
} HAL_vmidmt_ContextConfigType;

typedef enum {
	HAL_VMIDMT_INT_NO_ERROR,
	HAL_VMIDMT_INT_PROPERTY_HANDLE_ERROR,
	HAL_VMIDMT_INT_UNSUP_INST_FOR_TARGET,
} HAL_vmidmt_IntStatus;

typedef enum {
	HAL_VMIDMT_NO_ACCESS,
	HAL_VMIDMT_FULL_ACCESS,
	HAL_VMIDMT_ACCESS_COUNT,
} HAL_vmidmt_AccessType;

typedef struct {
	uint16_t uEntryCount;
	uint16_t uNumVmid;
	uint8_t uNumSSDIndexBits;
	uint8_t uNumStreamIDBits;
	uint8_t uInputAddrSize;
	uint8_t bStreamMatchSupport;
} HAL_vmidmt_IntVMIDMTDevParamsType;

typedef struct {
	uint64_t uBaseAddr;
	HAL_vmidmt_IntVMIDMTDevParamsType  devParams;
} HAL_vmidmt_InfoType;

typedef struct {
	uint8_t port;
	HAL_vmidmt_InfoType vmidmt_info;
} HAL_vmidmt_Port_MapType;

/* Initialize VMIDMT */
HAL_vmidmt_Status vmidmt_hal_init(
	HAL_vmidmt_InfoType *info,
	const HAL_vmidmt_DefaultSecureVmidConfigType *secure_def,
	const HAL_vmidmt_DefaultVmidConfigType *defcfg,
	char **err_str
);

/* Enable or disable client interface. */
HAL_vmidmt_Status vmidmt_hal_enable_client(
	const HAL_vmidmt_InfoType *info,
	bool enable
);

/* Configure Secure Status Determination Table (SSDT). */
HAL_vmidmt_Status vmidmt_hal_config_ssdt(
	const HAL_vmidmt_InfoType *info,
	const HAL_vmidmt_SecureStatusDetType *ssdt,
	uint32_t count
);

/* Configure a single context entry. */
HAL_vmidmt_Status vmidmt_hal_config_ctx(
	const HAL_vmidmt_InfoType *info,
	uint32_t index,
	const HAL_vmidmt_ContextConfigType *cfg
);

/* Configure multiple context entries (extended). */
HAL_vmidmt_Status vmidmt_hal_config_ctx_ext(
	const HAL_vmidmt_InfoType *info,
	uint32_t first_index,
	const uint32_t *index_list,
	uint32_t list_len,
	const HAL_vmidmt_ContextConfigType *cfg
);

/* Configure error reporting/handling options. */
HAL_vmidmt_Status vmidmt_hal_cfg_err(
	const HAL_vmidmt_InfoType *info,
	bool global_cfg,
	HAL_vmidmt_ErrorOptionConfigType opts
);

/* Check if an error is latched. */
bool vmidmt_hal_is_error(
	const HAL_vmidmt_InfoType *info,
	bool global_err
);

/* Read back the latched error record. */
void vmidmt_hal_get_error(
	const HAL_vmidmt_InfoType *info,
	bool global_err,
	HAL_vmidmt_ErrorType *err
);

/* Clear latched error(s). */
HAL_vmidmt_Status vmidmt_hal_clear_error(
	const HAL_vmidmt_InfoType *info,
	bool global_err
);

#endif /* _VMIDMT_HAL_H_ */

