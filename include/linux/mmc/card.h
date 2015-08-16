#ifndef _CARD_H_
#define _CARD_H_

#include <linux/device.h>

struct mmc_host;

struct mmc_card {
	struct mmc_host *host;
	struct device dev; /* the device */
};

static inline int mmc_card_mmc(struct mmc_card *card)
{
	return true;
}

#endif

