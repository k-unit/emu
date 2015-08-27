#ifndef _CORE_H_
#define _CORE_H_

struct mmc_host;

void mmc_claim_host(struct mmc_host *host);
void mmc_release_host(struct mmc_host *host);
int mmc_try_claim_host(struct mmc_host *host);

#endif

