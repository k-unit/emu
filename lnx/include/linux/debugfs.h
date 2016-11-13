#ifndef _DEBUGFS_H_
#define _DEBUGFS_H_

#include <linux/fs.h>
#include <linux/err.h>

#define VERIFY_READ 0
#define VERIFY_WRITE 1

struct dentry *debugfs_create_dir(const char *name, struct dentry *parent);
struct dentry *debugfs_create_file(const char *name, umode_t mode,
	struct dentry *parent, void *data, const struct file_operations *fops);
void debugfs_remove(struct dentry *dentry);
void debugfs_remove_recursive(struct dentry *dentry);

#endif

