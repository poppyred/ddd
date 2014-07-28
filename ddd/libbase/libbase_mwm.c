#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "libbase_mwm.h"
#include "libbase_memory.h"
#include "libbase_type.h"

#ifdef DEBUG
#define debug printf
#else
#define debug(X...)
#endif

#define inline

#define MID_PAT_LEN (32000)

//#define MWM_DEBUG
/*
 *   This macro enables use of the bitop test.
 */

//#define COPY_PATTERNS
/*
 ** Enables display of pattern group stats
 */
//#define SHOW_STATS
#define MWM_FEATURES "MWM:BC/BW-SHIFT + 2-BYTE-HASH"

#define MWM_TYPE_CHAR 0
#define MWM_TYPE_WORD 1

#define CHAR_TABLE_SIZE 256
#define WORD_TABLE_SIZE 256*256

/*
 ** Causes mbmAddPattern to check for and not allow duplicate patterns.
 ** By default we allow multiple duplicate patterns, since the AND clause
 ** may case the whole signature to be different. We trigger each pattern
 ** to be processesed by default.
 */

#define REQUIRE_UNIQUE_PATTERNS 0

/*
 *  Pattern Matching Methods - Boyer-Moore-Horspool or Modified Wu Manber
 */
#define MTH_MWM 0
#define MTH_BM  1

/*
 *
 *  Boyer-Moore-Horsepool for small pattern groups
 *
 */
typedef struct
{
    short bcShift[256];
    int M;
    u_char * P;
} HBM_STRUCT;

/*
 **  This struct is used internally my mwm.c
 */
typedef struct _apm_mwm_pattern_struct
{

    unsigned int psLen; // length of pattern in bytes
    u_char * psPat; // pattern array, no case
    int psOffset; // pos where to start searching in data
    int psDepth; // number of bytes after offset to search
    intptr_t ps_data; //internal ID, used by the pattern matcher

    HBM_STRUCT * psBmh;
    struct _apm_mwm_pattern_struct * next;
} APM_MWM_PATTERN_STRUCT;

/*
 ** Pattern GROUP Structure, this struct is is used publicly, but by reference only
 */
typedef struct _apm_mwm_struct
{

    /* Wu-Manber Hash Tables */
    /* Bad Word Shift Table */
    u_char * msHash_char; /* adam - 哈希每个模式的首个字符 */
    u_char * msShift_char;
    u_char * msNumArray_char; /* adam - 记录跟自己为相同首个字符的连续模式数量 */

    u_short * msHash_word; // 2+ character Pattern Big Hash Table
    u_char * msShift_word;
    u_short * msNumArray_word;

    unsigned msShiftLen;

    /*前缀匹配规则,最长匹配与最短匹配*/
    u_char msPrefixMatch;

    /* search function */
    int (*search)(mwm_handle pv,
            u_char * T, int n,
            void * in, void * out,
            action_proc match);

    /* Array of Group Counts, # of patterns in each hash group */
    APM_MWM_PATTERN_STRUCT * msPatArray; /* adam - 所有模式数组 */

    /* Number of Patterns loaded */
    int msNumPatterns;

    int msNumHashEntries;
    int msLargeShifts;

    /* Print Group Details */
    int msDetails;

    /* Pattern Group Stats  */
    int msSmallest;
    int msLargest;
    int msAvg;
    int msTotal;

    int msMethod; /* MTH_BM, MTH_MWM */
    int msMwmType;
    int msHashBytes;

    APM_MWM_PATTERN_STRUCT * plist;
} APM_MWM_STRUCT;

static inline APM_MWM_PATTERN_STRUCT * mwm_pt_st_alloc(int m)
{
    APM_MWM_PATTERN_STRUCT * p;
    p = (APM_MWM_PATTERN_STRUCT *) h_malloc(sizeof(APM_MWM_PATTERN_STRUCT));
    if (p == NULL)
    {
        goto __err;
    }
    memset(p, 0, sizeof(APM_MWM_PATTERN_STRUCT));

    /* Allocate and store the Pattern 'P' with NO CASE info*/
    p->psPat = (u_char *) h_malloc(m + 1);
    if (!p->psPat)
    {
        goto __err;
    }
    memset(p->psPat, 0, m + 1);

    return p;

__err:
    if (p != NULL)
    {
        h_free(p);
    }
    return NULL;

}

static inline void mwm_pt_st_free(APM_MWM_PATTERN_STRUCT * p)
{

    if (p == NULL)
    {
        return;
    }

    if (p->psPat)
    {
        h_free(p->psPat);
        p->psPat = NULL;
    }

    if (p->psBmh)
    {
        h_free(p->psBmh);
        p->psBmh = NULL;
    }

    h_free(p);
}

/*
 ** mwmAlloc:: Allocate and Init Big hash Table Verions
 ** 创建一个新的WM结构
 ** maxpats - max number of patterns to support
 **
 */
mwm_handle apm_mwmNew()
{
    APM_MWM_STRUCT * p =
        (APM_MWM_STRUCT *) h_malloc(sizeof(APM_MWM_STRUCT));
    if (!p)
    {
        debug("mwm: kmem create failed!\n");
        return 0;
    }
    memset(p, 0, sizeof(APM_MWM_STRUCT));

    p->msSmallest = MID_PAT_LEN;

    return (mwm_handle) p;
}

