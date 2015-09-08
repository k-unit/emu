#include <linux/mm_types.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/mmzone.h>
#include <linux/export.h>

#include <asm-generic/page.h>

#include "../../kut/mm/memory.h"

unsigned long nr_free_buffer_pages(void)
{
	return 100000;
}
EXPORT_SYMBOL_GPL(nr_free_buffer_pages);

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
	struct page *page;

	VM_BUG_ON((gfp_mask & __GFP_HIGHMEM) != 0);

	page = alloc_pages(gfp_mask, order);
	if (!page)
		return 0;
	return (unsigned long)page_address(page);
}
EXPORT_SYMBOL(__get_free_pages);

void __free_pages(struct page *page, unsigned int order)
{
	if (!page)
		return;

	WARN_ON(page->private);
	WARN_ON(page->order != order);

	kut_mem_unregister_by_page(page);

	kfree(page->virtual);
	kfree(page);
}
EXPORT_SYMBOL(__free_pages);

void free_pages(unsigned long addr, unsigned int order)
{
	if (addr != 0) {
		VM_BUG_ON(!virt_addr_valid((void *)addr));
		__free_pages(virt_to_page((void *)addr), order);
	}
}

struct page *alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	void *virtual;
	struct page *page;
	int i;
	int allocation_size;
	
	if (order >= MAX_ORDER)
		return NULL;

	allocation_size = 1 << order;

	virtual = kmalloc(allocation_size * PAGE_SIZE, gfp_mask);
	if (!virtual)
		return NULL;

	page = kzalloc(allocation_size * sizeof(struct page), gfp_mask);
	if (!page) {
		kfree(virtual);
		return NULL;
	}

	for (i = 0; i < allocation_size; i++) {
		set_page_address(&page[i], virtual + i * PAGE_SIZE);
		page[i].order = order;
		page[i].gfp_mask = gfp_mask;
		page[i].private = (unsigned long)i;
	}

	kut_mem_register(page, virtual);
	return page;
}
EXPORT_SYMBOL(alloc_pages);

