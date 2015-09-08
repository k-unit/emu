#ifndef _MM_H_
#define _MM_H_

#include <linux/mm_types.h>

#include <asm-generic/page.h>

#define offset_in_page(p)	((unsigned long)(p) & ~PAGE_MASK)

static inline void *page_address(const struct page *page)
{
	return page->virtual;
}

static inline void set_page_address(struct page *page, void *address)
{
	page->virtual = address;
}

#endif

