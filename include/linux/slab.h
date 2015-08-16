#ifndef _SLAB_H_
#define _SLAB_H_

#include <linux/kernel.h>
#include <stdlib.h>

#define kmalloc(size, gfp) malloc(size)
#define kzalloc(size, gfp) calloc(1, size)
#define kfree(ptr) free(ptr)

#endif

