#include <stdlib.h>

/*
 * kut_random_buf() - fill a buffer with random data
 * @buf:	the buffer to be filled
 * @length:	the buffer's length
 *
 * Fills a buffer with pseudo random data given by random(). The buffer may be
 * of any length.
 */
void kut_random_buf(char *buf, unsigned long length)
{
	int i, remainder, multiple, longp_cnt = length / sizeof(long);
	long last;

	/* fill the 1st sizeof(long)*(length/sizeof(long)) bytes */
	for (i = 0; i < longp_cnt; i += sizeof(long))
		((unsigned long*)buf)[i] = random();

	multiple = longp_cnt * sizeof(long);
	remainder = length - multiple;
	if (!remainder)
		return;

	/* fill remaining bytes, where 0 < bytes <= sizeof(long)-1 */
	last = random();
	for (i = 0; i < remainder; i++)
		buf[multiple *  + i] = ((char*)&last)[i];
}

