#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/list.h>

#include <linux/kut_types.h>
#include <linux/kut_namei.h>

#include <asm-generic/bug.h>

#include <unit_test.h>
#ifdef CONFIG_KUT_FS
#include <sys/types.h>
#include <dirent.h>
#endif

#define pr_kut(fmt, ...) pr_err("%s(): " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define list_verify_siblings(entry, siblings, sib_list, parent, chld_list) ({ \
	bool ret; \
	do { \
		if (!siblings && !parent) { \
			ret = list_empty(&entry->sib_list); \
			if (!ret) { \
				pr_kut("no siblings and no parent but " \
					#entry "->" #sib_list " is not empty");\
			} \
		} else { \
			if (!siblings) { \
				ret = \
					(((entry->sib_list.prev == \
					 entry->sib_list.next)) && \
					((entry->sib_list.next == \
					 &parent->chld_list))) \
					&& \
					(((parent->chld_list.prev == \
					 parent->chld_list.next)) && \
					((parent->chld_list.next == \
					 &entry->sib_list))); \
				if (!ret) { \
					pr_kut(#parent " and no " #siblings \
						" but " #entry "->" #sib_list \
						" does not consist of exactly "\
						#parent " and " #entry);\
				} \
			} else { \
				ret = ((entry->sib_list.prev != \
					 entry->sib_list.next)) \
					&& \
					((parent->chld_list.prev != \
					 parent->chld_list.next)); \
				if (!ret) { \
					pr_kut(#parent " and " #siblings \
						" exist but " #entry "->" \
						#sib_list " does not consist " \
						"of " #parent "->" #chld_list \
						" and at least one more " \
						"entry");\
				} \
			} \
		} \
	} while (0); \
	ret; \
})

#ifdef CONFIG_KUT_FS
static int verify_fs(struct path *path, umode_t mode, bool is_dir,
	bool is_create)
{
	struct stat st;
	int ret;

	/* stat the directory/file */
	ret = stat(path->p, &st);
	if (is_create) {
		if (ret)
			return -1;
	} else {
		/* the directory/file should have been removed so the
		 * return value is opposite to stat's success result */
		return ret ? 0 : -1;
	}

	/* verify directory/file were created as directory/file */
	if (is_dir) {
		if (!S_ISDIR(st.st_mode))
			return -1;
	} else {
		if (!S_ISREG(st.st_mode))
			return -1;
	}

	/* verify user permissions */
	if ((st.st_mode & S_IRWXU) != (mode & S_IRWXU))
		return -1;
	/* verify group permissions */
	if ((st.st_mode & S_IRWXG) != (mode & S_IRWXG))
		return -1;
	/* verify other permissions */
	if ((st.st_mode & S_IRWXO) != (mode & S_IRWXO))
		return -1;

	/* directory/file created as expected */
	return 0;
}

static int reset_dir(char *path)
{
	DIR *d;
	struct dirent *ent;
	int ret = -1;

	if (!(d = opendir(path)))
		return -1;

	while ((ent = readdir(d))) {
		char ent_path[DNAME_INLINE_LEN + 2 + sizeof(ent->d_name) + 1];
		struct stat sbuf;

		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		snprintf(ent_path, sizeof(ent_path), "%s/%s", path,
			ent->d_name);
		if (stat(ent_path, &sbuf))
			goto exit;

		if (S_ISDIR(sbuf.st_mode)) {
			if (reset_dir(ent_path))
				goto exit;
			if (rmdir(ent_path))
				goto exit;
		} else {
			if (unlink(ent_path))
				goto exit;
		}
	}

	ret = 0;
exit:
	closedir(d);
	return ret;
}
#else
static int verify_fs(struct path *path, umode_t mode, bool is_dir,
	bool is_create)
{
	return 0;
}

static int reset_dir(char *path)
{
	return 0;
}
#endif

/**
 * verify_dentry - test all the fields of a dentry and verify they're as
 * expected
 * @dentry: the dentry to test
 * @parent: the expected parent
 * @data: the expected data
 * @name: the expected name
 * @depth: the expected depth
 * @siblings: are any siblings expected
 * @dir: does this dentry represent a directory rather than a file 
 * @count: (if file) expected number of file openings for this dentry
 * @children: (if direcotry) are any children expected
 * @d_fops: expected file operations
 */
