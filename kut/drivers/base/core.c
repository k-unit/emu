#include <linux/kut_device.h>
#include <stdarg.h>

struct device *kut_dev_uninit(struct device *dev);

/*
 * kut_dev_set_name() - set a device name
 * @dev:	the device whose name to set.
 * @fmt:	printf style format string.
 * @vargs:	va list initialized by va_start().
 *
 * Set the name of a given device.
 *
 * Returns: 0 for success, EINVAL otherwise
 */
int kut_dev_set_name(struct device *dev, const char *fmt, va_list vargs)
{
	int ret;

	ret = vsnprintf(dev->__name, sizeof(dev->__name), fmt, vargs);
	if (ret >= sizeof(dev->__name))
		return -EINVAL;

	dev->init_name = dev->__name;
	return 0;
}

/*
 * kut_dev_init() - initialize a struct device
 * @dev:	the device to be initialized.
 * @parent:	an optional parent device.
 * @name:	a name to set for the device.
 *
 * Initializes a struct device. If dev is NULL, it is said to be dynamic
 * (dev->dynamic is set to true) and the new device is dynamically allocated
 * with kmalloc(). Otherwise, dev is taken to be the address of a pre-allocated
 * struct device and is said to be static (dev->dynamic is set to false).
 *
 * If parent is non NULL, it is set as the parent device of dev.
 *
 * dev is initialized with the format string name and the corresponding
 * subsequent arguemnts.
 *
 * Dynamic device initalization is useful for creating general purpose devices
 * such as arbirary parent devices or general purpose device hierarchies.
 * Static device initalization is useful for initializing devices embedded in
 * other structures.
 *
 * Returns: the address of the struct device on success, NULL otherwise
 */
struct device *kut_dev_init(struct device *dev, struct device *parent,
	const char *name, ...)
{
	va_list vargs;
	int ret;

	if (!(get_device(dev)))
		return NULL;
	if ((device_register(dev))) {
		put_device(dev);
		return NULL;
	}

	va_start(vargs, name);
	ret = kut_dev_set_name(dev, name, vargs);
	va_end(vargs);
	if (ret) {
		put_device(dev);
		return NULL;
	}

	dev->parent = parent;
	if (parent) {
		list_add_tail(&dev->p->knode_parent,
			&parent->p->klist_children);
	}

	return dev;
}

static int dev_uninit_fn(struct device *dev, void *data)
{
	if (kut_dev_uninit(dev))
		return -1;

	return 0;
}

/*
 * kut_dev_uninit() - uninitialize a struct device and its decendents
 * @dev:	the device to be uninitialized.
 *
 * Recursively traverse and free a device tree rooted at dev.
 *
 * dev and all its decendents must have been initialized using kut_dev_init().
 * Each of them can be dynamic or static indepedant of the other devices in the
 * tree.
 *
 * The function is particularly convenient for freeing an entire device tree
 * by issuing a single call with the tree's root device as the parameter.
 *
 * Returns: if the device is dynamic, NULL is returned. Otherwise, the address
 *          of the static device is returned.
 */
struct device *kut_dev_uninit(struct device *dev)
{
	int ret;

	if (!dev)
		return NULL;
	ret = device_for_each_child(dev, NULL, dev_uninit_fn);
	device_unregister(dev);
	put_device(dev);
	return ret ? dev : NULL;
}

