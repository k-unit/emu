#ifndef _KUT_CORE_H_
#define _KUT_CORE_H_

#include <linux/types.h>
#include <linux/mmc/mmc.h>

extern u8 kut_ext_csd[512];

/* these macros should be used after setting ext_csd[EXT_CSD_DATA_SECTOR_SIZE],
 * otherwise the default block sizeof 512 is used */
#define BYTES_TO_BLOCKS(x) ((x) >> \
	(kut_ext_csd[EXT_CSD_DATA_SECTOR_SIZE] ? 12 : 9))
#define BLOCKS_TO_BYTES(x) ((x) << \
	(kut_ext_csd[EXT_CSD_DATA_SECTOR_SIZE] ? 12 : 9))

int kut_mmc_ext_csd_set(u16 index, u32 value);
int kut_mmc_ext_csd_set_rev(s8 rev);
void kut_mmc_ext_csd_reset(u8 rev);
int kut_ext_csd_verify_configuration(void);
int kut_mmc_ext_csd_set_ffu(s8 ver, s8 ffu_status, s8 mode_operation_codes,
	s8 mode_config, s8 data_sector_size, s8 fw_config, s32 ffu_arg,
	s8 ffu_features, s8 operation_code_timeout, s8 supported_mode);

#endif

