//	$Id: fio_list.h 2013-05-23 likunxiang$
//

#ifndef _FIO_LIST_H
#define _FIO_LIST_H

typedef struct fio_list_head {
        struct fio_list_head *next, *prev;
} fio_list_head;

#define FIO_LIST_HEAD_INIT(name) { &(name), &(name) }

#define FIO_LIST_HEAD(name) \
        struct fio_list_head name = FIO_LIST_HEAD_INIT(name)

#define FIO_INIT_LIST_HEAD(ptr) do { \
        (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __fio_list_add(struct fio_list_head * it, struct fio_list_head * prev, struct fio_list_head * next)
{
        next->prev = it;
        it->next = next;
        it->prev = prev;
        prev->next = it;
}

/**
 * fio_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static __inline__ void fio_list_add(struct fio_list_head *it, struct fio_list_head *head)
{
        __fio_list_add(it, head, head->next);
}

/**
 * fio_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static __inline__ void fio_list_add_tail(struct fio_list_head *it, struct fio_list_head *head)
{
        __fio_list_add(it, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __fio_list_del(struct fio_list_head * prev,
                                  struct fio_list_head * next)
{
        next->prev = prev;
        prev->next = next;
}

/**
 * fio_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: fio_list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static __inline__ void fio_list_del(struct fio_list_head *entry)
{
        __fio_list_del(entry->prev, entry->next);
}

/**
 * fio_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static __inline__ void fio_list_del_init(struct fio_list_head *entry)
{
        __fio_list_del(entry->prev, entry->next);
        FIO_INIT_LIST_HEAD(entry); 
}

/**
 * fio_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static __inline__ int fio_list_empty(struct fio_list_head *head)
{
        return head->next == head;
}

static __inline__ void fio_list_assign( struct fio_list_head *dest, struct fio_list_head* src )
{
    *dest = *src;
    if( fio_list_empty( src ) ) {
        FIO_INIT_LIST_HEAD( dest );
    } else {
        src->next->prev = dest;
        src->prev->next = dest;
    } 
}

/**
 * fio_list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static __inline__ void fio_list_splice(struct fio_list_head *list, struct fio_list_head *head)
{
        struct fio_list_head *first = list->next;

        if (first != list) {
                struct fio_list_head *last = list->prev;
                struct fio_list_head *at = head->next;

                first->prev = head;
                head->next = first;

                last->next = at;
                at->prev = last;
        }
}

/**
 * fio_list_splice_tail - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static __inline__ void fio_list_splice_tail(struct fio_list_head *list, struct fio_list_head *head)
{
        if(!fio_list_empty(list) ) {
            struct fio_list_head *first = list->next;
            struct fio_list_head *last = list->prev;
            struct fio_list_head *at = head->prev;
            
            at->next = first;
            first->prev = at;

            last->next = head;
            head->prev = last;
        }
}



/**
 * fio_list_entry - get the struct for this entry
 * @ptr:        the &struct fio_list_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 * 把指针从0改成1，是为了消除警告
 */
#define fio_list_entry(ptr, type, member) \
        ((type *)((char *)(ptr) - ((unsigned long)(&((type*)1)->member) - 1) ))

/**
 * fio_list_for_each        -       iterate over a list
 * @pos:        the &struct fio_list_head to use as a loop counter.
 * @head:       the head for your list.
 */
#define fio_list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)
                
/*
#define fio_list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)
*/
#define fio_list_for_each_safe(pos, head) \
        struct fio_list_head* shit_pos;    \
        for (pos = (head)->next, shit_pos = pos->next; pos != (head); pos = shit_pos, shit_pos = pos->next)
/**
 * fio_list_for_each_prev   -       iterate over a list in reverse order
 * @pos:        the &struct fio_list_head to use as a loop counter.
 * @head:       the head for your list.
 */
#define fio_list_for_each_prev(pos, head) \
        for (pos = (head)->prev; pos != (head); pos = pos->prev )


static __inline__ int fio_list_count(struct fio_list_head *_head)
{
    int count = 0;
    struct fio_list_head *pos;
    fio_list_for_each(pos, _head) 
        count++;
    return count;
}

#endif


