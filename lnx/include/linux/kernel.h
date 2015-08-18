#ifndef _KERNEL_H_
#define _KERNEL_H_

/* kernel code emulation */

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/export.h>
#include <linux/printk.h>
#include <linux/stddef.h>
#include <asm-generic/page.h>
#include <asm-generic/bug.h>
#include <asm-generic/errno-base.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#endif

