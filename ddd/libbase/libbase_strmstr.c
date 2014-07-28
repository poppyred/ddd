/*
 * \file strmstr.c
 * implement multi-strings search, with ac-similar alg
 * \note
 * 1. with less then 1000 element, using EXTEND(default option), will create a
 *    DFA, this will make string match the best performance
 * 2. with more then 1000(or less) element, compiling DFA take times, so using
 *    NO_EXTEND option would be recommended
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "libbase_memory.h"
#include "libbase_error.h"
#include "libbase_strmstr.h"

#define mstr_tolower(c) ((c) >= 'A' && (c) <= 'Z' ? (c) + ('a' - 'A') : (c))

#define bit_set(intp, pos) do {                     \
    (intp)[(pos) >> 5] |= (1 << ((pos) & 0x1f));    \
} while(0)

#define bit_get(intp, pos) (((intp)[(pos) >> 5]) & (1 << ((pos) & 0x1f)))

struct mstr_node_st {
    unsigned int id;
    unsigned int isaccept;
    void *accept_sid;
    struct mstr_node_st *fail;      /* use on no extend */
    uint32_t link_map[8];
    struct mstr_node_st *child[256];
};

typedef struct mstr_node_st mstr_node_st;

#ifdef UNITED_MSTR
/* united nodes make state access more efficient(less CPU cache miss) */
typedef struct united_node_st {
    void *accept_sid;
    unsigned int isaccept;
    uint16_t id;
    uint16_t child[256];
} united_node_st;
#endif

struct mstr_ctrl_st {
    int ignore_case;
    int no_extend;

    int compiled;

    unsigned int nstate;
    unsigned int __statesize;
    mstr_node_st **states;

#ifdef UNITED_MSTR
    united_node_st *unites;
#endif
};

/* allocate node from ctrl, save the entry in mc->states */
static mstr_node_st *mstr_node_alloc(mstr_ctrl_st *mc)
{
    unsigned int node_id;

    if (mc->nstate >= mc->__statesize) {
        mc->__statesize = mc->__statesize ? mc->__statesize * 2 : 128;
        mc->states = h_realloc(mc->states,
                mc->__statesize * sizeof(mstr_node_st *));
        if (!mc->states)
            return NULL;
    }

    node_id = mc->nstate++;
    mc->states[node_id] = h_calloc(sizeof(mstr_node_st), 1);
    if (!mc->states[node_id]) {
        return NULL;
    }
    mc->states[node_id]->id = node_id;

    return mc->states[node_id];
}

/* creating mstr_ctrl_st */
mstr_ctrl_st *mstr_ctrl_create(unsigned int option)
{
    mstr_ctrl_st *r = h_calloc(sizeof(mstr_ctrl_st), 1);
    if (r == NULL) {
        return NULL;
    }

    r->ignore_case = option & MSTR_OPT_IGNORE_CASE ? 1 : 0;
    r->no_extend = option & MSTR_OPT_NO_EXTEND ? 1 : 0;

    /* create states[0], ignore retval */
    if (NULL == mstr_node_alloc(r)) {
        return NULL;
    }

    return r;
}

/* adding pattern to ctrl, for each char, create a TRIE tree node */
int mstr_ctrl_add_pattern(mstr_ctrl_st *mc,
        const unsigned char *str, unsigned int len,
        void *sid)
{
    mstr_node_st *cur;
    unsigned int oldlen = len;

    if (mc->compiled)
        return -1;

    cur = mc->states[0];
    while (len) {
        unsigned int idx = mc->ignore_case ? mstr_tolower(*str) : *str;

        if (!cur->child[idx])
            cur->child[idx] = mstr_node_alloc(mc);

        cur = cur->child[idx];

        if (len == 1) {     /* last char, add a terminal sign */
            if (cur->isaccept)
                return -2;

            cur->isaccept = oldlen;
            cur->accept_sid = sid;
        }
        str++;
        len--;
    }

    return 0;
}

/* make node be the mirror of from */
static void do_link_node_extend(mstr_node_st *node, mstr_node_st *from)
{
    int i;

    if (node == from)
        return;

    for (i = 0; i < 256; ++i) {
        if (!from->child[i])
            continue;

        /* no same child found, link it to mirror's same position
           (shared the same subtree) */

        if (!node->child[i]) {
            node->child[i] = from->child[i];
            bit_set(node->link_map, i);
            continue;
        }

        /* same children exist, the children should be mirror
           to each other */

        if (node->child[i] != from->child[i]) {
            do_link_node_extend(node->child[i], from->child[i]);
        }
    }
}

/* make the node be the mirror of root */
static void link_node_extend(mstr_node_st *node, mstr_node_st *root)
{
    int i;

    if (node != root)
        do_link_node_extend(node, root);

    for (i = 0; i < 256; ++i) {
        if (!bit_get(node->link_map, i) && node->child[i]) {
            link_node_extend(node->child[i], root);
        }
    }
}

