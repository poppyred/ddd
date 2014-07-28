/*
 * \file rbtree.c
 * 红黑树实现
 * \author wxc(原作者及版权声明见文件尾)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "libbase_memory.h"
#include "libbase_error.h"
#include "libbase_rbtree.h"
#include "libbase_comm.h"
#include "libbase_strutils.h"

/* 节点颜色 */
typedef enum rb_color_t {
    RB_BLACK,
    RB_RED,
} rb_color_t;

/**
 * 红黑树遍历光标结构，使用该光标取得有序的前后元素(类似于iterator概念)
 */
struct h_rb_cursor_st {
    uint32_t    ksize;                  /**< key的长度 */
    void        *data;                  /**< 插入的数据 */
    char        key[0];                 /**< key */
                                        /**< 此结构后续字段调用者不需要知道详情，且结构体不可复制 */
};

/* 节点数据，这个结构将当作rbtree_itor_st返回用于节点遍历 */
typedef struct rbtree_node_st {
    struct rbtree_node_st   *left, *right;
    struct rbtree_node_st   *parent;
    rb_color_t              color;
    h_rb_cursor_st        cs;
} rbtree_node_st;

/* 红黑树控制结构，与哈希表类似，key复制保存而val则使用创建时传入析构函数 */
struct h_rbtree_st {
    rbtree_node_st          *root;
    rbtree_node_st          *first, *last;
    uint32_t                nelem;
    rbtree_cmp_func_t       cmp_fn;
    rbtree_data_free_func_t data_free;
};

/* 一点节点信息宏 */
#define get_color(node)         ((node)->color)
#define set_color(col, node)    ((node)->color = (col))
#define get_parent(node)        ((node)->parent)
#define set_parent(par, node)   ((node)->parent = (par))
#define is_root(node)           (get_parent(node) == NULL)
#define is_black(node)          (get_color(node) == RB_BLACK)
#define is_red(node)            (get_color(node) != RB_BLACK)

/**
 * cursor_node - get the struct for this rbtree node
 * @ptr:    the &struct wns_rb_cursor_st pointer.
 */
#define cursor_node(ptr) \
    ((rbtree_node_st *)((char *)(ptr)-(unsigned long)(&((rbtree_node_st *)0)->cs)))

/* 取得node上的第一个节点 */
static inline rbtree_node_st *get_first(rbtree_node_st *node)
{
    while (node->left)
        node = node->left;
    return node;
}

/* 取得node上的最后一个节点 */
static inline rbtree_node_st *get_last(rbtree_node_st *node)
{
    while (node->right)
        node = node->right;
    return node;
}

/* 取得红黑树第一个节点 */
const h_rb_cursor_st *h_rbtree_first(const h_rbtree_st *tree)
{
    return tree->first == NULL ? NULL : &(tree->first->cs);
}

/* 取得红黑树最后一个节点 */
const h_rb_cursor_st *h_rbtree_last(const h_rbtree_st *tree)
{
    return tree->last == NULL ? NULL : &(tree->last->cs);
}

/* 下一结点 */
static rbtree_node_st *rb_next(const rbtree_node_st *node)
{
    rbtree_node_st *parent;

    if (node->right)
        return get_first(node->right);

    while ((parent = get_parent(node)) && parent->right == node)
        node = parent;

    return parent;
}

/* 取得当前itor的下一结点 */
const h_rb_cursor_st *h_rbtree_next(const h_rb_cursor_st *_itor)
{
    rbtree_node_st *next_node = rb_next(cursor_node(_itor));
    return next_node == NULL ? NULL : &(next_node->cs);
}

/* 上一节点 */
static rbtree_node_st *rb_prev(const rbtree_node_st *node)
{
    rbtree_node_st *parent;

    if (node->left)
        return get_last(node->left);

    while ((parent = get_parent(node)) && parent->left == node)
        node = parent;

    return parent;
}

/* 取得当前itor的上一结点 */
const h_rb_cursor_st *h_rbtree_prev(const h_rb_cursor_st *_itor)
{
    rbtree_node_st *prev_node = rb_prev(cursor_node(_itor));
    return prev_node == NULL ? NULL : &(prev_node->cs);
}

/*
 * 'pparent' and 'is_left' are only used for insertions. Normally GCC
 * will notice this and get rid of them for lookups.
 */
