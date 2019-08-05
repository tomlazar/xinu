/**
 * @file atomic.h
 *
 * This file includes the function prototypes for atomic ARM assembly routines.
 *
 */
/* Embedded Xinu, Copyright (C) 2019. All rights reserved. */

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

/**
 * @fn int _atomic_increment(int *var)
 *  
 * Atomically increments specified integer.
 * (Equivalent to ++var in C).
 * 
 * @param *var pointer to variable
 * @return value the variable held after
 *         being incremented.
 */
int _atomic_increment(int *var);

/**
 * @fn int _atomic_decrement(int *var)
 * 
 * Atomically decrements specified integer.
 * (Equivalent to --var in C).
 * 
 * @param *var pointer to variable
 * @return value the variable held after
 *         being decremented.
 *         
 */
int _atomic_decrement(int *var);

/**
 * @fn int _atomic_increment_mod(int *var, int mod)
 * 
 * Atomically increment and then mod with specified value.
 * Equivalent to var = (var + 1) % mod.
 * 
 * @param *var  pointer to variable
 * @param mod value to wrap-around if
 *            var exceeds it.
 * @return incremented var value
 */
int _atomic_increment_mod(int *var, int mod);


#endif                          /* _ATOMIC_H_ */
