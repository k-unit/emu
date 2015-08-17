#ifndef _LINUX_UACCESS_H_
#define _LINUX_UACCESS_H_

#include <linux/kernel.h>

static inline long copy_to_user(void __user *to, const void *from, long n)
{
	memcpy(to, from, n);
	return n;
}

static inline long copy_from_user(void *to, const void __user *from, long n)
{
	memcpy(to, from, n);
	return n;
}

#endif

