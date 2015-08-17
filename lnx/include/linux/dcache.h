#ifndef _DCACHE_H_
#define _DCACHE_H_

#include <linux/fs.h>

struct dentry {
	/* RCU lookup touched fields */
	struct dentry *d_parent;	/* parent directory */
	struct inode d_inode;		/* Where the name belongs to */
	unsigned char d_iname[DNAME_INLINE_LEN];	/* small names */
	struct list_head d_child;	/* child of parent list */
	union {
		unsigned int d_count;		/* open instances */
		struct list_head d_subdirs;	/* our children */
	} d_u;

	unsigned int d_depth;
	bool d_dir;
	const struct file_operations *d_fops;
};

#endif

