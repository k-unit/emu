#include <linux/mmc/host.h>

#include <linux/mmc/kut_bus.h>

/**
 *	mmc_claim_host - exclusively claim a host
 *	@host: mmc host to claim
 *
 *	Claim a host for a set of operations.
 */
void mmc_claim_host(struct mmc_host *host)
{
	host->claimed = 1;
}

/**
 *	mmc_release_host - release a host
 *	@host: mmc host to release
 *
 *	Release a MMC host, allowing others to claim the host
 *	for their operations.
 */
void mmc_release_host(struct mmc_host *host)
{
	host->claimed = 0;
}

/**
 *	mmc_try_claim_host - try exclusively to claim a host
 *	@host: mmc host to claim
 *
 *	Returns %1 if the host is claimed, %0 otherwise.
 */
int mmc_try_claim_host(struct mmc_host *host)
{
	host->claimed = 1;
	return 1;
}

/*
 * transfer with certain parameters
 */
int mmc_simple_transfer(struct mmc_card *card,
	struct scatterlist *sg, unsigned sg_len, unsigned dev_addr,
	unsigned blocks, unsigned blksz, int write)
{
	unsigned long length = blocks * blksz;

	kut_mmc_xfer_add(card, sg, sg_len, dev_addr, length, write);
	return 0;
}
EXPORT_SYMBOL(mmc_simple_transfer);

int mmc_flush_cache(struct mmc_card *card)
{
	return 0;
}
EXPORT_SYMBOL(mmc_flush_cache);

