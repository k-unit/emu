#ifndef _KUT_FS_H_
#define _KUT_FS_H_

#define KUT_RDONLY 1<<0
#define KUT_WRONLY 1<<1
#define KUT_RDWR 1<<2

void kut_file_close(struct file *file);
struct file *kut_file_open(struct dentry *dentry, int flags,
	const struct cred *cred);
int kut_file_read(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos);
int kut_file_write(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos);

#endif