static int verify_dentry(struct dentry *dentry, struct dentry *parent,
	void *data, const char *name, unsigned int depth, bool siblings,
	bool dir, unsigned int count, bool children,
	struct file_operations *d_fops)
{
	if (!dentry) {
		pr_kut("dentry not created");
		return -1;
	}
	if (dentry->d_parent != parent) {
		pr_kut("wrong parent");
		return -2;
	}
	if (dentry->d_inode.i_private != data) {
		pr_kut("wrong data");
		return -3;
	}
	if (dentry->d_depth != depth) {
		pr_kut("wrong depth");
		return -4;
	}
	if (!list_verify_siblings(dentry, siblings, d_child, parent,
		d_u.d_subdirs)) {
		return -5;
	}
	if (dentry->d_dir != dir) {
		pr_kut("dentry->d_dir set to %s instead of %s",
			dentry->d_dir ? "direcotry" : "file",
			dentry->d_dir ? "file" : "direcotry");
		return -6;
	}
	if (dentry->d_fops != d_fops) {
		pr_kut("wrong file operations");
		return -7;
	}
	if (dentry->d_dir) {
		if (!(children ^ list_empty(&dentry->d_u.d_subdirs))) {
			pr_kut("%schildren exist but " \
				"dentry->d_u.d_subdirs is %sempth",
				children ? "" : "no ",
				children ? "" : "not ");
			return -8;
		}
	} else {
		if (count != dentry->d_u.d_count) {
			pr_kut("wrong number of file openings");
			return -9;
		}
	}
	if (strcmp(name, (char*)dentry->d_iname)) {
		pr_kut("wrong dentry name");
		return -10;
	}

	return 0;
}

static int bug_on_test(void)
{
	int ret;

	kut_bug_on_do_exit_set(false);

	pr_kut("calling BUG_ON(1)...");
	KUT_CAN_BUG_ON(ret, BUG_ON(1));

	return !ret;
}

static int dentry_create_test(char *name, umode_t mode, bool is_dir)
{
	struct dentry *dentry;
	struct path path;
	int ret = 0;
	
	dentry = kut_dentry_create(name, &kern_root, mode, NULL,
		NULL, is_dir);
	if (verify_dentry(dentry, &kern_root, NULL, name, 1, false, is_dir, 0,
		false, NULL)) {
		return -1;
	}

	if (vfs_path_lookup(dentry, NULL, (char*)dentry->d_iname, 0, &path))
		return -1;

	if (verify_fs(&path, mode, is_dir, true))
		ret = -1;

	if (kut_dentry_remove(dentry))
		return -1;

	if (verify_fs(&path, mode, is_dir, false))
		ret = -1;

	return ret;
}

static int dentry_kernel_root(void)
{
	return verify_dentry(&kern_root, NULL, NULL, KSRC, 0, false, true,
		0, false, NULL);
}

static int dentry_create_dir(void)
{
	return dentry_create_test("test_dir", KUT_MODE_DEFAULT_DIR, true);
}

static int dentry_create_file(void)
{
	return dentry_create_test("test_file", KUT_MODE_DEFAULT_FILE, false);
}

static int dentry_create_file_in_dir(void)
{
	struct dentry *d_dir = NULL, *d_file = NULL;
	int ret = -1;

	/* create a directory */
	d_dir = kut_dentry_create("test_dir", &kern_root, KUT_MODE_DEFAULT_DIR,
		NULL, NULL, true);
	if (verify_dentry(d_dir, &kern_root, NULL, "test_dir", 1, false, true,
		0, false, NULL)) {
		goto exit;
	}

	/* create a file inside the direcotry */
	d_file = kut_dentry_create("test_file", d_dir, KUT_MODE_DEFAULT_FILE,
		NULL, NULL, false);
	if (verify_dentry(d_file, d_dir, NULL, "test_file", 2, false, false,
		0, false, NULL)) {
		goto exit;
	}

	/* verify d_dir now has children dentries */
	if (verify_dentry(d_dir, &kern_root, NULL, "test_dir", 1, false, true,
		0, true, NULL)) {
		goto exit;
	}

	/* try to remove the non empty direcotry */
	if (!kut_dentry_remove(d_dir))
		goto exit;

	ret = 0;

exit:
	if (d_file && kut_dentry_remove(d_file))
		ret = -1;
	if (d_dir && kut_dentry_remove(d_dir))
		ret = -1;

	return ret;
}

