#ifndef _BUG_H_
#define _BUG_H_

#include <linux/kut_bug.h>

#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#define __BUG_ON(condition, is_bug) \
	do { \
		if (condition) { \
			printf("%s() in file %s, line %d: (%s)", \
				is_bug ? "BUG" : "WARNING", __FILE__, \
				__LINE__, #condition); \
			if (is_bug) { \
				printf(": exiting...\n"); \
				if (kut_bug_on_do_exit_get()) \
					exit(1); \
				else \
					longjmp(kut_env, -1); \
			} else { \
				printf("\n"); \
			} \
		} \
	} while (0)

#define BUG_ON(condition) __BUG_ON(condition, 1)
#define WARN_ON(condition) __BUG_ON(condition, 0)

#endif

