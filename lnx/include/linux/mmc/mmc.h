#ifndef _MMC_H_
#define _MMC_H_

#include <linux/bitops.h>

#define EXT_CSD_CMDQ_MODE_EN		15	/* R/W/E_P */
#define EXT_CSD_SECURE_REMOVAL_TYPE	16	/* R/W & R */
#define EXT_CSD_PRODUCT_STATE_AWAERNESS_ENABLEMENT 17 /* R/W/E & R */
#define EXT_CSD_MAX_PRE_LOADING_DATA_SIZE 18	/*   */
#define EXT_CSD_PRE_LOADING_DATA_SIZE	22	/*   */
#define EXT_CSD_FFU_STATUS		26	/* R */
#define EXT_CSD_MODE_OPERATION_CODES	29	/* W */
#define EXT_CSD_MODE_CONFIG		30	/* R/W */
#define EXT_CSD_FLUSH_CACHE		32	/* W */
#define EXT_CSD_CACHE_CTRL		33	/* R/W */
#define EXT_CSD_POWER_OFF_NOTIFICATION	34	/* R/W */
#define EXT_CSD_PACKED_FAILURE_INDEX	35	/* RO */
#define EXT_CSD_PACKED_CMD_STATUS	36	/* RO */
#define EXT_CSD_EXP_EVENTS_STATUS	54	/* RO, 2 bytes */
#define EXT_CSD_EXP_EVENTS_CTRL		56	/* R/W, 2 bytes */
#define EXT_CSD_DATA_SECTOR_SIZE	61	/* R */
#define EXT_CSD_VENDOR_SPECIFIC_FIELDS_73 73 
#define EXT_CSD_GP_SIZE_MULT		143	/* R/W */
#define EXT_CSD_PARTITION_ATTRIBUTE	156	/* R/W */
#define EXT_CSD_PARTITION_SUPPORT	160	/* RO */
#define EXT_CSD_HPI_MGMT		161	/* R/W */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_BKOPS_EN		163	/* R/W */
#define EXT_CSD_BKOPS_START		164	/* W */
#define EXT_CSD_SANITIZE_START		165	/* W */
#define EXT_CSD_WR_REL_PARAM		166	/* RO */
#define EXT_CSD_RPMB_MULT		168	/* RO */
#define EXT_CSD_FW_CONFIG		169	/* R/W */
#define EXT_CSD_BOOT_WP			173	/* R/W */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_PART_CONFIG		179	/* R/W */
#define EXT_CSD_ERASED_MEM_CONT		181	/* RO */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_POWER_CLASS		187	/* R/W */
#define EXT_CSD_REV			192	/* RO */
#define EXT_CSD_STRUCTURE		194	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_DRIVE_STRENGTH		197	/* RO */
#define EXT_CSD_OUT_OF_INTERRUPT_TIME	198	/* RO */
#define EXT_CSD_PART_SWITCH_TIME	199	/* RO */
#define EXT_CSD_PWR_CL_52_195		200	/* RO */
#define EXT_CSD_PWR_CL_26_195		201	/* RO */
#define EXT_CSD_PWR_CL_52_360		202	/* RO */
#define EXT_CSD_PWR_CL_26_360		203	/* RO */
#define EXT_CSD_SEC_CNT			212	/* RO, 4 bytes */
#define EXT_CSD_S_A_TIMEOUT		217	/* RO */
#define EXT_CSD_REL_WR_SEC_C		222	/* RO */
#define EXT_CSD_HC_WP_GRP_SIZE		221	/* RO */
#define EXT_CSD_ERASE_TIMEOUT_MULT	223	/* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		226	/* RO */
#define EXT_CSD_SEC_TRIM_MULT		229	/* RO */
#define EXT_CSD_SEC_ERASE_MULT		230	/* RO */
#define EXT_CSD_SEC_FEATURE_SUPPORT	231	/* RO */
#define EXT_CSD_TRIM_MULT		232	/* RO */
#define EXT_CSD_PWR_CL_200_195		236	/* RO */
#define EXT_CSD_PWR_CL_200_360		237	/* RO */
#define EXT_CSD_PWR_CL_DDR_52_195	238	/* RO */
#define EXT_CSD_PWR_CL_DDR_52_360	239	/* RO */
#define EXT_CSD_CORRECTLY_PRG_SECTORS_NUM 242	/* RO, 4 bytes */
#define EXT_CSD_BKOPS_STATUS		246	/* RO */
#define EXT_CSD_POWER_OFF_LONG_TIME	247	/* RO */
#define EXT_CSD_GENERIC_CMD6_TIME	248	/* RO */
#define EXT_CSD_CACHE_SIZE		249	/* RO, 4 bytes */
#define EXT_CSD_PWR_CL_DDR_200_360	253	/* RO */
#define EXT_CSD_VENDOR_SPECIFIC_FIELDS_258 258 
#define EXT_CSD_NUM_OF_FW_SEC_PROG	302	/* RO */
#define EXT_CSD_CMDQ_DEPTH		307	/* RO */
#define EXT_CSD_CMDQ_SUPPORT		308	/* RO */
#define EXT_CSD_FFU_ARG			487	/* RO, 4 bytes */
#define EXT_CSD_OPERATION_CODE_TIMEOUT	491	/* RO */
#define EXT_CSD_FFU_FEATURES		492	/* RO */
#define EXT_CSD_SUPPORTED_MODE		493	/* RO */
#define EXT_CSD_TAG_UNIT_SIZE		498	/* RO */
#define EXT_CSD_DATA_TAG_SUPPORT	499	/* RO */
#define EXT_CSD_MAX_PACKED_WRITES	500	/* RO */
#define EXT_CSD_MAX_PACKED_READS	501	/* RO */
#define EXT_CSD_BKOPS_SUPPORT		502	/* RO */
#define EXT_CSD_HPI_FEATURES		503	/* RO */

