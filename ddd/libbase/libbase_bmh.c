/*!
 * \file libbase_bmh.c
 * \brief 单模式匹配算法
 *
 * \author Adam Xiao (iefcu), iefcuxy@gmail.com
 * \date 10/08/2012 10:50:17 AM
 */

#include <ctype.h>
#include <malloc.h>

#include "libbase_bmh.h"
#include "libbase_memory.h"
#include "libbase_type.h"

#define MAXCHAR 256
typedef struct bmh_handle_inner
{
    int m;
    const unsigned char * patt;
    unsigned short d[MAXCHAR];
} bmh_handle_t;

bmh_handle bmh_init(const unsigned char * patt, int len)
{
    int k;
    bmh_handle_t * bmh_obj = (bmh_handle_t *) h_malloc(sizeof(bmh_handle_t));
    if (bmh_obj == 0)
    {
        return (bmh_handle) 0;
    }

    bmh_obj->patt = patt;
    bmh_obj->m = len;

    for (k = 0; k < MAXCHAR; k++)
    {
        bmh_obj->d[k] = bmh_obj->m;
    }

    for (k = 0; k < bmh_obj->m - 1; k++)
    {
        bmh_obj->d[bmh_obj->patt[k]] = bmh_obj->m - k - 1;
    }
    return (bmh_handle) bmh_obj;
}

void bmh_uninit(bmh_handle bmh)
{
    h_free((void *) bmh);
}

const unsigned char * bmh_chr(bmh_handle bmh,
        const unsigned char * buf, int len)
{
    int i, j, k;
    bmh_handle_t * bmh_obj = (bmh_handle_t *) bmh;

    if (bmh_obj->m > len)
    {
        return 0;
    }

    k = bmh_obj->m - 1;

    while (k < len)
    {
        j = bmh_obj->m - 1;
        i = k;
        while (j >= 0 && buf[i] == bmh_obj->patt[j])
        {
            j--;
            i--;
        }

        if (j == -1)
        {
            return (buf + i + 1);
        }

        k += bmh_obj->d[buf[k]];
    }

    return 0;
}

const unsigned char * bmh_chrlower(bmh_handle bmh,
        const unsigned char * buf, int len)
{
    int i, j, k;
    bmh_handle_t * bmh_obj = (bmh_handle_t *) bmh;

    if (bmh_obj->m > len)
    {
        return 0;
    }

    k = bmh_obj->m - 1;

    while (k < len)
    {
        j = bmh_obj->m - 1;
        i = k;
        while (j >= 0 && ((unsigned char) tolower(buf[i])) == bmh_obj->patt[j])
        {
            j--;
            i--;
        }

        if (j == -1)
        {
            return (buf + i + 1);
        }

        k += bmh_obj->d[(unsigned char) tolower(buf[k])];
    }

    return 0;
}

