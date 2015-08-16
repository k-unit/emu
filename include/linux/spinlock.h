#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <linux/kernel.h>

#define spin_lock_init(lock) \
	do { \
	} while (0)
#define spin_lock_irqsave(lock, flags) \
	do { \
		if (0) \
			flags = 0; \
	} while (0)
#define spin_unlock_irqrestore(lock, flags) \
	do { \
		if (0) \
			*lock = (spinlock_t)flags; \
	} while (0)

typedef unsigned long spinlock_t;

#endif