/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_BKOPS_EN_MANUAL_EN	BIT(0)
#define EXT_CSD_BKOPS_EN_AUTO_EN	BIT(1)

#define EXT_CSD_WR_REL_PARAM_EN		(1<<2)

#define EXT_CSD_BOOT_WP_B_PWR_WP_DIS	(0x40)
#define EXT_CSD_BOOT_WP_B_PERM_WP_DIS	(0x10)
#define EXT_CSD_BOOT_WP_B_PERM_WP_EN	(0x04)
#define EXT_CSD_BOOT_WP_B_PWR_WP_EN	(0x01)

#define EXT_CSD_PART_CONFIG_ACC_MASK	(0x7)
#ifdef CONFIG_LGE_MMC_CQ_ENABLE
#define EXT_CSD_PART_CONFIG_USER	(0x0)
#endif
#define EXT_CSD_PART_CONFIG_ACC_BOOT0	(0x1)
#define EXT_CSD_PART_CONFIG_ACC_RPMB	(0x3)
#define EXT_CSD_PART_CONFIG_ACC_GP0	(0x4)

#define EXT_CSD_PART_SUPPORT_PART_EN	(0x1)

#define EXT_CSD_CMD_SET_NORMAL		(1<<0)
#define EXT_CSD_CMD_SET_SECURE		(1<<1)
#define EXT_CSD_CMD_SET_CPSECURE	(1<<2)

#define EXT_CSD_CARD_TYPE_26	(1<<0)	/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52	(1<<1)	/* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_MASK	0xFF	/* Mask out reserved bits */
#define EXT_CSD_CARD_TYPE_DDR_1_8V  (1<<2)   /* Card can run at 52MHz */
					     /* DDR mode @1.8V or 3V I/O */
#define EXT_CSD_CARD_TYPE_DDR_1_2V  (1<<3)   /* Card can run at 52MHz */
					     /* DDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_DDR_52       (EXT_CSD_CARD_TYPE_DDR_1_8V  \
					| EXT_CSD_CARD_TYPE_DDR_1_2V)
#define EXT_CSD_CARD_TYPE_SDR_1_8V	(1<<4)	/* Card can run at 200MHz */
#define EXT_CSD_CARD_TYPE_SDR_1_2V	(1<<5)	/* Card can run at 200MHz */
						/* SDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_HS200		(EXT_CSD_CARD_TYPE_SDR_1_8V  \
					| EXT_CSD_CARD_TYPE_SDR_1_2V)
#define EXT_CSD_CARD_TYPE_HS400_1_8V	(1<<6)	/* Card can run at 200MHz */
							/* DDR mode @1.8V I/O */
#define EXT_CSD_CARD_TYPE_HS400_1_2V	(1<<7)	/* Card can run at 200MHz */
							/* DDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_HS400		(EXT_CSD_CARD_TYPE_HS400_1_8V  \
					| EXT_CSD_CARD_TYPE_HS400_1_2V)

#define EXT_CSD_BUS_WIDTH_1	0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2	/* Card is in 8 bit mode */
#define EXT_CSD_DDR_BUS_WIDTH_4	5	/* Card is in 4 bit DDR mode */
#define EXT_CSD_DDR_BUS_WIDTH_8	6	/* Card is in 8 bit DDR mode */

#define EXT_CSD_SEC_ER_EN	BIT(0)
#define EXT_CSD_SEC_BD_BLK_EN	BIT(2)
#define EXT_CSD_SEC_GB_CL_EN	BIT(4)
#define EXT_CSD_SEC_SANITIZE	BIT(6)  /* v4.5 only */

#define EXT_CSD_RST_N_EN_MASK	0x3
#define EXT_CSD_RST_N_ENABLED	1	/* RST_n is enabled on card */

#define EXT_CSD_NO_POWER_NOTIFICATION	0
#define EXT_CSD_POWER_ON		1
#define EXT_CSD_POWER_OFF_SHORT		2
#define EXT_CSD_POWER_OFF_LONG		3

#define EXT_CSD_PWR_CL_8BIT_MASK	0xF0	/* 8 bit PWR CLS */
#define EXT_CSD_PWR_CL_4BIT_MASK	0x0F	/* 8 bit PWR CLS */
#define EXT_CSD_PWR_CL_8BIT_SHIFT	4
#define EXT_CSD_PWR_CL_4BIT_SHIFT	0

#define EXT_CSD_PACKED_EVENT_EN	BIT(3)

/*
 * EXCEPTION_EVENT_STATUS field
 */
#define EXT_CSD_URGENT_BKOPS		BIT(0)
#define EXT_CSD_DYNCAP_NEEDED		BIT(1)
#define EXT_CSD_SYSPOOL_EXHAUSTED	BIT(2)
#define EXT_CSD_PACKED_FAILURE		BIT(3)

#define EXT_CSD_PACKED_GENERIC_ERROR	BIT(0)
#define EXT_CSD_PACKED_INDEXED_ERROR	BIT(1)

/*
 * BKOPS status level
 */
#define EXT_CSD_BKOPS_LEVEL_2		0x2

/* CMDQ enable level */
#define EXT_CSD_CMDQ_MODE_OFF		0
#define EXT_CSD_CMDQ_MODE_ON		1

/*
 * MMC_SWITCH access modes
 */

#define MMC_SWITCH_MODE_CMD_SET		0x00	/* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01	/* Set bits which are 1 in value */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02	/* Clear bits which are 1 in value */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03	/* Set target to value */

#endif

