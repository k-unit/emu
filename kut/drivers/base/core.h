#ifndef _KUT_DRIVERS_BASE_CORE_
#define _KUT_DRIVERS_BASE_CORE_

#include <linux/device.h>
#include <stdarg.h>

int kut_dev_set_name(struct device *dev, const char *fmt, va_list vargs);

#endif

