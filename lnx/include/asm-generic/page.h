#ifndef _PAGE_H_
#define _PAGE_H_

#include <linux/gfp.h>

#include "../../../kut/mm/memory.h"

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE-1))

#define nth_page(page, n) ((page) + (n))

/* align addr on a size boundary - adjust address up/down if needed */
#define _ALIGN_UP(addr,size)	(((addr)+((size)-1))&(~((size)-1)))
#define _ALIGN_DOWN(addr,size)	((addr)&(~((size)-1)))

/* align addr on a size boundary - adjust address up if needed */
#define _ALIGN(addr,size)     _ALIGN_UP(addr,size)

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	_ALIGN(addr, PAGE_SIZE)

#define virt_to_page(addr) kut_mem_lookup_page_by_virtual(addr)
#define page_to_virt(addr) kut_mem_lookup_virtual_by_page(addr)

/**
 * get_order - Determine the allocation order of a memory size
 * @size: The size for which to get the order
 *
 * Determine the allocation order of a particular sized block of memory.  This
 * is on a logarithmic scale, where:
 *
 *	0 -> 2^0 * PAGE_SIZE and below
 *	1 -> 2^1 * PAGE_SIZE to 2^0 * PAGE_SIZE + 1
 *	2 -> 2^2 * PAGE_SIZE to 2^1 * PAGE_SIZE + 1
 *	3 -> 2^3 * PAGE_SIZE to 2^2 * PAGE_SIZE + 1
 *	4 -> 2^4 * PAGE_SIZE to 2^3 * PAGE_SIZE + 1
 *	...
 *
 * The order returned is used to find the smallest allocation granule required
 * to hold an object of the specified size.
 *
 * The result is undefined if the size is 0.
 *
 * This function may be used to initialise variables with compile time
 * evaluations of constants.
 */
static inline int get_order(int size)
{
	int cnt = 1, order = 0;

	if (!size)
		return 0;

	size--;
	size >>= PAGE_SHIFT;

	while (size) {
		if (size & 0x1)
			order = cnt;
		cnt++;
		size >>= 1;
	}

	return order;
}

#endif

