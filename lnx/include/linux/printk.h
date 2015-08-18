#ifndef _PRINTK_H_  
#define _PRINTK_H_

#include <stdio.h>

#define pr_kernel(type, str, ...) printf("pr_" type ": " str, ##__VA_ARGS__)
#define pr_emerg(str, ...) pr_kernel("emerg", str, ##__VA_ARGS__)
#define pr_alert(str, ...) pr_kernel("alert", str, ##__VA_ARGS__)
#define pr_crit(str, ...) pr_kernel("crit", str, ##__VA_ARGS__)
#define pr_err(str, ...) pr_kernel("err", str, ##__VA_ARGS__)
#define	pr_warn(str, ...) pr_kernel("warn", str, ##__VA_ARGS__)
#define pr_notice(str, ...) pr_kernel("notice", str, ##__VA_ARGS__)
#define pr_info(str, ...) pr_kernel("info", str, ##__VA_ARGS__)
#ifdef KUT_CONFIG_DEBUG
#define pr_debug(str, ...) pr_kernel("debug", str, ##__VA_ARGS__)
#else
#define pr_debug(str, ...) do {} while(0)
#endif

#endif

