#ifndef _SLAB_H_
#define _SLAB_H_

#include <linux/kernel.h>
#include <linux/slab_def.h>
#include <stdlib.h>

#define kzalloc(size, gfp) calloc(1, gfp ? size : 0)
#define krealloc(p, new_size, flags) realloc(p, new_size)

#endif

