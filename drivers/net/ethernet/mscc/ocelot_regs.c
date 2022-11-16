// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi Ocelot Switch driver
 *
 * Copyright (c) 2017 Microsemi Corporation
 */
#include "ocelot.h"

static const u32 ocelot_ana_regmap[] = {
	REG(ANA_ADVLEARN,                  0x009000),
	REG(ANA_VLANMASK,                  0x009004),
	REG(ANA_PORT_B_DOMAIN,             0x009008),
	REG(ANA_ANAGEFIL,                  0x00900c),
	REG(ANA_ANEVENTS,                  0x009010),
	REG(ANA_STORMLIMIT_BURST,          0x009014),
	REG(ANA_STORMLIMIT_CFG,            0x009018),
	REG(ANA_ISOLATED_PORTS,            0x009028),
	REG(ANA_COMMUNITY_PORTS,           0x00902c),
	REG(ANA_AUTOAGE,                   0x009030),
	REG(ANA_MACTOPTIONS,               0x009034),
	REG(ANA_LEARNDISC,                 0x009038),
	REG(ANA_AGENCTRL,                  0x00903c),
	REG(ANA_MIRRORPORTS,               0x009040),
	REG(ANA_EMIRRORPORTS,              0x009044),
	REG(ANA_FLOODING,                  0x009048),
	REG(ANA_FLOODING_IPMC,             0x00904c),
	REG(ANA_SFLOW_CFG,                 0x009050),
	REG(ANA_PORT_MODE,                 0x009080),
	REG(ANA_PGID_PGID,                 0x008c00),
	REG(ANA_TABLES_ANMOVED,            0x008b30),
	REG(ANA_TABLES_MACHDATA,           0x008b34),
	REG(ANA_TABLES_MACLDATA,           0x008b38),
	REG(ANA_TABLES_MACACCESS,          0x008b3c),
	REG(ANA_TABLES_MACTINDX,           0x008b40),
	REG(ANA_TABLES_VLANACCESS,         0x008b44),
	REG(ANA_TABLES_VLANTIDX,           0x008b48),
	REG(ANA_TABLES_ISDXACCESS,         0x008b4c),
	REG(ANA_TABLES_ISDXTIDX,           0x008b50),
	REG(ANA_TABLES_ENTRYLIM,           0x008b00),
	REG(ANA_TABLES_PTP_ID_HIGH,        0x008b54),
	REG(ANA_TABLES_PTP_ID_LOW,         0x008b58),
	REG(ANA_MSTI_STATE,                0x008e00),
	REG(ANA_PORT_VLAN_CFG,             0x007000),
	REG(ANA_PORT_DROP_CFG,             0x007004),
	REG(ANA_PORT_QOS_CFG,              0x007008),
	REG(ANA_PORT_VCAP_CFG,             0x00700c),
	REG(ANA_PORT_VCAP_S1_KEY_CFG,      0x007010),
	REG(ANA_PORT_VCAP_S2_CFG,          0x00701c),
	REG(ANA_PORT_PCP_DEI_MAP,          0x007020),
	REG(ANA_PORT_CPU_FWD_CFG,          0x007060),
	REG(ANA_PORT_CPU_FWD_BPDU_CFG,     0x007064),
	REG(ANA_PORT_CPU_FWD_GARP_CFG,     0x007068),
	REG(ANA_PORT_CPU_FWD_CCM_CFG,      0x00706c),
	REG(ANA_PORT_PORT_CFG,             0x007070),
	REG(ANA_PORT_POL_CFG,              0x007074),
	REG(ANA_PORT_PTP_CFG,              0x007078),
	REG(ANA_PORT_PTP_DLY1_CFG,         0x00707c),
	REG(ANA_OAM_UPM_LM_CNT,            0x007c00),
	REG(ANA_PORT_PTP_DLY2_CFG,         0x007080),
	REG(ANA_PFC_PFC_CFG,               0x008800),
	REG(ANA_PFC_PFC_TIMER,             0x008804),
	REG(ANA_IPT_OAM_MEP_CFG,           0x008000),
	REG(ANA_IPT_IPT,                   0x008004),
	REG(ANA_PPT_PPT,                   0x008ac0),
	REG(ANA_FID_MAP_FID_MAP,           0x000000),
	REG(ANA_AGGR_CFG,                  0x0090b4),
	REG(ANA_CPUQ_CFG,                  0x0090b8),
	REG(ANA_CPUQ_CFG2,                 0x0090bc),
	REG(ANA_CPUQ_8021_CFG,             0x0090c0),
	REG(ANA_DSCP_CFG,                  0x009100),
	REG(ANA_DSCP_REWR_CFG,             0x009200),
	REG(ANA_VCAP_RNG_TYPE_CFG,         0x009240),
	REG(ANA_VCAP_RNG_VAL_CFG,          0x009260),
	REG(ANA_VRAP_CFG,                  0x009280),
	REG(ANA_VRAP_HDR_DATA,             0x009284),
	REG(ANA_VRAP_HDR_MASK,             0x009288),
	REG(ANA_DISCARD_CFG,               0x00928c),
	REG(ANA_FID_CFG,                   0x009290),
	REG(ANA_POL_PIR_CFG,               0x004000),
	REG(ANA_POL_CIR_CFG,               0x004004),
	REG(ANA_POL_MODE_CFG,              0x004008),
	REG(ANA_POL_PIR_STATE,             0x00400c),
	REG(ANA_POL_CIR_STATE,             0x004010),
	REG(ANA_POL_STATE,                 0x004014),
	REG(ANA_POL_FLOWC,                 0x008b80),
	REG(ANA_POL_HYST,                  0x008bec),
	REG(ANA_POL_MISC_CFG,              0x008bf0),
};

