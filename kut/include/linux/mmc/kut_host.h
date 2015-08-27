#ifndef _KUT_HOST_H_
#define _KUT_HOST_H_

#include <linux/mmc/host.h>

#define KUNIT_HOST_NAME "kunit-hc"

struct kunit_host {
	struct device dev;
	struct mmc_host *mmc;
};

struct kunit_host *kut_kunit_host_alloc(struct device *bus);
void kut_kunit_host_free(struct kunit_host *host);

int kut_mmc_init(struct device *bus, struct kunit_host **host,
	struct mmc_host **mmc, struct mmc_card **card, u8 partitions);
void kut_mmc_uninit(struct kunit_host *host, struct mmc_host *mmc,
	struct mmc_card *card);

void kut_host_index_reset(void);
int kut_host_index_get_next(void);

#endif

