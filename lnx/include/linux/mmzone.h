#ifndef _MMZONE_H
#define _MMZONE_H

#include <linux/kut_mmzone.h>

/* Free memory management - zoned buddy allocator.  */
#define MAX_ORDER kut_max_order
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

#endif

