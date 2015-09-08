#ifndef _KMEMLEAK_H_
#define _KMEMLEAK_H_

#include <linux/gfp.h>
#include <linux/compiler.h>

void __ref kmemleak_alloc(const void *ptr, size_t size, int min_count,
			  gfp_t gfp);
extern void kmemleak_free(const void *ptr) __ref;

#endif