/*
 ** Release a mwm
 */
void apm_mwmFree(mwm_handle pv)
{
    APM_MWM_STRUCT * p = (APM_MWM_STRUCT *) pv;
    APM_MWM_PATTERN_STRUCT * list = 0;
    APM_MWM_PATTERN_STRUCT * next = 0;

    if (!p)
    {
        debug("mwm: APM_MWM_STRUCT is null!\n");
        return;
    }

    if (p->msPatArray)
    {
        int k = 0;
        for (k = 0; k < p->msNumPatterns; k++)
        {
            if (p->msPatArray[k].psBmh)
            {
                h_free(p->msPatArray[k].psBmh);
                p->msPatArray[k].psBmh = NULL;
            }
        }
        h_free(p->msPatArray);
    }
    if (p->msNumArray_char)
    {
        h_free(p->msNumArray_char);
    }
    if (p->msNumArray_word)
    {
        h_free(p->msNumArray_word);
    }
    if (p->msHash_char)
    {
        h_free(p->msHash_char);
    }
    if (p->msShift_char)
    {
        h_free(p->msShift_char);
    }
    if (p->msHash_word)
    {
        h_free(p->msHash_word);
    }
    if (p->msShift_word)
    {
        h_free(p->msShift_word);
    }

    p->msPatArray = NULL;
    p->msNumArray_char = NULL;
    p->msNumArray_word = NULL;
    p->msHash_char = NULL;
    p->msShift_char = NULL;
    p->msHash_word = NULL;
    p->msShift_word = NULL;

    debug("mwm: free all patterns in list.\n");
    list = p->plist;
    while (list != NULL)
    {
        next = list->next;
        mwm_pt_st_free(list);
        list = next;
    }
    p->plist = NULL;
    h_free(p);

    debug("mwm: free res finished.\n");
}

void apm_mwmSetPrefixMatch(mwm_handle pv, u_char method)
{
    APM_MWM_STRUCT * p = (APM_MWM_STRUCT *) pv;
    p->msPrefixMatch = method;
}

/*
 **
 ** returns -1: max patterns exceeded
 ** 0: already present, uniqueness compiled in
 ** 1: added
 */
int apm_mwmAddPatternEx(mwm_handle pv, const u_char * P, int len, int offset,
        int deep, intptr_t ud)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    APM_MWM_PATTERN_STRUCT * plist = 0;
    APM_MWM_PATTERN_STRUCT * p = NULL;

    // debug("mwmAddPatternEx:pv%p P%p m:%d\n",pv,P,m);
    if (!P || len < 2)
    {
        debug("mwm: error patter empty or length < 2.\n ");
        return -2; /* Empty Pat String Or Length < Word*/
    }

    if (ps->msNumPatterns > WORD_TABLE_SIZE - 1)
    {
        debug("mwm: error max pattern number %d %d limited!\n",
                ps->msNumPatterns, WORD_TABLE_SIZE);
        return -1;
    }

    if (ps->msNumPatterns > CHAR_TABLE_SIZE - 1)
    {
        ps->msMwmType = MWM_TYPE_WORD;
    }

    p = (APM_MWM_PATTERN_STRUCT *) mwm_pt_st_alloc(len);
    if (!p)
    {
        debug("mwm: error alloc mem for pattern!\n");
        return -1;
    }

#if REQUIRE_UNIQUE_PATTERNS
    /* 消除重复模式串 */
    for (plist = ps->plist; plist != NULL; plist = plist->next)
    {
        if (plist->psLen == (unsigned)m)
        {
            if (memcmp(P, plist->psPat, m) == 0)
            {
                return 0; /*already added */
            }
        }
    }
#endif //REQ UNIQ PATS
    if (ps->plist)
    {
        for (plist = ps->plist; plist->next != NULL; plist = plist->next) {
            // do nothing
        }
        plist->next = p;
    }
    else
    {
        ps->plist = p;
    }

    memcpy(p->psPat, P, len);

    p->psLen = len;
    p->psOffset = offset;
    p->psDepth = deep;
    p->ps_data = ud;

    ps->msNumPatterns++;

    if (p->psLen < (unsigned) ps->msSmallest)
    {
        ps->msSmallest = p->psLen;
    }
    if (p->psLen > (unsigned) ps->msLargest)
    {
        ps->msLargest = p->psLen;
    }

    ps->msTotal += p->psLen;
    ps->msAvg = ps->msTotal / ps->msNumPatterns;

    debug("mwm: add pattern len:%d, pt:%p, ud:%lx, '%s' finished.\n",
            p->psLen, p, p->ps_data, p->psPat);
    return 1;
}

/*
 ** 计算出不同长度的模式串个数
 ** Calc some pattern length stats
 */
static void mwmAnalyzePattens(mwm_handle pv)
{
}

/*
 **
 ** Build the hash table, and pattern groups
 *  modified by adam - 2012-10-26 大改, 清晰简化代码逻辑
 */
