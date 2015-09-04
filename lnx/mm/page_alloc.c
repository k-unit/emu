#include <linux/export.h>

unsigned long nr_free_buffer_pages(void)
{
	return 100000;
}
EXPORT_SYMBOL_GPL(nr_free_buffer_pages);

