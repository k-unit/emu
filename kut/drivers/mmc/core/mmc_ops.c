#include <linux/types.h>
#include <linux/export.h>
#include <linux/mmc/mmc.h>

#include <unit_test.h>

#define ext_csd_err(fmt, ...) pr_err("ext_csd error (%s): " fmt "\n", \
	__FUNCTION__, ##__VA_ARGS__)

#define EXT_CSD_TYPE_R 1<<0
#define EXT_CSD_TYPE_W 1<<1
#define EXT_CSD_TYPE_R_W 1<<2
#define EXT_CSD_TYPE_W_E 1<<3
#define EXT_CSD_TYPE_R_W_E 1<<4
#define EXT_CSD_TYPE_R_W_CP 1<<5
#define EXT_CSD_TYPE_R_W_EP 1<<6
#define EXT_CSD_TYPE_W_EP 1<<7

u8 kut_ext_csd[512];

/* the ext_csd_supported array must remain sorted by index */
static struct ext_csd_descriptor {
	u16 index;
	char *description;
	u8 size;
	u8 type;
	int rev;
} ext_csd_supported[] = {
	{
		.index = EXT_CSD_CMDQ_MODE_EN, /* 15 */
		.description = "Command Queue Mode Enable",
		.size = 1,
		.type = EXT_CSD_TYPE_R_W_EP,
		.rev = 8,
	},
	{
		.index = EXT_CSD_SECURE_REMOVAL_TYPE, /* 16 */
		.description = "Secure Removal Type",
		.size = 1,
		.type = EXT_CSD_TYPE_R_W | EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_PRODUCT_STATE_AWAERNESS_ENABLEMENT, /* 17 */
		.description = "Product state awareness enablement",
		.size = 1,
		.type = EXT_CSD_TYPE_R_W_E | EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_MAX_PRE_LOADING_DATA_SIZE, /* 18 */
		.description = "Max preloading data size",
		.size = 4,
		.type = EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_PRE_LOADING_DATA_SIZE, /* 22 */
		.description = "Pre loading data size",
		.size = 4,
		.type = EXT_CSD_TYPE_R_W_EP,
		.rev = 0,
	},
	{
		.index = EXT_CSD_FFU_STATUS, /* 26 */
		.description = "FFU status",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 7,
	},
	{
		.index = EXT_CSD_MODE_OPERATION_CODES, /* 29 */
		.description = "Mode operation codes",
		.size = 1,
		.type = EXT_CSD_TYPE_W_EP,
		.rev = 7,
	},
	{
		.index = EXT_CSD_MODE_CONFIG, /* 30 */
		.description = "Mode config",
		.size = 1,
		.type = EXT_CSD_TYPE_R_W_EP,
		.rev = 7,
	},
	{
		.index = EXT_CSD_DATA_SECTOR_SIZE, /* 61 */
		.description = "Sector size",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_FW_CONFIG, /* 169 */
		.description = "FW configuration",
		.size = 1,
		.type = EXT_CSD_TYPE_R_W,
		.rev = 7,
	},
	{
		.index = EXT_CSD_REV,  /*192 */
		.description = "Extended CSD revision",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_SEC_CNT,  /*212 */
		.description = "Sector Count",
		.size = 4,
		.type = EXT_CSD_TYPE_R,
		.rev = 2,
	},
	{
		.index = EXT_CSD_GENERIC_CMD6_TIME, /* 248 */
		.description = "Generic CMD6 timeout",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_NUM_OF_FW_SEC_PROG, /* 302 */
		.description = "Number of FW sectors correctly programmed",
		.size = 4,
		.type = EXT_CSD_TYPE_R,
		.rev = 0,
	},
	{
		.index = EXT_CSD_FFU_ARG, /* 487 */
		.description = "FFU Argument",
		.size = 4,
		.type = EXT_CSD_TYPE_R,
		.rev = 7,
	},
	{
		.index = EXT_CSD_OPERATION_CODE_TIMEOUT, /* 491 */
		.description = "Operation codes timeout",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 7,
	},
	{
		.index = EXT_CSD_FFU_FEATURES, /* 492 */
		.description = "FFU features",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 7,
	},
	{
		.index = EXT_CSD_SUPPORTED_MODE, /* 493 */
		.description = "Supported modes",
		.size = 1,
		.type = EXT_CSD_TYPE_R,
		.rev = 7,
	},
};

static u32 ext_csd_val_to_u32(u16 index)
{
	u32 val;

	val = kut_ext_csd[index] | kut_ext_csd[index + 1] << 8 |
		kut_ext_csd[index + 2] << 16 | kut_ext_csd[index + 3] << 24;

	return val;
}

