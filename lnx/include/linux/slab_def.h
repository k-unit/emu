#ifndef _SLAB_DEV_H_
#define _SLAB_DEV_H_

#include <stdlib.h>

#define kmalloc(size, gfp) malloc(gfp ? size : 0)
#define kfree(ptr) free(ptr)

#endif

