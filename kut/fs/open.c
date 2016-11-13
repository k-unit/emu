#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/namei.h>

#include <linux/kut_namei.h>

typedef ssize_t (*io_func)(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos);

/*
 * kut_file_close() - close a file
 * @file:	the file to close
 */
void kut_file_close(struct file *file)
{
	struct dentry *dentry = file->f_dentry;

	fclose(file->f);
	free(file);

	dentry->d_u.d_count--;
}

/*
 * kut_file_open() - open a virtual file
 * @dentry:	a file dentry corresponding to the file
 * @flags:	access permission flags
 * @cred:	the security context of a task [ignored]
 *
 * Open virtual a file with the provided access permissions.
 * The underlying dentry must have been initialized as a file dentry.
 *
 * The file object created can be used in the same way Kernel procfs, sysfs
 * and debugfs files are used. Real files are only created to enable debugging
 * of the virtual file's directory layout in the runtime direcotry.
 *
 * Returns: the allocated file descriptor on success, NULL otherwise.
 */
struct file *kut_file_open(struct dentry *dentry, int flags,
	const struct cred *cred)
{
	struct path path;
	struct file *filp;

	if (kut_dentry_dir(dentry)) {
		WARN_ON(1);
		return NULL;
	}
	
	if (vfs_path_lookup(dentry, NULL, (char*)dentry->d_iname, 0, &path))
		return NULL;

	if (!(filp = malloc(sizeof(struct file))))
		return NULL;

	filp->f = fopen(path.p, "w+");
	if (!filp->f)
		goto error;

	filp->f_inode = &dentry->d_inode;
	filp->f_op = dentry->d_fops;
	filp->f_dentry = dentry;
	filp->f_dentry->d_u.d_count++;

	if (filp->f_op && filp->f_op->open &&
		filp->f_op->open(filp->f_inode, filp)) {
		fclose(filp->f);
		goto error;
	}

	return filp;

error:
	free(filp);
	return NULL;
}

static int file_io(struct file *filp, char *ubuf, size_t cnt, loff_t *ppos,
	bool is_read)
{
	io_func func;

	if (!filp->f_op)
		return -1;

	func = is_read ? filp->f_op->read : filp->f_op->write;
	return func(filp, ubuf, cnt, ppos);
}

/*
 * kut_file_read() - read from a file
 * @filp:	the file descriptor from which to read
 * @ubuf:	user buffer to read into
 * @cnt:	number of bytes to read
 * @ppos:	starting offset [ignored]
 *
 * Read cnt bytes into ubuf from a virtual file, filp.
 *
 * Returns: the return value of the filp->f_op->read callback.
 */
int kut_file_read(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos)
{
	return file_io(filp, ubuf, cnt, ppos, true);
}

/*
 * kut_file_write() - write from a file
 * @filp:	the file descriptor to which to write
 * @ubuf:	user buffer to write from
 * @cnt:	number of bytes to write
 * @ppos:	starting offset [ignored]
 *
 * write cnt bytes into ubuf from a virtual file, filp.
 *
 * Returns: the return value of the filp->f_op->write callback.
 */
int kut_file_write(struct file *filp, char __user *ubuf, size_t cnt,
	loff_t *ppos)
{
	return file_io(filp, ubuf, cnt, ppos, false);
}

