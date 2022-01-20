#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/list.h>
#include <linux/pagemap.h>
#include <linux/mmzone.h>
#include <linux/scatterlist.h>

#include <asm-generic/page.h>

#include <linux/kut_types.h>
#include <linux/kut_device.h>
#include <linux/kut_namei.h>

#include <linux/kut_fs.h>
#include <linux/kut_mmzone.h>
#include <linux/kut_random.h>
#include <linux/kut_bug.h>
#include <linux/mmc/kut_host.h>
#include <linux/mmc/kut_bus.h>
#include <linux/mmc/kut_core.h>

#include <asm-generic/bug.h>

#include <unit_test.h>

#include <stdlib.h>
#include <string.h>
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

/**
 * verify_sgtable - test all the fields of a struct sg_table and verify they're
 * as expected
 * @table: the sg table to test
 * @nents: the number of entries expected in the sg table
 * @gfp_mask: the expected gfp mask
 * @mp: the level of memory pressure
 */
static int verify_sgtable(struct sg_table *table, unsigned int nents,
	gfp_t gfp_mask, enum kut_mem_pressure mp)
{
	struct scatterlist *sgl;
	int i;

	if (!table) {
		pr_kut("no sg table");
		return -1;
	}

	if (table->nents != nents) {
		pr_kut("wrong number of nents");
		return -2;
	}

	if (table->orig_nents != nents) {
		pr_kut("wrong number of original nents");
		return -3;
	}

	for_each_sg(table->sgl, sgl, table->orig_nents, i) {
		struct scatterlist dummy = {0};

		if (sg_is_last(sgl))
			sg_mark_end(&dummy);

		if (memcmp(sgl, &dummy, sizeof(struct scatterlist))) {
			int current_chain = i/SG_MAX_SINGLE_ALLOC;
			int remainder = i - current_chain * SG_MAX_SINGLE_ALLOC;

			pr_kut("sgl[%d][%d] is not initialized", current_chain,
				remainder);

			return -4;
		}
	}

	return 0;
}

/**
 * verify_host_controller - test all the fields of the kunit host controller and
 * verify they're as expected
 * @bus: the host controller's parent device
 * @hc: the host controller to test
 * @mmc: an optional struct mmc_host (can be NULL)
 */
static int verify_host_controller(struct device *bus, struct kunit_host *hc,
	struct mmc_host *mmc)
{
	char kunit_host_name[DEV_MAX_NAME] = KUNIT_HOST_NAME;

	if (!hc) {
		pr_kut("no host controller");
		return -1;
	}

	if (mmc) {
		int len = strlen(kunit_host_name);

		snprintf(kunit_host_name + len, DEV_MAX_NAME - len, ".%d",
			mmc->index);
	}

	if (verify_device(&hc->dev, &hc->dev, bus, kunit_host_name, false,
		mmc ? true : false)) {
		return -2;
	}

	if (hc->mmc != mmc) {
		pr_kut("wrong mmc");
		return -3;
	}

	return 0;
}

/**
 * verify_mmc_host - test all the fields of an mmc host and verify they're as
 * expected
 * @host: the host to test
 * @hc: an optional kunit host controller (can be NULL)
 * @siblings: are any siblings expected
 * @card: an optional mmc card (can be NULL)
 * @private: host private data
 */
static int verify_mmc_host(struct mmc_host *host, struct kunit_host *hc,
	bool siblings, struct mmc_card *card, void *private)
{
	char name[10];

	if (!host) {
		pr_kut("no host");
		return -1;
	}

	if (host->card != card) {
		pr_kut("wrong card");
	}

	snprintf(name, sizeof(name), "mmc%d", host->index);
	if (verify_device(&host->class_dev, &host->class_dev,
		hc ? &hc->dev : NULL, name, siblings, card ? true : false)) {
		return -2;
	}

	if (host->max_seg_size != 128 * 512) {
		pr_kut("wrong max_seg_size");
		return -3;
	}
	if (host->max_segs != 1 * 512) {
		pr_kut("wrong max_segs");
		return -4;
	}
	if (host->max_req_size != 65536 * 512) {
		pr_kut("wrong max_req_size");
		return -5;
	}
	if (host->max_blk_count != 65536) {
		pr_kut("wrong max_blk_count");
		return -7;
	}

	if (host->private != private) {
		pr_kut("wrong private");
		return -8;
	}

	return 0;
}

/**
 * verify_mmc_card - test all the fields of an mmc card and verify they're as
 * expected
 * @card: the card to test
 * @index: card index number within the host
 * @md_main: is a main partition device expected
 * @partitions: are any partitions expected
 */
