#include <stdio.h>
#include <stdlib.h>

#include "libbase_binsearch.h"

/* 取得数组元素 */
#define ELEM_PTR(base, n, esize) ((void *)(((int8_t *)base) + n * esize))

/* 二分法查找 */
int32_t h_binsearch(void *base, uint32_t nelem, uint32_t esize, void *key,
        h_binsearch_cmp_func_t compar)
{
    int32_t low = -1, high = nelem;

    while (high - low > 1) {
        int32_t mid = (int32_t)(((uint32_t)(high + low)) >> 1);
        if (compar(ELEM_PTR(base, mid, esize), key) > 0) {
            high = mid;
        } else {
            low = mid;
        }
    }

    if (low >= 0 && compar(ELEM_PTR(base, low, esize), key) == 0)
        return low;

    return -1;
}

/* 取得最接近key并且小于key的元素 */
int32_t h_binsearch_lower_bound(void *base, uint32_t nelem, uint32_t esize,
        void *key, h_binsearch_cmp_func_t compar)
{
    int32_t low = 0, high = nelem - 1;
    int32_t target = 0;

    if (compar(ELEM_PTR(base, low, esize), key) > 0)
        return -1;                                  /* 比第一个元素小 */

    if (compar(ELEM_PTR(base, high, esize), key) < 0)
        return high;                                /* 比最后一个元素大，返回最后一个元素 */

    while (low <= high) {
        int32_t mid = (int32_t)(((unsigned int)(high + low)) >> 1);
        void *mid_elem = ELEM_PTR(base, mid, esize);
        int32_t compv = compar(mid_elem, key);

        if (compv < 0) {
            target = mid;
            low = mid + 1;
        } else if (compv > 0) {
            high = mid - 1;
        } else {
            return mid;
        }
    }

    return target;
}
