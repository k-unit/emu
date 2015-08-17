#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <linux/slab.h>
#include <linux/list.h>
#include <stdarg.h>

#define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, _show, _store)

#define DEV_MAX_NAME 128
/**
 * struct device_private - structure to hold the private to the driver core portions of the device structure.
 *
 * @klist_children - klist containing all children of this device
 * @knode_parent - node in sibling list
 * @knode_driver - node in driver list
 * @knode_bus - node in bus list
 * @deferred_probe - entry in deferred_probe_list which is used to retry the
 *	binding of drivers which were unable to get all the resources needed by
 *	the device; typically because it depends on another driver getting
 *	probed first.
 * @driver_data - private pointer for driver specific info.  Will turn into a
 * list soon.
 * @device - pointer back to the struct class that this structure is
 * associated with.
 *
 * Nothing outside of the driver core should ever touch these fields.
 */
struct device_private {
	struct list_head klist_children;
	struct list_head knode_parent;
	void *driver_data;
	struct device *device;
};

/**
 * struct device - The basic device structure
 * @parent:	The device's "parent" device, the device to which it is attached.
 * 		In most cases, a parent device is some sort of bus or host
 * 		controller. If parent is NULL, the device, is a top-level device,
 * 		which is not usually what you want.
 * @p:		Holds the private data of the driver core portions of the device.
 * 		See the comment of the struct device_private for detail.
 * @init_name:	Initial name of the device.
 *
 * ...
 *
 * At the lowest level, every device in a Linux system is represented by an
 * instance of struct device. The device structure contains the information
 * that the device model core needs to model the system. Most subsystems,
 * however, track additional information about the devices they host. As a
 * result, it is rare for devices to be represented by bare device structures;
 * instead, that structure, like kobject structures, is usually embedded within
 * a higher-level representation of the device.
 */
struct device {
	struct device *parent;
	struct device_private *p;
	const char *init_name; /* initial name of the device */
	char __name[DEV_MAX_NAME];
};

static inline const char *dev_name(const struct device *dev)
{
	/* Use the init name until the kobject becomes available */
	return dev->init_name;
}

/*
 * High level routines for use by the bus drivers
 */
int __must_check device_register(struct device *dev);
void device_unregister(struct device *dev);
void device_initialize(struct device *dev);
int __must_check device_add(struct device *dev);
void device_del(struct device *dev);
int device_init(struct device *dev, const char *name);
int device_for_each_child(struct device *parent, void *data,
	int (*fn)(struct device *dev, void *data));

int dev_set_name(struct device *dev, const char *fmt, ...);

#define get_device(dev) ({ \
	do { \
		if (!dev) \
			dev = kzalloc(sizeof(struct device), GFP_KERNEL); \
	} while (0); \
	dev; \
})

#define put_device(dev) do { \
	kfree(dev); \
	dev = NULL; \
} while (0)

/*
 * Unit Test routenes
 */
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

