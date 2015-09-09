#ifndef _HIGHMEM_H_
#define _HIGHMEM_H_

#include <linux/kernel.h>
#include <linux/mm_types.h>
#include <linux/mm.h>

static inline void *kmap(struct page *page)
{
	might_sleep();
	return page_address(page);
}

static inline void *kmap_atomic(struct page *page)
{
	return page_address(page);
}

static inline void kunmap(struct page *page)
{
	might_sleep();
}

static inline void kunmap_atomic(struct page *page)
{
}

#endif