static inline rbtree_node_st *do_lookup(const h_rbtree_st *tree,
                                        const void *key,
                                        uint32_t ksize,
                                        rbtree_node_st **pparent,
                                        int *is_left)
{
    rbtree_node_st *node = tree->root;

    *pparent = NULL;
    *is_left = 0;

    while (node) {
        int res = tree->cmp_fn(node->cs.key, node->cs.ksize, key, ksize);
        if (res == 0)
            return node;

        *pparent = node;
        if ((*is_left = (res > 0)))
            node = node->left;
        else
            node = node->right;
    }
    return NULL;
}

/*
 * Rotate operations (They preserve the binary search tree property,
 * assuming that the keys are unique).
 */
static void rotate_left(h_rbtree_st *tree, rbtree_node_st *node)
{
    rbtree_node_st *p = node;
    rbtree_node_st *q = node->right; /* can't be NULL */
    rbtree_node_st *parent = get_parent(p);

    if (!is_root(p)) {
        if (parent->left == p)
            parent->left = q;
        else
            parent->right = q;
    } else
        tree->root = q;
    set_parent(parent, q);
    set_parent(q, p);

    p->right = q->left;
    if (p->right)
        set_parent(p, p->right);
    q->left = p;
}

static void rotate_right(h_rbtree_st *tree, rbtree_node_st *node)
{
    rbtree_node_st *p = node;
    rbtree_node_st *q = node->left; /* can't be NULL */
    rbtree_node_st *parent = get_parent(p);

    if (!is_root(p)) {
        if (parent->left == p)
            parent->left = q;
        else
            parent->right = q;
    } else
        tree->root = q;
    set_parent(parent, q);
    set_parent(q, p);

    p->left = q->right;
    if (p->left)
        set_parent(p, p->left);
    q->right = p;
}

/* 搜索key，从val中返回结果 */
int h_rbtree_search(const h_rbtree_st *tree, const void *key,
        uint32_t ksize, void **val)
{
    rbtree_node_st *parent;
    int is_left;
    rbtree_node_st *nd = do_lookup(tree, key, ksize, &parent, &is_left);

    if (!nd)
        return -1;
    if (val)
        *val = nd->cs.data;
    return 0;
}

/* 取得key用于遍历 */
const h_rb_cursor_st *h_rbtree_get_cursor(const h_rbtree_st *tree,
        const void *key, uint32_t ksize)
{
    rbtree_node_st *parent;
    int is_left;

    rbtree_node_st *result = do_lookup(tree, key, ksize, &parent, &is_left);
    return result == NULL ? NULL : &(result->cs);
}

/* 取得>=key的元素 */
const h_rb_cursor_st *h_rbtree_lower_bound(const h_rbtree_st *tree,
        const void *key, uint32_t ksize)
{
    rbtree_node_st *parent;
    int is_left;
    rbtree_node_st *ret = do_lookup(tree, key, ksize, &parent, &is_left);
    if (ret)
        return &(ret->cs);

    while (parent
            && tree->cmp_fn(parent->cs.key, parent->cs.ksize, key, ksize) < 0) {
        parent = rb_next(parent);
    }

    return parent == NULL ? NULL : &(parent->cs);
}

/* 取得<=key的元素 */
const h_rb_cursor_st *h_rbtree_upper_bound(const h_rbtree_st *tree,
        const void *key, uint32_t ksize)
{
    rbtree_node_st *parent;
    int is_left;
    rbtree_node_st *ret = do_lookup(tree, key, ksize, &parent, &is_left);
    if (ret)
        return &(ret->cs);

    while (parent
            && tree->cmp_fn(parent->cs.key, parent->cs.ksize, key, ksize) > 0) {
        parent = rb_prev(parent);
    }

    return parent == NULL ? NULL : &(parent->cs);
}

static void set_child(rbtree_node_st *child, rbtree_node_st *node, int left)
{
    if (left)
        node->left = child;
    else
        node->right = child;
}

