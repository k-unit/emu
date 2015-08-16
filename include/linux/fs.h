#ifndef _FS_H_
#define _FS_H_

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <dirent.h>

#define DNAME_INLINE_LEN 128U

struct dentry;
struct file;

struct vfsmount {
	char unused;
};
struct cred {
	char unused;
};

struct path {
	char p[PATH_MAX];
};

/*
 * Keep mostly read-only and often accessed (especially for
 * the RCU path lookup and 'stat' data) fields at the beginning
 * of the 'struct inode'
 */
struct inode {
	void *i_private; /* fs or device private pointer */
};

struct file_operations {
	int (*open)(struct inode *inode, struct file *filp);
	ssize_t (*read)(struct file *filp, char __user *ubuf,
		size_t cnt, loff_t *ppos);
	ssize_t (*write)(struct file *filp, char __user *ubuf,
		size_t cnt, loff_t *ppos);
};

struct file {
	FILE *f;
	struct inode *f_inode; /* cached value */
	const struct file_operations *f_op;
	void *private_data;

	struct dentry *f_dentry;
};

void kut_file_close(struct file *file);
struct file *kut_file_open(struct dentry *dentry, int flags,
	const struct cred *cred);
int kut_file_read(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos);
int kut_file_write(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos);
ssize_t simple_read_from_buffer(void __user *to, size_t count, loff_t *ppos,
	const void *from, size_t available);
ssize_t simple_write_to_buffer(void *to, size_t available, loff_t *ppos,
	const void __user *from, size_t count);

extern struct dentry kern_root;
#endif

