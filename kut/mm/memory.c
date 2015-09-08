#include <linux/kut_mmzone.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm-generic/errno-base.h>

#define KUT_MAX_ORDER_SCARCE 2
#define KUT_MAX_ORDER_AVERAGE 6
#define KUT_MAX_ORDER_ABUNDANCE 11

struct kut_memory {
	struct page *page;
	void *virtual;
	struct list_head list;
} __kut_memory = {
	.page = NULL,
	.virtual = NULL,
	.list = LIST_HEAD_INIT(__kut_memory.list),
};

/* order of pages allocated must be < kut_max_order */
int kut_max_order = KUT_MAX_ORDER_AVERAGE;

static struct kut_memory *kut_mem_alloc(struct page *page, void *virtual)
{
	struct kut_memory *m;

	m = kzalloc(sizeof(struct kut_memory), GFP_KERNEL);
	if (!m)
		return NULL;

	m->page = page;
	m->virtual = virtual;
	INIT_LIST_HEAD(&m->list);

	return m;
}

static void kut_mem_free(struct kut_memory *m)
{
	kfree(m);
}

int kut_mem_register(struct page *page, void *virtual)
{
	struct kut_memory *m;

	BUG_ON(!page || !virtual);

	m = kut_mem_alloc(page, virtual);
	if (!m)
		return -ENOMEM;

	list_add(&m->list, &__kut_memory.list);
	return 0;
}

static struct kut_memory *__kut_mem_lookup(void *addr, bool by_page)
{
	struct kut_memory *m;

	list_for_each_entry(m, &__kut_memory.list, list) {
		int order = m->page->order;
		int allocation_size = 1 << order;

		if (by_page &&
			(m->page <= (struct page*)addr &&
			 (struct page*)addr < m->page + allocation_size)) {
				return m;
		}
		if (!by_page && ((char*)m->virtual <= (char*)addr &&
			(char*)addr <
			(char*)m->virtual + allocation_size*PAGE_SIZE)) {
				return m;
		}
	}

	return NULL;
}

static int __kut_mem_unregister(void *addr, bool by_page)
{
	struct kut_memory *m;

	m = __kut_mem_lookup(addr, by_page);
	if (!m)
		return -ENOENT;

	list_del(&m->list);
	kut_mem_free(m);
	return 0;
}

int kut_mem_unregister_by_page(struct page *page)
{
	return __kut_mem_unregister((void*)page, true);
}

int kut_mem_unregister_by_virtual(void *virtual)
{
	return __kut_mem_unregister(virtual, false);
}

void *kut_mem_lookup_virtual_by_page(struct page *page)
{
	struct kut_memory *m;
	int offset;

	m = __kut_mem_lookup(page, true);
	if (!m)
		return NULL;

	offset = page - m->page;
	return (char*)m->virtual + (offset * PAGE_SIZE);
}

struct page *kut_mem_lookup_page_by_virtual(void *virtual)
{
	struct kut_memory *m;
	int offset;

	m = __kut_mem_lookup(virtual, false);
	if (!m)
		return NULL;

	offset = ((unsigned long)virtual - (unsigned long)m->virtual) >>
		PAGE_SHIFT;
	return m->page + offset;
}

/*
 * kut_mem_pressure_set() - set memory constraints
 * @level:	the level of constraints to apply
 *
 * The level of memory constraint applied by this function governs the maximum
 * order possible for a successful page allocation by alloc_pages().
 * - KUT_MEM_SCARCE:          maximum order is 1
 * - KUT_MAX_ORDER_AVERAGE:   maximum order is 5  [default]
 * - KUT_MAX_ORDER_ABUNDANCE: maximum order is 10
 *
 * Returns: 0 upon setting a valid level, -EINVAL otherwise.
 */
int kut_mem_pressure_set(enum kut_mem_pressure level)
{
	switch (level) {
	case KUT_MEM_SCARCE:
		kut_max_order = KUT_MAX_ORDER_SCARCE;
		break;
	case KUT_MEM_AVERAGE:
		kut_max_order = KUT_MAX_ORDER_AVERAGE;
		break;
	case KUT_MEM_ABUNDANCE:
		kut_max_order = KUT_MAX_ORDER_ABUNDANCE;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * kut_mem_pressure_get() - get the current memory constraint
 *
 * Returns: the current value of kut_max_order - the maximum order possible for
 *          a successful page allocation by alloc_pages().
 */
int kut_mem_pressure_get(void)
{
	return kut_max_order;
}