static void mwmPrepHashedPatternGroupsC(mwm_handle pv)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    unsigned first_char = 0; // 每个模式的首个字符, 用来作为哈希索引
    unsigned ningroup = 0; // 相同首字符的连续模式的数量

    // Init Hash table to default value - `0xFF`
    memset(ps->msHash_char, 0xff, CHAR_TABLE_SIZE);

    /*
     ** Add the patterns to the hash table
     ** (只索引前一个字节, 哈希值相同
     ** ---前缀相同, 放在同一个组里面)
     */
    int i = 0, j;
    while (i < ps->msNumPatterns) {
        // 索引每个模式的第一个字符
        first_char = ps->msPatArray[i].psPat[0];
        ps->msHash_char[first_char] = i;

        // 从下一个模式开始, 计算相同首字符的连续模式的数量
        ningroup = 1;
        for (j = i + 1;
                j < ps->msNumPatterns
                    && first_char == ps->msPatArray[j].psPat[0];
                j++) {
            ningroup++;
        }
        ps->msNumArray_char[i] = ningroup;

        // 更新循环变量, 从没有匹配到首字符的模式开始
        i = j;
    }
}

/*
 **
 ** Build the hash table, and pattern groups
 */
/*
 ** HASH ROUTINE - used during pattern setup, but inline during searches
 */
static unsigned HASH16(u_char * T)
{
    return (unsigned short)(((*T) << 8) | *(T + 1));
}

/*
 **
 ** Build the hash table, and pattern groups
 *  modified by adam - 2012-10-26 大改, 清晰简化代码逻辑
 */
static void mwmPrepHashedPatternGroupsW(mwm_handle pv)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    unsigned first2_char = 0; // 每个模式的首2个字符, 用来作为哈希索引
    unsigned ningroup = 0; // 相同首2个字符的连续模式的数量

    /*
     ** Mem has Allocated and here Init 2+ byte pattern hash table (256 * 256)
     ** Init Hash table to default value
     */
    memset(ps->msHash_word, 0xff, sizeof(u_short) * WORD_TABLE_SIZE);

    /*
     ** Add the patterns to the hash table
     ** (只索引前两个字节, 哈希值相同
     ** ---前缀相同, 放在同一个组里面)
     */
    int i = 0, j;
    while (i < ps->msNumPatterns) {
        // 索引每个模式的首2个个字符
        first2_char = HASH16(ps->msPatArray[i].psPat); /* 索引前两个字节 */
        ps->msHash_word[first2_char] = i; /* 记录该组的起始位置 */

        // 从下一个模式开始, 计算相同首字符的连续模式的数量
        ningroup = 1;
        for (j = i + 1;
                j < ps->msNumPatterns
                && first2_char == HASH16(ps->msPatArray[i].psPat);
                j++) {
            ningroup++;
        }
        ps->msNumArray_word[i] = ningroup; /* 记录该组pats 总数 */

        // 更新循环变量, 从没有匹配到首字符的模式开始
        i = j;
    }
}

/*
 * 以字节为索引的Shift Table
 * Standard Bad Character Multi-Pattern Skip Table
 */
static void mwmPrepBadCharTable(mwm_handle pv)
{
    unsigned short i, k, m, cindex, shift;
    unsigned small_value = MID_PAT_LEN, large_value = 0;
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;

    /* Determine largest and smallest pattern sizes */
    for (i = 0; i < ps->msNumPatterns; i++)
    {
        if (ps->msPatArray[i].psLen < small_value)
        {
            small_value = ps->msPatArray[i].psLen;
        }
        if (ps->msPatArray[i].psLen > large_value)
        {
            large_value = ps->msPatArray[i].psLen;
        }
    }

    m = (unsigned short) small_value;

    if (m > 255)
    {
        m = 255;
    }

    ps->msShiftLen = m;

    /* Initialze the default shift table. Max shift of 256 characters */
    for (i = 0; i < CHAR_TABLE_SIZE; i++)
    {
        ps->msShift_char[i] = (u_char) m;
    }

    /* Multi-Pattern BAD CHARACTER SHIFT */
    for (i = 0; i < ps->msNumPatterns; i++)
    {
        for (k = 0; k < m; k++)
        {
            shift = (unsigned short)(m - 1 - k);

            if (shift > 255)
            {
                shift = 255;
            }

            cindex = ps->msPatArray[i].psPat[k];

            if (shift < ps->msShift_char[cindex])
            {
                ps->msShift_char[cindex] = (u_char) shift;
            }
        }
    }
}

/*
 ** 以字为索引的BMH Shift 表
 ** Prep and Build a Bad Word Shift table
 */

static void mwmPrepBadWordTable(mwm_handle pv)
{
    int i;
    unsigned short k, m, cindex;
    unsigned small_value = MID_PAT_LEN, large_value = 0;
    unsigned shift;
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;

    /* Determine largest and smallest pattern sizes */
    for (i = 0; i < ps->msNumPatterns; i++)
    {
        if (ps->msPatArray[i].psLen < small_value)
        {
            small_value = ps->msPatArray[i].psLen;
        }
        if (ps->msPatArray[i].psLen > large_value)
        {
            large_value = ps->msPatArray[i].psLen;
        }
    }

    m = (unsigned short) small_value; /* Maximum Boyer-Moore Shift */

    /* Limit the maximum size of the smallest pattern to 255 bytes */
    if (m > MID_PAT_LEN)
    {
        m = MID_PAT_LEN;
    }

    ps->msShiftLen = m;

    /* Initialze the default shift table. */
    for (i = 0; i < WORD_TABLE_SIZE; i++)
    {
        ps->msShift_word[i] = (unsigned)(m - 1);
    }

    /* Multi-Pattern Bad Word Shift Table Values */
    for (i = 0; i < ps->msNumPatterns; i++)
    {
        for (k = 0; k < m - 1; k++)
        {
            shift = (unsigned short)(m - 2 - k);

            if (shift > MID_PAT_LEN)
            {
                shift = MID_PAT_LEN;
            }

            cindex = (ps->msPatArray[i].psPat[k]
                    | (ps->msPatArray[i].psPat[k + 1] << 8));

            if (shift < ps->msShift_word[cindex])
            {
                ps->msShift_word[cindex] = shift;
            }
        }
    }
}

