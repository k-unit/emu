#ifndef _MM_H_
#define _MM_H_

#include <linux/mm_types.h>

static inline void *page_address(const struct page *page)
{
	return page->virtual;
}

static inline void set_page_address(struct page *page, void *address)
{
	page->virtual = address;
}

#endif

