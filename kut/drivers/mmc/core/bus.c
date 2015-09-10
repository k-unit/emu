#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <linux/kut_device.h>
#include <linux/kut_random.h>
#include <linux/mmc/kut_bus.h>

static int __mmc_xfer_add(struct mmc_card *card, struct scatterlist *sg,
	unsigned nents, unsigned long addr, unsigned long length, bool write,
	void (*read_func)(char *buf, unsigned long length))
{
	struct kut_mmc_card_xfer *xfer;

	xfer = kzalloc(sizeof(struct kut_mmc_card_xfer), GFP_KERNEL);
	if (!xfer)
		return -ENOMEM;

	xfer->buf = kzalloc(length, GFP_KERNEL);
	if (!xfer->buf) {
		kfree(xfer);
		return -ENOMEM;
	}
	xfer->write = write;
	xfer->addr = addr;
	xfer->length = length;
	INIT_LIST_HEAD(&xfer->list);

	if (write) {
		sg_copy_to_buffer(sg, nents, xfer->buf, length);
	} else {
		/* initialize xfer->buf with content specified by read_func() */
		read_func(xfer->buf, length);
		sg_copy_from_buffer(sg, nents, xfer->buf, length);
	}

	list_add_tail(&xfer->list, &card->xfer);
	return 0;
}

/**
 *	kut_mmc_xfer_add - add a xfer chunk to the xfer list.
 *
 *	@card: card with which to issue xfer 
 *	@sg: sg list to xfer (for both read/write)
 *	@nents: number of sg list entries
 *	@addr: xfer address on card
 *	@lenth: xfer length in bytes 
 *	@write: read or write operation
 *
 *	The card maintains a xfer list in which entries are added upon each
 *	call to kut_mmc_xfer_add.
 *	Each entry in the list records what was transferred in its respective
 *	call to kut_mmc_xfer_add:
 *	- write:  read or write operation
 *	- addr:   destination address on card
 *	- length: transaction length in bytes
 *	- buf:    the transaction's resulting data
 *
 *	The correctness of data transfer can later be verified by calling
 *	kut_mmc_xfer_check.
 */
int kut_mmc_xfer_add(struct mmc_card *card, struct scatterlist *sg,
	unsigned nents, unsigned long addr, unsigned long length, bool write)
{
	return __mmc_xfer_add(card, sg, nents,  addr, length, write,
		kut_random_buf);
}

/**
 *	kut_mmc_xfer_reset - tear down a card's xfer list.
 *
 *	@card: card whos xfer list to tear down.
 */
void kut_mmc_xfer_reset(struct mmc_card *card)
{
	struct kut_mmc_card_xfer *xfer, *tmp;

	list_for_each_entry_safe(xfer, tmp, &card->xfer, list) {
		list_del(&xfer->list);
		kfree(xfer->buf);
		kfree(xfer);
	}
}

/**
 *	kut_mmc_xfer_subrange - assert if a xfer chunk is within a certain range.
 *
 *	@chunk: xfer data chunk in question.
 *	@range: range to be tested.
 */
static bool kut_mmc_xfer_subrange(struct kut_mmc_card_xfer *chunk,
	struct kut_mmc_card_xfer *range)
{
	return chunk->addr >= range->addr &&
		(chunk->addr + chunk->length) <= (range->addr + range->length);
}

/**
 *	kut_mmc_xfer_check - verify that a list of data has been correctly
 *	transferred for a list of segments
 *
 *	@card: card with which xfer was issued
 *	@expected: a list of data segments which have been transferred.
 *
 *	This function should be provided a list of data segments which are
 *	expected to have been written to or read from the card. These segments
 *	are refered to as the verification segments.
 *
 *	The function traverses all data chunks whos xfer has thus far been
 *	recorded (listed) by kut_mmc_xfer_add, and tries to match them with any
 *	of the verification data segments.
 *
 *	The function returns 0 if all verification segments have been fully
 *	matched. Otherwise, it returns -1.
 *
 *	Note: upon matching, the function resets the matched areas, hence the
 *	      verification segments are muted!
 */
int kut_mmc_xfer_check(struct mmc_card *card, struct list_head *expected)
{
	struct kut_mmc_card_xfer *xfer_chunk, *verify;

	/* try to match transferred chunks to verification segments */
	list_for_each_entry(xfer_chunk, &card->xfer, list) {
		list_for_each_entry(verify, expected, list) {
			int offset;

			if (xfer_chunk->write != verify->write)
				continue;
			if (!kut_mmc_xfer_subrange(xfer_chunk, verify))
				continue;

			offset = xfer_chunk->addr - verify->addr;
			if (memcmp(xfer_chunk->buf, verify->buf + offset,
				xfer_chunk->length)) {
				continue;
			}

			/* match found */
			memset(verify->buf + offset, 0, xfer_chunk->length);
			break;
		}
	}

	/* check that all verification segments were matched */
	list_for_each_entry(verify, expected, list) {
		int i;

		for (i = 0; i < verify->length; i++) {
			/* found a verification segment that was not matched */
			if (verify->buf[i])
				return -1;
		}
	}

	/* all verification buffers were matched */
	return 0;
}

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

	kut_mmc_xfer_reset(card);

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

	INIT_LIST_HEAD(&card->xfer);

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