/*
 *
 * Finds matches within a groups of patterns, these patterns all have at least 2 characters
 * This version walks thru all of the patterns in the group and applies a reverse string comparison
 * to minimize the impact of sequences of patterns that keep repeating intital character strings
 * with minimal differences at the end of the strings.
 *
 */
static int mwmGroupMatch2(mwm_handle pv,
        int pat_index,
        u_char * Tx,
        u_char * T,
        int Tleft,
        void * in, void * out,
        action_proc match)
{
    int k, sp, ep, st, len, nfound = 0;
    APM_MWM_PATTERN_STRUCT * patrn;
    APM_MWM_PATTERN_STRUCT * patrnEnd;
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;

    /* Process the Hash Group Patterns against the current Text Suffix */
    patrn = &ps->msPatArray[pat_index];
    if (ps->msMwmType == MWM_TYPE_CHAR)
    {
        patrnEnd = patrn + ps->msNumArray_char[pat_index];
    }
    else
    {
        patrnEnd = patrn + ps->msNumArray_word[pat_index];
    }

    /* Match Loop - Test each pattern in the group against the Text */
    for (; patrn < patrnEnd; patrn++)
    {
        u_char * p, *q;

        /* Test if this Pattern is to big for Text, not a possible match */
        if ((unsigned) Tleft < patrn->psLen)
        {
            continue;
        }

        /* Test the offset && deepth */
        len = patrn->psLen;
        sp = patrn->psOffset; //pat's start
        ep = patrn->psOffset + patrn->psDepth; //pat's max end
        st = T - Tx;
        if (!((st >= sp) && (st + len <= ep)))
        {
            continue;
        }

        /* Setup the reverse string compare */
        k = patrn->psLen - ps->msHashBytes - 1;
        q = patrn->psPat + ps->msHashBytes;
        p = T + ps->msHashBytes;

        /* Compare strings backward, unrolling does not help in perf tests. */
        while (k >= 0 && (q[k] == p[k]))
        {
            k--;
        }

        /* We have a content match - call the match routine for further processing */
        if (k < 0)
        {
            nfound++;
            //debug("mwm: matched %lx %lx, pat: %d '%s'\n", patrn, patrn->ps_data, patrn->psLen, patrn->psPat);
            if (match(patrn->ps_data, T, in, out))
            {
                return nfound;
            }
        }
    }

    /* not found or not matched */
    return -nfound;
}

/*
 **
 ** Uses Bad Character Shifts
 ** Handles pattern groups with 2 or more bytes per pattern
 ** Uses 2 byte hash table to group patterns
 **
 */
static int mwmSearchExCC(mwm_handle pv,
        u_char * Tx, int n,
        void * in, void * out,
        action_proc match)
{
    int Tleft, pat_index, nfound, tshift;
    u_char * T, *Tend, *B;
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    u_char * pshift = ps->msShift_char;
    u_char * phash = ps->msHash_char;

    /*MWM_PATTERN_STRUCT *patrn, *patrnEnd;*/

    nfound = 0;
    Tend = Tx + n;

    /* Test if text is shorter than the shortest pattern */
    if ((unsigned) n < ps->msShiftLen)
    {
        return 0;
    }

    /* Process each suffix of the Text, left to right, incrementing T so T = S[j] */
    for (T = Tx, B = Tx + ps->msShiftLen - 1,Tleft = n;
            B < Tend;
            T++, B++, Tleft--)
    {
        /* Multi-Pattern Bad Character Shift */
        while ((tshift = pshift[*B]) > 0)
        {
            B += tshift;
            T += tshift;
            Tleft -= tshift;
            if (B >= Tend)
            {
                return nfound;
            }

            tshift = pshift[*B];
            B += tshift;
            T += tshift;
            Tleft -= tshift;
            if (B >= Tend)
            {
                return nfound;
            }
        }

        /* Test for last char in Text, one byte pattern test was done above, were done. */
        if (Tleft <= 1)
        {
            return nfound;
        }

        /* Test if the 1 char prefix of this suffix shows up in the hash table */
        if ((pat_index = phash[(*T)]) == 0xff)
        {
            continue;
        }

        /* Match this group against the current suffix */
        nfound = mwmGroupMatch2(pv, pat_index, Tx, T, Tleft, in, out, match);
        if (nfound > 0)
        {
            return nfound;
        }
        else
        {
            nfound = 0;
        }
    }

    return nfound;
}

