#include <linux/kernel.h>
#include <linux/namei.h>
#include <linux/list.h>

struct dentry kern_root = {
	.d_iname = KSRC,
	.d_dir = true,

	.d_child = LIST_HEAD_INIT(kern_root.d_child),
	.d_u.d_subdirs = LIST_HEAD_INIT(kern_root.d_u.d_subdirs),
};

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

bool kut_dentry_dir(struct dentry *dentry)
{
	return dentry && dentry->d_dir;
}

bool kut_dentry_busy(struct dentry *dentry)
{
	return (kut_dentry_dir(dentry) &&
		!list_empty(&dentry->d_u.d_subdirs)) ||
		(!kut_dentry_dir(dentry) && dentry->d_u.d_count);
}

struct dentry *kut_dentry_create(const char *name, struct dentry *parent,
	umode_t mode, void *data, const struct file_operations *fops,
	bool is_dir)
{
	struct dentry *dentry;
	struct path path;
#ifdef CONFIG_FS
	int fd;
#endif

	if (!kut_dentry_dir(parent)) {
		WARN_ON(1);
		return NULL;
	}

	if (!(dentry = malloc(sizeof(struct dentry))))
		return NULL;

	dentry->d_parent = parent ? parent : &kern_root;
	snprintf((char*)dentry->d_iname, DNAME_INLINE_LEN, "%s", name);

	dentry->d_depth = dentry->d_parent->d_depth + 1;
	dentry->d_dir = is_dir;
	dentry->d_fops = fops;
	dentry->d_inode.i_private = data;

	INIT_LIST_HEAD(&dentry->d_child);

	if (vfs_path_lookup(dentry, NULL, (char*)dentry->d_iname, 0, &path))
		goto error;

	if (is_dir) {
		INIT_LIST_HEAD(&dentry->d_u.d_subdirs);

#ifdef CONFIG_FS
		fd = mkdir(path.p, S_IFDIR | mode);
		if (fd == -1)
			goto error;
#endif
	} else {
		dentry->d_u.d_count = 0;

#ifdef CONFIG_FS
		fd = open(path.p, O_CREAT | O_RDWR, mode);
		if (fd == -1)
			goto error;
#endif
	}

	list_add_tail(&dentry->d_child, &parent->d_u.d_subdirs);
	return dentry;

error:
	free(dentry);
	return NULL;
}

int kut_dentry_remove(struct dentry *dentry)
{
	struct path path;
#ifdef CONFIG_FS
	int ret;
#endif

	BUG_ON(dentry == &kern_root);

	if (kut_dentry_busy(dentry))
		return -1;

	if (vfs_path_lookup(dentry, NULL, (char*)dentry->d_iname, 0, &path))
		return -1;

#ifdef CONFIG_FS
	ret = kut_dentry_dir(dentry) ? rmdir(path.p) : unlink(path.p);
	if (ret)
		return -1;
#endif

	list_del(&dentry->d_child);
	free(dentry);
	return 0;
}

static void __dentry_remove_recursive(struct dentry *parent, bool *busy)
{
	struct dentry *child;

	/* try to remove the parent dentry */
	if (!kut_dentry_remove(parent))
		return;

	/* if a file is open - mark busy */
	if (!kut_dentry_dir(parent)) {
		*busy = true;
		return;
	}

	/* try to remove all sub dentries */
	list_for_each_entry(child, &parent->d_u.d_subdirs, d_child)
		__dentry_remove_recursive(child, busy);

	if (!*busy)
		kut_dentry_remove(parent);
}

int kut_dentry_remove_recursive(struct dentry *dentry)
{
	bool busy = false;

	__dentry_remove_recursive(dentry, &busy);
	return busy ? -1 : 0;
}

static int kut_dentry_io(struct dentry *dentry, char __user *ubuf, size_t cnt,
	bool is_read)
{
	struct file *filp;
	int ret;
	loff_t ppos;
	int (*rw_func)(struct file *filp, char __user *ubuf, size_t cnt,
		loff_t *ppos);

	if (!(filp = kut_file_open(dentry, 0, NULL)))
		return -1;

	rw_func = is_read ? kut_file_read : kut_file_write;
	ret = rw_func(filp, ubuf, cnt, &ppos);
	kut_file_close(filp);

	return ret;
}

int kut_dentry_read(struct dentry *dentry, char __user *ubuf, size_t cnt)
{
	return kut_dentry_io(dentry, ubuf, cnt, true);
}

int kut_dentry_write(struct dentry *dentry, char __user *ubuf, size_t cnt)
{
	return kut_dentry_io(dentry, ubuf, cnt, false);
}

