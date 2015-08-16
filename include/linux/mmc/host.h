#ifndef _HOST_H_
#define _HOST_H_

#include <linux/device.h>

struct mmc_card;

struct mmc_host {
	struct device *parent;
	struct device class_dev;
	struct mmc_card *card;
};

#endif

