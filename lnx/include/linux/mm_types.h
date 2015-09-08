#ifndef _MM_TYPES_H_
#define _MM_TYPES_H_

#include <linux/types.h>

struct page {
	void *virtual;
	int order;
	gfp_t gfp_mask;
	union {
		unsigned long private;
	};
};

#endif

