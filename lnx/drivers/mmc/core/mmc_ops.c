#include <linux/types.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>

int mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value,
	unsigned int timeout_ms)
{
	switch (index) {
	case EXT_CSD_MODE_CONFIG:
	case EXT_CSD_MODE_OPERATION_CODES:
		break;

	default:
		/* add supported values as required */
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(mmc_switch);

int mmc_send_ext_csd(struct mmc_card *card, u8 *ext_csd)
{
	memset(ext_csd, 0, 512);

	ext_csd[EXT_CSD_OPERATION_CODE_TIMEOUT] = 0x1;
	return 0;
}
EXPORT_SYMBOL_GPL(mmc_send_ext_csd);

