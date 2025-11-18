/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/utils_def.h>
#include <vmidmt_hal.h>
#include <vmidmt_hal_hwio.h>

#define HAL_VMIDMT_DEFAULT_CR0  0x00000111
#define HAL_VMIDMT_DEFAULT_CR2  0
#define HAL_VMIDMT_DEFAULT_ACR  0
#define HAL_VMIDMT_DEFAULT_S2VR 0x00010000
#define HAL_VMIDMT_MAX_SSD 4

#define VMIDMT_ALL_VMID_MASK  0xFFFFFFFF
#define VMIDMT_NUM_NSSTATE_BITS  32
#define VMIDMT_MEMATTR_MAX_BITS  3

#define IS_NUM_SSD_VALID(uNumSSD) (uNumSSD <= HAL_VMIDMT_MAX_SSD)

#define ALL_ERROR_OPTIONS (HAL_VMIDMT_ERROR_O_SMCFCFG_EN | \
		HAL_VMIDMT_ERROR_O_USFCFG_EN | \
		HAL_VMIDMT_ERROR_O_GCFGFIE | \
		HAL_VMIDMT_ERROR_O_GCFGFRE | \
		HAL_VMIDMT_ERROR_O_GFIE)

#define CR0_ERE_MASK (VMIDMT_FMSK(CR0, SMCFCFG) | \
		VMIDMT_FMSK(CR0, USFCFG) | \
		VMIDMT_FMSK(CR0, GCFGFIE) | \
		VMIDMT_FMSK(CR0, GCFGFRE) | \
		VMIDMT_FMSK(CR0, GFIE))

#define CR0_BUS_MASK (VMIDMT_FMSK(CR0, WACFG) | \
		VMIDMT_FMSK(CR0, RACFG) | \
		VMIDMT_FMSK(CR0, SHCFG) | \
		VMIDMT_FMSK(CR0, MTCFG) | \
		VMIDMT_FMSK(CR0, MEMATTR) | \
		VMIDMT_FMSK(CR0, TRANSIENTCFG))

static void read_id_registers(HAL_vmidmt_InfoType *info)
{
	uint32_t reg;

	reg = VMIDMT_IN(info->uBaseAddr, IDR0);
	info->devParams.bStreamMatchSupport = VMIDMT_INFC(reg, IDR0, SMS);
	info->devParams.uNumStreamIDBits = VMIDMT_INFC(reg, IDR0, NUMSIDB);
	info->devParams.uEntryCount = VMIDMT_INFC(reg, IDR0, NUMSMRG);
	reg = VMIDMT_IN(info->uBaseAddr, IDR1);
	info->devParams.uNumSSDIndexBits = VMIDMT_INFC(reg, IDR1, NUMSSDNDXB);
	reg = VMIDMT_IN(info->uBaseAddr, IDR2);
	info->devParams.uInputAddrSize = VMIDMT_INFC(reg, IDR2, IAS);
	reg = VMIDMT_IN(info->uBaseAddr, IDR5);
	info->devParams.uNumVmid = VMIDMT_INFC(reg, IDR5, NVMID);
}

static inline
HAL_vmidmt_AccessType get_permissions(const HAL_vmidmt_AccessConfigType *cfg,
				      uint32_t vmid)
{
	uint32_t field = vmid % HAL_VMIDMT_PERMS_PER_FIELD;
	uint32_t word = vmid / HAL_VMIDMT_PERMS_PER_FIELD;
	uint32_t shift = field * HAL_VMIDMT_PERM_WIDTH;
	uint32_t perm = cfg->auVMIDPerm[word];

	return (HAL_vmidmt_AccessType)((perm >> shift) & HAL_VMIDMT_PERM_MASK);
}

static inline bool mask_bit_set(const HAL_vmidmt_AccessConfigType *cfg,
				uint32_t vmid)
{
	uint32_t field = vmid % HAL_VMIDMT_MASKS_PER_FIELD;
	uint32_t word = vmid / HAL_VMIDMT_MASKS_PER_FIELD;
	uint32_t bit = field * HAL_VMIDMT_PERM_WIDTH;
	uint32_t mask = (uint32_t)BIT(bit);

	return (cfg->auVMID[word] & mask) != 0U;
}

