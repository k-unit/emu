#ifndef _CARD_H_
#define _CARD_H_

#include <linux/types.h>
#include <linux/device.h>
#include <linux/mmc/core.h>

struct mmc_host;

struct mmc_ext_csd {
	u8			rev;
	unsigned int		generic_cmd6_time;	
	bool			ffu_capable;
	bool 			ffu_mode_op;
	unsigned int            data_sector_size;       

	unsigned int            feature_support;
};

struct mmc_card {
	struct mmc_host *host;
	struct device dev; /* the device */
	struct mmc_ext_csd ext_csd;	
	bool md_main;

	struct list_head xfer;
};

static inline int mmc_card_mmc(struct mmc_card *card)
{
	return true;
}

#endif