#if 0
/*
 **
 ** Uses Bad Character Shifts
 ** Handles pattern groups with 2 or more bytes per pattern
 ** Uses 2 byte hash table to group patterns
 **
 */
static int mwmSearchExBC(mwm_handle pv,
        u_char * Tx, int n,
        void * in, void * out,
        action_proc match)
{
    int Tleft, pat_index, nfound, tshift;
    u_char * T, *Tend, *B;
    MWM_STRUCT * ps = (MWM_STRUCT *)pv;
    u_char * pshift = ps->msShift_char;
    HASH_TYPE * phash = ps->msHash_word;
    /*MWM_PATTERN_STRUCT *patrn, *patrnEnd;*/

    nfound = 0;

    Tleft = n;
    Tend = Tx + n;

    /* Test if text is shorter than the shortest pattern */
    if ((unsigned)n < ps->msShiftLen)
    {
        return 0;
    }

    /* Process each suffix of the Text, left to right, incrementing T so T = S[j] */
    for (T = Tx, B = Tx + ps->msShiftLen - 1; B < Tend; T++, B++, Tleft--)
    {
        /* Multi-Pattern Bad Character Shift */
        while ((tshift = pshift[*B]) > 0)
        {
            B += tshift;
            T += tshift;
            Tleft -= tshift;
            if (B >= Tend)
            {
                return nfound;
            }

            tshift = pshift[*B];
            B += tshift;
            T += tshift;
            Tleft -= tshift;
            if (B >= Tend)
            {
                return nfound;
            }
        }

        /* Test for last char in Text, one byte pattern test was done above, were done. */
        if (Tleft == 1)
        {
            return nfound;
        }

        /* Test if the 2 char prefix of this suffix shows up in the hash table */
        if ((pat_index = phash[((*T) << 8) | *(T + 1)]) == 0xff)
        {
            continue;
        }

        /* Match this group against the current suffix */
        nfound = mwmGroupMatch2(pv, pat_index, Tx, T, Tleft, in, out, match);
        if (nfound > 0)
        {
            return nfound;
        }
    }

    return nfound;
}
#endif
/*
 **
 ** Uses Bad Word Shifts
 ** Handles pattern groups with 2 or more bytes per pattern
 ** Uses 2 byte hash table to group patterns
 **
 ** search the Text Tx with case infor
 ** return value: the number of match rule
 */

static int mwmSearchExBW(mwm_handle pv,
        u_char * Tx, int n,
        void * in, void * out,
        action_proc match)
{
    int Tleft, pat_index, nfound, tshift;
    u_char * T, *Tend, *B;
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    u_char * pshift2 = ps->msShift_word;
    u_short * phash = ps->msHash_word;

    nfound = 0;
    Tend = Tx + n;

    /* Test if text is shorter than the shortest pattern */
    if ((unsigned) n < ps->msShiftLen)
    {
        return 0;
    }

    /* Process each suffix of the Text, left to right, incrementing T so T = S[j] */
    for (T = Tx, B = Tx + ps->msShiftLen - 1, Tleft = n;
            B < Tend;
            T++, B++, Tleft--)
    {
        /* Multi-Pattern Bad Word Shift */
        tshift = pshift2[((*B) << 8) | *(B - 1)];
        while (tshift)
        {
            B += tshift;
            T += tshift;
            Tleft -= tshift;
            if (B >= Tend)
            {
                return nfound;
            }
            tshift = pshift2[((*B) << 8) | *(B - 1)];
        }

        /* Test for last char in Text, we are done, one byte pattern test was done above. */
        if (Tleft == 1)
        {
            return nfound;
        }

        /* Test if the 2 char prefix of this suffix shows up in the hash table */
        if ((pat_index = phash[((*T) << 8) | *(T + 1)]) == 0xffff)
        {
            continue;
        }

        /* Match this group against the current suffix */
        // modified by adam 2012-10-26 -------------------------------------
//        int ng = mwmGroupMatch2(pv, pat_index, Tx, T, Tleft, in, out, match);
        (void)mwmGroupMatch2(pv, pat_index, Tx, T, Tleft, in, out, match);
        // end of modified by adam  ----------------------------------------
        if (nfound > 0)
        {
            return nfound;
        }
    }

    return nfound;
}

/*
 * Boyer-Moore Horspool
 * Does NOT use Sentinel Byte(s)
 * Scan and Match Loops are unrolled and separated
 * Optimized for 1 byte patterns as well
 */
