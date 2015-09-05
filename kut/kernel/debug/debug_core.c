#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/list.h>
#include <linux/mmzone.h>

#include <asm-generic/page.h>

#include <linux/kut_types.h>
#include <linux/kut_device.h>
#include <linux/kut_namei.h>

#include <linux/kut_fs.h>
#include <linux/kut_mmzone.h>
#include <linux/kut_bug.h>

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

/**
 * verify_file - test all the fields of a file and verify they're as expected
 * @filp: the file to test
 * @i_private_data: expected inode underlying private data
 * @f_private_data: expected file private data
 * @f_dentry: expected corresponding dentry
 */
static int verify_file(struct file *filp, struct file_operations *f_op,
	void *i_private_data, void *f_private_data, struct dentry *f_dentry)
{
	if (!filp) {
		pr_kut("no filp");
		return -1;
	}

	if (filp->f_op != f_op) {
		pr_kut("wrong file operations");
		return -2;
	}

	if (filp->f_inode->i_private != i_private_data) {
		pr_kut("f_inode->i_private contains wrong data");
		return -3;
	}

	if (filp->private_data != f_private_data) {
		pr_kut("filp contains wrong data");
		return -4;
	}

	if (filp->f_dentry != f_dentry) {
		pr_kut("wrong dentry");
		return -5;
	}

#ifdef CONFIG_KUT_FS
	if (!filp->f) {
		pr_kut("no file created");
		return -6;
	}
#endif

	return 0;
}

/**
 * verify_device - test all the fields of a device and verify they're as
 * expected
 * @dev: the device to test
 * @expected_addr: if a static device - its address, NULL otherwise
 * @parent: the expected parent
 * @name: the expected name
 * @siblings: are any siblings expected
 * @children: are any children expected
 */
static int verify_device(struct device *dev, struct device *expected_addr,
	struct device *parent, const char *name, bool siblings, bool children)
{
	if (!dev) {
		pr_kut("device not created");
		return -1;
	}
	if (expected_addr) {
		if ((dev != expected_addr) || dev->dynamic) {
			pr_kut("static device marked as dynamic");
			return -2;
		}
	} else {
		if (!dev->dynamic) {
			pr_kut("dynamic device marked as static");
			return -3;
		}
	}
	if (dev->parent != parent) {
		pr_kut("wrong parent");
		return -4;
	}
	if (strcmp(dev_name(dev), name)) {
		pr_kut("wrong device name");
		return -5;
	}
	if (!dev->p) { /* XXX needs to follow kernel's semantics! */
		pr_kut("device has no private data");
		return -6;
	}
	if (!list_verify_siblings(dev, siblings, p->knode_parent, parent,
		p->klist_children)) {
		return -7;
	}
	if (!(children ^ list_empty(&dev->p->klist_children))) {
		pr_kut("expected %schilren but dev->p->klist_children list is "\
			"%sempty", children ? "" : "no ",
			children ? "" : "not ");
		return -8;
	}
	if (dev->p->driver_data) {
		pr_kut("device has data which it should not have");
		return -9;
	}
	if (dev->p->device != dev) {
		pr_kut("private data's device doesn't point back to dev");
		return -10;
	}

	return 0;
}

/**
 * verify_page - test all the fields of a struct page and verify they're as
 * expected
 * @page: the page test
 * @gfp_mask: the expected gfp mask
 * @order: the expected order of the page
 * @pages_to_test: is the page the 1st in an allocation segment
 */
