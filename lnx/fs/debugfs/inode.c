#include <linux/debugfs.h>
#include <linux/kut_namei.h>

static struct dentry *debugfs_create_dentry(const char *name,
	struct dentry *parent, umode_t mode, void *data,
	const struct file_operations *fops, bool is_dir)
{
	struct dentry *dentry;
	char *err_str;
	enum {
		ERR_CREAT_FILE,
		ERR_CREAT_DIR,
		ERR_NOMEM,
	} err;

	if (!(dentry = kut_dentry_create(name, parent, mode, data, fops,
		is_dir))) {
		err = ERR_NOMEM;
		goto error;
	}

	return dentry;

error:
	free(dentry);

	switch (err) {
	case ERR_CREAT_FILE:
		err_str = "could not create file";
		break;
	case ERR_CREAT_DIR:
		err_str = "could not create directory";
		break;
	case ERR_NOMEM:
		err_str = "could not allocate a new dentry";
		break;
	default:
		err_str = "unknown";
		break;
	}

	pr_err("%s: %s\n", err_str, name);
	return NULL;
}

/**
 * debugfs_create_dir - create a directory in the debugfs filesystem
 * @name: a pointer to a string containing the name of the directory to
 *        create.
 * @parent: a pointer to the parent dentry for this file.  This should be a
 *          directory dentry if set.  If this paramater is NULL, then the
 *          directory will be created in the root of the debugfs filesystem.
 *
 * This function creates a directory in debugfs with the given name.
 *
 * This function will return a pointer to a dentry if it succeeds.  This
 * pointer must be passed to the debugfs_remove() function when the file is
 * to be removed (no automatic cleanup happens if your module is unloaded,
 * you are responsible here.)  If an error occurs, %NULL will be returned.
 *
 * If debugfs is not enabled in the kernel, the value -%ENODEV will be
 * returned.
 */
struct dentry *debugfs_create_dir(const char *name, struct dentry *parent)
{
	return debugfs_create_dentry(name, parent, S_IFDIR | S_IRWXU | S_IRUSR |
		S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH, NULL, NULL,
		true);
}
EXPORT_SYMBOL_GPL(debugfs_create_dir);

/**
 * debugfs_create_file - create a file in the debugfs filesystem
 * @name: a pointer to a string containing the name of the file to create.
 * @mode: the permission that the file should have.
 * @parent: a pointer to the parent dentry for this file.  This should be a
 *          directory dentry if set.  If this paramater is NULL, then the
 *          file will be created in the root of the debugfs filesystem.
 * @data: a pointer to something that the caller will want to get to later
 *        on.  The inode.i_private pointer will point to this value on
 *        the open() call.
 * @fops: a pointer to a struct file_operations that should be used for
 *        this file.
 *
 * This is the basic "create a file" function for debugfs.  It allows for a
 * wide range of flexibility in creating a file, or a directory (if you want
 * to create a directory, the debugfs_create_dir() function is
 * recommended to be used instead.)
 *
 * This function will return a pointer to a dentry if it succeeds.  This
 * pointer must be passed to the debugfs_remove() function when the file is
 * to be removed (no automatic cleanup happens if your module is unloaded,
 * you are responsible here.)  If an error occurs, %NULL will be returned.
 *
 * If debugfs is not enabled in the kernel, the value -%ENODEV will be
 * returned.
 */
struct dentry *debugfs_create_file(const char *name, umode_t mode,
	struct dentry *parent, void *data, const struct file_operations *fops)
{
	return debugfs_create_dentry(name, parent, mode, data, fops, false);
}
EXPORT_SYMBOL_GPL(debugfs_create_file);

/**
 * debugfs_remove - removes a file or directory from the debugfs filesystem
 * @dentry: a pointer to a the dentry of the file or directory to be
 *          removed.
 *
 * This function removes a file or directory in debugfs that was previously
 * created with a call to another debugfs function (like
 * debugfs_create_file() or variants thereof.)
 *
 * This function is required to be called in order for the file to be
 * removed, no automatic cleanup of files will happen when a module is
 * removed, you are responsible here.
 */
void debugfs_remove(struct dentry *dentry)
{
	kut_dentry_remove(dentry);
}
EXPORT_SYMBOL_GPL(debugfs_remove);

/**
 * debugfs_remove_recursive - recursively removes a directory
 * @dentry: a pointer to a the dentry of the directory to be removed.
 *
 * This function recursively removes a directory tree in debugfs that
 * was previously created with a call to another debugfs function
 * (like debugfs_create_file() or variants thereof.)
 *
 * This function is required to be called in order for the file to be
 * removed, no automatic cleanup of files will happen when a module is
 * removed, you are responsible here.
 */
void debugfs_remove_recursive(struct dentry *dentry)
{
	kut_dentry_remove_recursive(dentry);
}
EXPORT_SYMBOL_GPL(debugfs_remove_recursive);

