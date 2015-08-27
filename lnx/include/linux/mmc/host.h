#ifndef _HOST_H_
#define _HOST_H_

#include <linux/device.h>
#include <linux/mmc/core.h>

#define cls_dev_to_mmc_host(d) container_of(d, struct mmc_host, class_dev)
#define mmc_hostname(x) (dev_name(&(x)->class_dev))

struct mmc_card;

struct mmc_host {
	struct device *parent;
	struct device class_dev;
	int index;
	struct mmc_card *card;
	unsigned int claimed:1; /* host exclusively claimed */

	unsigned int max_seg_size; /* see blk_queue_max_segment_size */
	unsigned short max_segs; /* see blk_queue_max_segments */
	unsigned int max_req_size; /* maximum number of bytes in one req */
	unsigned int max_blk_count; /* maximum number of blocks in one req */

	void *private;
};

struct mmc_host *mmc_alloc_host(int extra, struct device *dev);
void mmc_free_host(struct mmc_host *host);

static inline void *mmc_priv(struct mmc_host *host)
{
	return (void*)host->private;
}
#endif

