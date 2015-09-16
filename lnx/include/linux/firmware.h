#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include <linux/types.h>
#include <linux/device.h>

struct firmware {
	size_t size;
	const u8 *data;
	struct page **pages;

	/* firmware loader private fields */
	void *priv;
};

int request_firmware(const struct firmware **fw, const char *name,
	struct device *device);
void release_firmware(const struct firmware *fw);

#endif

