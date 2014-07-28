/*
 * Inspired from FreeBSD src/sys/amd64/include/atomic.h
 * Copyright (c) 1998 Doug Rabson
 * All rights reserved.
 */

/**
 * @file
 * Atomic Operations on x86_64
 */

#if 0
#ifndef _SF_ATOMIC_H_
#error "don't include this file directly, please include generic <wns_atomic.h>"
#endif
#endif

#ifndef __LIBWNS_ATOMIC_X86_64_H__
#define __LIBWNS_ATOMIC_X86_64_H__

#include "libbase_type.h"

/*
 * This file provides an API for atomic operations on x86_64.
 */

#if SF_MAX_LCORE == 1
#define	MPLOCKED /**< Do not need to insert MP lock prefix. */
#else
#define	MPLOCKED	"lock ; " /**< Insert MP lock prefix. */
#endif

/**
 * General memory barrier.
 *
 * Guarantees that LOAD and STORE operations generated before the
 * barrier occur before the LOAD and STORE operations generated after.
 */
#define	wns_mb()  asm volatile("mfence;" : : : "memory")

/**
 * Write memory barrier.
 *
 * Guarantees that the STORE operations generated before the barrier
 * occur before the STORE operations generated after.
 */
#define	wns_wmb() asm volatile("sfence;" : : : "memory")

/**
 * Read memory barrier.
 *
 * Guarantees that the LOAD operations generated before the barrier
 * occur before the LOAD operations generated after.
 */
#define	wns_rmb() asm volatile("lfence;" : : : "memory")

/**
 * Atomic compare and set
 *
 * (atomic) equivalent to:
 *   if (*dst == exp)
 *     *dst = src (all 32 bit words)
 *
 * @param dst
 *   The destination location into which the value will be written.
 * @param exp
 *   The expected value.
 * @param src
 *   The new value.
 * @return
 *   Non-zero on success; 0 on failure.
 */
static inline int32_t
wns_atomic32_cmpset(volatile uint32_t *dst, uint32_t exp, uint32_t src)
{
    uint8_t res;

    asm volatile(
        "	" MPLOCKED "  			"
        "	cmpxchgl %[src],%[dst] ;	"
        "       sete	%[res] ;		"
        : [res] "=a"(res),			/* 0 */
        [dst] "=m"(*dst)			/* 1 */
        : [src] "r"(src),			/* 2 */
        "a"(exp),				/* 3 */
        "m"(*dst)				/* 4 */
        : "memory");

    return res;
}

/**
 * The atomic counter structure.
 */
typedef struct {
    volatile int32_t cnt; /**< An internal counter value. */
} wns_atomic32_t;

/**
 * Static initializer for an atomic counter.
 */
#define WNS_ATOMIC32_INIT(val) { (val) }

/**
 * Initialize an atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
wns_atomic32_init(wns_atomic32_t *v)
{
    v->cnt = 0;
}

/**
 * Atomically read a 32-bit value from a counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   The value of the counter.
 */
static inline int32_t
wns_atomic32_read(wns_atomic32_t *v)
{
    return v->cnt;
}

/**
 * Atomically set a counter to a 32-bit value.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param new
 *   The new value for the counter.
 */
static inline void
wns_atomic32_set(wns_atomic32_t *v, int32_t new)
{
    v->cnt = new;
}

/**
 * Atomically add a 32-bit value to an atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 */
static inline void
wns_atomic32_add(wns_atomic32_t *v, int32_t inc)
{
    asm volatile(MPLOCKED
            "addl %[inc],%[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : [inc] "ir"(inc),    /* input (1) */
            "m"(v->cnt)         /* input (2) */
            );                    /* no clobber-list */
}

/**
 * Atomically subtract a 32-bit value from an atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 */
static inline void
wns_atomic32_sub(wns_atomic32_t *v, int32_t dec)
{
    asm volatile(MPLOCKED
            "subl %[dec],%[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : [dec] "ir"(dec),    /* input (1) */
            "m"(v->cnt)         /* input (2) */
            );                    /* no clobber-list */
}

/**
 * Atomically increment a counter by one.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
wns_atomic32_inc(wns_atomic32_t *v)
{
    asm volatile(MPLOCKED
            "incl %[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : "m"(v->cnt)         /* input (1) */
            );                    /* no clobber-list */
}

/**
 * Atomically decrement a counter by one.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
wns_atomic32_dec(wns_atomic32_t *v)
{
    asm volatile(MPLOCKED
            "decl %[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : "m"(v->cnt)         /* input (1) */
            );                    /* no clobber-list */
}