static const u32 ocelot_qs_regmap[] = {
	REG(QS_XTR_GRP_CFG,                0x000000),
	REG(QS_XTR_RD,                     0x000008),
	REG(QS_XTR_FRM_PRUNING,            0x000010),
	REG(QS_XTR_FLUSH,                  0x000018),
	REG(QS_XTR_DATA_PRESENT,           0x00001c),
	REG(QS_XTR_CFG,                    0x000020),
	REG(QS_INJ_GRP_CFG,                0x000024),
	REG(QS_INJ_WR,                     0x00002c),
	REG(QS_INJ_CTRL,                   0x000034),
	REG(QS_INJ_STATUS,                 0x00003c),
	REG(QS_INJ_ERR,                    0x000040),
	REG(QS_INH_DBG,                    0x000048),
};

static const u32 ocelot_hsio_regmap[] = {
	REG(HSIO_PLL5G_CFG0,               0x000000),
	REG(HSIO_PLL5G_CFG1,               0x000004),
	REG(HSIO_PLL5G_CFG2,               0x000008),
	REG(HSIO_PLL5G_CFG3,               0x00000c),
	REG(HSIO_PLL5G_CFG4,               0x000010),
	REG(HSIO_PLL5G_CFG5,               0x000014),
	REG(HSIO_PLL5G_CFG6,               0x000018),
	REG(HSIO_PLL5G_STATUS0,            0x00001c),
	REG(HSIO_PLL5G_STATUS1,            0x000020),
	REG(HSIO_PLL5G_BIST_CFG0,          0x000024),
	REG(HSIO_PLL5G_BIST_CFG1,          0x000028),
	REG(HSIO_PLL5G_BIST_CFG2,          0x00002c),
	REG(HSIO_PLL5G_BIST_STAT0,         0x000030),
	REG(HSIO_PLL5G_BIST_STAT1,         0x000034),
	REG(HSIO_RCOMP_CFG0,               0x000038),
	REG(HSIO_RCOMP_STATUS,             0x00003c),
	REG(HSIO_SYNC_ETH_CFG,             0x000040),
	REG(HSIO_SYNC_ETH_PLL_CFG,         0x000048),
	REG(HSIO_S1G_DES_CFG,              0x00004c),
	REG(HSIO_S1G_IB_CFG,               0x000050),
	REG(HSIO_S1G_OB_CFG,               0x000054),
	REG(HSIO_S1G_SER_CFG,              0x000058),
	REG(HSIO_S1G_COMMON_CFG,           0x00005c),
	REG(HSIO_S1G_PLL_CFG,              0x000060),
	REG(HSIO_S1G_PLL_STATUS,           0x000064),
	REG(HSIO_S1G_DFT_CFG0,             0x000068),
	REG(HSIO_S1G_DFT_CFG1,             0x00006c),
	REG(HSIO_S1G_DFT_CFG2,             0x000070),
	REG(HSIO_S1G_TP_CFG,               0x000074),
	REG(HSIO_S1G_RC_PLL_BIST_CFG,      0x000078),
	REG(HSIO_S1G_MISC_CFG,             0x00007c),
	REG(HSIO_S1G_DFT_STATUS,           0x000080),
	REG(HSIO_S1G_MISC_STATUS,          0x000084),
	REG(HSIO_MCB_S1G_ADDR_CFG,         0x000088),
	REG(HSIO_S6G_DIG_CFG,              0x00008c),
	REG(HSIO_S6G_DFT_CFG0,             0x000090),
	REG(HSIO_S6G_DFT_CFG1,             0x000094),
	REG(HSIO_S6G_DFT_CFG2,             0x000098),
	REG(HSIO_S6G_TP_CFG0,              0x00009c),
	REG(HSIO_S6G_TP_CFG1,              0x0000a0),
	REG(HSIO_S6G_RC_PLL_BIST_CFG,      0x0000a4),
	REG(HSIO_S6G_MISC_CFG,             0x0000a8),
	REG(HSIO_S6G_OB_ANEG_CFG,          0x0000ac),
	REG(HSIO_S6G_DFT_STATUS,           0x0000b0),
	REG(HSIO_S6G_ERR_CNT,              0x0000b4),
	REG(HSIO_S6G_MISC_STATUS,          0x0000b8),
	REG(HSIO_S6G_DES_CFG,              0x0000bc),
	REG(HSIO_S6G_IB_CFG,               0x0000c0),
	REG(HSIO_S6G_IB_CFG1,              0x0000c4),
	REG(HSIO_S6G_IB_CFG2,              0x0000c8),
	REG(HSIO_S6G_IB_CFG3,              0x0000cc),
	REG(HSIO_S6G_IB_CFG4,              0x0000d0),
	REG(HSIO_S6G_IB_CFG5,              0x0000d4),
	REG(HSIO_S6G_OB_CFG,               0x0000d8),
	REG(HSIO_S6G_OB_CFG1,              0x0000dc),
	REG(HSIO_S6G_SER_CFG,              0x0000e0),
	REG(HSIO_S6G_COMMON_CFG,           0x0000e4),
	REG(HSIO_S6G_PLL_CFG,              0x0000e8),
	REG(HSIO_S6G_ACJTAG_CFG,           0x0000ec),
	REG(HSIO_S6G_GP_CFG,               0x0000f0),
	REG(HSIO_S6G_IB_STATUS0,           0x0000f4),
	REG(HSIO_S6G_IB_STATUS1,           0x0000f8),
	REG(HSIO_S6G_ACJTAG_STATUS,        0x0000fc),
	REG(HSIO_S6G_PLL_STATUS,           0x000100),
	REG(HSIO_S6G_REVID,                0x000104),
	REG(HSIO_MCB_S6G_ADDR_CFG,         0x000108),
	REG(HSIO_HW_CFG,                   0x00010c),
	REG(HSIO_HW_QSGMII_CFG,            0x000110),
	REG(HSIO_HW_QSGMII_STAT,           0x000114),
	REG(HSIO_CLK_CFG,                  0x000118),
	REG(HSIO_TEMP_SENSOR_CTRL,         0x00011c),
	REG(HSIO_TEMP_SENSOR_CFG,          0x000120),
	REG(HSIO_TEMP_SENSOR_STAT,         0x000124),
};