/* 插入key, value */
int h_rbtree_insert(h_rbtree_st *tree, const void *key,
        uint32_t ksize, void *val)
{
    rbtree_node_st *parent;
    int is_left;
    rbtree_node_st *node = do_lookup(tree, key, ksize, &parent, &is_left);

    if (node) {
        if ((void *)tree->data_free)
            tree->data_free(node->cs.data);

        node->cs.data = val;
        return 0;
    }

    if ((node = h_calloc(1, sizeof(rbtree_node_st) + ksize)) == NULL)
        return -1;

    memcpy_s(node->cs.key, key, ksize);
    node->cs.ksize = ksize;
    node->cs.data = val;

    node->left = NULL;
    node->right = NULL;
    set_color(RB_RED, node);
    set_parent(parent, node);

    if (parent) {
        if (is_left) {
            if (parent == tree->first)
                tree->first = node;
        } else {
            if (parent == tree->last)
                tree->last = node;
        }
        set_child(node, parent, is_left);
    } else {
        tree->root = node;
        tree->first = node;
        tree->last = node;
    }

    /*
     * Fixup the modified tree by recoloring nodes and performing
     * rotations (2 at most) hence the red-black tree properties are
     * preserved.
     */
    while ((parent = get_parent(node)) && is_red(parent)) {
        rbtree_node_st *grandpa = get_parent(parent);

        if (parent == grandpa->left) {
            rbtree_node_st *uncle = grandpa->right;

            if (uncle && is_red(uncle)) {
                set_color(RB_BLACK, parent);
                set_color(RB_BLACK, uncle);
                set_color(RB_RED, grandpa);
                node = grandpa;
            } else {
                if (node == parent->right) {
                    rotate_left(tree, parent);
                    node = parent;
                    parent = get_parent(node);
                }
                set_color(RB_BLACK, parent);
                set_color(RB_RED, grandpa);
                rotate_right(tree, grandpa);
            }
        } else {
            rbtree_node_st *uncle = grandpa->left;

            if (uncle && is_red(uncle)) {
                set_color(RB_BLACK, parent);
                set_color(RB_BLACK, uncle);
                set_color(RB_RED, grandpa);
                node = grandpa;
            } else {
                if (node == parent->left) {
                    rotate_right(tree, parent);
                    node = parent;
                    parent = get_parent(node);
                }
                set_color(RB_BLACK, parent);
                set_color(RB_RED, grandpa);
                rotate_left(tree, grandpa);
            }
        }
    }
    set_color(RB_BLACK, tree->root);
    tree->nelem++;
    return 0;
}

/* 删除key */
int h_rbtree_delete(h_rbtree_st *tree, const void *key, uint32_t ksize)
{
    rbtree_node_st *parent, *left, *right, *next;
    rb_color_t color;
    int is_left;
    rbtree_node_st *oldnode = do_lookup(tree, key, ksize, &parent, &is_left);
    rbtree_node_st *node;

    if (!oldnode)
        return -1;

    node = oldnode;
    left = node->left;
    right = node->right;

    if (node == tree->first)
        tree->first = rb_next(node);
    if (node == tree->last)
        tree->last = rb_prev(node);

    if (!left)
        next = right;
    else if (!right)
        next = left;
    else
        next = get_first(right);

    if (parent)
        set_child(next, parent, parent->left == node);
    else
        tree->root = next;

    if (left && right) {
        color = get_color(next);
        set_color(get_color(node), next);

        next->left = left;
        set_parent(next, left);

        if (next != right) {
            parent = get_parent(next);
            set_parent(get_parent(node), next);

            node = next->right;
            parent->left = node;

            next->right = right;
            set_parent(next, right);
        } else {
            set_parent(parent, next);
            parent = next;
            node = next->right;
        }
    } else {
        color = get_color(node);
        node = next;
    }
    /*
     * 'node' is now the sole successor's child and 'parent' its
     * new parent (since the successor can have been moved).
     */
    if (node)
        set_parent(parent, node);

    /*
     * The 'easy' cases.
     */
    if (color == RB_RED)
        goto end_delete;
    if (node && is_red(node)) {
        set_color(RB_BLACK, node);
        goto end_delete;
    }

    do {
        if (node == tree->root)
            break;

        if (node == parent->left) {
            rbtree_node_st *sibling = parent->right;

            if (is_red(sibling)) {
                set_color(RB_BLACK, sibling);
                set_color(RB_RED, parent);
                rotate_left(tree, parent);
                sibling = parent->right;
            }
            if ((!sibling->left  || is_black(sibling->left)) &&
                (!sibling->right || is_black(sibling->right))) {
                set_color(RB_RED, sibling);
                node = parent;
                parent = get_parent(parent);
                continue;
            }
            if (!sibling->right || is_black(sibling->right)) {
                set_color(RB_BLACK, sibling->left);
                set_color(RB_RED, sibling);
                rotate_right(tree, sibling);
                sibling = parent->right;
            }
            set_color(get_color(parent), sibling);
            set_color(RB_BLACK, parent);
            set_color(RB_BLACK, sibling->right);
            rotate_left(tree, parent);
            node = tree->root;
            break;
        } else {
            rbtree_node_st *sibling = parent->left;

            if (is_red(sibling)) {
                set_color(RB_BLACK, sibling);
                set_color(RB_RED, parent);
                rotate_right(tree, parent);
                sibling = parent->left;
            }
            if ((!sibling->left  || is_black(sibling->left)) &&
                (!sibling->right || is_black(sibling->right))) {
                set_color(RB_RED, sibling);
                node = parent;
                parent = get_parent(parent);
                continue;
            }
            if (!sibling->left || is_black(sibling->left)) {
                set_color(RB_BLACK, sibling->right);
                set_color(RB_RED, sibling);
                rotate_left(tree, sibling);
                sibling = parent->left;
            }
            set_color(get_color(parent), sibling);
            set_color(RB_BLACK, parent);
            set_color(RB_BLACK, sibling->left);
            rotate_right(tree, parent);
            node = tree->root;
            break;
        }
    } while (is_black(node));

    if (node)
        set_color(RB_BLACK, node);

end_delete:
    if ((void *)tree->data_free)
        tree->data_free(oldnode->cs.data);

    h_free(oldnode);
    tree->nelem--;

    return 0;
}

