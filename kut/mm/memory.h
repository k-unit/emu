#ifndef _KUT_MM_MEMORY_H_
#define _KUT_MM_MEMORY_H_

#include <linux/mm_types.h>

int kut_mem_register(struct page *page, void *virtual);
int kut_mem_unregister_by_page(struct page *page);
int kut_mem_unregister_by_virtual(void *virtual);
void *kut_mem_lookup_virtual_by_page(struct page *page);
struct page *kut_mem_lookup_page_by_virtual(void *virtual);

#endif

