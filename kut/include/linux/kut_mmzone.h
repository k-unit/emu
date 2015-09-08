#ifndef _KUT_MMZONE_H_
#define _KUT_MMZONE_H_

#include <linux/mm_types.h>

enum kut_mem_pressure {
	KUT_MEM_SCARCE,
	KUT_MEM_AVERAGE,
	KUT_MEM_ABUNDANCE,
};

int kut_mem_pressure_set(enum kut_mem_pressure level);
int kut_mem_pressure_get(void);

#endif

