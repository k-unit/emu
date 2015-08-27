#include <linux/kernel.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <linux/kut_namei.h>
#include <linux/kut_fs.h>

#include <string.h>

struct dentry kern_root = {
	.d_iname = KSRC,
	.d_dir = true,

	.d_child = LIST_HEAD_INIT(kern_root.d_child),
	.d_u.d_subdirs = LIST_HEAD_INIT(kern_root.d_u.d_subdirs),
};

/*
 * kut_dentry_dir() - tests if a dentry represents a dentry
 * @dentry:	The dentry to test
 *
 * Returns: true if the dentry represents a vfsdirectory, false if it
 *          represents a vfs file.
 */
bool kut_dentry_dir(struct dentry *dentry)
{
	return dentry && dentry->d_dir;
}

/*
 * kut_dentry_busy() - tests if a dentry is busy
 * @dentry:	The dentry to test
 *
 * A dentry representing a vfs directory is busy if it has offsprints. A
 * dentry representing a vfs file is busy if it is open.
 *
 * Returns: true if the dentry is busy, false otherwise.
 */
bool kut_dentry_busy(struct dentry *dentry)
{
	return (kut_dentry_dir(dentry) &&
		!list_empty(&dentry->d_u.d_subdirs)) ||
		(!kut_dentry_dir(dentry) && dentry->d_u.d_count);
}

/*
 * kut_dentry_create() - create a new dentry
 * @name:	A null terminated string - the dentry name
 * @parent:	parent, NULL for kern_root (runtime root directory).
 * @mode:	access mode
 * @data:	context to hang on the corresponding inode
 * @fops:	file operation callbacks for the dentry
 * @is_dir:	true for creating a direcotry, false for creating a file
 *
 * A dentry (directory entry) represents either a directory in the vfs or a file
 * in the vfs.
 * The dentry hierarchy is a tree rooted at kern_root (kunit runtime). Any
 * dentry represending a directory can maintain a list of children dentries.
 *
 * Returns: the dentry if successfuly created, NULL otherwise.
 */
struct dentry *kut_dentry_create(const char *name, struct dentry *parent,
	umode_t mode, void *data, const struct file_operations *fops,
	bool is_dir)
{
	struct dentry *dentry;
	struct path path;
#ifdef CONFIG_KUT_FS
	int fd;
#endif

	if (!parent)
		parent = &kern_root;

	if (!kut_dentry_dir(parent)) {
		WARN_ON(1);
		return NULL;
	}

	if (!(dentry = kzalloc(sizeof(struct dentry), GFP_KERNEL)))
		return NULL;

	dentry->d_parent = parent;
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

#ifdef CONFIG_KUT_FS
		fd = mkdir(path.p, S_IFDIR | mode);
		if (fd == -1)
			goto error;
		close(fd);
#endif
	} else {
		dentry->d_u.d_count = 0;

#ifdef CONFIG_KUT_FS
		fd = open(path.p, O_CREAT | O_RDWR, mode);
		if (fd == -1)
			goto error;
		close(fd);
#endif
	}

	list_add_tail(&dentry->d_child, &parent->d_u.d_subdirs);
	return dentry;

error:
	free(dentry);
	return NULL;
}

/*
 * kut_dentry_remove() - remove a dentry
 * @dentry:	The dentry to remove
 *
 * Removes a single dentry created by kut_dentry_create().
 * If the dentry is busy, removal will fail. A dentry is busy if it represents
 * a directory in vfs which has existing children, or a file in vfs which is
 * open.
 *
 * Returns: 0 if successful, error otherwise.
 */
int kut_dentry_remove(struct dentry *dentry)
{
	struct path path;
#ifdef CONFIG_KUT_FS
	int ret;
#endif

	BUG_ON(dentry == &kern_root);

	if (kut_dentry_busy(dentry))
		return -EBUSY;

	if (vfs_path_lookup(dentry, NULL, (char*)dentry->d_iname, 0, &path))
		return -ENOENT;

#ifdef CONFIG_KUT_FS
	ret = kut_dentry_dir(dentry) ? rmdir(path.p) : unlink(path.p);
	if (ret)
		return -EBUSY;
#endif

	list_del(&dentry->d_child);
	free(dentry);
	return 0;
}

static void __dentry_remove_recursive(struct dentry *parent, bool *busy)
{
	struct dentry *child, *tmp;

	/* try to remove the parent dentry */
	if (parent != &kern_root && !kut_dentry_remove(parent))
		return;

	/* if a file is open - mark busy */
	if (!kut_dentry_dir(parent)) {
		*busy = true;
		return;
	}

	/* try to remove all sub dentries */
	list_for_each_entry_safe(child, tmp, &parent->d_u.d_subdirs, d_child)
		__dentry_remove_recursive(child, busy);

	if (parent != &kern_root && !*busy)
		kut_dentry_remove(parent);
}

/*
 * kut_dentry_remove_recursive() - remove a dentry and all its children
 * @dentry:	The dentry tree root
 *
 * Removes a dentry tree rooted at dentry.
 * If any of the dentry is busy, niether it nor any of its ansesters gets
 * removed.
 * If dentry represents a file, then the call is equivilent to
 * kut_dentry_remove().
 *
 * Returns: 0 if successful, error otherwise.
 */
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
	loff_t ppos = 0;
	int (*rw_func)(struct file *filp, char __user *ubuf, size_t cnt,
		loff_t *ppos);

	if (!(filp = kut_file_open(dentry, is_read ? KUT_RDONLY : KUT_WRONLY,
		NULL))) {
		return -1;
	}

	rw_func = is_read ? kut_file_read : kut_file_write;
	ret = rw_func(filp, ubuf, cnt, &ppos);
	kut_file_close(filp);

	return ret;
}

/*
 * kut_dentry_read() - read from a file dentry 
 * @dentry:	The dentry to read from
 * @ubuf:	Wehre to copy to
 * @cnt:	The number of bytes to copy
 *
 * Reads from a file dentry.
 *
 * Returns: the number of copied bytes, or -1 on error.
 */
int kut_dentry_read(struct dentry *dentry, char __user *ubuf, size_t cnt)
{
	return kut_dentry_io(dentry, ubuf, cnt, true);
}

/*
 * kut_dentry_write() - write to a file dentry 
 * @dentry:	The dentry to read from
 * @ubuf:	Wehre to copy from
 * @cnt:	The number of bytes to copy
 *
 * Writes to a file dentry.
 *
 * Returns: the number of copied bytes, or -1 on error.
 */
int kut_dentry_write(struct dentry *dentry, char __user *ubuf, size_t cnt)
{
	return kut_dentry_io(dentry, ubuf, cnt, false);
}

static struct dentry *__dentry_lookup(struct dentry *base, char *name)
{
	struct dentry *pos;
	
	list_for_each_entry(pos, &base->d_u.d_subdirs, d_child) {
		if (strcmp(name, (char*)pos->d_iname))
			continue;

		name = strtok(NULL, "/");
		if (!name)
			return pos;
		if (!kut_dentry_dir(pos))
			return NULL;
		return __dentry_lookup(pos, name);
	}

	return NULL;
}

struct dentry *kut_dentry_lookup(struct dentry *base, char *path)
{
	char p[PATH_MAX];
	char *name;

	if (!base)
		base = &kern_root;
	if (!kut_dentry_dir(base))
		return NULL;

	if (strlen(path) >= PATH_MAX)
		return NULL;

	strncpy(p, path, PATH_MAX);
	name = strtok(p, "/");
	if (!name)
		return NULL;

	return __dentry_lookup(base, name);
}