/*
 * Lookup f331's *dentry in the following directory tree starging at KSRC
 * .
 * ├── d1
 * ├── f2
 * ├── d3
 * │   ├── d31
 * │   ├── f32
 * │   └── d33
 * │       └── f331
 * └── d4
 */
static int dentry_lookup_test(void)
{
	struct dentry *d1 = NULL, *f2 = NULL, *d3 = NULL, *d4 = NULL;
	struct dentry *d31 = NULL, *f32 = NULL, *d33 = NULL;
	struct dentry *f331 = NULL;
	struct dentry *dentry;
	int ret = -1;

	/* create d1 subtree */
	d1 = kut_dentry_create("d1", &kern_root, KUT_MODE_DEFAULT_DIR, NULL,
		NULL, true);
	if (!d1)
		goto exit;

	/* create f2 file */
	f2 = kut_dentry_create("f2", &kern_root, KUT_MODE_DEFAULT_FILE, NULL,
		NULL, false);
	if (!f2)
		goto exit;

	/* create d3 subtree */
	d3 = kut_dentry_create("d3", &kern_root, KUT_MODE_DEFAULT_DIR, NULL,
		NULL, true);
	if (!d3)
		goto exit;

	d31 = kut_dentry_create("d31", d3, KUT_MODE_DEFAULT_DIR, NULL, NULL,
		true);
	if (!d31)
		goto exit;

	f32 = kut_dentry_create("f32", d3, KUT_MODE_DEFAULT_FILE, NULL, NULL,
		false);
	if (!f32)
		goto exit;

	d33 = kut_dentry_create("d33", d3, KUT_MODE_DEFAULT_DIR, NULL, NULL,
		true);
	if (!d33)
		goto exit;

	f331 = kut_dentry_create("f331", d33, KUT_MODE_DEFAULT_FILE, NULL, NULL,
		false);
	if (!f331)
		goto exit;

	/* create d4 subtree */
	d4 = kut_dentry_create("d4", &kern_root, KUT_MODE_DEFAULT_DIR, NULL,
		NULL, true);
	if (!d4)
		goto exit;

	/* find f331's *dentry */
	dentry = kut_dentry_lookup(NULL, "d3/d33/f331");
	if (dentry != f331)
		goto exit;

	/* return NULL on a non existing path */
	dentry = kut_dentry_lookup(NULL, "d3/d34/f345");
	if (dentry)
		goto exit;

	ret = 0;
exit:
	kut_dentry_remove_recursive(&kern_root);
	return ret;
}

static int pre_post_test(void)
{
	return reset_dir(KSRC);
}

struct single_test kernel_tests[] = {
	{
		.description = "lib: BUG_ON with no exit",
		.func = bug_on_test,
	},
	{
		.description = "dentry: test kernel root directory",
		.func = dentry_kernel_root,
	},
	{
		.description = "dentry: create a simple directory",
		.func = dentry_create_dir,
	},
	{
		.description = "dentry: create a simple file",
		.func = dentry_create_file,
	},
	{
		.description = "dentry: create a file inside a direcotry",
		.func = dentry_create_file_in_dir,
	},
	{
		.description = "dentry: lookup a dentry by path",
		.func = dentry_lookup_test,
	},
};

struct unit_test ut_kernel = {
	.module = "kernel",
	.description = "Kernel Emulation",
	.pre_single_test = pre_post_test,
	.post_single_test = pre_post_test,
	.tests = kernel_tests,
	.count = ARRAY_SZ(kernel_tests),
};