static const u32 ocelot_qsys_regmap[] = {
	REG(QSYS_PORT_MODE,                0x011200),
	REG(QSYS_SWITCH_PORT_MODE,         0x011234),
	REG(QSYS_STAT_CNT_CFG,             0x011264),
	REG(QSYS_EEE_CFG,                  0x011268),
	REG(QSYS_EEE_THRES,                0x011294),
	REG(QSYS_IGR_NO_SHARING,           0x011298),
	REG(QSYS_EGR_NO_SHARING,           0x01129c),
	REG(QSYS_SW_STATUS,                0x0112a0),
	REG(QSYS_EXT_CPU_CFG,              0x0112d0),
	REG(QSYS_PAD_CFG,                  0x0112d4),
	REG(QSYS_CPU_GROUP_MAP,            0x0112d8),
	REG(QSYS_QMAP,                     0x0112dc),
	REG(QSYS_ISDX_SGRP,                0x011400),
	REG(QSYS_TIMED_FRAME_ENTRY,        0x014000),
	REG(QSYS_TFRM_MISC,                0x011310),
	REG(QSYS_TFRM_PORT_DLY,            0x011314),
	REG(QSYS_TFRM_TIMER_CFG_1,         0x011318),
	REG(QSYS_TFRM_TIMER_CFG_2,         0x01131c),
	REG(QSYS_TFRM_TIMER_CFG_3,         0x011320),
	REG(QSYS_TFRM_TIMER_CFG_4,         0x011324),
	REG(QSYS_TFRM_TIMER_CFG_5,         0x011328),
	REG(QSYS_TFRM_TIMER_CFG_6,         0x01132c),
	REG(QSYS_TFRM_TIMER_CFG_7,         0x011330),
	REG(QSYS_TFRM_TIMER_CFG_8,         0x011334),
	REG(QSYS_RED_PROFILE,              0x011338),
	REG(QSYS_RES_QOS_MODE,             0x011378),
	REG(QSYS_RES_CFG,                  0x012000),
	REG(QSYS_RES_STAT,                 0x012004),
	REG(QSYS_EGR_DROP_MODE,            0x01137c),
	REG(QSYS_EQ_CTRL,                  0x011380),
	REG(QSYS_EVENTS_CORE,              0x011384),
	REG(QSYS_CIR_CFG,                  0x000000),
	REG(QSYS_EIR_CFG,                  0x000004),
	REG(QSYS_SE_CFG,                   0x000008),
	REG(QSYS_SE_DWRR_CFG,              0x00000c),
	REG(QSYS_SE_CONNECT,               0x00003c),
	REG(QSYS_SE_DLB_SENSE,             0x000040),
	REG(QSYS_CIR_STATE,                0x000044),
	REG(QSYS_EIR_STATE,                0x000048),
	REG(QSYS_SE_STATE,                 0x00004c),
	REG(QSYS_HSCH_MISC_CFG,            0x011388),
};

