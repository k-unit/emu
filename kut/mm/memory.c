#include <linux/kut_mmzone.h>

int kut_max_order; /* max order of pages allocated must be less than this */

int kut_memory_init(enum kut_mem_pressure mp)
{
	switch (mp) {
	case KUT_MEM_SCARCE:
		kut_max_order = 2;
		break;
	case KUT_MEM_AVERAGE:
		kut_max_order = 6;
		break;
	case KUT_MEM_ABUNDANCE:
		kut_max_order = 11;
		break;
	default:
		return -1;
	}

	return 0;
}

