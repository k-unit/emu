#ifndef _KUT_BUS_H_
#define _KUT_BUS_H_

#include <linux/device.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

/* A kUnit structure for verifying data transfer (read/write) by various mmc
 * driver functions.
 */
struct kut_mmc_card_xfer {
	bool write;		/* transfer expected direction */
	unsigned long addr;	/* transfer expected start address */
	unsigned long length;	/* transfer expected length */
	char *buf;		/* transfer expected content */
	struct list_head list;	/* list for linking multiple xfer operations */
};

int kut_mmc_xfer_add(struct mmc_card *card, struct scatterlist *sg,
	unsigned nents, unsigned long addr, unsigned long length, bool write);
void kut_mmc_xfer_reset(struct mmc_card *card);
int kut_mmc_xfer_check(struct mmc_card *card, struct list_head *expected);

int kut_mmc_free_card(struct mmc_card *card);
struct mmc_card *kut_mmc_alloc_card(struct mmc_host *host,
	struct device_type *type, bool create_main);

struct device *kut_mmc_card_md_main(struct mmc_card *card);
struct device *kut_mmc_add_partition(struct mmc_card *card, int index);
#endif

