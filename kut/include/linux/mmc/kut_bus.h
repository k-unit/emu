#ifndef _KUT_BUS_H_
#define _KUT_BUS_H_

#include <linux/device.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

int kut_mmc_free_card(struct mmc_card *card);
struct mmc_card *kut_mmc_alloc_card(struct mmc_host *host,
	struct device_type *type, bool create_main);

struct device *kut_mmc_card_md_main(struct mmc_card *card);
struct device *kut_mmc_add_partition(struct mmc_card *card, int index);
#endif

