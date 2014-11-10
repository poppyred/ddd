//	$Id: log_list.h 2014-04-08 likunxiang $
//

#ifndef _LOG_LIST_H
#define _LOG_LIST_H

typedef struct log_list_head {
        struct log_list_head *next, *prev;
} log_list_head;

#define LOG_LIST_HEAD_INIT(name) { &(name), &(name) }

#define LOG_LIST_HEAD(name) \
        struct log_list_head name = LOG_LIST_HEAD_INIT(name)

#define LOG_INIT_LIST_HEAD(ptr) do { \
        (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __log_list_add(struct log_list_head * it, struct log_list_head * prev, struct log_list_head * next)
{
        next->prev = it;
        it->next = next;
        it->prev = prev;
        prev->next = it;
}

/**
 * log_list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static __inline__ void log_list_add(struct log_list_head *it, struct log_list_head *head)
{
        __log_list_add(it, head, head->next);
}

/**
 * log_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static __inline__ void log_list_add_tail(struct log_list_head *it, struct log_list_head *head)
{
        __log_list_add(it, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __log_list_del(struct log_list_head * prev,
                                  struct log_list_head * next)
{
        next->prev = prev;
        prev->next = next;
}

/**
 * log_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: log_list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static __inline__ void log_list_del(struct log_list_head *entry)
{
        __log_list_del(entry->prev, entry->next);
}

/**
 * log_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static __inline__ void log_list_del_init(struct log_list_head *entry)
{
        __log_list_del(entry->prev, entry->next);
        LOG_INIT_LIST_HEAD(entry); 
}

/**
 * log_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static __inline__ int log_list_empty(struct log_list_head *head)
{
        return head->next == head;
}

static __inline__ void log_list_assign( struct log_list_head *dest, struct log_list_head* src )
{
    *dest = *src;
    if( log_list_empty( src ) ) {
        LOG_INIT_LIST_HEAD( dest );
    } else {
        src->next->prev = dest;
        src->prev->next = dest;
    } 
}

/**
 * log_list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static __inline__ void log_list_splice(struct log_list_head *list, struct log_list_head *head)
{
        struct log_list_head *first = list->next;

        if (first != list) {
                struct log_list_head *last = list->prev;
                struct log_list_head *at = head->next;

                first->prev = head;
                head->next = first;

                last->next = at;
                at->prev = last;
        }
}

/**
 * log_list_splice_tail - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static __inline__ void log_list_splice_tail(struct log_list_head *list, struct log_list_head *head)
{
        if(!log_list_empty(list) ) {
            struct log_list_head *first = list->next;
            struct log_list_head *last = list->prev;
            struct log_list_head *at = head->prev;
            
            at->next = first;
            first->prev = at;

            last->next = head;
            head->prev = last;
        }
}



/**
 * log_list_entry - get the struct for this entry
 * @ptr:        the &struct log_list_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 * 把指针从0改成1，是为了消除警告
 */
#define log_list_entry(ptr, type, member) \
        ((type *)((char *)(ptr) - ((unsigned long)(&((type*)1)->member) - 1) ))

/**
 * log_list_for_each        -       iterate over a list
 * @pos:        the &struct log_list_head to use as a loop counter.
 * @head:       the head for your list.
 */
#define log_list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)
                
/*
#define log_list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)
*/
#define log_list_for_each_safe(pos, head) \
        struct log_list_head* shit_pos;    \
        for (pos = (head)->next, shit_pos = pos->next; pos != (head); pos = shit_pos, shit_pos = pos->next)
/**
 * log_list_for_each_prev   -       iterate over a list in reverse order
 * @pos:        the &struct log_list_head to use as a loop counter.
 * @head:       the head for your list.
 */
#define log_list_for_each_prev(pos, head) \
        for (pos = (head)->prev; pos != (head); pos = pos->prev )


static __inline__ int log_list_count(struct log_list_head *_head)
{
    int count = 0;
    struct log_list_head *pos;
    log_list_for_each(pos, _head) 
        count++;
    return count;
}

#endif


