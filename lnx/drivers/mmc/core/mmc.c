#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/types.h>
#include <asm-generic/bug.h>

/*
 * Decode extended CSD.
 */
int mmc_read_ext_csd(struct mmc_card *card, u8 *ext_csd)
{
	int err = 0;

	BUG_ON(!card);

	if (!ext_csd)
		return 0;

	card->ext_csd.rev = ext_csd[EXT_CSD_REV];
	if (card->ext_csd.rev > 8) {
		pr_err("%s: unrecognised EXT_CSD revision %d\n",
			mmc_hostname(card->host), card->ext_csd.rev);
		err = -EINVAL;
		goto out;
	}

	/* eMMC v4.5 or later */
	if (card->ext_csd.rev >= 6) {
		card->ext_csd.generic_cmd6_time = 10 *
			ext_csd[EXT_CSD_GENERIC_CMD6_TIME];

		if (ext_csd[EXT_CSD_DATA_SECTOR_SIZE] == 1)
			card->ext_csd.data_sector_size = 4096;
		else
			card->ext_csd.data_sector_size = 512;
	}

	/* eMMC v5 or later */
	if (card->ext_csd.rev >= 7) {
		card->ext_csd.ffu_capable =
			((ext_csd[EXT_CSD_SUPPORTED_MODE] & 0x1) == 0x1) &&
			((ext_csd[EXT_CSD_FW_CONFIG] & 0x1) == 0x0);
		card->ext_csd.ffu_mode_op = ext_csd[EXT_CSD_FFU_FEATURES];
	} else {
			card->ext_csd.ffu_capable = false;
	}

out:
	return err;
}

