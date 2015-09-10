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

