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

/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define min3(x, y, z) ({			\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	typeof(z) _min3 = (z);			\
	(void) (&_min1 == &_min2);		\
	(void) (&_min1 == &_min3);		\
	_min1 < _min2 ? (_min1 < _min3 ? _min1 : _min3) : \
		(_min2 < _min3 ? _min2 : _min3); })

#define max3(x, y, z) ({			\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	typeof(z) _max3 = (z);			\
	(void) (&_max1 == &_max2);		\
	(void) (&_max1 == &_max3);		\
	_max1 > _max2 ? (_max1 > _max3 ? _max1 : _max3) : \
		(_max2 > _max3 ? _max2 : _max3); })

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })

int __must_check kstrtoull(const char *s, unsigned int base,
	unsigned long long *res);
int __must_check kstrtoll(const char *s, unsigned int base, long long *res);
int __must_check kstrtoul(const char *s, unsigned int base, unsigned long *res);
int __must_check kstrtol(const char *s, unsigned int base, long *res);
int __must_check kstrtouint(const char *s, unsigned int base,
	unsigned int *res);
int __must_check kstrtoint(const char *s, unsigned int base, int *res);
int __must_check kstrtou16(const char *s, unsigned int base, u16 *res);
int __must_check kstrtos16(const char *s, unsigned int base, s16 *res);
int __must_check kstrtou8(const char *s, unsigned int base, u8 *res);
int __must_check kstrtos8(const char *s, unsigned int base, s8 *res);

int __must_check kstrtoull_from_user(const char __user *s, size_t count,
	unsigned int base, unsigned long long *res);
int __must_check kstrtoll_from_user(const char __user *s, size_t count,
	unsigned int base, long long *res);
int __must_check kstrtoul_from_user(const char __user *s, size_t count,
	unsigned int base, unsigned long *res);
int __must_check kstrtol_from_user(const char __user *s, size_t count,
	unsigned int base, long *res);
int __must_check kstrtouint_from_user(const char __user *s, size_t count,
	unsigned int base, unsigned int *res);
int __must_check kstrtoint_from_user(const char __user *s, size_t count,
	unsigned int base, int *res);
int __must_check kstrtou16_from_user(const char __user *s, size_t count,
	unsigned int base, u16 *res);
int __must_check kstrtos16_from_user(const char __user *s, size_t count,
	unsigned int base, s16 *res);
int __must_check kstrtou8_from_user(const char __user *s, size_t count,
	unsigned int base, u8 *res);
int __must_check kstrtos8_from_user(const char __user *s, size_t count,
	unsigned int base, s8 *res);

#endif