static inline u_char * bmhSearch(HBM_STRUCT * px, u_char * text, int n)
{
    u_char * pat, *t, *et, *q;
    int m1, k;
    short * bcShift;

    if (!px)
    {
        return 0;
    }

    m1 = px->M - 1;
    pat = px->P;
    bcShift = px->bcShift;

    t = text + m1;
    et = text + n;

    /* Handle 1 Byte patterns - it's a faster loop */
    if (!m1)
    {
        for (; t < et; t++)
            if (*t == *pat)
            {
                return t;
            }
        return 0;
    }

    /* Handle MultiByte Patterns */
    while (t < et)
    {
        /* Scan Loop - Bad Character Shift */
        do
        {
            t += bcShift[*t];
            if (t >= et)
            {
                return 0;
            }

            t += (k = bcShift[*t]);
            if (t >= et)
            {
                return 0;
            }

        }
        while (k);

        /* Unrolled Match Loop */
        k = m1;
        q = t - m1;
        while (k >= 4)
        {
            if (pat[k] != q[k])
            {
                goto NoMatch;
            }
            k--;
            if (pat[k] != q[k])
            {
                goto NoMatch;
            }
            k--;
            if (pat[k] != q[k])
            {
                goto NoMatch;
            }
            k--;
            if (pat[k] != q[k])
            {
                goto NoMatch;
            }
            k--;
        }
        /* Finish Match Loop */
        while (k >= 0)
        {
            if (pat[k] != q[k])
            {
                goto NoMatch;
            }
            k--;
        }
        /* If matched - return 1st char of pattern in text */
        return q;

NoMatch:

        /* Shift by 1, this replaces the good suffix shift */
        t++;
    }

    return 0;
}

/*
 ** Search a body of text or data for paterns
 */
int apm_mwmSearch(mwm_handle pv,
        u_char * T, int n,
        void * in, void * out,
        action_proc match)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    int pats, pate, s, plen;

    int i, nfound = 0;

    if (ps->msNumPatterns < 1)
    {
        return 0;    //no found
    }

    /* Boyer-Moore */
    if (ps->msMethod == MTH_BM)
    {
        u_char * Tx;

        for (i = 0; i < ps->msNumPatterns; i++)
        {
            APM_MWM_PATTERN_STRUCT * patt = &ps->msPatArray[i];
            Tx = bmhSearch(patt->psBmh, T, n);
            if (Tx)
            {
                plen = patt->psLen;
                pats = patt->psOffset;
                pate = patt->psOffset + patt->psDepth;
                s = Tx - T;
                if (!(s >= pats && s + plen < pate))
                {
                    continue;
                }

                nfound++;
                if (match(patt->ps_data, Tx, in, out))
                {
                    return nfound;
                }
            }
        }
        return nfound;
    }
    else
    {
        return ps->search(pv, T, n, in, out, match);
    }

}

/*
 ** bcompare::
 **
 ** Perform a Binary comparsion of 2 byte sequences of possibly
 ** differing lengths.
 **
 ** returns -1 a < b
 ** +1 a > b
 ** 0 a = b
 */
static int bcompare(const u_char * a, int alen, const u_char * b, int blen)
{
    int stat;
    if (alen == blen)
    {
        return memcmp(a, b, alen);
    }
    else if (alen < blen)
    {
        if ((stat = memcmp(a, b, alen)) != 0)
        {
            return stat;
        }
        return -1;
    }
    else
    {
        if ((stat = memcmp(a, b, blen)) != 0)
        {
            return stat;
        }
        return +1;
    }
}

/*
 ** sortcmp:: qsort callback
 */
/*升序*/
static int sortcmp_asc(const void * e1, const void * e2)
{
    const APM_MWM_PATTERN_STRUCT *r1 = (const APM_MWM_PATTERN_STRUCT *)e1;
    const APM_MWM_PATTERN_STRUCT *r2 = (const APM_MWM_PATTERN_STRUCT *)e2;
    return bcompare(r1->psPat, r1->psLen, r2->psPat, r2->psLen);
}
/*降序*/
static int sortcmp_desc(const void * e1, const void * e2)
{
    const APM_MWM_PATTERN_STRUCT *r1 = (const APM_MWM_PATTERN_STRUCT *)e2;
    const APM_MWM_PATTERN_STRUCT *r2 = (const APM_MWM_PATTERN_STRUCT *)e1;
    return bcompare(r1->psPat, r1->psLen, r2->psPat, r2->psLen);
}

/*
 return : 0 suc <0 error
 */

