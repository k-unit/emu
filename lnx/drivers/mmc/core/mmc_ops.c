#include <linux/types.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>

#include <linux/mmc/kut_core.h>

#include "../../../../kut/drivers/mmc/core/mmc_ops.h"

#include <string.h>

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

	if (kut_mmc_ext_csd_set(index, value))
		return -EINVAL;

	kut_ext_csd_verify_configuration();
	return 0;
}
EXPORT_SYMBOL_GPL(mmc_switch);

int mmc_send_ext_csd(struct mmc_card *card, u8 *ext_csd)
{
	int ret;

	/* verify ext_csd has been initialized legaly during pre-test */
	ret = kut_ext_csd_verify_configuration();
	if (ret)
		return ret;

	memcpy(ext_csd, kut_ext_csd, 512);
	return 0;
}
EXPORT_SYMBOL_GPL(mmc_send_ext_csd);