static int verify_page(struct page *page, gfp_t gfp_mask, int order,
	int pages_to_test)
{
	int i;
	void *virtual;

	if (!page) {
		if (order >= MAX_ORDER)
			return 0;

		pr_kut("page segment not created");
		return -1;
	}

	if (!page_address(page)) {
		pr_kut("virtual not allocated");
		return -2;
	}

	virtual = page_address(page);

	for (i = 0; i < pages_to_test; i++) {
		if (page_address(&page[i]) != virtual + i * PAGE_SIZE) {
			pr_kut("page %d: virtual incorrectly assigned", i);
			return -3;
		}

		if (kut_mem_lookup_virtual_by_page(&page[i]) !=
			page_address(&page[i])) {
			pr_kut("page %d: "
				"wrong kut_mem_lookup_virtual_by_page()", i);
			return -4;
		}

		if (kut_mem_lookup_page_by_virtual(virtual + i * PAGE_SIZE) !=
			&page[i]) {
			pr_kut("page %d: "
				"wrong kut_mem_lookup_page_by_virtual()", i);
			return -5;
		}

		if (page[i].order >= MAX_ORDER) {
			pr_kut("page %d: page order to high", i);
			return -6;
		}

		if (page[i].order != order) {
			pr_kut("page %d: wrong order value", i);
			return -7;
		}

		if (page[i].gfp_mask != gfp_mask) {
			pr_kut("page %d: wrong gfp_mask value", i);
			return -8;
		}

		if (page[i].private != i) {
			pr_kut("page %d: wrong private value", i);
			return -9;
		}
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

static int file_test_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t file_test_read(struct file *filp,
	char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int *data = filp->private_data;
	int len, ret;
	char buf[30];

	len = snprintf(buf, sizeof(buf), "%d", *data);
	ret = simple_read_from_buffer(ubuf, len, ppos, buf, len);
	return ret;
}

static ssize_t file_test_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int *data = filp->private_data;
	int ret;

	ret = kstrtoint_from_user(ubuf, strlen(ubuf), 0, data);
	return ret ? 0 : cnt;
}

static struct file_operations file_test_fops = {
	.open = file_test_open,
	.read = file_test_read,
	.write = file_test_write,
};

static int simple_file_test(void)
{
	struct dentry *dentry;
	int data = 4, test;
	struct file *filp = NULL;
	int ret = -1;
	char buf[30] = {0};
	loff_t ppos = 0;

	/* create file dentry */
	dentry = kut_dentry_create("test_file", &kern_root,
		KUT_MODE_DEFAULT_FILE, &data, &file_test_fops, false);
	if (verify_dentry(dentry, &kern_root, &data, "test_file", 1, 0, false,
		0, false, &file_test_fops)) {
		goto exit;
	}

	/* file open */
	filp = kut_file_open(dentry, KUT_RDWR, NULL);
	if (verify_file(filp, &file_test_fops, &data, &data, dentry))
		goto exit;

	/* file read */
	kut_file_read(filp, buf, sizeof(buf), &ppos);
	ret = kstrtoint(buf, 10, &test);
	if (ret || test != 4) {
		ret = -1;
		goto exit;
	}

	/* file write */
	kut_file_write(filp, "5", strlen("5"), &ppos);
	if (data != 5)
		goto exit;

	/* read to verify write */
	kut_file_read(filp, buf, sizeof(buf), &ppos);
	ret = kstrtoint(buf, 10, &test);
	if (ret || test != 5) {
		ret = -1;
		goto exit;
	}

exit:
	/* close all */
	if (filp)
		kut_file_close(filp);
	if (dentry)
		kut_dentry_remove(dentry);
	return ret;
}

static int file_dentry_read_write(void)
{
	struct dentry *dentry;
	int data = 4, test;
	int ret = -1;
	char buf[30] = {0};

	/* create file dentry */
	dentry = kut_dentry_create("test_file", &kern_root,
		KUT_MODE_DEFAULT_FILE, &data, &file_test_fops, false);
	if (verify_dentry(dentry, &kern_root, &data, "test_file", 1, 0, false,
		0, false, &file_test_fops)) {
		goto exit;
	}

	/* dentry read */
	kut_dentry_read(dentry, buf, sizeof(buf));
	ret = kstrtoint(buf, 10, &test);
	if (ret || test != 4) {
		ret = -1;
		goto exit;
	}

	/* dentry write */
	kut_dentry_write(dentry, "5", strlen("5"));
	if (data != 5)
		goto exit;

	/* read to verify write */
	kut_dentry_read(dentry, buf, sizeof(buf));
	ret = kstrtoint(buf, 10, &test);
	if (ret || test != 5) {
		ret = -1;
		goto exit;
	}

exit:
	/* close dentry */
	if (dentry)
		kut_dentry_remove(dentry);
	return ret;
}

static int file_dentry_ref_count(void)
{
	struct dentry *dentry;
	struct file *filp1 = NULL, *filp2 = NULL;
	int ret = -1;

	/* create file dentry */
	dentry = kut_dentry_create("test_file", &kern_root,
		KUT_MODE_DEFAULT_FILE, NULL, &file_test_fops, false);
	if (verify_dentry(dentry, &kern_root, NULL, "test_file", 1, false,
		false, 0, false, &file_test_fops)) {
		goto exit;
	}

	/* filp1 open */
	filp1 = kut_file_open(dentry, KUT_RDONLY, NULL);
	if (verify_file(filp1, &file_test_fops, NULL, NULL, dentry))
		goto exit;
	if (verify_dentry(dentry, &kern_root, NULL, "test_file", 1, false,
		false, 1, false, &file_test_fops)) {
		goto exit;
	}

	/* try to remove the dentry */
	ret = kut_dentry_remove(dentry);
	if (ret != -EBUSY) {
		pr_kut("tried to remove a busy dentry and got wrong result: %d",
			ret);
		if (!ret)
			dentry = NULL;
		goto exit;
	}
	/* filp2 open */
	filp2 = kut_file_open(dentry, KUT_WRONLY, NULL);
	if (verify_file(filp2, &file_test_fops, NULL, NULL, dentry))
		goto exit;
	if (verify_dentry(dentry, &kern_root, NULL, "test_file", 1, false,
		false, 2, false, &file_test_fops)) {
		goto exit;
	}

	/* try to remove the dentry */
	ret = kut_dentry_remove(dentry);
	if (ret != -EBUSY) {
		pr_kut("tried to remove a busy dentry and got wrong result: %d",
			ret);
		if (!ret)
			dentry = NULL;
		goto exit;
	}

	/* filp1 close */
	kut_file_close(filp1);
	filp1 = NULL;

	/* try to remove the dentry */
	ret = kut_dentry_remove(dentry);
	if (ret != -EBUSY) {
		pr_kut("tried to remove a busy dentry and got wrong result: %d",
			ret);
		if (!ret)
			dentry = NULL;
		goto exit;
	}

	/* filp2 close */
	kut_file_close(filp2);
	filp2 = NULL;

	/* try to remove the dentry */
	ret = kut_dentry_remove(dentry);
	if (ret) {
		pr_kut("could not remove an idle dentry: %d", ret);
		goto exit;
	}
	dentry = NULL;

	ret = 0;
exit:
	/* close all */
	if (filp1)
		kut_file_close(filp1);
	if (filp2)
		kut_file_close(filp2);
	if (dentry)
		kut_dentry_remove(dentry);
	return ret;
}

static int debugfs_basic(void)
{
	struct dentry *dentry_d, *dentry_f = NULL;
	int data = 10;
	int ret = -1;
	char buf[30] = {0};

	dentry_d = debugfs_create_dir("debugfs_test_dir", NULL);
	if (verify_dentry(dentry_d, &kern_root, NULL, "debugfs_test_dir", 1,
		false, true, 0, false, NULL)) {
		goto exit;
	}
	dentry_f = debugfs_create_file("debugfs_test_file",
		KUT_MODE_DEFAULT_FILE, dentry_d, &data, &file_test_fops);
	if (verify_dentry(dentry_f, dentry_d, &data, "debugfs_test_file", 2,
		false, false, 0, false, &file_test_fops)) {
		goto exit;
	}

	kut_dentry_read(dentry_f, buf, sizeof(buf));
	if (strcmp(buf, "10")) {
		pr_kut("expected to read 10 but got: %s", buf);
		goto exit;
	}

	kut_dentry_write(dentry_f, "21", strlen("21"));
	if (data != 21) {
		pr_kut("data should have been written to 21, but is: %d", data);
		goto exit;
	}

	ret = 0;
exit:
	if (dentry_d)
		kut_dentry_remove_recursive(dentry_d);
	return ret;
}

static int device_life_cycle(void)
{
	struct device *dev;
	int ret = -1;

	dev = kut_dev_init(NULL, NULL, "parent");
	if (verify_device(dev, NULL, NULL, "parent", false, false))
		goto exit;

	dev = kut_dev_uninit(dev);
	if (dev)
		goto exit;

	ret = 0;
exit:
	if (dev) {
		kfree(dev->p);
		kfree(dev);
	}

	return ret;
}

static int simple_device_hierarchy_fn(struct device *dev, void *data)
{
	int *generation = data;
	char fmt[20], output[50];

	if (*generation) {
		snprintf(fmt, sizeof(fmt), "%%-%ds|\n", *generation);
		snprintf(output, sizeof(output), fmt, "");
		pr_info("%s", output);
	}
	snprintf(fmt, sizeof(fmt), "%%-%ds+-- %%s (%%s)\n", *generation);
	snprintf(output, sizeof(output), fmt, "", dev_name(dev),
		dev->dynamic ? "dynamic" : "static");
	pr_info("%s", output);

	*generation+=2;
	device_for_each_child(dev, generation, simple_device_hierarchy_fn);
	*generation-=2;
	return 0;
}

static int simple_device_hierarchy(void)
{
	struct device grandpa = {0}, son1 = {0}; /* statically allocated */
	struct device *dad, *daughter1, *daughter2; /*dynamically allocated */
	struct device *tmp;
	int ret = -1;
	int generation = 0;

	tmp = kut_dev_init(&grandpa, NULL, "grandpa");
	if (verify_device(tmp, &grandpa, NULL, "grandpa", false, false))
		goto exit;

	dad = kut_dev_init(NULL, &grandpa, "dad");
	if (verify_device(dad, NULL, &grandpa, "dad", false, false))
		goto exit;

	daughter1 = kut_dev_init(NULL, dad, "daughter1");
	if (verify_device(daughter1, NULL, dad, "daughter1", false, false))
		goto exit;

	tmp = kut_dev_init(&son1, dad, "son1");
	if (verify_device(tmp, &son1, dad, "son1", true, false)) {
		goto exit;
	}

	daughter2 = kut_dev_init(NULL, dad, "daughter2");
	if (verify_device(daughter2, NULL, dad, "daughter2", true, false))
		goto exit;

	/* verify grandpa and dad now have children and that daughter1 now has
	 * siblings */
	if (verify_device(&grandpa, &grandpa, NULL, "grandpa", false, true) ||
		verify_device(dad, NULL, &grandpa, "dad", false, true) ||
		verify_device(daughter1, NULL, dad, "daughter1", true, false)) {
		goto exit;
	}

	ret = simple_device_hierarchy_fn(&grandpa, &generation);

exit:
	if (kut_dev_uninit(&grandpa))
		ret = -1;

	return ret;
}

static int get_order_test(void)
{
	int orders[][2] = { /* [test value][expected result]*/
		{ 0, 0 }, 
		{ 1, 0 },
		{ 1000, 0},
		{ PAGE_SIZE - 1, 0 },
		{ PAGE_SIZE, 0 },
		{ PAGE_SIZE + 1, 1 },
		{ 2*PAGE_SIZE - 1, 1 },
		{ 2*PAGE_SIZE, 1 },
		{ 2*PAGE_SIZE + 1, 2 },
		{ 3*PAGE_SIZE + 1, 2 },
		{ 4*PAGE_SIZE - 1, 2 },
		{ 4*PAGE_SIZE, 2 },
		{ 4*PAGE_SIZE + 1, 3 },
		{ 7*PAGE_SIZE + 1, 3 },
		{ 8*PAGE_SIZE - 1, 3 },
		{ 8*PAGE_SIZE, 3 },
		{ 8*PAGE_SIZE + 1, 4 }
	};
	int i, ret = 0;

	for (i = 0; i < ARRAY_SZ(orders); i++) {
		int val = get_order(orders[i][0]);

		printf("get_order(%d): %d\n", orders[i][0], val);
		if (val != orders[i][1])
			ret = -1;
	}

	return ret;
}

static int alloc_free_pages_order(int order)
{
	struct page *page;
	int ret = -1;

	page = alloc_pages(GFP_KERNEL, order);
	if (verify_page(page, GFP_KERNEL, order, 1 << order))
		goto exit;

	ret = 0;

exit:
	__free_pages(page, order);
	return ret;
}

static int alloc_free_pages(void)
{
	int order;

	for (order = 0; order <= MAX_ORDER; order++)
		if (alloc_free_pages_order(order)) {
			pr_kut("failed on order: %d", order);
			return -1;
	};

	return 0;
}

static int mem_pressure_control(void)
{
	int i, mp[] = {
		KUT_MEM_SCARCE,
		KUT_MEM_AVERAGE,
		KUT_MEM_ABUNDANCE,
	};

	for (i = 0; i < ARRAY_SZ(mp); i++) {
		int ret;

		kut_mem_pressure_set(mp[i]);

		ret = alloc_free_pages();
		if (ret) {
			pr_kut("failed on memory presure level: %d", mp[i]);
			return ret;
		}
	}

	return 0;
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
	{
		.description = "virtual file: create, read and write",
		.func = simple_file_test,
	},
	{
		.description = "virtual file: dentry read/write",
		.func = file_dentry_read_write,
	},
	{
		.description = "virtual file: dentry reference count",
		.func = file_dentry_ref_count,
	},
	{
		.description = "debugfs: basic usability",
		.func = debugfs_basic,
	},
	{
		.description = "device: init and uninit a device",
		.func = device_life_cycle,
	},
	{
		.description = "device: simple device hierarchy",
		.func = simple_device_hierarchy,
	},
	{
		.description = "mm: verify get_order() functionality",
		.func = get_order_test,
	},
	{
		.description = "mm: alloc_pages() and __free_pages()",
		.func = alloc_free_pages,
	},
	{
		.description = "mm: memory pressure configuration",
		.func = mem_pressure_control,
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

