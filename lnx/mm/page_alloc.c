#include <linux/mm_types.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/mmzone.h>
#include <linux/export.h>

#include <asm-generic/page.h>

unsigned long nr_free_buffer_pages(void)
{
	return 100000;
}
EXPORT_SYMBOL_GPL(nr_free_buffer_pages);

void __free_pages(struct page *page, unsigned int order)
{
	if (!page)
		return;

	WARN_ON(!page->is_first);
	WARN_ON(page->order != order);

	kfree(page->virtual);
	kfree(page);
}
EXPORT_SYMBOL(__free_pages);

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
	}

	page->is_first = true;
	return page;
}
EXPORT_SYMBOL(alloc_pages);

