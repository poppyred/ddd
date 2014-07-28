/**
 * @file
 * Atomic Operations on atheros mips32
 */

#ifndef __LIBWNS__ATOMIC__MIPS32__H
#define __LIBWNS__ATOMIC__MIPS32__H

#include "libbase_type.h"

/*
 * This file provides an API for atomic operations on atheros mips32.
 */

/**
 * The atomic counter structure.
 */
typedef struct {
	volatile int32_t cnt; /**< An internal counter value. */
} wns_atomic32_t;




/**
 * General memory barrier.
 *
 * Guarantees that LOAD and STORE operations generated before the
 * barrier occur before the LOAD and STORE operations generated after.
 */
#define	wns_mb()  \
	asm volatile(			\
		".set	push\n\t"		\
		".set	noreorder\n\t"		\
		".set	mips2\n\t"		\
		"sync\n\t"			\
		".set	pop"			\
		: /* no output */		\
		: /* no input */		\
		: "memory")

/**
 * Write memory barrier.
 *
 * Guarantees that the STORE operations generated before the barrier
 * occur before the STORE operations generated after.
 */
#define	wns_rmb()  \
		asm volatile(			\
			".set	push\n\t"		\
			".set	noreorder\n\t"		\
			".set	mips2\n\t"		\
			"sync\n\t"			\
			".set	pop"			\
			: /* no output */		\
			: /* no input */		\
			: "memory")


/**
 * Read memory barrier.
 *
 * Guarantees that the LOAD operations generated before the barrier
 * occur before the LOAD operations generated after.
 */
#define	wns_wmb()  \
		asm volatile(			\
			".set	push\n\t"		\
			".set	noreorder\n\t"		\
			".set	mips2\n\t"		\
			"sync\n\t"			\
			".set	pop"			\
			: /* no output */		\
			: /* no input */		\
			: "memory")


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

static inline uint32_t
wns_atomic32_cmpset(volatile uint32_t *dst, uint32_t exp, uint32_t src)
{
	uint32_t res=0;

	asm volatile(
		"	.set	push				\n"
		"	.set	noat				\n"
		"	.set	mips3				\n"
		"1:  ll 	%0, %2		# __cmpxchg_asm \n"
		"	bne %0, %z3, 2f 		\n" \
		"	.set	mips0				\n"
		"	move	$1, %z4 			\n"
		"	.set	mips3				\n"
		"	sc  	$1, %1				\n"
		"	beqz	$1, 3f				\n"
		"2: 					\n"
		"	.subsection 2				\n"
		"3: b	1b				\n"
		"	.previous				\n"
		"	.set	pop 			\n"
		: "=&r" (res), "=R" (*dst)
		: "R" (*dst), "Jr" (exp), "Jr" (src)
		: "memory");

	if(res==exp)
		return 1;

	return 0;
}



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

	uint32_t temp;

	asm volatile(
		"	.set	mips3					\n"
		"1: ll	%0, %1		# atomic_add		\n"
		"	addu	%0, %2					\n"
		"	sc	%0, %1					\n"
		"	beqz	%0, 2f					\n"
		"	.subsection 2					\n"
		"2: b	1b					\n"
		"	.previous					\n"
		"	.set	mips0					\n"
		: "=&r" (temp), "=m" (v->cnt)
		: "Ir" (inc), "m" (v->cnt));

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
	int temp;

	asm volatile(
		"	.set	mips3					\n"
		"1:	ll	%0, %1		# atomic_sub		\n"
		"	subu	%0, %2					\n"
		"	sc	%0, %1					\n"
		"	beqz	%0, 2f					\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	.previous					\n"
		"	.set	mips0					\n"
		: "=&r" (temp), "=m" (v->cnt)
		: "Ir" (dec), "m" (v->cnt));
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
	wns_atomic32_add(v,1);
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
	wns_atomic32_sub(v,1);
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
	int32_t result;

	int32_t temp;

	wns_mb();

	asm volatile(
		"	.set	mips3					\n"
		"1:	ll	%1, %2		# atomic_add_return	\n"
		"	addu	%0, %1, %3				\n"
		"	sc	%0, %2					\n"
		"	beqz	%0, 2f					\n"
		"	addu	%0, %1, %3				\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	.previous					\n"
		"	.set	mips0					\n"
		: "=&r" (result), "=&r" (temp), "=m" (v->cnt)
		: "Ir" (inc), "m" (v->cnt)
		: "memory");

	wns_mb();

	return result;
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
	int32_t result;
	int32_t temp;

	wns_mb();
	asm volatile(
		"	.set	mips3					\n"
		"1:	ll	%1, %2		# atomic_sub_return	\n"
		"	subu	%0, %1, %3				\n"
		"	sc	%0, %2					\n"
		"	beqz	%0, 2f					\n"
		"	subu	%0, %1, %3				\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	.previous					\n"
		"	.set	mips0					\n"
		: "=&r" (result), "=&r" (temp), "=m" (v->cnt)
		: "Ir" (dec), "m" (v->cnt)
		: "memory");

	wns_mb();

	return result;
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
	return (wns_atomic32_add_return(v,1)==0);
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
	return (wns_atomic32_sub_return(v,1)==0);
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



/**
 * The atomic counter structure.
 */
typedef struct atomic64_t wns_atomic64_t;


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
int32_t wns_atomic64_cmpset(wns_atomic64_t *dst, uint64_t exp, uint64_t src);






/**
 * Initialize the atomic counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
void wns_atomic64_init(wns_atomic64_t *v);

/**
 * Atomically read a 64-bit counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @return
 *   The value of the counter.
 */
int64_t wns_atomic64_read(wns_atomic64_t *v);

/**
 * Atomically set a 64-bit counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param new
 *   The new value of the counter.
 */
void wns_atomic64_set(wns_atomic64_t *v, int64_t new);

/**
 * Atomically add a 64-bit value to a counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param inc
 *   The value to be added to the counter.
 */
void wns_atomic64_add(wns_atomic64_t *v, int64_t inc);

/**
 * Atomically subtract a 64-bit value from a counter.
 *
 * @param v
 *   A pointer to the atomic counter.
 * @param dec
 *   The value to be substracted from the counter.
 */
void wns_atomic64_sub(wns_atomic64_t *v, int64_t dec);

/**
 * Atomically increment a 64-bit counter by one and test.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
void wns_atomic64_inc(wns_atomic64_t *v);

/**
 * Atomically decrement a 64-bit counter by one and test.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
void wns_atomic64_dec(wns_atomic64_t *v);

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
int64_t wns_atomic64_add_return(wns_atomic64_t *v, int64_t inc);

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
int64_t wns_atomic64_sub_return(wns_atomic64_t *v, int64_t dec);

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
int32_t wns_atomic64_inc_and_test(wns_atomic64_t *v);

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
int32_t wns_atomic64_dec_and_test(wns_atomic64_t *v);

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
int32_t wns_atomic64_test_and_set(wns_atomic64_t *v);

/**
 * Atomically set a 64-bit counter to 0.
 *
 * @param v
 *   A pointer to the atomic counter.
 */
void wns_atomic64_clear(wns_atomic64_t *v);


#endif /* __LIBWNS__ATOMIC__MIPS32__H */
