#ifndef _NAMEI_H_
#define _NAMEI_H_ 

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/mount.h>
#include <linux/path.h>

int vfs_path_lookup(struct dentry *dentry, struct vfsmount *mnt,
	const char *name, unsigned int flags, struct path *path);

#endif