static const u32 ocelot_rew_regmap[] = {
	REG(REW_PORT_VLAN_CFG,             0x000000),
	REG(REW_TAG_CFG,                   0x000004),
	REG(REW_PORT_CFG,                  0x000008),
	REG(REW_DSCP_CFG,                  0x00000c),
	REG(REW_PCP_DEI_QOS_MAP_CFG,       0x000010),
	REG(REW_PTP_CFG,                   0x000050),
	REG(REW_PTP_DLY1_CFG,              0x000054),
	REG(REW_DSCP_REMAP_DP1_CFG,        0x000690),
	REG(REW_DSCP_REMAP_CFG,            0x000790),
	REG(REW_STAT_CFG,                  0x000890),
	REG(REW_PPT,                       0x000680),
};

static const u32 ocelot_sys_regmap[] = {
	REG(SYS_COUNT_RX_OCTETS,           0x000000),
	REG(SYS_COUNT_RX_UNICAST,          0x000004),
	REG(SYS_COUNT_RX_MULTICAST,        0x000008),
	REG(SYS_COUNT_RX_BROADCAST,        0x00000c),
	REG(SYS_COUNT_RX_SHORTS,           0x000010),
	REG(SYS_COUNT_RX_FRAGMENTS,        0x000014),
	REG(SYS_COUNT_RX_JABBERS,          0x000018),
	REG(SYS_COUNT_RX_CRC_ALIGN_ERRS,   0x00001c),
	REG(SYS_COUNT_RX_SYM_ERRS,         0x000020),
	REG(SYS_COUNT_RX_64,               0x000024),
	REG(SYS_COUNT_RX_65_127,           0x000028),
	REG(SYS_COUNT_RX_128_255,          0x00002c),
	REG(SYS_COUNT_RX_256_1023,         0x000030),
	REG(SYS_COUNT_RX_1024_1526,        0x000034),
	REG(SYS_COUNT_RX_1527_MAX,         0x000038),
	REG(SYS_COUNT_RX_PAUSE,            0x00003c),
	REG(SYS_COUNT_RX_CONTROL,          0x000040),
	REG(SYS_COUNT_RX_LONGS,            0x000044),
	REG(SYS_COUNT_RX_CLASSIFIED_DROPS, 0x000048),
	REG(SYS_COUNT_TX_OCTETS,           0x000100),
	REG(SYS_COUNT_TX_UNICAST,          0x000104),
	REG(SYS_COUNT_TX_MULTICAST,        0x000108),
	REG(SYS_COUNT_TX_BROADCAST,        0x00010c),
	REG(SYS_COUNT_TX_COLLISION,        0x000110),
	REG(SYS_COUNT_TX_DROPS,            0x000114),
	REG(SYS_COUNT_TX_PAUSE,            0x000118),
	REG(SYS_COUNT_TX_64,               0x00011c),
	REG(SYS_COUNT_TX_65_127,           0x000120),
	REG(SYS_COUNT_TX_128_511,          0x000124),
	REG(SYS_COUNT_TX_512_1023,         0x000128),
	REG(SYS_COUNT_TX_1024_1526,        0x00012c),
	REG(SYS_COUNT_TX_1527_MAX,         0x000130),
	REG(SYS_COUNT_TX_AGING,            0x000170),
	REG(SYS_RESET_CFG,                 0x000508),
	REG(SYS_CMID,                      0x00050c),
	REG(SYS_VLAN_ETYPE_CFG,            0x000510),
	REG(SYS_PORT_MODE,                 0x000514),
	REG(SYS_FRONT_PORT_MODE,           0x000548),
	REG(SYS_FRM_AGING,                 0x000574),
	REG(SYS_STAT_CFG,                  0x000578),
	REG(SYS_SW_STATUS,                 0x00057c),
	REG(SYS_MISC_CFG,                  0x0005ac),
	REG(SYS_REW_MAC_HIGH_CFG,          0x0005b0),
	REG(SYS_REW_MAC_LOW_CFG,           0x0005dc),
	REG(SYS_CM_ADDR,                   0x000500),
	REG(SYS_CM_DATA,                   0x000504),
	REG(SYS_PAUSE_CFG,                 0x000608),
	REG(SYS_PAUSE_TOT_CFG,             0x000638),
	REG(SYS_ATOP,                      0x00063c),
	REG(SYS_ATOP_TOT_CFG,              0x00066c),
	REG(SYS_MAC_FC_CFG,                0x000670),
	REG(SYS_MMGT,                      0x00069c),
	REG(SYS_MMGT_FAST,                 0x0006a0),
	REG(SYS_EVENTS_DIF,                0x0006a4),
	REG(SYS_EVENTS_CORE,               0x0006b4),
	REG(SYS_CNT,                       0x000000),
	REG(SYS_PTP_STATUS,                0x0006b8),
	REG(SYS_PTP_TXSTAMP,               0x0006bc),
	REG(SYS_PTP_NXT,                   0x0006c0),
	REG(SYS_PTP_CFG,                   0x0006c4),
};

