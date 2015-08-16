#ifndef _LINUX_TYPES_H_
#define _LINUX_TYPES_H_

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MODE_DEFAULT_DIR (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define MODE_DEFAULT_FILE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

typedef enum {
	false = 0,
	true = 1
} bool;

typedef unsigned int u32;
typedef mode_t umode_t;

typedef struct {
	int counter;
} atomic_t;

struct list_head {
	struct list_head *next, *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#endif /* _LINUX_TYPES_H_ */
