#include <linux/pagemap.h>
#include <linux/device.h>
#include <linux/mmc/host.h>

#include <linux/kut_device.h>
#include <linux/mmc/kut_core.h>
#include <linux/mmc/kut_host.h>

/**
 *	mmc_alloc_host - initialise the per-host structure.
 *	@extra: sizeof private data structure
 *	@dev: pointer to host device model structure
 *
 *	Initialise the per-host structure.
 */
struct mmc_host *mmc_alloc_host(int extra, struct device *dev)
{
	struct mmc_host *host;

	host = kzalloc(sizeof(struct mmc_host) + extra, GFP_KERNEL);
	if (!host)
		return NULL;

	host->parent = dev;
	host->class_dev.parent = dev;
	host->index = kut_host_index_get_next();

	kut_dev_init(&host->class_dev, dev, "mmc%d", host->index);

	/*
	 * By default, hosts do not support SGIO or large requests.
	 * They have to set these according to their abilities.
	 */
	host->max_segs = 1 * 512;
	host->max_seg_size = 128 * 512;

	host->max_req_size = 65536 * 512;
	host->max_blk_count = 65536;

	return host;
}
EXPORT_SYMBOL(mmc_alloc_host);

/**
 *	mmc_free_host - free the host structure
 *	@host: mmc host
 *
 *	Free the host once all references to it have been dropped.
 */
void mmc_free_host(struct mmc_host *host)
{
	if (!host)
		return;
	kut_dev_uninit(&host->class_dev);
	kfree(host);
}
EXPORT_SYMBOL(mmc_free_host);

int mmc_power_save_host(struct mmc_host *host)
{
	return 0;
}
EXPORT_SYMBOL(mmc_power_save_host);

int mmc_power_restore_host(struct mmc_host *host)
{
	return 0;
}
EXPORT_SYMBOL(mmc_power_restore_host);

