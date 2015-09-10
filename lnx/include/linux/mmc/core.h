#ifndef _CORE_H_
#define _CORE_H_

#include <linux/scatterlist.h>

struct mmc_host;

void mmc_claim_host(struct mmc_host *host);
void mmc_release_host(struct mmc_host *host);
int mmc_try_claim_host(struct mmc_host *host);

struct mmc_card;
int mmc_simple_transfer(struct mmc_card *card,
	struct scatterlist *sg, unsigned sg_len, unsigned dev_addr,
	unsigned blocks, unsigned blksz, int write);

#endif

