#ifndef _ASM_BUG_H_
#define _ASM_BUG_H_

#include <linux/kut_bug.h>

#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#define __BUG_ON(condition, is_bug) \
	do { \
		if (condition) { \
			printf("%s() in file %s, line %d: (%s)\n", \
				is_bug ? "BUG" : "WARNING", __FILE__, \
				__LINE__, #condition); \
			if (is_bug) { \
				printf("exiting...\n"); \
				if (kut_bug_on_do_exit_get()) \
					exit(1); \
				else \
					longjmp(kut_env, -1); \
			} \
		} \
	} while (0)

#define BUG() __BUG_ON(1, 1)
#define BUG_ON(condition) __BUG_ON(condition, 1)
#define VM_BUG_ON(condition) BUG_ON(condition)
#define WARN_ON(condition) __BUG_ON(condition, 0)
#define WARN_ON_ONCE(condition) ({ \
	WARN_ON(condition); \
	condition; \
})
#endif