/**
 * Atomically add a 32-bit value to a counter and return the result.
 *
 * Atomically adds the 32-bits value (inc) to the atomic counter (v) and
 * returns the value of v after addition.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 * @return
 *   The value of v after the addition.
 */
static inline int32_t
wns_atomic32_add_return(wns_atomic32_t *v, int32_t inc)
{
    int32_t prev = inc;

    asm volatile(MPLOCKED
            "xaddl %[prev], %[cnt]"
            : [prev] "+r"(prev),      /* output (0) */
            [cnt] "=m"(v->cnt)      /* output (1) */
            : "m"(v->cnt)             /* input (2) */
            );                        /* no clobber-list */
    return prev + inc;
}

/**
 * Atomically subtract a 32-bit value from a counter and and return
 * the result.
 *
 * Atomically subtracts the 32-bit value (inc) from the atomic counter (v)
 * and returns the value of v after the substraction.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 * @return
 *   The value of v after the substraction.
 */
static inline int32_t
wns_atomic32_sub_return(wns_atomic32_t *v, int32_t dec)
{
    return wns_atomic32_add_return(v, -dec);
}

/**
 * Atomically increment a 32-bit counter by one and test.
 *
 * Atomically increments the atomic counter (v) by one and returns true if
 * the result is 0, or false in all other cases.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   True if the result after the increment operation is 0; false otherwise.
 */
static inline int32_t wns_atomic32_inc_and_test(wns_atomic32_t *v)
{
    uint8_t ret;

    asm volatile(MPLOCKED
            "incl %[cnt] ; "
            "sete %[ret]"
            : [cnt] "+m"(v->cnt),    /* output (0) */
            [ret] "=qm"(ret)       /* output (1) */
            :                        /* no input */
            );                       /* no clobber-list */
    return ret != 0;
}

/**
 * Atomically decrement a 32-bit counter by one and test.
 *
 * Atomically decrements the atomic counter (v) by one and returns true if
 * the result is 0, or false in all other cases.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   True if the result after the decrement operation is 0; false otherwise.
 */
static inline int32_t wns_atomic32_dec_and_test(wns_atomic32_t *v)
{
    uint8_t ret;

    asm volatile(MPLOCKED
            "decl %[cnt] ; "
            "sete %[ret]"
            : [cnt] "+m"(v->cnt),    /* output (0) */
            [ret] "=qm"(ret)       /* output (1) */
            :                        /* no input */
            );                       /* no clobber-list */
    return ret != 0;
}

/**
 * Atomically test and set a 32-bit atomic counter.
 *
 * If the counter value is already set, return 0 (failed). Otherwise, set
 * the counter value to 1 and return 1 (success).
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   0 if failed; else 1, success.
 */
static inline int32_t wns_atomic32_test_and_set(wns_atomic32_t *v)
{
    return !!wns_atomic32_cmpset((uint32_t *)&v->cnt, 0, 1);
}

/**
 * Atomically set a 32-bit counter to 0.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void wns_atomic32_clear(wns_atomic32_t *v)
{
    v->cnt = 0;
}

/* 64 bits */

/**
 * An atomic compare and set function used by the mutex functions.
 * (atomic) equivalent to:
 *   if (*dst == exp)
 *     *dst = src (all 64-bit words)
 * @param dst
 *   The destination into which the value will be written.
 * @param exp
 *   The expected value.
 * @param src
 *   The new value.
 * @return
 *   Non-zero on success; 0 on failure.
 */
static inline int32_t
wns_atomic64_cmpset(volatile uint64_t *dst, uint64_t exp, uint64_t src)
{
    uint8_t res;

    asm volatile(
        "	" MPLOCKED "			"
        "	cmpxchgq %[src],%[dst] ;	"
        "       sete	%[res] ;		"
        "1:					"
        "# atomic_cmpset_long"
        : [res] "=a"(res),		/* 0 */
        [dst] "=m"(*dst)		/* 1 */
        : [src] "r"(src),		/* 2 */
        "a"(exp),			/* 3 */
        "m"(*dst)			/* 4 */
        : "memory");

    return res;
}

/**
 * The atomic counter structure.
 */
typedef struct {
    volatile int64_t cnt; /**< internal counter value */
} wns_atomic64_t;

/**
 * Static initializer for an atomic counter.
 */
#define WNS_ATOMIC64_INIT(val) { (val) }

/**
 * Initialize the atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
wns_atomic64_init(wns_atomic64_t *v)
{
    v->cnt = 0;
}

/**
 * Atomically read a 64-bit counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   The value of the counter.
 */
static inline int64_t
wns_atomic64_read(wns_atomic64_t *v)
{
    return v->cnt;
}

/**
 * Atomically set a 64-bit counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param new
 *   The new value of the counter.
 */
