#ifndef _NAMEI_H_
#define _NAMEI_H_ 

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/dcache.h>

int vfs_path_lookup(struct dentry *dentry, struct vfsmount *mnt,
	const char *name, unsigned int flags, struct path *path);

struct dentry *kut_dentry_create(const char *name, struct dentry *parent,
	umode_t mode, void *data, const struct file_operations *fops,
	bool is_dir);
int kut_dentry_remove(struct dentry *dentry);
int kut_dentry_remove_recursive(struct dentry *dentry);

int kut_dentry_read(struct dentry *dentry, char __user *ubuf, size_t cnt);
int kut_dentry_write(struct dentry *dentry, char __user *ubuf, size_t cnt);

bool kut_dentry_dir(struct dentry *dentry);
bool kut_dentry_busy(struct dentry *dentry);
#endif