#ifdef UNITED_MSTR
/* make united child for efficient memory access(pray) */
static void mstr_make_united(mstr_ctrl_st *mc)
{
    unsigned int i;

    if (mc->no_extend || !mc->compiled || mc->nstate >= 65536)
        return;

    mc->unites = h_calloc(mc->nstate * sizeof(united_node_st));
    if (!mc->unites) {
        return ;
    }
    for (i = 0; i < mc->nstate; ++i) {
        int j;
        mc->unites[i].id = (uint16_t)mc->states[i]->id;
        mc->unites[i].accept_sid = mc->states[i]->accept_sid;
        mc->unites[i].isaccept = mc->states[i]->isaccept;

        for (j = 0; j < 256; ++j) {
            mc->unites[i].child[j] = (uint16_t)mc->states[i]->child[j]->id;
        }
    }
}
#endif

/* compile the mc, this may take a while if the pattern is in huge amount */
static void mstr_ctrl_compile_extend(mstr_ctrl_st *mc)
{
    unsigned int i, n;

    if (mc->compiled)
        return;

    link_node_extend(mc->states[0], mc->states[0]);

    /* for each empty child, link it to root node */
    for (n = 0; n < mc->nstate; ++n) {
        mstr_node_st *node = mc->states[n];
        for (i = 0; i < 256; ++i) {
            if (!node->child[i]) {
                node->child[i] = mc->states[0];
            }
        }
    }
    mc->compiled = 1;

#ifdef UNITED_MSTR
    mstr_make_united(mc);
#endif
}

/* link node's fail to from node, and apply in every children */
static void do_link_node_no_extend(mstr_node_st *node, mstr_node_st *from)
{
    unsigned int i;

    if (!node->fail)
        node->fail = from;

    for (i = 0; i < 256; ++i) {
        if (node->child[i] && from->child[i])
            do_link_node_no_extend(node->child[i], from->child[i]);
    }
}

static void link_node_no_extend(mstr_node_st *node, mstr_node_st *root)
{
    unsigned int i;

    if (node != root)
        do_link_node_no_extend(node, root);

    for (i = 0; i < 256; ++i) {
        if (node->child[i])
            link_node_no_extend(node->child[i], root);
    }
}

/* compile with no extend */
static void mstr_ctrl_compile_no_extend(mstr_ctrl_st *mc)
{
    unsigned int n;

    if (mc->compiled)
        return;

    link_node_no_extend(mc->states[0], mc->states[0]);

    for (n = 0; n < mc->nstate; ++n) {
        mstr_node_st *node = mc->states[n];
        if (!node->fail)
            node->fail = mc->states[0];
    }
    mc->compiled = 1;
}

/* compile */
void mstr_ctrl_compile(mstr_ctrl_st *mc)
{
    if (mc->no_extend)
        mstr_ctrl_compile_no_extend(mc);
    else
        mstr_ctrl_compile_extend(mc);
}

/* destroy ctrl */
void mstr_ctrl_destroy(mstr_ctrl_st *mc)
{
    unsigned int n;

#ifdef UNITED_MSTR
    h_free(mc->unites);
#endif

    for (n = 0; n < mc->nstate; ++n)
        h_free(mc->states[n]);

    h_free(mc->states);
    h_free(mc);
}

#ifdef DEBUG_STRMSTR
/* debug ctrl */
void mstr_ctrl_debug(mstr_ctrl_st *mc)
{
    unsigned int i, n;
    for (n = 0; n < mc->nstate; ++n) {
        mstr_node_st *node = mc->states[n];
        printf("state %3d, %p%s: \n",
                n, node->accept_sid, node->isaccept ? "(ACCEPT)" : "");
        if (mc->no_extend && node->fail) {
            printf("fail to %d\n", node->fail->id);
        }
        for (i = 0; i < 256; ++i) {
            if (node->child[i])
                printf("%3d ", node->child[i]->id);
            else
                printf("  - ");
        }
        printf("\n");
    }
}
#endif

/* reset state */
void mstr_state_reset(mstr_state_st *ms)
{
    *ms = 0;
}

/* search all string in mc, extended, use dfa jump */
static int mstr_search_extend(const mstr_ctrl_st *mc, mstr_state_st *ms,
        const unsigned char *src, unsigned int len,
        void *userdata,
        mstr_func_t callback)
{
    const mstr_node_st *curr = mc->states[0];
    const unsigned char *oldsrc = src;
    int ignore_case, r;

    if (!mc->compiled)
        return 0;

    if (ms && *ms < mc->nstate)
        curr = mc->states[*ms];

    ignore_case = mc->ignore_case;

    while (len) {
        curr = curr->child[ignore_case ? mstr_tolower(*src) : *src];

        src++;
        len--;

        if (!curr->isaccept)
            continue;

        /* accept state found */
        if ((void *)callback) {
            const unsigned char *pos = src - curr->isaccept;
            if ((r = callback(curr->accept_sid, pos - oldsrc, userdata))) {
                if (ms)
                    *ms = curr->id;
                return r;
            }
        } else {
            if (ms)
                *ms = curr->id;
            return 1;
        }
    }

    if (ms)
        *ms = curr->id;
    return 0;
}

