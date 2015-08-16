#ifndef _KERNEL_H_
#define _KERNEL_H_

/* kernel code emulation */

#include <linux/types.h>
#include <linux/page.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

/* generic stuff */
#define __user
#define EXPORT_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
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

/* kernel log */
#define pr_kernel(type, str, ...) printf("pr_" type ": " str, ##__VA_ARGS__)
#define pr_emerg(str, ...) pr_kernel("emerg", str, ##__VA_ARGS__)
#define pr_alert(str, ...) pr_kernel("alert", str, ##__VA_ARGS__)
#define pr_crit(str, ...) pr_kernel("crit", str, ##__VA_ARGS__)
#define pr_err(str, ...) pr_kernel("err", str, ##__VA_ARGS__)
#define	pr_warn(str, ...) pr_kernel("warn", str, ##__VA_ARGS__)
#define pr_notice(str, ...) pr_kernel("notice", str, ##__VA_ARGS__)
#define pr_info(str, ...) pr_kernel("info", str, ##__VA_ARGS__)

/* bug/warn on */
#define __BUG_ON(condition, is_bug) \
	do { \
		if (condition) { \
			printf("%s() in file %s, line %d: (%s)", \
				is_bug ? "BUG" : "WARNING", __FILE__, \
				__LINE__, #condition); \
			if (is_bug) { \
				printf(": exiting...\n"); \
				exit(1); \
			} else { \
				printf("\n"); \
			} \
		} \
	} while (0)

#define BUG_ON(condition) __BUG_ON(condition, 1)
#define WARN_ON(condition) __BUG_ON(condition, 0)

/* error numbers */
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */
#define ESUPER		50	/*ext4 superblock is damaged */

#endif

