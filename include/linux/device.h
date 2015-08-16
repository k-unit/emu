#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <linux/list.h>

#define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, _show, _store)

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
	struct device *p;
	const char *init_name; /* initial name of the device */

	struct device *next;
};

static inline const char *dev_name(const struct device *dev)
{
	/* Use the init name until the kobject becomes available */
	return dev->init_name;
}

int device_for_each_child(struct device *parent, void *data,
	int (*fn)(struct device *dev, void *data));

#endif