static const u32 ocelot_s2_regmap[] = {
	REG(S2_CORE_UPDATE_CTRL,           0x000000),
	REG(S2_CORE_MV_CFG,                0x000004),
	REG(S2_CACHE_ENTRY_DAT,            0x000008),
	REG(S2_CACHE_MASK_DAT,             0x000108),
	REG(S2_CACHE_ACTION_DAT,           0x000208),
	REG(S2_CACHE_CNT_DAT,              0x000308),
	REG(S2_CACHE_TG_DAT,               0x000388),
};

static const u32 *ocelot_regmap[] = {
	[ANA] = ocelot_ana_regmap,
	[QS] = ocelot_qs_regmap,
	[HSIO] = ocelot_hsio_regmap,
	[QSYS] = ocelot_qsys_regmap,
	[REW] = ocelot_rew_regmap,
	[SYS] = ocelot_sys_regmap,
	[S2] = ocelot_s2_regmap,
};

static const struct reg_field ocelot_regfields[] = {
	[ANA_ADVLEARN_VLAN_CHK] = REG_FIELD(ANA_ADVLEARN, 11, 11),
	[ANA_ADVLEARN_LEARN_MIRROR] = REG_FIELD(ANA_ADVLEARN, 0, 10),
	[ANA_ANEVENTS_MSTI_DROP] = REG_FIELD(ANA_ANEVENTS, 27, 27),
	[ANA_ANEVENTS_ACLKILL] = REG_FIELD(ANA_ANEVENTS, 26, 26),
	[ANA_ANEVENTS_ACLUSED] = REG_FIELD(ANA_ANEVENTS, 25, 25),
	[ANA_ANEVENTS_AUTOAGE] = REG_FIELD(ANA_ANEVENTS, 24, 24),
	[ANA_ANEVENTS_VS2TTL1] = REG_FIELD(ANA_ANEVENTS, 23, 23),
	[ANA_ANEVENTS_STORM_DROP] = REG_FIELD(ANA_ANEVENTS, 22, 22),
	[ANA_ANEVENTS_LEARN_DROP] = REG_FIELD(ANA_ANEVENTS, 21, 21),
	[ANA_ANEVENTS_AGED_ENTRY] = REG_FIELD(ANA_ANEVENTS, 20, 20),
	[ANA_ANEVENTS_CPU_LEARN_FAILED] = REG_FIELD(ANA_ANEVENTS, 19, 19),
	[ANA_ANEVENTS_AUTO_LEARN_FAILED] = REG_FIELD(ANA_ANEVENTS, 18, 18),
	[ANA_ANEVENTS_LEARN_REMOVE] = REG_FIELD(ANA_ANEVENTS, 17, 17),
	[ANA_ANEVENTS_AUTO_LEARNED] = REG_FIELD(ANA_ANEVENTS, 16, 16),
	[ANA_ANEVENTS_AUTO_MOVED] = REG_FIELD(ANA_ANEVENTS, 15, 15),
	[ANA_ANEVENTS_DROPPED] = REG_FIELD(ANA_ANEVENTS, 14, 14),
	[ANA_ANEVENTS_CLASSIFIED_DROP] = REG_FIELD(ANA_ANEVENTS, 13, 13),
	[ANA_ANEVENTS_CLASSIFIED_COPY] = REG_FIELD(ANA_ANEVENTS, 12, 12),
	[ANA_ANEVENTS_VLAN_DISCARD] = REG_FIELD(ANA_ANEVENTS, 11, 11),
	[ANA_ANEVENTS_FWD_DISCARD] = REG_FIELD(ANA_ANEVENTS, 10, 10),
	[ANA_ANEVENTS_MULTICAST_FLOOD] = REG_FIELD(ANA_ANEVENTS, 9, 9),
	[ANA_ANEVENTS_UNICAST_FLOOD] = REG_FIELD(ANA_ANEVENTS, 8, 8),
	[ANA_ANEVENTS_DEST_KNOWN] = REG_FIELD(ANA_ANEVENTS, 7, 7),
	[ANA_ANEVENTS_BUCKET3_MATCH] = REG_FIELD(ANA_ANEVENTS, 6, 6),
	[ANA_ANEVENTS_BUCKET2_MATCH] = REG_FIELD(ANA_ANEVENTS, 5, 5),
	[ANA_ANEVENTS_BUCKET1_MATCH] = REG_FIELD(ANA_ANEVENTS, 4, 4),
	[ANA_ANEVENTS_BUCKET0_MATCH] = REG_FIELD(ANA_ANEVENTS, 3, 3),
	[ANA_ANEVENTS_CPU_OPERATION] = REG_FIELD(ANA_ANEVENTS, 2, 2),
	[ANA_ANEVENTS_DMAC_LOOKUP] = REG_FIELD(ANA_ANEVENTS, 1, 1),
	[ANA_ANEVENTS_SMAC_LOOKUP] = REG_FIELD(ANA_ANEVENTS, 0, 0),
	[ANA_TABLES_MACACCESS_B_DOM] = REG_FIELD(ANA_TABLES_MACACCESS, 18, 18),
	[ANA_TABLES_MACTINDX_BUCKET] = REG_FIELD(ANA_TABLES_MACTINDX, 10, 11),
	[ANA_TABLES_MACTINDX_M_INDEX] = REG_FIELD(ANA_TABLES_MACTINDX, 0, 9),
	[QSYS_TIMED_FRAME_ENTRY_TFRM_VLD] = REG_FIELD(QSYS_TIMED_FRAME_ENTRY, 20, 20),
	[QSYS_TIMED_FRAME_ENTRY_TFRM_FP] = REG_FIELD(QSYS_TIMED_FRAME_ENTRY, 8, 19),
	[QSYS_TIMED_FRAME_ENTRY_TFRM_PORTNO] = REG_FIELD(QSYS_TIMED_FRAME_ENTRY, 4, 7),
	[QSYS_TIMED_FRAME_ENTRY_TFRM_TM_SEL] = REG_FIELD(QSYS_TIMED_FRAME_ENTRY, 1, 3),
	[QSYS_TIMED_FRAME_ENTRY_TFRM_TM_T] = REG_FIELD(QSYS_TIMED_FRAME_ENTRY, 0, 0),
	[SYS_RESET_CFG_CORE_ENA] = REG_FIELD(SYS_RESET_CFG, 2, 2),
	[SYS_RESET_CFG_MEM_ENA] = REG_FIELD(SYS_RESET_CFG, 1, 1),
	[SYS_RESET_CFG_MEM_INIT] = REG_FIELD(SYS_RESET_CFG, 0, 0),
};