int mwmPrepMem(void * pv)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;

    /* Build an array of pointers to the list of Pattern nodes */
    ps->msPatArray = (APM_MWM_PATTERN_STRUCT *)
        h_malloc(sizeof(APM_MWM_PATTERN_STRUCT) * ps->msNumPatterns);
    if (!ps->msPatArray)
    {
        debug("mwm: kmalloc %d pattern nodes failed!\n", ps->msNumPatterns);
        goto __err;
    }
    memset(ps->msPatArray, 0,
            sizeof(APM_MWM_PATTERN_STRUCT) * ps->msNumPatterns);

    if (ps->msMwmType == MWM_TYPE_CHAR)
    {
        ps->msNumArray_char =
            (u_char *) h_malloc(sizeof(u_char) * ps->msNumPatterns);
        if (!ps->msNumArray_char)
        {
            goto __err;
        }
        memset(ps->msNumArray_char, 0, sizeof(u_char) * ps->msNumPatterns);
    }
    else
    {
        ps->msNumArray_word =
            (u_short *) h_malloc(sizeof(u_short) * ps->msNumPatterns);
        if (!ps->msNumArray_word)
        {
            goto __err;
        }
        memset(ps->msNumArray_word, 0, sizeof(u_short) * ps->msNumPatterns);
    }

    if (ps->msMethod == MTH_MWM)
    {
        if (ps->msMwmType == MWM_TYPE_CHAR)
        {
            ps->msNumHashEntries = CHAR_TABLE_SIZE;
            ps->msHash_char =
                (u_char *) h_malloc(sizeof(u_char) * ps->msNumHashEntries);
            if (!ps->msHash_char)
            {
                debug("mwm: %s kmalloc for hash failed!"
                        " may be not enough memory\n", __FUNCTION__);
                goto __err;
            }
            memset(ps->msHash_char, 0, sizeof(u_char) * ps->msNumHashEntries);
            ps->msShift_char =
                (u_char *) h_malloc(CHAR_TABLE_SIZE * sizeof(char));
            if (!(ps->msShift_char))
            {
                debug("mwm: %s kmalloc for Shift2 failed!"
                        "may be not enough memory.\n", __FUNCTION__);
                goto __err;
            }
            memset(ps->msShift_char, 0, CHAR_TABLE_SIZE * sizeof(char));
        }
        else
        {
            ps->msNumHashEntries = WORD_TABLE_SIZE;
            ps->msHash_word =
                (u_short *) h_malloc(sizeof(u_short) * ps->msNumHashEntries);
            if (!ps->msHash_word)
            {
                debug("mwm: %s kmalloc for hash failed!"
                        "may be not enough memory\n", __FUNCTION__);
                goto __err;
            }
            memset(ps->msHash_word, 0, sizeof(u_short) * ps->msNumHashEntries);
            ps->msShift_word =
                (u_char *) h_malloc(WORD_TABLE_SIZE * sizeof(char));
            if (!(ps->msShift_word))
            {
                debug("mwm: %s kmalloc for Shift2 failed!"
                        "may be not enough memory.\n", __FUNCTION__);
                goto __err;
            }
            memset(ps->msShift_word, 0, WORD_TABLE_SIZE * sizeof(char));
        }
    }
    return 0;

__err:

    if (ps->msPatArray)
    {
        h_free(ps->msPatArray);
    }
    if (ps->msNumArray_char)
    {
        h_free(ps->msNumArray_char);
    }
    if (ps->msNumArray_word)
    {
        h_free(ps->msNumArray_word);
    }
    if (ps->msHash_word)
    {
        h_free(ps->msHash_word);
    }
    if (ps->msShift_word)
    {
        h_free(ps->msShift_word);
    }

    ps->msPatArray = NULL;
    ps->msNumArray_char = NULL;
    ps->msNumArray_word = NULL;
    ps->msHash_word = NULL;
    ps->msShift_word = NULL;

    return -1;
}

HBM_STRUCT * bmhPrepBmh(HBM_STRUCT * p, u_char * pat, int m)
{
    int k;

    if (!m)
    {
        return 0;
    }
    p->P = pat;
    p->M = m;

    /* Compute normal Boyer-Moore Bad Character Shift */
    for (k = 0; k < 256; k++)
    {
        p->bcShift[k] = m;
    }
    for (k = 0; k < m; k++)
    {
        p->bcShift[pat[k]] = m - k - 1;
    }

    return p;
}

/*
 * BMH 算法, 对于少量pat时使用
 */
HBM_STRUCT * bmhPrepEx(u_char * pat, int m)
{
    HBM_STRUCT * p;

    p = (HBM_STRUCT *) h_malloc(sizeof(HBM_STRUCT));
    if (!p)
    {
        debug("mwm: bmh malloc failed!\n");
        return 0;
    }

    return bmhPrepBmh(p, pat, m);
}

/*
 **
 ** mwmPrepPatterns:: Prepare the pattern group for searching
 **
 */
int apm_mwmPrepPatterns(mwm_handle pv)
{
    int k;
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    APM_MWM_PATTERN_STRUCT * plist;

    if (!pv)
    {
        debug("mwm: MWM STRUCT is null !\n");
        return 1;
    }

    if (ps->msNumPatterns <= 0)
    {
        debug("mwm: Number of patterns is zero!\n ");
        return 0;
    }

    ps->msMethod = ps->msNumPatterns < 2 ? MTH_BM : MTH_MWM;
    if (mwmPrepMem(ps) < 0)
    {
        debug("apm_mwmPrepPatterns() mwmPrep_Mem failed\n");
        goto __err;
    }

    /* Calc Pats's Length info */
    mwmAnalyzePattens(pv);

    /* Copy the list node info into the Array */
    for (k = 0, plist = ps->plist; plist != NULL && k < ps->msNumPatterns;
            plist = plist->next)
    {
        memcpy(&ps->msPatArray[k++], plist, sizeof(APM_MWM_PATTERN_STRUCT));
    }

    /* Initialize the MWM or Boyer-Moore Pattern data */
    if (ps->msMethod == MTH_MWM)
    {
        /* Sort the patterns */
        if (ps->msPrefixMatch == SHORT_PREFIX_MATCH)
        {
            qsort(ps->msPatArray, ps->msNumPatterns,
                    sizeof(APM_MWM_PATTERN_STRUCT), sortcmp_asc);
        }
        else
        {
            qsort(ps->msPatArray, ps->msNumPatterns,
                    sizeof(APM_MWM_PATTERN_STRUCT), sortcmp_desc);
        }

        /* Build the Hash table, and pattern groups, per Wu & Manber */
        if (ps->msMwmType == MWM_TYPE_CHAR)
        {
            mwmPrepHashedPatternGroupsC(pv);
            mwmPrepBadCharTable(pv);
            ps->search = mwmSearchExCC;
            ps->msHashBytes = sizeof(u_char);
        }
        else
        {
            mwmPrepHashedPatternGroupsW(pv);
            mwmPrepBadWordTable(pv);
            ps->search = mwmSearchExBW;
            ps->msHashBytes = sizeof(u_short);
        }
    }
    else if (ps->msMethod == MTH_BM)
    {
        /* Allocate and initialize the BMH data for each pattern */
        if (ps->msPrefixMatch == SHORT_PREFIX_MATCH)
        {
            qsort(ps->msPatArray, ps->msNumPatterns,
                    sizeof(APM_MWM_PATTERN_STRUCT), sortcmp_asc);
        }
        else
        {
            qsort(ps->msPatArray, ps->msNumPatterns,
                    sizeof(APM_MWM_PATTERN_STRUCT), sortcmp_desc);
        }

        for (k = 0; k < ps->msNumPatterns; k++)
        {
            ps->msPatArray[k].psBmh = bmhPrepEx(ps->msPatArray[k].psPat,
                    ps->msPatArray[k].psLen);
        }
    }

    debug("mwm: PrepPatterns finished: %d.\n", ps->msNumPatterns);
    return 1;

__err:
    return -1;
}

