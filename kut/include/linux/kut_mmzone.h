#ifndef _KUT_MMZONE_H_
#define _KUT_MMZONE_H_

enum kut_mem_pressure {
	KUT_MEM_SCARCE,
	KUT_MEM_AVERAGE,
	KUT_MEM_ABUNDANCE,
};

extern int kut_max_order;

int kut_memory_init(enum kut_mem_pressure mp);

#endif

