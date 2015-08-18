#ifndef _KUT_DEVICE_H_
#define _KUT_DEVICE_H_

#define kunit_dev_init(dev, par, name, ...) ({ \
	int ret = -1; \
	do { \
		struct device *__par = par; \
		dev = NULL; \
		if (!(get_device(dev))) \
			break; \
		if ((ret = device_register(dev))) { \
			put_device(dev); \
			break; \
		} \
		if ((ret = dev_set_name(dev, name, ##__VA_ARGS__))) { \
			put_device(dev); \
			break; \
		} \
		dev->parent = __par; \
		if (__par) { \
			list_add_tail(&dev->p->knode_parent, \
				&__par->p->klist_children); \
		} \
	} while (0); \
	ret; \
})

#define kunit_dev_uninit(dev) ({ \
	int ret; \
	do { \
		ret = device_for_each_child(dev, &dev, kunit_dev_uninit_fn); \
		device_unregister(dev); \
		put_device(dev); \
		dev = NULL; \
	} while (0); \
	ret; \
})

static inline int kunit_dev_uninit_fn(struct device *dev, void *data)
{
	struct device **pdev = (struct device**)data;

	if (kunit_dev_uninit(dev))
		return -1;

	*pdev = NULL;
	return 0;
}

#endif

