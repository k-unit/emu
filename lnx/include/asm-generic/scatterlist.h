#ifndef _ASM_SCATTERLIST_H_
#define _ASM_SCATTERLIST_H_

struct scatterlist {
#ifdef CONFIG_DEBUG_SG
	unsigned long	sg_magic;
#endif
	unsigned long	page_link;
	unsigned int	offset;
	unsigned int	length;
};

#endif