#ifdef UNITED_MSTR
/* search all string in mc, extended, use dfa jump, with united nodes */
static int mstr_search_unites(const mstr_ctrl_st *mc, mstr_state_st *ms,
        const unsigned char *src, unsigned int len,
        void *userdata,
        mstr_func_t callback)
{
    uint16_t curr = 0;
    const unsigned char *oldsrc = src;
    int r;
    register int ignore_case;
    register united_node_st *unites = mc->unites;

    if (!mc->compiled)
        return 0;

    if (ms && *ms < mc->nstate)
        curr = (uint16_t)*ms;

    ignore_case = mc->ignore_case;

    while (len) {
        curr = unites[curr].child[ignore_case ? mstr_tolower(*src) : *src];

        src++;
        len--;

        if (!unites[curr].isaccept)
            continue;

        /* accept state found */
        if ((void *)callback) {
            const unsigned char *pos = src - unites[curr].isaccept;
            r = callback(unites[curr].accept_sid, pos - oldsrc, userdata);
            if (r) {
                if (ms)
                    *ms = unites[curr].id;
                return r;
            }
        } else {
            if (ms)
                *ms = unites[curr].id;
            return 1;
        }
    }

    if (ms)
        *ms = unites[curr].id;
    return 0;
}
#endif

static int mstr_search_no_extend(const mstr_ctrl_st *mc, mstr_state_st *ms,
        const unsigned char *src, unsigned int len,
        void *userdata,
        mstr_func_t callback)
{
    const mstr_node_st *curr = mc->states[0];
    const mstr_node_st *root = mc->states[0];
    const unsigned char *oldsrc = src;
    int ignore_case, r;

    if (!mc->compiled)
        return 0;

    if (ms && *ms < mc->nstate)
        curr = mc->states[*ms];

    ignore_case = mc->ignore_case;

    while (len) {
        unsigned int chr = ignore_case ? mstr_tolower(*src) : *src;

        /* shift to fail, till root reach */
        while (!curr->child[chr] && curr != root)
            curr = curr->fail;

        /* last try, thift to root if no char match */
        curr = curr->child[chr];
        if (!curr)
            curr = root;

        src++;
        len--;

        if (!curr->isaccept)
            continue;

        /* accept state found */
        if ((void *)callback) {
            const unsigned char *pos = src - curr->isaccept;
            if ((r = callback(curr->accept_sid, pos - oldsrc, userdata))) {
                if (ms)
                    *ms = curr->id;
                return r;
            }
        } else {
            if (ms)
                *ms = curr->id;
            return 1;
        }
    }

    if (ms)
        *ms = curr->id;

    return 0;
}

int mstr_search(const mstr_ctrl_st *mc, mstr_state_st *ms,
                const unsigned char *src, unsigned int len,
                void *userdata,
                mstr_func_t callback)
{
    if (mc->no_extend)
        return mstr_search_no_extend(mc, ms, src, len, userdata, callback);

#ifdef UNITED_MSTR
    if (mc->unites)
        return mstr_search_unites(mc, ms, src, len, userdata, callback);
#endif

    return mstr_search_extend(mc, ms, src, len, userdata, callback);
}

#ifdef MSTR_COMPILER
unsigned int translate(const unsigned char *in, unsigned int ilen,
        unsigned char *out, unsigned int olen)
{
    unsigned int ocur = 0;
    unsigned int i;
    for (i = 0; i < ilen && ocur < olen; ++i) {
        if (in[i] == '\\' && i < ilen - 1) {
            i++;
            switch (in[i]) {
            case 'n':
                out[ocur++] = '\n';
                break;
            case 'r':
                out[ocur++] = '\r';
                break;
            case 'v':
                out[ocur++] = '\v';
                break;
            case 't':
                out[ocur++] = '\t';
                break;
            default:
                out[ocur++] = '\\';
                if (ocur < olen)
                    out[ocur++] = in[i];
                break;
            }
        } else {
            out[ocur++] = in[i];
        }
    }
    return ocur;
}

int main(int argc, char **argv)
{
    mstr_ctrl_st *ctrl = mstr_ctrl_create(MSTR_OPT_IGNORE_CASE);
    int i;

    for (i = 1; i < argc; ++i) {
        unsigned char buf[1024];
        unsigned int n = translate(argv[i], strlen(argv[i]), buf, sizeof(buf));

        if (!n)
            wns_die("can not translate: %s\n", argv[i]);

        if (mstr_ctrl_add_pattern(ctrl, buf, n, (void *)i)) {
            wns_die("can not add pattern: %s\n", argv[i]);
        }
    }

    mstr_ctrl_compile(ctrl);

    for (i = 0; i < ctrl->nstate; ++i) {
        mstr_node_st *state = ctrl->states[i];
        int j;

        printf("%d:%d:%d:", state->id, state->isaccept, (int)state->accept_sid);
        printf("%d", state->child[0]->id);
        for (j = 1; j < 256; ++j) {
            printf(" %d", state->child[j]->id);
        }
        printf("\n");
    }

    mstr_ctrl_destroy(ctrl);
    return 0;
}

#endif
