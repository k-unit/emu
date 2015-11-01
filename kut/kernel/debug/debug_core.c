#include <linux/fs.h>
#include <linux/list.h>

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
static int reset_dir(char *path)
{
	return 0;
}
#endif

static int bug_on_test(void)
{
	int ret;

	kut_bug_on_do_exit_set(false);

	pr_kut("calling BUG_ON(1)...");
	KUT_CAN_BUG_ON(ret, BUG_ON(1));

	return !ret;
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
};

struct unit_test ut_kernel = {
	.module = "kernel",
	.description = "Kernel Emulation",
	.pre_single_test = pre_post_test,
	.post_single_test = pre_post_test,
	.tests = kernel_tests,
	.count = ARRAY_SZ(kernel_tests),
};