static int verify_mmc_card(struct mmc_card *card, int index, bool md_main,
	bool partitions)
{
	char name[20];

	if (!card) {
		pr_kut("no card");
		return -1;
	}

	if (!card->host) {
		pr_kut("no card->host");
		return -2;
	}

	snprintf(name, sizeof(name), "%s:%.4d", mmc_hostname(card->host),
		index);
	if (card->md_main) {
		struct device_private *md_dev_private;

		if (!md_main) {
			pr_kut("should not have md_main");
			return -3;
		}

		if (verify_device(&card->dev, &card->dev,
			&card->host->class_dev, name, false, true)) {
			return -4;
		}

		if ((card->xfer.next != card->xfer.prev) ||
			!list_empty(&card->xfer)) {
			pr_kut("card xfer list is not correctly initialized");
			return -5;
		}

		md_dev_private = list_entry(card->dev.p->klist_children.next,
			struct device_private, knode_parent);
		snprintf(name, sizeof(name), "mmcblk%d", card->host->index);
		if (verify_device(md_dev_private->device, NULL, &card->dev,
			name, false, partitions)) {
			return -6;
		}

		if (md_dev_private->device != kut_mmc_card_md_main(card)) {
			pr_kut("kut_mmc_card_md_main() returns wrong device");
			return -7;
		}
	} else {
		if (md_main) {
			pr_kut("should have md_main");
			return -8;
		}

		if (verify_device(&card->dev, &card->dev,
			&card->host->class_dev, name, false, partitions)) {
			return -9;
		}
	}

	return 0;
}

static int length2nents(int length)
{
	return DIV_ROUND_UP(length, PAGE_SIZE << (kut_mem_pressure_get() - 1));
}

static int bug_on_test(void)
{
	int ret;

	kut_bug_on_do_exit_set(false);

	pr_kut("calling BUG_ON(1)...");
	KUT_CAN_BUG_ON(ret, BUG_ON(1));

	return !ret;
}

static void sg_table_teardown(struct sg_table *table)
{
	struct scatterlist *sg;
	int i;

	if (!table->sgl)
		return;

	for_each_sg(table->sgl, sg, table->orig_nents, i) {
		struct page *page;
		
		page = sg_page(sg);
		if (!page)
			break;

		__free_pages(page, kut_mem_pressure_get() - 1);
	}
	sg_free_table(table);
}

