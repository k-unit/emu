#include <linux/kmemleak.h>
#include <linux/printk.h>
#include <linux/export.h>

/**
 * kmemleak_alloc - register a newly allocated object
 * @ptr:	pointer to beginning of the object
 * @size:	size of the object
 * @min_count:	minimum number of references to this object. If during memory
 *		scanning a number of references less than @min_count is found,
 *		the object is reported as a memory leak. If @min_count is 0,
 *		the object is never reported as a leak. If @min_count is -1,
 *		the object is ignored (not scanned and not reported as a leak)
 * @gfp:	kmalloc() flags used for kmemleak internal memory allocations
 *
 * This function is called from the kernel allocators when a new object
 * (memory block) is allocated (kmem_cache_alloc, kmalloc, vmalloc etc.).
 */
void __ref kmemleak_alloc(const void *ptr, size_t size, int min_count,
			  gfp_t gfp)
{
	pr_debug("%s(0x%p, %zu, %d)\n", __func__, ptr, size, min_count);
}
EXPORT_SYMBOL_GPL(kmemleak_alloc);

/**
 * kmemleak_free - unregister a previously registered object
 * @ptr:	pointer to beginning of the object
 *
 * This function is called from the kernel allocators when an object (memory
 * block) is freed (kmem_cache_free, kfree, vfree etc.).
 */
void __ref kmemleak_free(const void *ptr)
{
	pr_debug("%s(0x%p)\n", __func__, ptr);
}
EXPORT_SYMBOL_GPL(kmemleak_free);