static const struct ocelot_stat_layout ocelot_stats_layout[] = {
	{ .name = "rx_octets", .offset = 0x00, },
	{ .name = "rx_unicast", .offset = 0x01, },
	{ .name = "rx_multicast", .offset = 0x02, },
	{ .name = "rx_broadcast", .offset = 0x03, },
	{ .name = "rx_shorts", .offset = 0x04, },
	{ .name = "rx_fragments", .offset = 0x05, },
	{ .name = "rx_jabbers", .offset = 0x06, },
	{ .name = "rx_crc_align_errs", .offset = 0x07, },
	{ .name = "rx_sym_errs", .offset = 0x08, },
	{ .name = "rx_frames_below_65_octets", .offset = 0x09, },
	{ .name = "rx_frames_65_to_127_octets", .offset = 0x0A, },
	{ .name = "rx_frames_128_to_255_octets", .offset = 0x0B, },
	{ .name = "rx_frames_256_to_511_octets", .offset = 0x0C, },
	{ .name = "rx_frames_512_to_1023_octets", .offset = 0x0D, },
	{ .name = "rx_frames_1024_to_1526_octets", .offset = 0x0E, },
	{ .name = "rx_frames_over_1526_octets", .offset = 0x0F, },
	{ .name = "rx_pause", .offset = 0x10, },
	{ .name = "rx_control", .offset = 0x11, },
	{ .name = "rx_longs", .offset = 0x12, },
	{ .name = "rx_classified_drops", .offset = 0x13, },
	{ .name = "rx_red_prio_0", .offset = 0x14, },
	{ .name = "rx_red_prio_1", .offset = 0x15, },
	{ .name = "rx_red_prio_2", .offset = 0x16, },
	{ .name = "rx_red_prio_3", .offset = 0x17, },
	{ .name = "rx_red_prio_4", .offset = 0x18, },
	{ .name = "rx_red_prio_5", .offset = 0x19, },
	{ .name = "rx_red_prio_6", .offset = 0x1A, },
	{ .name = "rx_red_prio_7", .offset = 0x1B, },
	{ .name = "rx_yellow_prio_0", .offset = 0x1C, },
	{ .name = "rx_yellow_prio_1", .offset = 0x1D, },
	{ .name = "rx_yellow_prio_2", .offset = 0x1E, },
	{ .name = "rx_yellow_prio_3", .offset = 0x1F, },
	{ .name = "rx_yellow_prio_4", .offset = 0x20, },
	{ .name = "rx_yellow_prio_5", .offset = 0x21, },
	{ .name = "rx_yellow_prio_6", .offset = 0x22, },
	{ .name = "rx_yellow_prio_7", .offset = 0x23, },
	{ .name = "rx_green_prio_0", .offset = 0x24, },
	{ .name = "rx_green_prio_1", .offset = 0x25, },
	{ .name = "rx_green_prio_2", .offset = 0x26, },
	{ .name = "rx_green_prio_3", .offset = 0x27, },
	{ .name = "rx_green_prio_4", .offset = 0x28, },
	{ .name = "rx_green_prio_5", .offset = 0x29, },
	{ .name = "rx_green_prio_6", .offset = 0x2A, },
	{ .name = "rx_green_prio_7", .offset = 0x2B, },
	{ .name = "tx_octets", .offset = 0x40, },
	{ .name = "tx_unicast", .offset = 0x41, },
	{ .name = "tx_multicast", .offset = 0x42, },
	{ .name = "tx_broadcast", .offset = 0x43, },
	{ .name = "tx_collision", .offset = 0x44, },
	{ .name = "tx_drops", .offset = 0x45, },
	{ .name = "tx_pause", .offset = 0x46, },
	{ .name = "tx_frames_below_65_octets", .offset = 0x47, },
	{ .name = "tx_frames_65_to_127_octets", .offset = 0x48, },
	{ .name = "tx_frames_128_255_octets", .offset = 0x49, },
	{ .name = "tx_frames_256_511_octets", .offset = 0x4A, },
	{ .name = "tx_frames_512_1023_octets", .offset = 0x4B, },
	{ .name = "tx_frames_1024_1526_octets", .offset = 0x4C, },
	{ .name = "tx_frames_over_1526_octets", .offset = 0x4D, },
	{ .name = "tx_yellow_prio_0", .offset = 0x4E, },
	{ .name = "tx_yellow_prio_1", .offset = 0x4F, },
	{ .name = "tx_yellow_prio_2", .offset = 0x50, },
	{ .name = "tx_yellow_prio_3", .offset = 0x51, },
	{ .name = "tx_yellow_prio_4", .offset = 0x52, },
	{ .name = "tx_yellow_prio_5", .offset = 0x53, },
	{ .name = "tx_yellow_prio_6", .offset = 0x54, },
	{ .name = "tx_yellow_prio_7", .offset = 0x55, },
	{ .name = "tx_green_prio_0", .offset = 0x56, },
	{ .name = "tx_green_prio_1", .offset = 0x57, },
	{ .name = "tx_green_prio_2", .offset = 0x58, },
	{ .name = "tx_green_prio_3", .offset = 0x59, },
	{ .name = "tx_green_prio_4", .offset = 0x5A, },
	{ .name = "tx_green_prio_5", .offset = 0x5B, },
	{ .name = "tx_green_prio_6", .offset = 0x5C, },
	{ .name = "tx_green_prio_7", .offset = 0x5D, },
	{ .name = "tx_aged", .offset = 0x5E, },
	{ .name = "drop_local", .offset = 0x80, },
	{ .name = "drop_tail", .offset = 0x81, },
	{ .name = "drop_yellow_prio_0", .offset = 0x82, },
	{ .name = "drop_yellow_prio_1", .offset = 0x83, },
	{ .name = "drop_yellow_prio_2", .offset = 0x84, },
	{ .name = "drop_yellow_prio_3", .offset = 0x85, },
	{ .name = "drop_yellow_prio_4", .offset = 0x86, },
	{ .name = "drop_yellow_prio_5", .offset = 0x87, },
	{ .name = "drop_yellow_prio_6", .offset = 0x88, },
	{ .name = "drop_yellow_prio_7", .offset = 0x89, },
	{ .name = "drop_green_prio_0", .offset = 0x8A, },
	{ .name = "drop_green_prio_1", .offset = 0x8B, },
	{ .name = "drop_green_prio_2", .offset = 0x8C, },
	{ .name = "drop_green_prio_3", .offset = 0x8D, },
	{ .name = "drop_green_prio_4", .offset = 0x8E, },
	{ .name = "drop_green_prio_5", .offset = 0x8F, },
	{ .name = "drop_green_prio_6", .offset = 0x90, },
	{ .name = "drop_green_prio_7", .offset = 0x91, },
};