static int sg_table_setup(struct sg_table *table, int nents, int chunk_size,
	gfp_t gfp_mask)
{
	struct scatterlist *sg;
	int i, max_chunk_size = 1 << (kut_mem_pressure_get() - 1);

	/* chunk_size cannot exceed the maximum page segment allocation size */
	if (max_chunk_size < chunk_size)
		return -1;

	/* unless otherwise specified, use maximum segment allocation size */
	if (!chunk_size)
		chunk_size = max_chunk_size;

	if (sg_alloc_table(table, nents, gfp_mask))
		goto error;

	for_each_sg(table->sgl, sg, table->orig_nents, i) {
		struct page *page;
		
		page = alloc_pages(gfp_mask, kut_mem_pressure_get() - 1);
		if (!page)
			goto error;

		sg_set_page(sg, page, chunk_size * PAGE_SIZE, 0);
	}

	return 0;

error:
	/* allocated lists and pages must be freed by sg_table_teardown */
	pr_kut("sg_alloc_table failed");

	return -ENOMEM;
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

static int sg_table_test(int nents)
{
	struct sg_table table = {0};
	int ret = -1;

	if (sg_alloc_table(&table, nents, GFP_KERNEL)) {
		pr_kut("sg_alloc_table failed");
		return -1;
	}
	if (verify_sgtable(&table, nents, GFP_KERNEL, KUT_MEM_SCARCE))
		goto exit;

	ret = 0;
exit:

	sg_free_table(&table);
	return ret;
}

static int sg_table_test_no_chain(void)
{
	return sg_table_test(SG_MAX_SINGLE_ALLOC);
}

static int sg_table_test_multi_chain(void)
{
	int nents = 3 * SG_MAX_SINGLE_ALLOC + (SG_MAX_SINGLE_ALLOC >> 1);

	return sg_table_test(nents);
}

static int sg_copy_buffer_test(void)
{
	struct sg_table table = {0};
	int nents, alloc_size;
	char *buf1 = NULL, *buf2 = NULL;
	int ret = -1;

	kut_mem_pressure_set(KUT_MEM_SCARCE);

	nents = 3 * SG_MAX_SINGLE_ALLOC + (SG_MAX_SINGLE_ALLOC >> 1);
	alloc_size = nents * (PAGE_SIZE << (kut_mem_pressure_get() - 1));

	buf1 = calloc(alloc_size, sizeof(char));
	if (!buf1) {
		pr_kut("could not allocate 1st test buffer");
		goto exit;
	}
	buf2 = calloc(alloc_size, sizeof(char));
	if (!buf2) {
		pr_kut("could not allocate 2nd test buffer");
		goto exit;
	}

	if (sg_table_setup(&table, nents, 0, GFP_KERNEL))
		goto exit;

	kut_random_buf(buf1, alloc_size);

	sg_copy_from_buffer(table.sgl, nents, buf1, alloc_size);
	sg_copy_to_buffer(table.sgl, nents, buf2, alloc_size);

	ret = memcmp(buf1, buf2, alloc_size);

exit:
	sg_table_teardown(&table);
	sg_free_table(&table);
	free(buf2);
	free(buf1);

	return ret;
}

static int mmc_init_host_controller_test(void)
{
	struct device bus = {0};
	struct kunit_host *hc;
	int ret;

	kut_dev_init(&bus, NULL, "bus");

	hc = kut_kunit_host_alloc(&bus);
	ret = verify_host_controller(&bus, hc, NULL) ? -1 : 0;
	kut_kunit_host_free(hc);

	kut_dev_uninit(&bus);
	return ret;
}

static int mmc_init_host_test(void)
{
	struct mmc_host *host;
	int ret;

	host = mmc_alloc_host(0, NULL);
	ret = verify_mmc_host(host, NULL, false, NULL, NULL);
	mmc_free_host(host);

	return ret;
}

static int mmc_init_card_test(void)
{
	struct mmc_host host;
	struct mmc_card *card = NULL;
	struct device *md_dev, *part_dev;
	int ret = -1;

	memset(&host, 0, sizeof(struct mmc_host));
	kut_dev_init(&host.class_dev, NULL, "parent");
	if (verify_device(&host.class_dev, &host.class_dev, NULL, "parent",
		false, false)) {
		goto exit;
	}

	host.index = 0;

	/* card without md_data */
	card = kut_mmc_alloc_card(&host, NULL, false);
	if (verify_mmc_card(card, 1, false, false))
		goto exit;

	part_dev = kut_mmc_add_partition(card, 1);
	if (verify_device(part_dev, NULL, &card->dev, "parent:0001p1", false,
		false)) {
		goto exit;
	}
	if (verify_mmc_card(card, 1, false, true))
		goto exit;

	kut_mmc_free_card(card);

	/* card with md_data */
	card = kut_mmc_alloc_card(&host, NULL, true);
	if (verify_mmc_card(card, 1, true, false))
		goto exit;
	md_dev = kut_mmc_card_md_main(card);
	part_dev = kut_mmc_add_partition(card, 1);
	if (verify_device(part_dev, NULL, md_dev, "mmcblk0p1", false, false))
		goto exit;
	if (verify_mmc_card(card, 1, true, true))
		goto exit;

	ret = 0;
exit:
	kut_mmc_free_card(card);
	kut_dev_uninit(&host.class_dev);
	return ret;
}
 
static int mmc_init_test(void)
{
	struct kunit_host *hc;
	struct mmc_host *host;
	struct mmc_card *card;
	int ret;

	/* test host controller only */
	ret = kut_mmc_init(NULL, &hc, NULL, NULL, 0);
	if (ret || verify_host_controller(NULL, hc, NULL))
		return -1;
	kut_mmc_uninit(hc, NULL, NULL);

	/* test mmc host only */
	ret = kut_mmc_init(NULL, NULL, &host, NULL, 0);
	if (ret || verify_mmc_host(host, NULL, false, NULL, NULL))
		return -1;
	kut_mmc_uninit(NULL, host, NULL);

	/* test mmc host and card only */
	ret = kut_mmc_init(NULL, NULL, &host, &card, 0);
	if (ret || verify_mmc_host(host, NULL, false, card, NULL) ||
		verify_mmc_card(card, 1, true, 0)) {
		return -1;
	}
	kut_mmc_uninit(NULL, host, card);

	/* test host controller, mmc host and card */
	ret = kut_mmc_init(NULL, &hc, &host, &card, 2);
	if (ret || verify_host_controller(NULL, hc, host) ||
		verify_mmc_host(host, hc, false, card, hc) ||
		verify_mmc_card(card, 1, true, true)) {
		return -1;
	}
	kut_mmc_uninit(hc, host, card);

	return 0;
}

static int mmc_simple_transfer_test(void)
{
	struct mmc_host *host = NULL;
	struct mmc_card *card = NULL;
	struct list_head xfer_expected = {0};
	struct kut_mmc_card_xfer segment = {0};
	struct sg_table table;
	int nents, ret = -1;
	
	/* setup mmc card and host */
	if (kut_mmc_init(NULL, NULL, &host, &card, 0))
		return -1;

	/* set memory pressure */
	kut_mem_pressure_set(KUT_MEM_SCARCE);

	/* initialize expected transfer list */
	INIT_LIST_HEAD(&xfer_expected);

	/* setup transfer segment */
	segment.length = PAGE_SIZE << kut_mem_pressure_get(); /* spill over! */
	segment.buf = calloc(segment.length, sizeof(char));
	if (!segment.buf)
		goto exit;

	INIT_LIST_HEAD(&segment.list);

	segment.write = true;
	segment.addr = 10 * PAGE_SIZE;

	list_add(&segment.list, &xfer_expected);

	/* setup sg_table for transferring segment */
	nents = length2nents(segment.length);
	if (sg_table_setup(&table, nents, 0, GFP_KERNEL))
		goto exit;

	/* write scenario */

	/* test: wrong addr */
	kut_random_buf(segment.buf, segment.length);
	sg_copy_from_buffer(table.sgl, nents, segment.buf, segment.length);
	mmc_simple_transfer(card, table.sgl, nents, 0, segment.length / 512,
		512, segment.write);
	if (!kut_mmc_xfer_check(card, &xfer_expected)) /* should fail */
		goto exit;

	/* test: wrong length */
	kut_random_buf(segment.buf, segment.length);
	sg_copy_from_buffer(table.sgl, nents, segment.buf, segment.length);
	mmc_simple_transfer(card, table.sgl, nents, segment.addr,
		(segment.length-1) / 512, 512, segment.write);
	if (!kut_mmc_xfer_check(card, &xfer_expected)) /* should fail */
		goto exit;

	/* test: wrong io direction */
	kut_random_buf(segment.buf, segment.length);
	sg_copy_from_buffer(table.sgl, nents, segment.buf, segment.length);
	mmc_simple_transfer(card, table.sgl, nents, segment.addr,
		segment.length / 512, 512, !segment.write);
	if (!kut_mmc_xfer_check(card, &xfer_expected)) /* should fail */
		goto exit;

	/* test: wrong data */
	kut_random_buf(segment.buf, segment.length);
	sg_copy_from_buffer(table.sgl, nents, segment.buf, segment.length);
	mmc_simple_transfer(card, table.sgl, nents, segment.addr,
		segment.length / 512, 512, segment.write);
	kut_random_buf(segment.buf, segment.length);
	sg_copy_from_buffer(table.sgl, nents, segment.buf, segment.length);
	if (!kut_mmc_xfer_check(card, &xfer_expected)) /* should fail */
		goto exit;

	/* test: perfect match */
	kut_random_buf(segment.buf, segment.length);
	sg_copy_from_buffer(table.sgl, nents, segment.buf, segment.length);
	mmc_simple_transfer(card, table.sgl, nents, segment.addr,
		segment.length / 512, 512, segment.write);
	if (kut_mmc_xfer_check(card, &xfer_expected)) /* should pass */
		goto exit;

	/* read scenario */
	sg_table_teardown(&table);
	if (sg_table_setup(&table, nents, 0, GFP_KERNEL))
		goto exit;

	segment.write = false;
	segment.addr = 512;
	memset(segment.buf, 0, segment.length);

	mmc_simple_transfer(card, table.sgl, nents, segment.addr,
		segment.length / 512, 512, segment.write);
	sg_copy_to_buffer(table.sgl, nents, segment.buf, segment.length);
	if (kut_mmc_xfer_check(card, &xfer_expected)) /* should pass */
		goto exit;

	ret = 0;

exit:
	sg_table_teardown(&table);
	free(segment.buf);
	kut_mmc_uninit(NULL, host, card);

	return ret;
}

static int mmc_ext_csd_version_test(void)
{
	struct mmc_host *host = NULL;
	struct mmc_card *card = NULL;
	int i, ret = -1;
	
	/* setup mmc card and host */
	if (kut_mmc_init(NULL, NULL, &host, &card, 0))
		return -1;

	/* test valid versions */
	for (i = 0; i < 10; i++) {
		int valid = kut_mmc_ext_csd_set_rev(i) ? 0 : 1;

		if ((i < 9) ^ valid)
			goto exit;
	}
	ret = 0;
exit:
	kut_mmc_uninit(NULL, host, card);
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
	{
		.description = "sg: scatter-gather table - no chain",
		.func = sg_table_test_no_chain,
	},
	{
		.description = "sg: scatter-gather table - multi chain",
		.func = sg_table_test_multi_chain,
	},
	{
		.description = "sg: copy to/from buffer",
		.func = sg_copy_buffer_test,
	},
	{
		.description = "mmc: simple host controller test",
		.func = mmc_init_host_controller_test,
	},
	{
		.description = "mmc: simple host test",
		.func = mmc_init_host_test,
	},
	{
		.description = "mmc: simple card test",
		.func = mmc_init_card_test,
	},
	{
		.description = "mmc: complete init test",
		.func = mmc_init_test,
	},
	{
		.description = "mmc: simple transfer test",
		.func = mmc_simple_transfer_test,
	},
	{
		.description = "mmc: ext_csd version test",
		.func = mmc_ext_csd_version_test,
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

