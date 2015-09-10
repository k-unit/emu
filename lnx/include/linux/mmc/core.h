#ifndef _CORE_H_
#define _CORE_H_

#include <linux/scatterlist.h>
#include <linux/types.h>
#include <linux/mmc/card.h>

struct mmc_host;

struct mmc_card;
int mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value,
	unsigned int timeout_ms);

void mmc_claim_host(struct mmc_host *host);
void mmc_release_host(struct mmc_host *host);
int mmc_try_claim_host(struct mmc_host *host);

int mmc_simple_transfer(struct mmc_card *card,
	struct scatterlist *sg, unsigned sg_len, unsigned dev_addr,
	unsigned blocks, unsigned blksz, int write);

#endif

