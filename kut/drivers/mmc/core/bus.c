#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <linux/kut_device.h>

/**
 *	kut_mmc_free_card - free card resources
 *
 *	@card: the card to free
 *
 *	This function should be called to free a struct mmc_card allocated by
 *	kut_mmc_alloc_card().
 *	The function frees any transfer lists, uninitialies the underlying
 *	device and frees the data structure.
 *
 *	Return 0 for successful freeing of resources, -1 otehrwise.
 */
int kut_mmc_free_card(struct mmc_card *card)
{
	if (!card)
		return 0;

	if (kut_dev_uninit(&card->dev))
		return -1;

	kfree(card);
	return 0;
}

/**
 *	mmc_card - allocate card resources
 *
 *	@host: the card's underlying host.
 *	@type: device type.
 *	@md_main: create an mmc data main partition.
 *
 *	This function allocates a struct mmc_card for kUnit tests.
 *	The card's underlying host must have been allocated by mmc_alloc_host()
 *	and cannot be NULL.
 *	The device type is ignored and remains unused by kUnit. It is included
 *	in the parameter list to maintain similarity with the kernel's
 *	mmc_alloc_card() function.
 *	If md_main is true, a child device will be created for the card's device
 *	standing for the card's mmc data main partition and all file system
 *	partition device will be its children. Otherwise, the file system
 *	partition devices will be direct children of the mmc_card's device.
 *	By default.
 *
 *	Return the allocated struct mmc_card on success, NULL otehrwise.
 */
struct mmc_card *kut_mmc_alloc_card(struct mmc_host *host,
	struct device_type *type, bool md_main)
{
	struct mmc_card *card;
	struct list_head *pos;
	int index = 1;

	BUG_ON(!host);

	card = kzalloc(sizeof(struct mmc_card), GFP_KERNEL);
	if (!card)
		return NULL;

	list_for_each(pos, &host->class_dev.p->klist_children)
		index++;
	if (!kut_dev_init(&card->dev, &host->class_dev, "%s:%.4d",
		mmc_hostname(host), index)) {
		kfree(card);
		return NULL;
	}

	if (md_main) {
		struct device *md_dev;

		md_dev = kut_dev_init(NULL, &card->dev, "mmcblk%d",
			host->index);
		if (!md_dev) {
			kut_mmc_free_card(card);
			return NULL;
		}

		card->md_main = true;
	}

	card->host = host;
	host->card = card;

	return card;
}

/**
 *	kut_mmc_card_md_main - get the card's md_main underlying device.
 *
 *	@card: the mmc card.
 *
 *	Return the card md_main partition's underlying device if such exists,
 *	       NULL otherwise.
 */
struct device *kut_mmc_card_md_main(struct mmc_card *card)
{
	struct device_private *md_dev_private;

	if (!card->md_main)
		return NULL;

	md_dev_private = list_entry(card->dev.p->klist_children.next,
		struct device_private, knode_parent);

	return md_dev_private->device;
}

/**
 *	kut_mmc_add_partition - add a partition to an mmc card
 *
 *	@card: the mmc card.
 *	@index: the index of the new partition.
 *
 *	Creates a new child device represending a file system partition for an
 *	mmc card. The new device will be created as a child device of the
 *	card's md_main partition if such exists, otherwise as a direct child
 *	of the card's underlying partition.
 *
 *	Return the device on success, NULL otherwise.
 */
struct device *kut_mmc_add_partition(struct mmc_card *card, int index)
{
	struct device *parent;

	parent = card->md_main ? kut_mmc_card_md_main(card) : &card->dev;
	return kut_dev_init(NULL, parent, "%sp%d", dev_name(parent), index);
}

