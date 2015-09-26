#ifndef _BITOPTS_H_
#define _BITOPTS_H_

#include <linux/kernel.h>

#ifndef __WORDSIZE
#define __WORDSIZE (sizeof(long) * 8)
#endif

#define BITS_PER_LONG		__WORDSIZE
#define BITS_PER_BYTE		8
#define BIT(nr)			(1UL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define BITS_TO_U64(nr)         DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(u64))
#define BITS_TO_U32(nr)         DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(u32))
#define BITS_TO_BYTES(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE)

#endif

