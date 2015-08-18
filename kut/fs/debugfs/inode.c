#include <linux/debugfs.h>
#include <linux/kut_namei.h>

enum error_string {
	ERR_CREAT_FILE,
	ERR_CREAT_DIR,
	ERR_NOMEM,
};

static struct dentry *debugfs_create_dentry(const char *name,
	struct dentry *parent, umode_t mode, void *data,
	const struct file_operations *fops, bool is_dir)
{
	struct dentry *dentry;
	enum error_string err;
	char *err_str;
	struct path path;

	if (!(dentry = kut_dentry_create(name, parent, mode, data, fops,
		is_dir))) {
		*path.p = 0;
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

	pr_err("%s: %s%s%s\n", err_str, path.p, *path.p ? "/" : "", name);
	return NULL;
}

struct dentry *debugfs_create_dir(const char *name, struct dentry *parent)
{
	return debugfs_create_dentry(name, parent, S_IFDIR | S_IRWXU | S_IRUSR |
		S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH, NULL, NULL,
		true);
}

struct dentry *debugfs_create_file(const char *name, umode_t mode,
	struct dentry *parent, void *data, const struct file_operations *fops)
{
	return debugfs_create_dentry(name, parent, mode, data, fops, false);
}

void debugfs_remove(struct dentry *dentry)
{
	kut_dentry_remove(dentry);
}

void debugfs_remove_recursive(struct dentry *dentry)
{
	kut_dentry_remove_recursive(dentry);
}