static HAL_vmidmt_Status configure_acr(uint64_t addr,
				       HAL_vmidmt_AccessConfigType *cfg)
{
	HAL_vmidmt_AccessType perm;
	uint32_t acr_val = 0;
	uint32_t acr = 0;

	for (size_t vmid = 0; vmid <= HAL_VMIDMT_MAX_VMID; vmid++) {
		if (!mask_bit_set(cfg, vmid))
			continue;

		/* only set bits for full access masters */
		perm = get_permissions(cfg, vmid);
		if (perm == HAL_VMIDMT_FULL_ACCESS)
			acr |= 1 << vmid;
	}

	VMIDMT_OUTM(addr, VMIDMTACR, cfg->auVMID[0], acr);
	acr_val = VMIDMT_INM(addr, VMIDMTACR, cfg->auVMID[0]);
	if (acr_val != acr)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static uint32_t set_default_config_bus(HAL_vmidmt_BusAttribType *bus)
{
	uint32_t attrib = 0;

	if (bus->eWACFG < HAL_VMIDMT_WACFG_DEFAULT)
		attrib |= bus->eWACFG << VMIDMT_SHFT(CR0, WACFG);

	if (bus->eRACFG < HAL_VMIDMT_RACFG_DEFAULT)
		attrib |= bus->eRACFG << VMIDMT_SHFT(CR0, RACFG);

	if (bus->eSHCFG < HAL_VMIDMT_SHCFG_DEFAULT)
		attrib |= bus->eSHCFG << VMIDMT_SHFT(CR0, SHCFG);

	if (bus->eMTCFG < HAL_VMIDMT_MTCFG_DEFAULT)
		attrib |= bus->eMTCFG << VMIDMT_SHFT(CR0, MTCFG);

	attrib |= bus->uMemAttr << VMIDMT_SHFT(CR0, MEMATTR);

	if (bus->eTransientCfg < HAL_VMIDMT_TRANSIENTCFG_DEFAULT)
		attrib |= bus->eTransientCfg << VMIDMT_SHFT(CR0, TRANSIENTCFG);

	return attrib;
}

static uint32_t set_default_config_aux(HAL_vmidmt_AuxConfigType *aux)
{
	uint32_t config = 0;

	if (aux->eRCNSH < HAL_VMIDMT_RCNSH_DEFAULT)
		config |= aux->eRCNSH << VMIDMT_SHFT(ACR, BPRCNSH);

	if (aux->eRCISH < HAL_VMIDMT_RCISH_DEFAULT)
		config |= aux->eRCISH << VMIDMT_SHFT(ACR, BPRCISH);

	if (aux->eRCOSH < HAL_VMIDMT_RCOSH_DEFAULT)
		config |= aux->eRCOSH << VMIDMT_SHFT(ACR, BPRCOSH);

	if (aux->eReqPriorityCfg < HAL_VMIDMT_REQPRICFG_DEFAULT)
		config |=
		aux->eReqPriorityCfg << VMIDMT_SHFT(ACR, REQPRIORITYCFG);

	if (aux->eReqPriority < HAL_VMIDMT_REQPRI_DEFAULT)
		config |= aux->eReqPriority << VMIDMT_SHFT(ACR, REQPRIORITY);

	return config;
}


static HAL_vmidmt_Status set_default_config(const HAL_vmidmt_InfoType *p,
				    const HAL_vmidmt_DefaultVmidConfigType *cfg,
				    bool sec)
{
	HAL_vmidmt_BusAttribType *bypass_bus = cfg->pBypassBusAttrib;
	HAL_vmidmt_AuxConfigType *bypass_aux = cfg->pBypassAuxConfig;
	uint32_t attrib = 0;
	uint32_t config = 0;
	uint32_t val = 0;
	HAL_vmidmt_Status rc = HAL_VMIDMT_NO_ERROR;

	if (!cfg)
		return HAL_VMIDMT_INVALID_PARAM;

	if (bypass_bus &&
	    bypass_bus->uMemAttr > (1 << VMIDMT_MEMATTR_MAX_BITS) - 1)
		return HAL_VMIDMT_INVALID_PARAM;

	if (cfg->pAccessControl) {
		rc = configure_acr(p->uBaseAddr, cfg->pAccessControl);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	}

	if (bypass_bus)
		attrib |= set_default_config_bus(bypass_bus);

	if (bypass_aux)
		config |= set_default_config_aux(bypass_aux);

	if (sec) {
		if (bypass_bus && bypass_bus->eNSCFG < HAL_VMIDMT_NSCFG_DEFAULT)
			attrib |=
			bypass_bus->eNSCFG << VMIDMT_SHFT(SCR0, NSCFG);

		VMIDMT_OUTM(p->uBaseAddr, SCR0,
			    (CR0_BUS_MASK | VMIDMT_FMSK(SCR0, NSCFG)), attrib);
		val = VMIDMT_INM(p->uBaseAddr, SCR0,
				 (CR0_BUS_MASK | VMIDMT_FMSK(SCR0, NSCFG)));
		if (val != attrib)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->uBaseAddr, SCR0,
			    VMIDPNE, cfg->bVmidPrivateNamespaceEnable);
		val = VMIDMT_INF(p->uBaseAddr, SCR0, VMIDPNE);
		if (val != (uint32_t)cfg->bVmidPrivateNamespaceEnable)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->uBaseAddr, SCR2, BPVMID, cfg->bypassVmid);
		val = VMIDMT_INF(p->uBaseAddr, SCR2, BPVMID);
		if (val != (uint32_t)cfg->bypassVmid)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTM(p->uBaseAddr, SACR, VMIDMT_RMSK(SACR), config);
		val = VMIDMT_INM(p->uBaseAddr, SACR, VMIDMT_RMSK(SACR));
		if (val != config)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;
	} else {
		VMIDMT_OUTM(p->uBaseAddr, NSCR0, CR0_BUS_MASK, attrib);
		val = VMIDMT_INM(p->uBaseAddr, NSCR0, CR0_BUS_MASK);
		if (val != attrib)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->uBaseAddr,
			    NSCR0, VMIDPNE, cfg->bVmidPrivateNamespaceEnable);
		val = VMIDMT_INF(p->uBaseAddr, NSCR0, VMIDPNE);
		if (val != (uint32_t)cfg->bVmidPrivateNamespaceEnable)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->uBaseAddr, NSCR2, BPVMID, cfg->bypassVmid);
		val = VMIDMT_INF(p->uBaseAddr, NSCR2, BPVMID);
		if (val != (uint32_t)cfg->bypassVmid)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTM(p->uBaseAddr, NSACR, VMIDMT_RMSK(NSACR), config);
		val = VMIDMT_INM(p->uBaseAddr, NSACR, VMIDMT_RMSK(NSACR));
		if (val != config)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;
	}

	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status swap_context(const HAL_vmidmt_InfoType *info,
				      uint32_t old_idx,
				      uint32_t new_idx)
{
	uint32_t s2vr_msk = VMIDMT_RMSK(S2VRn);
	uint32_t src_val = 0;
	uint32_t dst_val = 0;

	src_val = VMIDMT_INI(info->uBaseAddr, SMRn, old_idx);

	VMIDMT_OUTI(info->uBaseAddr, SMRn, new_idx, src_val);

	dst_val = VMIDMT_INI(info->uBaseAddr, SMRn, new_idx);
	if (dst_val != src_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	src_val = VMIDMT_INMI(info->uBaseAddr, S2VRn, old_idx,
			      s2vr_msk);

	VMIDMT_OUTMI(info->uBaseAddr, S2VRn, new_idx,
		     s2vr_msk, src_val);

	dst_val = VMIDMT_INMI(info->uBaseAddr, S2VRn, new_idx,
			      s2vr_msk);
	if (dst_val != src_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	src_val = VMIDMT_INI(info->uBaseAddr, AS2VRn, old_idx);

	VMIDMT_OUTI(info->uBaseAddr, AS2VRn, new_idx, src_val);

	dst_val = VMIDMT_INI(info->uBaseAddr, AS2VRn, new_idx);
	if (dst_val != src_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status erase_context(const HAL_vmidmt_InfoType *info,
				       uint32_t ctx_idx)
{
	uint32_t s2vr_mask = VMIDMT_RMSK(S2VRn);
	uint32_t rd_val;

	VMIDMT_OUTI(info->uBaseAddr, SMRn, ctx_idx, 0);

	rd_val = VMIDMT_INI(info->uBaseAddr, SMRn, ctx_idx);
	if (rd_val != 0U)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	VMIDMT_OUTMI(info->uBaseAddr, S2VRn, ctx_idx,
		     s2vr_mask, 0);

	rd_val = VMIDMT_INMI(info->uBaseAddr, S2VRn, ctx_idx,
			     s2vr_mask);
	if (rd_val != 0U)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	VMIDMT_OUTI(info->uBaseAddr, AS2VRn, ctx_idx, 0);

	rd_val = VMIDMT_INI(info->uBaseAddr, AS2VRn, ctx_idx);
	if (rd_val != 0U)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static uint32_t configure_context_bus(const HAL_vmidmt_BusAttribType *bus)
{
	uint32_t s2vr = 0;

	if (bus->eNSCFG < HAL_VMIDMT_NSCFG_DEFAULT)
		s2vr |= bus->eNSCFG << VMIDMT_SHFT(S2VRn, NSCFG);

	if (bus->eWACFG < HAL_VMIDMT_WACFG_DEFAULT)
		s2vr |= bus->eWACFG << VMIDMT_SHFT(S2VRn, WACFG);

	if (bus->eRACFG < HAL_VMIDMT_RACFG_DEFAULT)
		s2vr |= bus->eRACFG << VMIDMT_SHFT(S2VRn, RACFG);

	if (bus->eSHCFG < HAL_VMIDMT_SHCFG_DEFAULT)
		s2vr |= bus->eSHCFG << VMIDMT_SHFT(S2VRn, SHCFG);

	if (bus->eMTCFG < HAL_VMIDMT_MTCFG_DEFAULT)
		s2vr |= bus->eMTCFG << VMIDMT_SHFT(S2VRn, MTCFG);

	s2vr |= bus->uMemAttr << VMIDMT_SHFT(S2VRn, MEMATTR);

	if (bus->eTransientCfg < HAL_VMIDMT_TRANSIENTCFG_DEFAULT)
		s2vr |= bus->eTransientCfg << VMIDMT_SHFT(S2VRn, TRANSIENTCFG);

	return s2vr;
}

static uint32_t configure_context_aux(const HAL_vmidmt_AuxConfigType *aux)
{
	uint32_t as2vr = 0;

	if (aux->eRCNSH < HAL_VMIDMT_RCNSH_DEFAULT)
		as2vr |= aux->eRCNSH << VMIDMT_SHFT(AS2VRn, RCNSH);

	if (aux->eRCISH < HAL_VMIDMT_RCISH_DEFAULT)
		as2vr |= aux->eRCISH << VMIDMT_SHFT(AS2VRn, RCISH);

	if (aux->eRCOSH < HAL_VMIDMT_RCOSH_DEFAULT)
		as2vr |= aux->eRCOSH << VMIDMT_SHFT(AS2VRn, RCOSH);

	if (aux->eReqPriorityCfg < HAL_VMIDMT_REQPRICFG_DEFAULT)
		as2vr |= aux->eReqPriorityCfg <<
			 VMIDMT_SHFT(AS2VRn, REQPRIORITYCFG);

	if (aux->eReqPriority < HAL_VMIDMT_REQPRI_DEFAULT)
		as2vr |= aux->eReqPriority << VMIDMT_SHFT(AS2VRn, REQPRIORITY);

	return as2vr;
}

static HAL_vmidmt_Status configure_context(const HAL_vmidmt_InfoType *info,
				   uint32_t index,
				   const HAL_vmidmt_ContextConfigType *config)
{
	const HAL_vmidmt_BusAttribType *bus_attrib = config->pBusAttrib;
	const HAL_vmidmt_AuxConfigType *aux_config = config->pAuxConfig;
	uint32_t as2vr_val = 0;
	uint32_t s2vr_val = 0;
	uint32_t readback;

	s2vr_val |= config->uVmid << VMIDMT_SHFT(S2VRn, VMID);

	if (bus_attrib != NULL)
		s2vr_val |= configure_context_bus(bus_attrib);

	VMIDMT_OUTMI(info->uBaseAddr, S2VRn, index,
		     VMIDMT_RMSK(S2VRn), s2vr_val);

	readback = VMIDMT_INMI(info->uBaseAddr, S2VRn, index,
			       VMIDMT_RMSK(S2VRn));
	if (readback != s2vr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	if (aux_config != NULL)
		as2vr_val |= configure_context_aux(aux_config);

	VMIDMT_OUTI(info->uBaseAddr, AS2VRn, index, as2vr_val);

	readback = VMIDMT_INI(info->uBaseAddr, AS2VRn, index);
	if (readback != as2vr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

HAL_vmidmt_Status vmidmt_hal_enable_client(const HAL_vmidmt_InfoType *p,
					   bool sec)
{
	uint32_t read_back = 0;

	if (sec) {
		VMIDMT_OUTF(p->uBaseAddr, SCR0, CLIENTPD, read_back);
		read_back = VMIDMT_INF(p->uBaseAddr, SCR0, CLIENTPD);
	} else  {
		VMIDMT_OUTF(p->uBaseAddr, NSCR0, CLIENTPD, read_back);
		read_back = VMIDMT_INF(p->uBaseAddr, NSCR0, CLIENTPD);
	}

	return read_back ? HAL_VMIDMT_READ_WRITE_MISMATCH : HAL_VMIDMT_NO_ERROR;
}

HAL_vmidmt_Status vmidmt_hal_config_ssdt(const HAL_vmidmt_InfoType *p,
			     const HAL_vmidmt_SecureStatusDetType *status,
			     uint32_t elements)
{
	uint32_t length = 0;

	if (!p || !status || !IS_NUM_SSD_VALID(elements))
		return HAL_VMIDMT_INVALID_PARAM;

	length = 1 << p->devParams.uNumSSDIndexBits;

	if (length < VMIDMT_NUM_NSSTATE_BITS) {
		if (elements > 1)
			return HAL_VMIDMT_INVALID_PARAM;
	} else {
		if (elements > length / VMIDMT_NUM_NSSTATE_BITS)
			return HAL_VMIDMT_INVALID_PARAM;
	}

	for (size_t i = 0; i < elements; i++)
		VMIDMT_OUTMI(p->uBaseAddr, SSDRn, i, ~status[i], status[i]);

	return HAL_VMIDMT_NO_ERROR;
}

HAL_vmidmt_Status vmidmt_hal_init(HAL_vmidmt_InfoType *info,
		const HAL_vmidmt_DefaultSecureVmidConfigType *sec_cfg,
		const HAL_vmidmt_DefaultVmidConfigType *nsec_cfg,
		char **ver)
{
	uint32_t ssd_words = 0;
	uint32_t ssd_mask_bits = 0;
	uint8_t  sec_ext = 0;
	uint32_t scr1_init = 0;
	uint32_t rb_val = 0;
	uint32_t idx;
	HAL_vmidmt_Status st = HAL_VMIDMT_NO_ERROR;

	if (sec_cfg && nsec_cfg)
		return HAL_VMIDMT_INVALID_PARAM;

	read_id_registers(info);

	VMIDMT_OUT(info->uBaseAddr, VMIDMTACR, VMIDMT_ALL_VMID_MASK);

	if (sec_cfg) {
		sec_ext = sec_cfg->secureExtensions;

		if (sec_cfg->secureExtensions ==
		    HAL_VMIDMT_SECURE_EXT_DEFAULT) {

			sec_ext = info->devParams.uEntryCount;

		} else if (sec_cfg->secureExtensions >
			   info->devParams.uEntryCount) {

			st = HAL_VMIDMT_INVALID_PARAM;
			goto out;
		}

		st = set_default_config(info,
					sec_cfg->pDefaultSecureConfig,
					true);
		if (st != HAL_VMIDMT_NO_ERROR)
			goto out;

		VMIDMT_OUTF(info->uBaseAddr, SCR1, GASRAE,
			    sec_cfg->bGlbAddrSpaceRestrictedAccEnable);

		rb_val = VMIDMT_INF(info->uBaseAddr, SCR1, GASRAE);
		if (rb_val != sec_cfg->bGlbAddrSpaceRestrictedAccEnable) {
			st = HAL_VMIDMT_READ_WRITE_MISMATCH;
			goto out;
		}

		VMIDMT_OUTF(info->uBaseAddr, SCR1, NSNUMSMRGO, sec_ext);

		rb_val = VMIDMT_INF(info->uBaseAddr, SCR1, NSNUMSMRGO);
		if (rb_val != sec_ext) {
			st = HAL_VMIDMT_READ_WRITE_MISMATCH;
			goto out;
		}

		VMIDMT_OUT(info->uBaseAddr, NSCR0,
			   HAL_VMIDMT_DEFAULT_CR0);

		VMIDMT_OUT(info->uBaseAddr, NSCR2,
			   HAL_VMIDMT_DEFAULT_CR2);

		VMIDMT_OUT(info->uBaseAddr, NSACR,
			   HAL_VMIDMT_DEFAULT_ACR);

	} else {
		VMIDMT_OUT(info->uBaseAddr, SCR0,
			   HAL_VMIDMT_DEFAULT_CR0);

		scr1_init = info->devParams.uEntryCount <<
			VMIDMT_SHFT(SCR1, NSNUMSMRGO);

		VMIDMT_OUT(info->uBaseAddr, SCR1, scr1_init);

		VMIDMT_OUT(info->uBaseAddr, SCR2,
			   HAL_VMIDMT_DEFAULT_CR2);

		VMIDMT_OUT(info->uBaseAddr, SACR,
			   HAL_VMIDMT_DEFAULT_ACR);

		st = set_default_config(info, nsec_cfg, false);
		if (st != HAL_VMIDMT_NO_ERROR)
			goto out;
	}

	VMIDMT_OUT(info->uBaseAddr, SVMIDMTCR0, 1);
	VMIDMT_OUT(info->uBaseAddr, NSVMIDMTCR0, 1);
	VMIDMT_OUT(info->uBaseAddr, SGFSRRESTORE, 0);
	VMIDMT_OUT(info->uBaseAddr, NSGFSRRESTORE, 0);

	ssd_words = 1;
	ssd_mask_bits = BIT(info->devParams.uNumSSDIndexBits);

	if (ssd_mask_bits >= VMIDMT_NUM_NSSTATE_BITS) {
		ssd_words =
			ssd_mask_bits / VMIDMT_NUM_NSSTATE_BITS;

		if (!IS_NUM_SSD_VALID(ssd_words)) {
			st = HAL_VMIDMT_INVALID_HW_VALUE;
			goto out;
		}
	}

	for (idx = 0; idx < ssd_words; idx++) {
		VMIDMT_OUTI(info->uBaseAddr, SSDRn, idx, 0xFFFFFFFF);
	}

out:
	for (idx = 0; idx < info->devParams.uEntryCount; idx++) {

		if (info->devParams.bStreamMatchSupport)
			VMIDMT_OUTI(info->uBaseAddr, SMRn, idx, 0);

		VMIDMT_OUTI(info->uBaseAddr, S2VRn, idx,
			    HAL_VMIDMT_DEFAULT_S2VR);

		VMIDMT_OUTI(info->uBaseAddr, AS2VRn, idx, 0);
	}

	return st;
}

static HAL_vmidmt_Status validate_ctx_args(const HAL_vmidmt_InfoType *info,
				uint32_t stream,
				const HAL_vmidmt_ContextConfigType *cfg,
				bool *secure)
{
	const uint32_t max_attr = (1U << VMIDMT_MEMATTR_MAX_BITS) - 1U;
	uint32_t max_stream_id;

	if (cfg == NULL)
		return HAL_VMIDMT_INVALID_PARAM;

	max_stream_id = (1U << info->devParams.uNumStreamIDBits) - 1U;

	if (stream > max_stream_id && stream >= info->devParams.uEntryCount)
		return HAL_VMIDMT_INVALID_PARAM;

	*secure = false;

	if (!cfg->pBusAttrib)
		return HAL_VMIDMT_NO_ERROR;

	if (cfg->pBusAttrib->uMemAttr > max_attr)
		return HAL_VMIDMT_INVALID_PARAM;

	if (cfg->pBusAttrib->eNSCFG == HAL_VMIDMT_NSCFG_SECURE)
		*secure = true;

	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status update_ssdrn(const HAL_vmidmt_InfoType *info,
				      uint32_t stream,
				      bool is_secure)
{
	uint32_t ssd_index;
	uint32_t bit_mask;
	uint32_t desired;
	uint32_t ssdrn_val;

	ssd_index = stream / VMIDMT_NUM_NSSTATE_BITS;
	bit_mask  = BIT(stream % VMIDMT_NUM_NSSTATE_BITS);

	/* 0 = secure, 1 = non-secure */
	desired = is_secure ? 0U : bit_mask;

	VMIDMT_OUTMI(info->uBaseAddr, SSDRn, ssd_index, bit_mask, desired);

	ssdrn_val = VMIDMT_INMI(info->uBaseAddr, SSDRn, ssd_index, bit_mask);

	if ((ssdrn_val & bit_mask) != desired)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static int32_t find_matching_smr(const HAL_vmidmt_InfoType *info,
				 uint32_t entry_count,
				 uint32_t smr_val)
{
	for (int32_t i = 0; i < (int32_t)entry_count; i++) {
		if (VMIDMT_INI(info->uBaseAddr, SMRn, (uint32_t)i) == smr_val)
			return i;
	}

	return -1;
}

static HAL_vmidmt_Status move_to_ns_tail(const HAL_vmidmt_InfoType *info,
					 uint32_t ns_smr_count,
					 int32_t *idx)
{
	HAL_vmidmt_Status rc = HAL_VMIDMT_NO_ERROR;
	int32_t i = *idx;

	for (; (uint32_t)i < ns_smr_count - 1U; i++) {
		rc = swap_context(info, (uint32_t)i + 1U, (uint32_t)i);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	}

	*idx = i;
	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status move_to_ns_head(const HAL_vmidmt_InfoType *info,
					 uint32_t *ns_smr_count,
					 int32_t *idx)
{
	HAL_vmidmt_Status rc = HAL_VMIDMT_NO_ERROR;
	uint32_t read_back;
	int32_t i = *idx;

	for (; (uint32_t)i > *ns_smr_count; i--) {
		rc = swap_context(info, (uint32_t)i - 1U, (uint32_t)i);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	}

	*idx = i;
	(*ns_smr_count)++;

	VMIDMT_OUTF(info->uBaseAddr, SCR1, NSNUMSMRGO, *ns_smr_count);

	read_back = VMIDMT_INF(info->uBaseAddr, SCR1, NSNUMSMRGO);
	if (read_back != *ns_smr_count)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status find_secure_slot(const HAL_vmidmt_InfoType *info,
					  uint32_t entry_count,
					  uint32_t ns_smr_count,
					  uint32_t stream,
					  int32_t *idx)
{
	int32_t i;

	for (i = (int32_t)entry_count - 1; i >= 0; i--) {
		if (VMIDMT_INFI(info->uBaseAddr, SMRn, (uint32_t)i, VALID)
		    != 1U)
			break;

		if (VMIDMT_INFI(info->uBaseAddr, SMRn, (uint32_t)i, ID)
		    == stream)
			break;
	}

	if (i < 0 || (uint32_t)i < ns_smr_count - 1U)
		return HAL_VMIDMT_INVALID_PARAM;

	/* found */
	*idx = i;
	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status find_nonsecure_slot(const HAL_vmidmt_InfoType *info,
					     uint32_t ns_smr_count,
					     uint32_t stream,
					     int32_t *idx)
{
	int32_t i;

	for (i = 0; (uint32_t)i < ns_smr_count; i++) {
		if (VMIDMT_INFI(info->uBaseAddr, SMRn, (uint32_t)i, VALID)
		    != 1U)
			break;

		if (VMIDMT_INFI(info->uBaseAddr, SMRn, (uint32_t)i, ID)
		    == stream)
			break;
	}

	if ((uint32_t)i >= ns_smr_count)
		return HAL_VMIDMT_INVALID_PARAM;

	/* found */
	*idx = i;
	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status write_smr(const HAL_vmidmt_InfoType *info,
				   bool is_secure,
				   uint32_t smr_val,
				   int32_t idx,
				   uint32_t *ns_smr_count)
{
	uint32_t rd_val;
	uint32_t i = (uint32_t)idx;

	VMIDMT_OUTI(info->uBaseAddr, SMRn, i, smr_val);

	rd_val = VMIDMT_INI(info->uBaseAddr, SMRn, i);
	if (rd_val != smr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	if (is_secure && i < *ns_smr_count) {
		(*ns_smr_count)--;

		VMIDMT_OUTF(info->uBaseAddr, SCR1, NSNUMSMRGO, *ns_smr_count);

		rd_val = VMIDMT_INF(info->uBaseAddr, SCR1, NSNUMSMRGO);
		if (rd_val != *ns_smr_count)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;
	}

	return HAL_VMIDMT_NO_ERROR;
}

static HAL_vmidmt_Status update_smr(const HAL_vmidmt_InfoType *info,
				    uint32_t stream,
				    bool is_secure,
				    uint32_t *index_out)
{
	HAL_vmidmt_Status rc = HAL_VMIDMT_NO_ERROR;
	uint32_t entry_count = info->devParams.uEntryCount;
	uint32_t ns_smr_count;
	uint32_t smr_val = 0U;
	int32_t i;

	ns_smr_count = VMIDMT_INF(info->uBaseAddr, SCR1, NSNUMSMRGO);

	smr_val |= BIT(VMIDMT_SHFT(SMRn, VALID));
	smr_val |= stream << VMIDMT_SHFT(SMRn, ID);

	i = find_matching_smr(info, entry_count, smr_val);

	if (i >= 0) {
		/* Existing SMR entry with same value. */
		if (is_secure && (uint32_t)i >= ns_smr_count - 1U)
			goto write_and_adjust;

		if (!is_secure && (uint32_t)i < ns_smr_count)
			goto write_and_adjust;

		if ((uint32_t)i < ns_smr_count) {
			rc = move_to_ns_tail(info, ns_smr_count, &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		} else {
			rc = move_to_ns_head(info, &ns_smr_count, &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		}

		rc = erase_context(info, (uint32_t)i);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	} else {
		/* No existing SMR entry with this value; find a slot. */
		if (is_secure) {
			rc = find_secure_slot(info, entry_count, ns_smr_count,
					      stream, &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		} else {
			rc = find_nonsecure_slot(info, ns_smr_count, stream,
						 &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		}
	}

write_and_adjust:
	rc = write_smr(info, is_secure, smr_val, i, &ns_smr_count);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return rc;

	*index_out = (uint32_t)i;

	return HAL_VMIDMT_NO_ERROR;
}

HAL_vmidmt_Status vmidmt_hal_config_ctx(const HAL_vmidmt_InfoType *info,
					uint32_t stream,
					const HAL_vmidmt_ContextConfigType *cfg)
{
	HAL_vmidmt_Status rc;
	bool secure = false;
	uint32_t ctx_index = 0;

	rc = validate_ctx_args(info, stream, cfg, &secure);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return rc;

	rc = update_ssdrn(info, stream, secure);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return rc;

	if (info->devParams.bStreamMatchSupport) {
		rc = update_smr(info, stream, secure, &ctx_index);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	} else {
		ctx_index = 0;
	}

	return configure_context(info, ctx_index, cfg);
}

HAL_vmidmt_Status vmidmt_hal_config_ctx_ext(const HAL_vmidmt_InfoType *info,
				uint32_t smr_index,
				const uint32_t *stream_list,
				uint32_t stream_count,
				const HAL_vmidmt_ContextConfigType *ctx)
{
	const uint32_t max_attr = (1U << VMIDMT_MEMATTR_MAX_BITS) - 1U;
	uint32_t valid_range_mask = 0;
	uint32_t id_or = 0;
	uint32_t mask_or = 0;
	uint8_t sid_bits = 0;
	uint32_t smr_val = 0;
	uint32_t rd_val = 0;
	uint32_t i;

	if (!info || !ctx)
		return HAL_VMIDMT_INVALID_PARAM;

	/* Simple case: single stream → reuse normal API */
	if (stream_count == 1U)
		return vmidmt_hal_config_ctx(info, smr_index, ctx);

	if (smr_index >= info->devParams.uEntryCount)
		return HAL_VMIDMT_INVALID_PARAM;

	if (ctx->pBusAttrib && ctx->pBusAttrib->uMemAttr > max_attr)
		return HAL_VMIDMT_INVALID_PARAM;

	if (!info->devParams.bStreamMatchSupport)
		goto configure_ctx;

	if (!stream_list || stream_count == 0U)
		return HAL_VMIDMT_INVALID_PARAM;

	sid_bits = info->devParams.uNumStreamIDBits;
	if (sid_bits == 0U) {
		valid_range_mask = 1U;
	} else if (sid_bits >= 32U) {
		valid_range_mask = UINT32_MAX;
	} else {
		valid_range_mask = (1U << sid_bits) - 1U;
	}

	for (i = 0; i < stream_count; i++) {
		id_or |= stream_list[i];

		if ((i + 1U) < stream_count)
			mask_or |= (stream_list[i] ^ stream_list[i + 1U]);
	}

	if ((id_or | mask_or) & ~valid_range_mask)
		return HAL_VMIDMT_INVALID_PARAM;

	smr_val = (1UL << VMIDMT_SHFT(SMRn, VALID)) |
		  (mask_or << VMIDMT_SHFT(SMRn, MASK)) |
		  (id_or   << VMIDMT_SHFT(SMRn, ID));

	VMIDMT_OUTI(info->uBaseAddr, SMRn, smr_index, smr_val);

	rd_val = VMIDMT_INI(info->uBaseAddr, SMRn, smr_index);

	if (rd_val != smr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

configure_ctx:
	return configure_context(info, smr_index, ctx);
}

HAL_vmidmt_Status vmidmt_hal_cfg_err(const HAL_vmidmt_InfoType *p, bool sec,
				     HAL_vmidmt_ErrorOptionConfigType err)
{
	uint32_t val = 0;
	uint32_t raw = 0;

	if (err & ~ALL_ERROR_OPTIONS)
		return HAL_VMIDMT_INVALID_PARAM;

	if ((err & HAL_VMIDMT_ERROR_O_SMCFCFG_EN) &&
	    p->devParams.bStreamMatchSupport)
		val |= BIT(VMIDMT_SHFT(CR0, SMCFCFG));

	if (err & HAL_VMIDMT_ERROR_O_USFCFG_EN)
		val |= BIT(VMIDMT_SHFT(CR0, USFCFG));

	if (err & HAL_VMIDMT_ERROR_O_GCFGFIE)
		val |= BIT(VMIDMT_SHFT(CR0, GCFGFIE));

	if (err & HAL_VMIDMT_ERROR_O_GCFGFRE)
		val |= BIT(VMIDMT_SHFT(CR0, GCFGFRE));

	if (err & HAL_VMIDMT_ERROR_O_GFIE)
		val |= BIT(VMIDMT_SHFT(CR0, GFIE));

	if (sec) {
		VMIDMT_OUTM(p->uBaseAddr, SCR0, CR0_ERE_MASK, val);
		raw = VMIDMT_INM(p->uBaseAddr, SCR0, CR0_ERE_MASK);
	} else {
		VMIDMT_OUTM(p->uBaseAddr, NSCR0, CR0_ERE_MASK, val);
		raw = VMIDMT_INM(p->uBaseAddr, NSCR0, CR0_ERE_MASK);
	}

	if (raw != val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

bool vmidmt_hal_is_error(const HAL_vmidmt_InfoType *pVmidmt, bool sec)
{
	if (sec)
		return VMIDMT_IN(pVmidmt->uBaseAddr, SGFSR) != 0;

	return VMIDMT_IN(pVmidmt->uBaseAddr, NSGFSR) != 0;
}

static void vmidmt_hal_get_error_sec(const HAL_vmidmt_InfoType *p,
				     HAL_vmidmt_ErrorType *err,
				     uint32_t *sr,
				     uint32_t *s0,
				     uint32_t *s1,
				     uint32_t *s2)
{
	*sr = VMIDMT_IN(p->uBaseAddr, SGFSR);
	err->uPhysicalAddressLower32 = VMIDMT_IN(p->uBaseAddr, SGFAR0);

	if (p->devParams.uInputAddrSize)
		err->uPhysicalAddressUpper32 = VMIDMT_IN(p->uBaseAddr, SGFAR1);

	*s0 = VMIDMT_IN(p->uBaseAddr, SGFSYNDR0);
	*s1 = VMIDMT_IN(p->uBaseAddr, SGFSYNDR1);
	*s2 = VMIDMT_IN(p->uBaseAddr, SGFSYNDR2);
}

static void vmidmt_hal_get_error_nsec(const HAL_vmidmt_InfoType *p,
				      HAL_vmidmt_ErrorType *err,
				      uint32_t *sr,
				      uint32_t *s0, uint32_t *s1,
				      uint32_t *s2)
{
	*sr = VMIDMT_IN(p->uBaseAddr, NSGFSR);

	err->uPhysicalAddressLower32 = VMIDMT_IN(p->uBaseAddr, NSGFAR0);

	if (p->devParams.uInputAddrSize)
		err->uPhysicalAddressUpper32 = VMIDMT_IN(p->uBaseAddr, NSGFAR1);

	*s0 = VMIDMT_IN(p->uBaseAddr, NSGFSYNDR0);

	*s1 = VMIDMT_IN(p->uBaseAddr, NSGFSYNDR1);

	*s2 = VMIDMT_IN(p->uBaseAddr, NSGFSYNDR2);
}

void vmidmt_hal_get_error(const HAL_vmidmt_InfoType *p, bool sec,
		      HAL_vmidmt_ErrorType *err)
{
	uint32_t s0, s1, s2, sr;

	if (!err)
		return;

	memset(err, 0, sizeof(*err));

	if (sec) {
		vmidmt_hal_get_error_sec(p, err, &sr, &s0, &s1, &s2);
	} else {
		vmidmt_hal_get_error_nsec(p, err, &sr, &s0, &s1, &s2);
	}

	if (sr & VMIDMT_FMSK(GFSR, CLMULTI))
		err->uErrorFlags |= HAL_VMIDMT_ERROR_F_CLMULTI;

	if (sr & VMIDMT_FMSK(GFSR, CFGMULTI))
		err->uErrorFlags |= HAL_VMIDMT_ERROR_F_CFGMULTI;

	if (sr & VMIDMT_FMSK(GFSR, PF))
		err->uErrorFlags |= HAL_VMIDMT_ERROR_F_PF;

	if (sr & VMIDMT_FMSK(GFSR, CAF))
		err->uErrorFlags |= HAL_VMIDMT_ERROR_F_CAF;

	if (sr & VMIDMT_FMSK(GFSR, SMCF))
		err->uErrorFlags |= HAL_VMIDMT_ERROR_F_SMCF;

	if (sr & VMIDMT_FMSK(GFSR, USF))
		err->uErrorFlags |= HAL_VMIDMT_ERROR_F_USF;

	if (s0 & VMIDMT_FMSK(GFSYNDR0, NSATTR))
		err->uBusFlags |= HAL_VMIDMT_BUS_F_ERROR_NSATTR;

	if (s0 & VMIDMT_FMSK(GFSYNDR0, NSSTATE))
		err->uBusFlags |= HAL_VMIDMT_BUS_F_ERROR_NSSTATE;

	if (s0 & VMIDMT_FMSK(GFSYNDR0, WNR))
		err->uBusFlags |= HAL_VMIDMT_BUS_F_ERROR_WNR;

	err->uSSDIndex = VMIDMT_INFC(s1, GFSYNDR1, SSD_INDEX);

	err->uMasterId = VMIDMT_INFC(s2, GFSYNDR2, AMID);

	err->uAVMID = VMIDMT_INFC(s2, GFSYNDR2, AVMID);

	err->uATID = VMIDMT_INFC(s2, GFSYNDR2, ATID);

	err->uABID = VMIDMT_INFC(s2, GFSYNDR2, ABID);

	err->uAPID = VMIDMT_INFC(s2, GFSYNDR2, APID);

	err->uSID = VMIDMT_INFC(s1, GFSYNDR1, SID);
}

HAL_vmidmt_Status vmidmt_hal_clear_error(const HAL_vmidmt_InfoType *p, bool sec)
{
	uint32_t read_back = 0;

	if (sec) {
		VMIDMT_OUT(p->uBaseAddr, SGFSRRESTORE, 0);

		read_back = VMIDMT_IN(p->uBaseAddr, SGFSRRESTORE);
	} else {
		VMIDMT_OUT(p->uBaseAddr, NSGFSRRESTORE, 0);

		read_back = VMIDMT_IN(p->uBaseAddr, NSGFSRRESTORE);
	}

	return read_back ? HAL_VMIDMT_READ_WRITE_MISMATCH : HAL_VMIDMT_NO_ERROR;
}
