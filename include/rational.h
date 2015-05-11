/*
 * rational.h
 *
 *  Created on: Jan 13, 2014
 *      Author: dejan
 */

#pragma once

#include "poly.h"

#include <stdio.h>
#include <gmp.h>

#include "integer.h"
#include "dyadic_rational.h"

/**
 * Construct rational 0/1.
 */
void lp_rational_construct(lp_rational_t* q);

/**
 * Construct rational a/b.
 */
void lp_rational_construct_from_int(lp_rational_t* q, long a, unsigned long b);

/**
 * Construct rational a/1.
 */
void lp_rational_construct_from_integer(lp_rational_t* q, const lp_integer_t* a);

/**
 * Construct a rational representation of x.
 */
void lp_rational_construct_from_double(lp_rational_t* q, double x);

/**
 * Construct a rational number from a dyadic rational.
 */
void lp_rational_construct_from_dyadic(lp_rational_t* q, const lp_dyadic_rational_t* qd);

/**
 * Construct a copy of the given coefficient. The coefficient will be
 * normalized according to the given ring.
 */
void lp_rational_construct_copy(lp_rational_t* q, const lp_rational_t* from);

/**
 * Assign the rational a given rational.
 */
void lp_rational_assign(lp_rational_t* q, const lp_rational_t* from);

/**
 * Assign the rational a given a/b.
 */
void lp_rational_assign_int(lp_rational_t* q, long a, unsigned long b);

/**
 * Deallocates the rational.
 */
void lp_rational_destruct(lp_rational_t* q);

/**
 * Prints the rational to the given stream.
 */
int lp_rational_print(const lp_rational_t* c, FILE* out);

/**
 * Returns the string representation of the rational.
 */
char* lp_rational_to_string(const lp_rational_t* q);

/**
 * Return the double representation of the rational.
 */
double lp_rational_to_double(const lp_rational_t* q);

/**
 * Returns the sign of the rational.
 */
int lp_rational_sgn(const lp_rational_t* q);

/**
 * Compare the two rationals.
 */
int lp_rational_cmp(const lp_rational_t* q1, const lp_rational_t* q2);

/**
 * Compare to a dyadic rational.
 */
int lp_rational_cmp_dyadic_rational(const lp_rational_t* q1, const lp_dyadic_rational_t* q2);

/**
 * Compare to an integer.
 */
int lp_rational_cmp_integer(const lp_rational_t* q1, const lp_integer_t* q2);

/**
 * Swap two rationals.
 */
void lp_rational_swap(lp_rational_t* q1, lp_rational_t* q2);

/**
 * Compute sum = a + b.
 */
void lp_rational_add(lp_rational_t* sum, const lp_rational_t* a, const lp_rational_t* b);

/**
 * Compute sum = a + b.
 */
void lp_rational_add_integer(lp_rational_t* sum, const lp_rational_t* a, const lp_integer_t* b);

/**
 * Compute sub = a - b.
 */
void lp_rational_sub(lp_rational_t* sub, const lp_rational_t* a, const lp_rational_t* b);

/**
 * Compute neg = -a.
 */
void lp_rational_neg(lp_rational_t* neg, const lp_rational_t* a);

/**
 * Compute the inverse of a. Assumes a != 0.
 */
void lp_rational_inv(lp_rational_t* inv, const lp_rational_t* a);

/**
 * Compute product = a * b.
 */
void lp_rational_mul(lp_rational_t* mul, const lp_rational_t* a, const lp_rational_t* b);

/**
 * Compute product = a*2^n
 */
void lp_rational_mul_2exp(lp_rational_t* mul, const lp_rational_t* a, unsigned n);

/**
 * Compute power = a^n in the given ring.
 */
void lp_rational_pow(lp_rational_t* pow, const lp_rational_t* a, unsigned n);

/**
 * Compute a/b = div.
 */
void lp_rational_div(lp_rational_t* div, const lp_rational_t* a, const lp_rational_t* b);

/**
 * Compute div = a/2^n
 */
void lp_rational_div_2exp(lp_rational_t* div, const lp_rational_t* a, unsigned n);