#ifdef DEBUG_MWM
/*********************************************************/
/* property 一些属性配置接口 */
/*********************************************************/

/*
 ** mwmGetNpatterns:: 返回模式串个数
 */
int apm_mwmGetNumPatterns(mwm_handle pv)
{
    APM_MWM_STRUCT * p = (APM_MWM_STRUCT *) pv;
    return p->msNumPatterns;
}

uint32_t apm_mwmMemCourst()
{
    static uint32_t szMemCourst = 0;
    return szMemCourst / 1024;
}

/************************************************************************/
/* 一些调试接口 */
/************************************************************************/
/*
 ** Print some Pattern Stats
 */
void apm_mwmShowStats(mwm_handle pv)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    (void) ps;
    debug("Pattern Stats\n");
    debug("-------------\n");
    debug("Patterns : %d\n" , ps->msNumPatterns);
    debug("Average : %d chars\n", ps->msAvg);
    debug("Smallest : %d chars\n", ps->msSmallest);
    debug("Largest : %d chars\n", ps->msLargest);
    debug("Total chars: %d\n" , ps->msTotal);
}

/* Display function for testing */
static void show_bytes(unsigned n, u_char * p)
{
    int i;
    for (i = 0; i < (int) n; i++)
    {
        if (p[i] >= 32 && p[i] <= 127)
        {
            debug("%c", p[i]);
        }
        else
        {
            debug("\\x%2.2X", p[i]);
        }
    }

}

/*
 ** Display patterns in this group
 */
void apm_mwmGroupDetails(mwm_handle pv)
{
    APM_MWM_STRUCT * ps = (APM_MWM_STRUCT *) pv;
    int pat_index, i, m, gmax = 0, total = 0, subgroups;
    static int k = 0;
    (void) k;
    APM_MWM_PATTERN_STRUCT * patrn, *patrnEnd;
    if (!ps)
    {
        debug("mwm: null mwm struct for group info.\n");
        return;
    }
    if (ps->msMethod == MTH_BM)
    {
        debug("mwm: bmh used.\n");
        return;
    }

    debug("*** MWM-Pattern-STURCT: %d ***\n", k++);

    subgroups = 0;
    for (i = 0; i < ps->msNumHashEntries; i++)
    {
        if (ps->msMwmType == MWM_TYPE_CHAR)
        {
            if ((pat_index = ps->msHash_char[i]) == 0xff)
            {
                continue;
            }

            patrn = &ps->msPatArray[pat_index]; /* 1st pattern of hash group is here */
            patrnEnd = patrn + ps->msNumArray_char[pat_index];/* never go here... */

            debug("  mwm: Sub-Pattern-Group: %d-%d:%d, patn:%d\n",
                    subgroups, i, pat_index, ps->msNumArray_char[pat_index]);
        }
        else
        {
            if ((pat_index = ps->msHash_word[i]) == 0xffff)
            {
                continue;
            }

            patrn = &ps->msPatArray[pat_index]; /* 1st pattern of hash group is here */
            patrnEnd = patrn + ps->msNumArray_word[pat_index];/* never go here... */

            debug("  mwm: Sub-Pattern-Group: %d-%d:%d, patn:%d\n",
                    subgroups, i, pat_index, ps->msNumArray_word[pat_index]);
        }

        subgroups++;
        for (m = 0; patrn < patrnEnd; m++, patrn++) /* Test all patterns in the group */
        {
            debug("\tmwm: Pattern[%d]: ", m);
            show_bytes(patrn->psLen, patrn->psPat);
            debug("\n");
        }

        if (m > gmax)
        {
            gmax = m;
        }
        total += m;
    }

    debug("Total Group Patterns: %d\n", total);
    debug(" Number of Sub-Groups: %d\n", subgroups);
    debug(" Sub-Group Max Patterns: %d\n", gmax);
    debug(" Sub-Group Avg Patterns: %d\n", total / subgroups);
}

/*
 **
 */
void apm_mwmFeatures(void)
{
    debug("%s\n", MWM_FEATURES);
}
#endif
