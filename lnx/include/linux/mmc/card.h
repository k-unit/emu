#ifndef _CARD_H_
#define _CARD_H_

#include <linux/types.h>
#include <linux/device.h>
#include <linux/mmc/core.h>

struct mmc_host;

struct mmc_ext_csd {
	u8			rev;
	u8			erase_group_def;
	u8			sec_feature_support;
	u8			rel_sectors;
	u8			rel_param;
	u8			part_config;
	u8			cache_ctrl;
	u8			rst_n_function;
	u8			max_packed_writes;
	u8			max_packed_reads;
	u8			packed_event_en;
	unsigned int		part_time;		
	unsigned int		sa_timeout;		
	unsigned int		generic_cmd6_time;	
	unsigned int            power_off_longtime;     
	u8			power_off_notification;	
	unsigned int		hs_max_dtr;
#define MMC_HIGH_26_MAX_DTR	26000000
#define MMC_HIGH_52_MAX_DTR	52000000
#define MMC_HIGH_DDR_MAX_DTR	52000000
#define MMC_HS200_MAX_DTR	200000000
#define MMC_HS400_MAX_DTR	200000000
	unsigned int		sectors;
	unsigned int		card_type;
	unsigned int		hc_erase_size;		
	unsigned int		hc_erase_timeout;	
	unsigned int		sec_trim_mult;	
	unsigned int		sec_erase_mult;	
	unsigned int		trim_timeout;		
	bool			enhanced_area_en;	
	unsigned long long	enhanced_area_offset;	
	unsigned int		enhanced_area_size;	
	unsigned int		cache_size;		
	bool			hpi_en;			
	bool			hpi;			
	unsigned int		hpi_cmd;		
	bool			ffu_capable;
	bool 			ffu_mode_op;
	bool			bkops;		
	bool			bkops_en;	
	unsigned int            data_sector_size;       
	unsigned int            data_tag_unit_size;     
	unsigned int		boot_ro_lock;		
	bool			boot_ro_lockable;
	u8			raw_exception_status;	
	u8			raw_partition_support;	
	u8			raw_rpmb_size_mult;	
	u8			raw_erased_mem_count;	
	u8			raw_ext_csd_structure;	
	u8			raw_card_type;		
	u8			raw_drive_strength;	
	u8			out_of_int_time;	
	u8			raw_s_a_timeout;		
	u8			raw_hc_erase_gap_size;	
	u8			raw_erase_timeout_mult;	
	u8			raw_hc_erase_grp_size;	
	u8			raw_sec_trim_mult;	
	u8			raw_sec_erase_mult;	
	u8			raw_sec_feature_support;
	u8			raw_trim_mult;		
	u8			raw_bkops_status;	
	u8			raw_sectors[4];		

	unsigned int            feature_support;
#define MMC_DISCARD_FEATURE	BIT(0)                  
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