static void ocelot_pll5_init(struct ocelot *ocelot)
{
	/* Configure PLL5. This will need a proper CCF driver
	 * The values are coming from the VTSS API for Ocelot
	 */
	ocelot_write(ocelot, HSIO_PLL5G_CFG4_IB_CTRL(0x7600) |
		     HSIO_PLL5G_CFG4_IB_BIAS_CTRL(0x8), HSIO_PLL5G_CFG4);
	ocelot_write(ocelot, HSIO_PLL5G_CFG0_CORE_CLK_DIV(0x11) |
		     HSIO_PLL5G_CFG0_CPU_CLK_DIV(2) |
		     HSIO_PLL5G_CFG0_ENA_BIAS |
		     HSIO_PLL5G_CFG0_ENA_VCO_BUF |
		     HSIO_PLL5G_CFG0_ENA_CP1 |
		     HSIO_PLL5G_CFG0_SELCPI(2) |
		     HSIO_PLL5G_CFG0_LOOP_BW_RES(0xe) |
		     HSIO_PLL5G_CFG0_SELBGV820(4) |
		     HSIO_PLL5G_CFG0_DIV4 |
		     HSIO_PLL5G_CFG0_ENA_CLKTREE |
		     HSIO_PLL5G_CFG0_ENA_LANE, HSIO_PLL5G_CFG0);
	ocelot_write(ocelot, HSIO_PLL5G_CFG2_EN_RESET_FRQ_DET |
		     HSIO_PLL5G_CFG2_EN_RESET_OVERRUN |
		     HSIO_PLL5G_CFG2_GAIN_TEST(0x8) |
		     HSIO_PLL5G_CFG2_ENA_AMPCTRL |
		     HSIO_PLL5G_CFG2_PWD_AMPCTRL_N |
		     HSIO_PLL5G_CFG2_AMPC_SEL(0x10), HSIO_PLL5G_CFG2);
}

