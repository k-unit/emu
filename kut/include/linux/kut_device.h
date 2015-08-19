#ifndef _KUT_DEVICE_H_
#define _KUT_DEVICE_H_

#include <linux/device.h>
#include <stdarg.h>

struct device *kut_dev_init(struct device *dev, struct device *parent,
	const char *name, ...);
struct device *kut_dev_uninit(struct device *dev);
#endif

