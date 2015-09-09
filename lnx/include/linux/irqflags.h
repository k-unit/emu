#ifndef _IRQFLAGS_H_
#define _IRQFLAGS_H_

#define local_irq_save(flags) \
	do { \
		if (0) \
			flags = 0; \
	} while (0 && flags)

#define local_irq_restore(flags) \
	do { \
		if (0) \
			flags = 0; \
	} while (0 && flags)

#endif

