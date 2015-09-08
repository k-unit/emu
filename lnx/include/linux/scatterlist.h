#ifndef _SCATTERLIST_H_
#define _SCATTERLIST_H_

#include <linux/types.h>
#include <linux/mm_types.h>

#include <asm-generic/bug.h>
#include <asm-generic/page.h>
#include <asm-generic/scatterlist.h>

#define ARCH_HAS_SG_CHAIN

struct sg_table {
	struct scatterlist *sgl;	/* the list */
	unsigned int nents;		/* number of mapped entries */
	unsigned int orig_nents;	/* original size of list */
};

/*
 * Notes on SG table design.
 *
 * Architectures must provide an unsigned long page_link field in the
 * scatterlist struct. We use that to place the page pointer AND encode
 * information about the sg table as well. The two lower bits are reserved
 * for this information.
 *
 * If bit 0 is set, then the page_link contains a pointer to the next sg
 * table list. Otherwise the next entry is at sg + 1.
 *
 * If bit 1 is set, then this sg entry is the last element in a list.
 *
 * See sg_next().
 *
 */

#define SG_MAGIC	0x87654321

/*
 * We overload the LSB of the page pointer to indicate whether it's
 * a valid sg entry, or whether it points to the start of a new scatterlist.
 * Those low bits are there for everyone! (thanks mason :-)
 */
#define sg_is_chain(sg)		((sg)->page_link & 0x01)
#define sg_is_last(sg)		((sg)->page_link & 0x02)
#define sg_chain_ptr(sg)	\
	((struct scatterlist *) ((sg)->page_link & ~0x03))

/**
 * sg_assign_page - Assign a given page to an SG entry
 * @sg:		    SG entry
 * @page:	    The page
 *
 * Description:
 *   Assign page to sg entry. Also see sg_set_page(), the most commonly used
 *   variant.
 *
 **/
static inline void sg_assign_page(struct scatterlist *sg, struct page *page)
{
	unsigned long page_link = sg->page_link & 0x3;

	/*
	 * In order for the low bit stealing approach to work, pages
	 * must be aligned at a 32-bit boundary as a minimum.
	 */
	BUG_ON((unsigned long) page & 0x03);
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sg->sg_magic != SG_MAGIC);
	BUG_ON(sg_is_chain(sg));
#endif
	sg->page_link = page_link | (unsigned long) page;
}

/**
 * sg_set_page - Set sg entry to point at given page
 * @sg:		 SG entry
 * @page:	 The page
 * @len:	 Length of data
 * @offset:	 Offset into page
 *
 * Description:
 *   Use this function to set an sg entry pointing at a page, never assign
 *   the page directly. We encode sg table information in the lower bits
 *   of the page pointer. See sg_page() for looking up the page belonging
 *   to an sg entry.
 *
 **/
static inline void sg_set_page(struct scatterlist *sg, struct page *page,
			       unsigned int len, unsigned int offset)
{
	sg_assign_page(sg, page);
	sg->offset = offset;
	sg->length = len;
}

static inline struct page *sg_page(struct scatterlist *sg)
{
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sg->sg_magic != SG_MAGIC);
	BUG_ON(sg_is_chain(sg));
#endif
	return (struct page *)((sg)->page_link & ~0x3);
}

/**
 * sg_chain - Chain two sglists together
 * @prv:	First scatterlist
 * @prv_nents:	Number of entries in prv
 * @sgl:	Second scatterlist
 *
 * Description:
 *   Links @prv@ and @sgl@ together, to form a longer scatterlist.
 *
 **/
static inline void sg_chain(struct scatterlist *prv, unsigned int prv_nents,
			    struct scatterlist *sgl)
{
#ifndef ARCH_HAS_SG_CHAIN
	BUG();
#endif

	/*
	 * offset and length are unused for chain entry.  Clear them.
	 */
	prv[prv_nents - 1].offset = 0;
	prv[prv_nents - 1].length = 0;

	/*
	 * Set lowest bit to indicate a link pointer, and make sure to clear
	 * the termination bit if it happens to be set.
	 */
	prv[prv_nents - 1].page_link = ((unsigned long) sgl | 0x01) & ~0x02;
}

/**
 * sg_mark_end - Mark the end of the scatterlist
 * @sg:		 SG entryScatterlist
 *
 * Description:
 *   Marks the passed in sg entry as the termination point for the sg
 *   table. A call to sg_next() on this entry will return NULL.
 *
 **/
static inline void sg_mark_end(struct scatterlist *sg)
{
#ifdef CONFIG_DEBUG_SG
	BUG_ON(sg->sg_magic != SG_MAGIC);
#endif
	/*
	 * Set termination bit, clear potential chain bit
	 */
	sg->page_link |= 0x02;
	sg->page_link &= ~0x01;
}

/**
 * sg_set_buf - Set sg entry to point at given data
 * @sg:		 SG entry
 * @buf:	 Data
 * @buflen:	 Data length
 *
 **/
static inline void sg_set_buf(struct scatterlist *sg, void *buf,
			      unsigned int buflen)
{
#ifdef CONFIG_DEBUG_SG
	BUG_ON(!virt_addr_valid(buf));
#endif
	sg_set_page(sg, virt_to_page(buf), buflen, offset_in_page(buf));
}

/*
 * Loop over each sg element, following the pointer to a new list if necessary
 */
#define for_each_sg(sglist, sg, nr, __i)	\
	for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = sg_next(sg))

struct scatterlist *sg_next(struct scatterlist *sgl);
int sg_nents(struct scatterlist *sg);
struct scatterlist *sg_last(struct scatterlist *sgl, unsigned int nents);
void sg_init_table(struct scatterlist *sgl, unsigned int nents);
void sg_init_one(struct scatterlist *sg, void *buf, unsigned int buflen);

typedef struct scatterlist *(sg_alloc_fn)(unsigned int nents, gfp_t gfp_mask);
typedef void (sg_free_fn)(struct scatterlist *sgl, unsigned int nents);

void __sg_free_table(struct sg_table *table, unsigned int max_ents,
	sg_free_fn *free_fn);
void sg_free_table(struct sg_table *table);
int __sg_alloc_table(struct sg_table *table, unsigned int nents,
	unsigned int max_ents, gfp_t gfp_mask, sg_alloc_fn *alloc_fn);
int sg_alloc_table(struct sg_table *table, unsigned int nents, gfp_t gfp_mask);

/*
 * Maximum number of entries that will be allocated in one piece, if
 * a list larger than this is required then chaining will be utilized.
 */
#define SG_MAX_SINGLE_ALLOC		(PAGE_SIZE / sizeof(struct scatterlist))

#endif

