#include <linux/types.h>

#include <setjmp.h>

jmp_buf kut_env;
static bool bug_on_do_exit;

/*
 * kunit's BUG_ON() has two modes of operation:
 * 1. system exit the program
 *    This should be used within testing code for verifying test valid
 *    configurations. If a test configuration is found to be invalid before
 *    running the test, kunit should exit and not proceed to execute the
 *    invalid test.
 * 2. use a setjmp/longjmp combination returning -1
 *    Unit tests can be written such that a BUG_ON() statement is expected to
 *    be reached in the tested code. In this case, the kunit BUG_ON() should
 *    perform a longjmp() and set setjmp()'s return value to -1 so the test
 *    can assert tested function failure.
 */

/*
 * kut_bug_on_do_exit_set() - set BUG_ON() behaviour
 * @enable:	enable/disable system exit of kunit upon BUG_ON().
 *
 * Set enable=true to force kunit to exit on BUG_ON() statements.
 * Set enable=false to have BUG_ON() set bug_on_asserted and return.
 */
void kut_bug_on_do_exit_set(bool enable)
{
	bug_on_do_exit = enable;
}

/*
 * kut_bug_on_do_exit_get() - get BUG_ON() behaviour
 *
 * Return true if BUG_ON() is to system exit on failure, false otherwise.
 */
bool kut_bug_on_do_exit_get(void)
{
	return bug_on_do_exit;
}

