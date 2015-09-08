#ifndef _MMZONE_H
#define _MMZONE_H

#include <linux/kut_mmzone.h>

/* Free memory management - zoned buddy allocator.  */
#define MAX_ORDER kut_mem_pressure_get()
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

#define virt_addr_valid(kaddr) ((~(unsigned long)kaddr & 0x3) == 0x3)

#endif