/* 默认的节点比较函数，按内存大小比较 */
static int default_cmp(const void *key1, uint32_t ksize1,
        const void *key2, uint32_t ksize2)
{
    uint32_t kmin = ksize1 < ksize2 ? ksize1 : ksize2;
    int res;

    if ((res = memcmp(key1, key2, kmin)))
        return res;

    return ((int)ksize1 - (int)ksize2);
}

/* 创建 */
h_rbtree_st *h_rbtree_create(rbtree_data_free_func_t data_free,
        rbtree_cmp_func_t cmp)
{
    h_rbtree_st *rb;

    if ((rb= h_calloc(1, sizeof(h_rbtree_st))) == NULL)
        return NULL;

    rb->data_free = data_free;
    rb->cmp_fn = cmp ? cmp : default_cmp;
    return rb;
}

/* 节点递归销毁 */
static void node_destroy(h_rbtree_st *tree, rbtree_node_st *node)
{
    if (node->left)
        node_destroy(tree, node->left);
    if (node->right)
        node_destroy(tree, node->right);

    if ((void *)tree->data_free)
        tree->data_free(node->cs.data);

    h_free(node);
}

/* 销毁 */
void h_rbtree_destroy(h_rbtree_st *tree)
{
    if (tree->root)
        node_destroy(tree, tree->root);

    h_free(tree);
}

/* 取得个数 */
uint32_t h_rbtree_count(h_rbtree_st *rb)
{
    return rb->nelem;
}

/* 有序遍历rbtree */
int h_rbtree_walk(h_rbtree_st *rb, void *data, rbtree_walk_func_t fn)
{
    const h_rb_cursor_st *cs;
    int res;

    if (!(void *)fn)
        return 0;

    for (cs = h_rbtree_first(rb); cs; cs = h_rbtree_next(cs)) {
        if ((res = fn(cs->key, cs->ksize, cs->data, data)))
            return res;
    }

    return 0;
}

/* cursor结构的 内部数据访问 的外部接口 */
inline void *h_rb_cursor_get_key(const h_rb_cursor_st *itor)
{
    if (itor == NULL)
        return NULL;
    return (void*)itor->key;
}
inline uint32_t h_rb_cursor_get_ksize(const h_rb_cursor_st *itor)
{
    if (itor == NULL)
        return 0;
    return itor->ksize;
}
inline void *h_rb_cursor_get_val(const h_rb_cursor_st *itor)
{
    if (itor == NULL)
        return NULL;
    return itor->data;
}

#ifdef RB_TEST


