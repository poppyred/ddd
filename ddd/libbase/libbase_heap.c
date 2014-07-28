/**
 * \file heap.c
 * 堆的实现
 * \author wxc@sangfor.com
 * 2011-11-3
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libbase_error.h"
#include "libbase_memory.h"

#include "libbase_heap.h"

#define DEFAULT_HEAP_SIZE   16

struct heap_st {
    void **bases;
    uint32_t nbase;
    uint32_t __size_base;
    heap_comp_func_t __cmp;
    int32_t __fixed;
};

uint32_t h_heap_count(h_heap_st *hp)
{
    return hp->nbase;
}

void * h_heap_gettop_simple(h_heap_st *hp)
{
    return (hp->nbase > 0 ? (hp)->bases[0] : NULL);
}

void * h_heap_gettop_unsafe(h_heap_st * hp)
{
    return (hp->bases[0]);
}

static intptr_t default_cmp(void *a, void *b)
{
    return (intptr_t)a - (intptr_t)b;
}

h_heap_st *h_heap_create(heap_comp_func_t cmp, uint32_t fixed_size)
{
    h_heap_st *hp = h_malloc(sizeof(h_heap_st));

    if(unlikely(hp == NULL))
        return NULL;

    bzero(hp, sizeof(h_heap_st));

    if (cmp)
        hp->__cmp = cmp;
    else
        hp->__cmp = default_cmp;

    if (fixed_size) {
        hp->__fixed = 1;
        hp->__size_base = fixed_size;

        hp->bases = h_malloc(fixed_size * sizeof(void *));
        if (!hp->bases) {
            h_free(hp);
            return NULL;
        }
    }
    return hp;
}

#define swap_node(a, b) do {            \
        void *tmp = a;                  \
        a = b;                          \
        b = tmp;                        \
    } while (0)

int32_t h_heap_push(h_heap_st *hp, void *data)
{
    uint32_t curr_index;

    if (hp->nbase >= hp->__size_base) {
        if (hp->__fixed)
            return -1;

        hp->__size_base =
            hp->__size_base ?  hp->__size_base * 2 : DEFAULT_HEAP_SIZE;
        hp->bases = h_realloc(hp->bases, hp->__size_base * sizeof(void *));
        if (!hp->bases)
        {
            return -1;
        }
    }

    curr_index = hp->nbase++;
    hp->bases[curr_index] = data;

    while (curr_index != 0) {
        uint32_t parent_index = (curr_index - 1) >> 1;

        if (hp->__cmp(hp->bases[curr_index], hp->bases[parent_index]) >= 0)
            break;

        swap_node(hp->bases[parent_index], hp->bases[curr_index]);
        curr_index = parent_index;
    }
    return 0;
}

int32_t h_heap_pop(h_heap_st *hp, void **ret)
{
    uint32_t curr_index;

    if (hp->nbase == 0)
        return -1;

    if (ret)
        *ret = hp->bases[0];

    --hp->nbase;
    hp->bases[0] = hp->bases[hp->nbase];

    curr_index = 0;
    while (curr_index < hp->nbase) {
        uint32_t c1 = (curr_index << 1) + 1;
        uint32_t c2 = (curr_index << 1) + 2;
        uint32_t child_index;

        if (c1 >= hp->nbase)
            break;

        if (c2 >= hp->nbase) {
            child_index = c1;
        } else {
            child_index = hp->__cmp(hp->bases[c1], hp->bases[c2]) < 0 ? c1 : c2;
        }

        if (hp->__cmp(hp->bases[curr_index], hp->bases[child_index]) < 0) {
            break;
        }

        swap_node(hp->bases[curr_index], hp->bases[child_index]);
        curr_index = child_index;
    }

    return 0;
}

void h_heap_destroy(h_heap_st *hp)
{
    h_free(hp->bases);
    h_free(hp);
}

#ifdef __HEAP_TEST__
int32_t main() {
    h_heap_st *p = h_heap_create(NULL, 0);

    h_heap_push(p, (void *)10);

    h_heap_push(p, (void *)50);
    h_heap_push(p, (void *)20);
    h_heap_push(p, (void *)8);
    h_heap_push(p, (void *)27);
    h_heap_push(p, (void *)45);
    h_heap_push(p, (void *)120);
    h_heap_push(p, (void *)6);

    void *a;
    while ((h_heap_pop(p, &a) == 0)) {
        printf("%d\n", (int32_t)a);
    }

    h_heap_push(p, (void *)45);
    h_heap_push(p, (void *)0);
    h_heap_push(p, (void *)6);

    while ((h_heap_pop(p, &a) == 0)) {
        printf("%d\n", (int32_t)a);
    }

    h_heap_destroy(p);
    return 0;
}

#endif