static bool ext_csd_supported_locate_index(u16 ext_csd_index, u16 *index)
{
	u16 start = 0;
	u16 end = ARRAY_SZ(ext_csd_supported) - 1;

	while (start <= end) {
		u16 mid = (start + end) / 2;

		if (ext_csd_supported[mid].index == ext_csd_index) {
			*index = mid;
			return true;
		}

		if (ext_csd_supported[mid].index < ext_csd_index)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return false;
}

static int kut_ext_csd_verify_entry(u16 index, u32 value)
{
	switch (index) {
	/*
	   |-+-+-+-+-+-+-+-|
	   |R|R|R|R|R|R|R| |
	   |-+-+-+-+-+-+-+-|
	   |7 6 5 4 3 2 1 0|
	*/
	case EXT_CSD_CMDQ_MODE_EN:
	case EXT_CSD_DATA_SECTOR_SIZE:
	case EXT_CSD_FW_CONFIG:
	case EXT_CSD_FFU_FEATURES:
		if ((u8)value & ~(u8)1)
			return -EINVAL;
		break;
	/*
	   |-+-+-+-+-+-+-+-|
	   |R|R| | | | | | |
	   |-+-+-+-+-+-+-+-|
	   |7 6 5 4 3 2 1 0|
	*/
	case EXT_CSD_SECURE_REMOVAL_TYPE:
	case EXT_CSD_PRODUCT_STATE_AWAERNESS_ENABLEMENT:
		if ((u8)value & (u8)0xc0)
			return -EINVAL;
		break;
	/* any value is valid */
	case EXT_CSD_MAX_PRE_LOADING_DATA_SIZE:
	case EXT_CSD_PRE_LOADING_DATA_SIZE:
	case EXT_CSD_GENERIC_CMD6_TIME:
	case EXT_CSD_NUM_OF_FW_SEC_PROG:
	case EXT_CSD_FFU_ARG:
		break;
	case EXT_CSD_FFU_STATUS:
		if ((u8)value != 0x00 && (u8)value != 0x10 &&
			(u8)value != 0x11 && (u8)value != 0x12) {
			return -EINVAL;
		}
		break;
	case EXT_CSD_MODE_OPERATION_CODES:
		if ((u8)value != 0x00 && (u8)value != 0x01 && (u8)value != 0x02)
			return -EINVAL;
		break;
	case EXT_CSD_MODE_CONFIG:
		if ((u8)value != 0x00 && (u8)value != 0x01 && (u8)value != 0x10)
			return -EINVAL;
		break;
	case EXT_CSD_REV:
		if (8 < (u8)value)
			return -EINVAL;
		break;
	case EXT_CSD_SEC_CNT:
		if (4294967295 < value)
			return -EINVAL;
		break;
	case EXT_CSD_OPERATION_CODE_TIMEOUT:
		if (0x17 < (u8)value)
			return -EINVAL;
		break;
	/*
	   |-+-+-+-+-+-+-+-|
	   |R|R|R|R|R|R| | |
	   |-+-+-+-+-+-+-+-|
	   |7 6 5 4 3 2 1 0|
	*/
	case EXT_CSD_SUPPORTED_MODE:
		if ((u8)value & (u8)0xfc)
			return -EINVAL;
		break;
	default:
		ext_csd_err("ext_csd[%d] not supported", index);
		return -EINVAL;
	}

	return 0;
}

int kut_mmc_ext_csd_set(u16 index, u32 value)
{
	u16 i = 0;

	kut_bug_on_do_exit_set(true);

	BUG_ON(!ext_csd_supported_locate_index(index, &i));
	BUG_ON(kut_ext_csd_verify_entry(index, value));

	switch (ext_csd_supported[i].size) {
	case 1:
		kut_ext_csd[index] = (u8)value;
		break;
	case 2:
		kut_ext_csd[index + 0] = (u8)(value & 0xff);
		kut_ext_csd[index + 1] = (u8)((value >> 8) & 0xff);
		break;
	case 4:
		kut_ext_csd[index + 0] = (u8)(value & 0xff);
		kut_ext_csd[index + 1] = (u8)((value >> 8) & 0xff);
		kut_ext_csd[index + 2] = (u8)((value >> 16) & 0xff);
		kut_ext_csd[index + 3] = (u8)((value >> 24) & 0xff);
		break;
	default:
		BUG_ON(ext_csd_supported[i].size || 1);
	}

	kut_bug_on_do_exit_set(false);

	return 0;
}

int kut_mmc_ext_csd_set_rev(s8 rev)
{
	u8 __rev;

	if (rev == -1)
		return 0;

	__rev = (u32)rev;
	switch (__rev) {
	case 0: /* Revision 1.0 (for MMC v4.0) */
	case 1: /* Revision 1.1 (for MMC v4.1) */
	case 2: /* Revision 1.2 (for MMC v4.2) */
	case 3: /* Revision 1.3 (for MMC v4.3) */
	case 4: /* Revision 1.4 (Obsolete) */
	case 5: /* Revision 1.5 (for MMC v4.41) */
	case 6: /* Revision 1.6 (for MMC v4.5, v4.51) */
	case 7: /* Revision 1.7 (for MMC v5.0, v5.01) */
	case 8: /* Revision 1.8 (for MMC v5.1) */
		break;
	default: /* Reserved */
		ext_csd_err("unsuported ext_csd revision: %d", __rev);
		return -1;
	}

	return kut_mmc_ext_csd_set(EXT_CSD_REV, __rev);
}

void kut_mmc_ext_csd_reset(u8 rev)
{
	memset(kut_ext_csd, 0, sizeof(kut_ext_csd));
	kut_mmc_ext_csd_set_rev((s8)rev);

	/* ext csd supported by revision:
	 * ------------------------------
	 *
	 * rev >= 2
	 * - sector count
	 *
	 * rev >= 3 
	 * - sleep/awake timeout
	 * - partition config 
	 * - switch timeout
	 * - high density erase group definition
	 * - erase timeout
	 * - high capacity erase group size
	 * - reliable count write sector count
	 * - boot partition size
	 *
	 * rev >= 4
	 * - partition support
	 * - general purpose partition size
	 * - trim
	 * - erase
	 * - secure feature support
	 * - boot write protect
	 *
	 * rev >= 5
	 * - hpi
	 * - bkops
	 * - write reliability parameter register
	 * - hw reset function
	 * - rpmb size multiple
	 *
	 * rev >= 6
	 * - discard support
	 * - generic cmdd6 timeout
	 * - power off long timeout
	 * - cache size
	 * - sector size
	 * - data tag
	 * - packed commands
	 *
	 * rev >= 7
	 * - production state awareness
	 * - ffu
	 *
	 * rev >= 8
	 * - cmdq support
	 */
}

int kut_ext_csd_verify_configuration(void)
{
	int i;

	kut_bug_on_do_exit_set(true);

	/* verify revision conformance */
	for (i = 0; i < ARRAY_SZ(ext_csd_supported); i++) {
		u16 index = ext_csd_supported[i].index;

		/* test for revision violation */
		if (ext_csd_supported[i].rev > kut_ext_csd[EXT_CSD_REV]) {
			int j;

			for (j = 0; j < ext_csd_supported[i].size &&
				!kut_ext_csd[index + j]; j++);
			if (j != ext_csd_supported[i].size) {
				ext_csd_err("ext_csd revision violation " \
					"(ext_csd[%d]:set conflicts with " \
					"ext_csd[EXT_CSD_REV]:%d)", index + j,
					kut_ext_csd[EXT_CSD_REV]);
				BUG_ON(-EINVAL);
			}

			continue;
		}
	}

	/* verify field dependencies */
	if (ext_csd_val_to_u32(EXT_CSD_PRE_LOADING_DATA_SIZE) >
		ext_csd_val_to_u32(EXT_CSD_MAX_PRE_LOADING_DATA_SIZE)) {
			BUG_ON(-EINVAL);
	}

	kut_bug_on_do_exit_set(false);

	return 0;
}

int kut_mmc_ext_csd_set_ffu(s8 ver, s8 mode_operation_codes, s8 mode_config,
	s8 data_sector_size, s8 fw_config, s32 ffu_arg, s8 ffu_features,
	s8 supported_mode)
{
	int ret;

	kut_mmc_ext_csd_reset((u8)ver);

	if (mode_operation_codes != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_MODE_OPERATION_CODES,
			(u8)mode_operation_codes);
		if (ret)
			return ret;
	}

	if (mode_config != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_MODE_CONFIG, (u8)mode_config);
		if (ret)
			return ret;
	}

	if (data_sector_size != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_DATA_SECTOR_SIZE,
			(u8)data_sector_size);
		if (ret)
			return ret;
	}

	if (fw_config != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_FW_CONFIG, (u8)fw_config);
		if (ret)
			return ret;
	}

	if (ffu_arg != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_FFU_ARG, (u8)ffu_arg);
		if (ret)
			return ret;
	}

	if (ffu_features != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_FFU_FEATURES,
			(u8)ffu_features);
		if (ret)
			return ret;
	}

	if (supported_mode != -1) {
		ret = kut_mmc_ext_csd_set(EXT_CSD_SUPPORTED_MODE,
			(u8)supported_mode);
		if (ret)
			return ret;
	}

	return kut_ext_csd_verify_configuration();
}

