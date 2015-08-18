#include <linux/kernel.h>
#include <linux/namei.h>

/**
 * vfs_path_lookup - lookup a file path relative to a dentry-vfsmount pair
 * @dentry:  pointer to dentry of the base directory
 * @mnt: pointer to vfs mount of the base directory
 * @name: pointer to file name
 * @flags: lookup flags
 * @path: pointer to struct path to fill
 */
int vfs_path_lookup(struct dentry *dentry, struct vfsmount *mnt,
	const char *name, unsigned int flags, struct path *path)
{
	if (!dentry->d_depth) {
		sprintf(path->p, "%s", (char*)dentry->d_iname);
		return 0;
	}

	if (vfs_path_lookup(dentry->d_parent, mnt,
		(char*)dentry->d_parent->d_iname, flags, path)) {
		return -1;
	}

	strcat(path->p, "/");
	strcat(path->p, (char*)dentry->d_iname);
	return 0;
}

