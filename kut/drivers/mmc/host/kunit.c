#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <linux/kut_device.h>
#include <linux/mmc/kut_host.h>
#include <linux/mmc/kut_bus.h>

void kut_kunit_host_free(struct kunit_host *host)
{
	if (!host)
		return;
	kut_dev_uninit(&host->dev);
	kfree(host);
}

struct kunit_host *kut_kunit_host_alloc(struct device *bus)
{
	struct kunit_host *host;

	host = kzalloc(sizeof(struct kunit_host), GFP_KERNEL);
	if (!host)
		return NULL;

	if (!kut_dev_init(&host->dev, bus, KUNIT_HOST_NAME)) {
		kfree(host);
		return NULL;
	}

	return host;
}

void kut_mmc_uninit(struct kunit_host *host, struct mmc_host *mmc,
	struct mmc_card *card)
{
	kut_mmc_free_card(card);
	mmc_free_host(mmc);
	kut_kunit_host_free(host);
}

int kut_mmc_init(struct device *bus, struct kunit_host **host,
	struct mmc_host **mmc, struct mmc_card **card, u8 partitions)
{
	struct kunit_host *__host = NULL;
	struct mmc_host *__mmc = NULL;
	struct mmc_card *__card = NULL;

	if (host) {
		__host = kut_kunit_host_alloc(bus);
		if (!__host)
			goto error;
	}

	if (mmc) {
		int extra;
		struct device *dev;

		if (host) {
			extra = sizeof(struct kunit_host);
			dev = &__host->dev;
		} else {
			extra = 0;
			dev = NULL;
		}

		__mmc = mmc_alloc_host(extra, dev);
		if (!__mmc)
			goto error;

		if (__host) {
			__mmc->private = __host;
			__host->mmc = __mmc;
		}

		if (host) {
			int len = strlen(dev->__name);

			snprintf(dev->__name + len, DEV_MAX_NAME - len, ".%d",
				__mmc->index);
		}
	}

	if (card) {
		struct device *md_dev;
		int p;

		__card = kut_mmc_alloc_card(__mmc, NULL, true);
		if (!__card)
			goto error;

		md_dev = kut_mmc_card_md_main(__card);
		for (p = 1; p <= partitions; p++) {
			struct device *dev;

			dev = kut_dev_init(NULL, md_dev, "%sp%d",
				dev_name(md_dev), p);
			if (!dev)
				goto error;
		}
	}

	if (host)
		*host = __host;
	if (mmc)
		*mmc = __mmc;
	if (card)
		*card = __card;

	return 0;

error:
	kut_mmc_uninit(__host, __mmc, __card);
	return -1;
}

