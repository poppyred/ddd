
#ifndef __LIBWNS_HASHLIST_H__
#define __LIBWNS_HASHLIST_H__

#ifdef __KERNEL__
#include <linux/stddef.h>
#else
#include <stddef.h>
#endif

/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1	((void *) 0x00100100)
#define LIST_POISON2	((void *) 0x00200200)

#ifndef INIT_HLIST_HEAD
/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT		{ NULL }
#define HLIST_HEAD(name)	struct hlist_head name = { NULL }
#define INIT_HLIST_HEAD(ptr)		((ptr)->first = NULL)
#define INIT_HLIST_NODE(ptr)		((ptr)->next = NULL, (ptr)->pprev = NULL)

static inline int hlist_unhashed(const struct hlist_node *n)
{
	return !n->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = (struct hlist_node *)LIST_POISON1;
	n->pprev = (struct hlist_node **)LIST_POISON2;
}

static inline void hlist_del_init(struct hlist_node *n)
{
	if (n->pprev)  {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* 将n添加到next前面 */
/* next must be != NULL */
static inline void hlist_add_before(struct hlist_node *n,
					struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

/* 将next添加到n后面 */
static inline void hlist_add_after(struct hlist_node *n,
					struct hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

#define hlist_entry(ptr, type, member)  ((type *)((char *)ptr - offsetof(type,member)))

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos; pos = pos->next)

/**
 * 注意：safe的版本主要是防止pos在循环中被改变，比如内存被释放等等
 */
#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && (n = pos->next, 1); pos = n)


/**
 * hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop counter.
 * @pos:	the &struct hlist_node to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(tpos, pos, head, type, member)	\
	for (pos = (head)->first;	\
		pos && (tpos = hlist_entry(pos, type, member), 1); \
		pos = pos->next)

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after existing point
 * @tpos:	the type * to use as a loop counter.
 * @pos:	the &struct hlist_node to use as a loop counter.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(tpos, pos, type, member)	\
	for (pos = (pos)->next;	\
		pos && (tpos = hlist_entry(pos, type, member), 1);	\
		pos = pos->next)

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from existing point
 * @tpos:	the type * to use as a loop counter.
 * @pos:	the &struct hlist_node to use as a loop counter.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(tpos, pos, type, member)	\
	for (; pos &&	 (tpos = hlist_entry(pos, type, member), 1);	\
		pos = pos->next)

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @tpos:	the type * to use as a loop counter.
 * @pos:	the &struct hlist_node to use as a loop counter.
 * @n:		another &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 * 注意：safe的版本主要是防止pos在循环中被改变，比如内存被释放等等
 */
#define hlist_for_each_entry_safe(tpos, pos, n, head, type, member)	\
	for (pos = (head)->first;	\
		pos && (n = pos->next, 1) &&	\
		(tpos = hlist_entry(pos, type, member), 1);	\
		pos = n)

/**
 * hlist_count - get total entrys number
 * @head: the head of the list
 */
static inline unsigned long hlist_count(const struct hlist_head *head)
{
	const struct hlist_node *p;
	unsigned long cnt = 0;

	hlist_for_each(p, head)
		cnt++;
	return cnt;
}
#endif /* INIT_HLIST_HEAD */


#endif /* __LIBWNS_HASHLIST_H__ */


