#include <linux/device.h>

int device_for_each_child(struct device *parent, void *data,
	int (*fn)(struct device *dev, void *data))
{
	struct device *child;

	for (child = parent->p; child; child = child->next)
		fn(child, data);

	return 0;
}