static inline void
wns_atomic64_set(wns_atomic64_t *v, int64_t new)
{
    v->cnt = new;
}

/**
 * Atomically add a 64-bit value to a counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 */
static inline void
wns_atomic64_add(wns_atomic64_t *v, int64_t inc)
{
    asm volatile(MPLOCKED
            "addq %[inc],%[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : [inc] "ir"(inc),    /* input (1) */
            "m"(v->cnt)         /* input (2) */
            );                    /* no clobber-list */
}

/**
 * Atomically subtract a 64-bit value from a counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 */
static inline void
wns_atomic64_sub(wns_atomic64_t *v, int64_t dec)
{
    asm volatile(MPLOCKED
            "subq %[dec],%[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : [dec] "ir"(dec),    /* input (1) */
            "m"(v->cnt)         /* input (2) */
            );                    /* no clobber-list */
}

/**
 * Atomically increment a 64-bit counter by one and test.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
wns_atomic64_inc(wns_atomic64_t *v)
{
    asm volatile(MPLOCKED
            "incq %[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : "m"(v->cnt)         /* input (1) */
            );                    /* no clobber-list */
}

/**
 * Atomically decrement a 64-bit counter by one and test.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void
wns_atomic64_dec(wns_atomic64_t *v)
{
    asm volatile(MPLOCKED
            "decq %[cnt]"
            : [cnt] "=m"(v->cnt)  /* output (0) */
            : "m"(v->cnt)         /* input (1) */
            );                    /* no clobber-list */
}

/**
 * Add a 64-bit value to an atomic counter and return the result.
 *
 * Atomically adds the 64-bit value (inc) to the atomic counter (v) and
 * returns the value of v after the addition.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 * @return
 *   The value of v after the addition.
 */
static inline int64_t
wns_atomic64_add_return(wns_atomic64_t *v, int64_t inc)
{
    int64_t prev = inc;

    asm volatile(MPLOCKED
            "xaddq %[prev], %[cnt]"
            : [prev] "+r"(prev),      /* output (0) */
            [cnt] "=m"(v->cnt)      /* output (1) */
            : "m"(v->cnt)             /* input (2) */
            );                        /* no clobber-list */
    return prev + inc;
}

/**
 * Subtract a 64-bit value from an atomic counter and return the result.
 *
 * Atomically subtracts the 64-bit value (dec) from the atomic counter (v)
 * and returns the value of v after the substraction.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 * @return
 *   The value of v after the substraction.
 */
static inline int64_t
wns_atomic64_sub_return(wns_atomic64_t *v, int64_t dec)
{
    return wns_atomic64_add_return(v, -dec);
}

/**
 * Atomically increment a 64-bit counter by one and test.
 *
 * Atomically increments the atomic counter (v) by one and returns true if
 * the result is 0, or false in all other cases.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   True if the result after the addition is 0; false otherwise.
 */
static inline int32_t wns_atomic64_inc_and_test(wns_atomic64_t *v)
{
    uint8_t ret;

    asm volatile(MPLOCKED
            "incq %[cnt] ; "
            "sete %[ret]"
            : [cnt] "+m"(v->cnt),    /* output (0) */
            [ret] "=qm"(ret)       /* output (1) */
            :                        /* no input */
            );                       /* no clobber-list */
    return ret != 0;
}

/**
 * Atomically decrement a 64-bit counter by one and test.
 *
 * Atomically decrements the atomic counter (v) by one and returns true if
 * the result is 0, or false in all other cases.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   True if the result after substraction is 0; false otherwise.
 */
static inline int32_t wns_atomic64_dec_and_test(wns_atomic64_t *v)
{
    uint8_t ret;

    asm volatile(MPLOCKED
            "decq %[cnt] ; "
            "sete %[ret]"
            : [cnt] "+m"(v->cnt),    /* output (0) */
            [ret] "=qm"(ret)       /* output (1) */
            :                        /* no input */
            );                       /* no clobber-list */
    return ret != 0;
}

/**
 * Atomically test and set a 64-bit atomic counter.
 *
 * If the counter value is already set, return 0 (failed). Otherwise, set
 * the counter value to 1 and return 1 (success).
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   0 if failed; else 1, success.
 */
static inline int32_t wns_atomic64_test_and_set(wns_atomic64_t *v)
{
    return !!wns_atomic64_cmpset((uint64_t *)&v->cnt, 0, 1);
}

/**
 * Atomically set a 64-bit counter to 0.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
static inline void wns_atomic64_clear(wns_atomic64_t *v)
{
    v->cnt = 0;
}

#endif /* __LIBWNS_ATOMIC_X86_64_H__ */