static void ocelot_port_pcs_init(struct ocelot_port *port)
{
	/* Disable HDX fast control */
	ocelot_port_writel(port, DEV_PORT_MISC_HDX_FAST_DIS, DEV_PORT_MISC);

	/* SGMII only for now */
	ocelot_port_writel(port, PCS1G_MODE_CFG_SGMII_MODE_ENA,
			   PCS1G_MODE_CFG);
	ocelot_port_writel(port, PCS1G_SD_CFG_SD_SEL, PCS1G_SD_CFG);

	/* Enable PCS */
	ocelot_port_writel(port, PCS1G_CFG_PCS_ENA, PCS1G_CFG);

	/* No aneg on SGMII */
	ocelot_port_writel(port, 0, PCS1G_ANEG_CFG);

	/* No loopback */
	ocelot_port_writel(port, 0, PCS1G_LB_CFG);
}

int ocelot_chip_init(struct ocelot *ocelot)
{
	int ret;

	ocelot->map = ocelot_regmap;
	ocelot->stats_layout = ocelot_stats_layout;
	ocelot->num_stats = ARRAY_SIZE(ocelot_stats_layout);
	ocelot->shared_queue_sz = 224 * 1024;
	ocelot->port_pcs_init = ocelot_port_pcs_init;

	ret = ocelot_regfields_init(ocelot, ocelot_regfields);
	if (ret)
		return ret;

	ocelot_pll5_init(ocelot);

	eth_random_addr(ocelot->base_mac);
	ocelot->base_mac[5] &= 0xf0;

	return 0;
}
EXPORT_SYMBOL(ocelot_chip_init);