int main(int argc, char **argv)
{
    const char *kv[] = {
        "a", "b", "c", "xxxx", "a", "abc", "test", "asdf", "wqru2u", "alsdf",
        "asjdfoiu", "basdf", "asfw", "wr", "xv", "asxz", "ljlkj", "shit",
        "fuck", "linux", "windows", "bill", "gate", "asxz"
    };

    int i, j;
    h_rbtree_st *rb = h_rbtree_create((rbtree_data_free_func_t)h_free,
            NULL);
    char *str;

    for (j = 0; j < 100; ++j) {
        for (i = 0; i < sizeof(kv) / sizeof(kv[0]); ++i) {
            char buf[100];
            snprintf(buf, sizeof(buf), "%s: %d", kv[i], i);

            h_rbtree_insert(rb, kv[i], strlen(kv[i]) + 1, strdup(buf));
        }
    }

    if (h_rbtree_search(rb, "abc", 4, (void **)&str) == 0) {
        printf("search abc: %s\n", str);
    } else {
        bug("shit");
    }

    if (h_rbtree_search(rb, "asxz", 5, (void **)&str) == 0) {
        printf("search asxz: %s\n", str);
    } else {
        bug("shit");
    }

    h_rbtree_delete(rb, "abc", 4);

    if (h_rbtree_search(rb, "abc", 4, (void **)&str) == 0) {
        bug("fuck");
    }

    printf("all elememts:\n");
    {
        const h_rb_cursor_st *cs;
        for (cs = h_rbtree_first(rb); cs; cs = h_rbtree_next(cs)) {
            printf("%s[%u] %20s\n",
                    (const char *)cs->key, cs->ksize, (const char *)cs->data);
        }
    }

    printf("get cs:\n");
    {
        int i;
        const h_rb_cursor_st *cs = h_rbtree_get_cursor(rb, "basdf", 6);
        if (cs) {
            printf("%s[%u] %20s\n",
                    (const char *)cs->key, cs->ksize, (const char *)cs->data);
        } else {
            bug("shit");
        }
        printf("lower bound: basdf:\n");
        cs = h_rbtree_lower_bound(rb, "basdf", 6);
        if (cs) {
            printf("%s[%u] %20s\n",
                    (const char *)cs->key, cs->ksize, (const char *)cs->data);
        }

        printf("lower bound: basdfx:\n");
        cs = h_rbtree_lower_bound(rb, "basdfx", 7);
        if (cs) {
            printf("%s[%u] %20s\n",
                    (const char *)cs->key, cs->ksize, (const char *)cs->data);
        }

        printf("lower bound: basde:\n");
        cs = h_rbtree_lower_bound(rb, "basde", 6);
        if (cs) {
            printf("%s[%u] %20s\n",
                    (const char *)cs->key, cs->ksize, (const char *)cs->data);
        }

        printf("lower bound: basdg:\n");
        cs = h_rbtree_lower_bound(rb, "basdg", 6);
        if (cs) {
            printf("%s[%u] %20s\n",
                    (const char *)cs->key, cs->ksize, (const char *)cs->data);
        }

        for (i = 1; i < argc; ++i) {
            cs = h_rbtree_lower_bound(rb, argv[i], strlen(argv[i]) + 1);
            if (cs) {
                printf("lower bound: ARG: %s: %s[%u] %20s\n",
                        argv[i], (const char *)cs->key, cs->ksize,
                        (const char *)cs->data);
            } else {
                printf("lower bound: ARG: %s: not found\n", argv[i]);
            }

            cs = h_rbtree_upper_bound(rb, argv[i], strlen(argv[i]) + 1);
            if (cs) {
                printf("upper bound: ARG: %s: %s[%u] %20s\n",
                        argv[i], (const char *)cs->key,
                        cs->ksize, (const char *)cs->data);
            } else {
                printf("upper bound: ARG: %s: not found\n", argv[i]);
            }
        }
    }
    h_rbtree_destroy(rb);
    return 0;
}

#endif

/*
 * rbtree - Implements a red-black tree with parent pointers.
 *
 * Copyright (C) 2010 Franck Bui-Huu <fbuihuu@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 * For recall a red-black tree has the following properties:
 *
 *     1. All nodes are either BLACK or RED
 *     2. Leafs are BLACK
 *     3. A RED node has BLACK children only
 *     4. Path from a node to any leafs has the same number of BLACK nodes.
 *
 */
