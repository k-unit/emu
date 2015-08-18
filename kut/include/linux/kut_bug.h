#ifndef _KUT_BUG_H_
#define _KUT_BUG_H_

#include <linux/types.h>

#include <setjmp.h>

extern jmp_buf kut_env;

#define KUT_CAN_BUG_ON(val, expr) do { \
	val = setjmp(kut_env); \
	if (!val) \
		expr; \
} while (0)

void kut_bug_on_do_exit_set(bool enable);
bool kut_bug_on_do_exit_get(void);

#endif

