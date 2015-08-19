#include <linux/device.h>
#include <stdarg.h>

#include "../../../kut/drivers/base/core.h"

int device_private_init(struct device *dev)
{
	dev->p = kzalloc(sizeof(*dev->p), GFP_KERNEL);
	if (!dev->p)
		return -ENOMEM;
	dev->p->device = dev;
	INIT_LIST_HEAD(&dev->p->klist_children);
	INIT_LIST_HEAD(&dev->p->knode_parent);

	return 0;
}

/**
 * dev_set_name - set a device name
 * @dev: device
 * @fmt: format string for the device's name
 */
int dev_set_name(struct device *dev, const char *fmt, ...)
{
	va_list vargs;
	int ret;

	va_start(vargs, fmt);
	ret = kut_dev_set_name(dev, fmt, vargs);
	va_end(vargs);

	return ret;
}
EXPORT_SYMBOL_GPL(dev_set_name);

/**
 * device_initialize - init device structure.
 * @dev: device.
 *
 * This prepares the device for use by other layers by initializing
 * its fields.
 * It is the first half of device_register(), if called by
 * that function, though it can also be called separately, so one
 * may use @dev's fields. In particular, get_device()/put_device()
 * may be used for reference counting of @dev after calling this
 * function.
 *
 * All fields in @dev must be initialized by the caller to 0, except
 * for those explicitly set to some other value.  The simplest
 * approach is to use kzalloc() to allocate the structure containing
 * @dev.
 *
 * NOTE: Use put_device() to give up your reference instead of freeing
 * @dev directly once you have called this function.
 */
void device_initialize(struct device *dev)
{
	/* nothing to do */
}

/**
 * device_add - add device to device hierarchy.
 * @dev: device.
 *
 * This is part 2 of device_register(), though may be called
 * separately _iff_ device_initialize() has been called separately.
 *
 * This adds @dev to the kobject hierarchy via kobject_add(), adds it
 * to the global and sibling lists for the device, then
 * adds it to the other relevant subsystems of the driver model.
 *
 * Do not call this routine or device_register() more than once for
 * any device structure.  The driver model core is not designed to work
 * with devices that get unregistered and then spring back to life.
 * (Among other things, it's very hard to guarantee that all references
 * to the previous incarnation of @dev have been dropped.)  Allocate
 * and register a fresh new struct device instead.
 *
 * NOTE: _Never_ directly free @dev after calling this function, even
 * if it returned an error! Always use put_device() to give up your
 * reference instead.
 */
int __must_check device_add(struct device *dev)
{
	if (dev->p)
		return 0;

	return device_private_init(dev);
}

int __must_check device_register(struct device *dev)
{
	device_initialize(dev);
	return device_add(dev);
}

void device_del(struct device *dev)
{
	list_del(&dev->p->knode_parent);
	kfree(dev->p);
}

void device_unregister(struct device *dev)
{
	device_del(dev);
}

int device_for_each_child(struct device *dev, void *data,
	int (*fn)(struct device *dev, void *data))
{
	struct device_private *p, *tmp;
	int ret = 0;


	list_for_each_entry_safe(p, tmp, &dev->p->klist_children,
		knode_parent) {
		if (fn(p->device, data))
			ret = -1;
	}

	return ret;
}

