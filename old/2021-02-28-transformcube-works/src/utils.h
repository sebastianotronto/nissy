/* General utility functions */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

/* Some useful constants */
#define pow2to11    2048
#define pow2to12    4096
#define pow3to7     2187
#define pow3to8     6561
#define pow12to4    20736
#define factorial4  24
#define factorial6  720
#define factorial8  40320
#define factorial12 479001600
#define binom12on4  495
#define binom8on4   70

/* Generic utility functions */
void swap(int *a, int *b);
void intarrcopy(int *src, int *dst, int n);
int sum(int *a, int n);
bool is_perm(int *a, int n);
bool is_perm(int *a, int n);


/* Standard mathematical functions */
int powint(int a, int b);
int factorial(int n);
int binomial(int n, int k);

/* Converts the integer a to its representation in base b (first n digits
 * only) and saves the result in r. */
void int_to_digit_array(int a, int b, int n, int *r);
int digit_array_to_int(int *a, int n, int b);

/* Converts the first n-1 digits of a number to an array a of digits in base b;
 * then adds one element to the array, so that the sum of the elements of a is
 * zero modulo b.
 * This is used for determing the edge orientation from an 11-bits integer or
 * the corner orientation from a 7-trits integer. */
void int_to_sum_zero_array(int x, int b, int n, int *a);
 
/* Converts a permutation on [0..(n-1)] into the integer i which is the index
 * of the permutation in the sorted list of all n! such permutations. */
int perm_to_index(int *a, int n);
void index_to_perm(int p, int n, int *r);

/* Determine the sign of a permutation */
int perm_sign(int a[], int n);

/* Converts a k-element subset of a set from an array of n elements, of which k
 * are 1 and n-k are 0, to its index in the sorted list of all such subsets. */
int subset_to_index(int *a, int n, int k);
void index_to_subset(int s, int n, int k, int *r);

int ordered_subset_to_index(int *a, int n, int k);
void index_to_ordered_subset(int s, int n, int k, int *r);

void apply_permutation(int *perm, int *set, int n);

/* b[i] = (a[i]+b[i])%m for i=1,...,n */
void sum_arrays_mod(int *a, int *b, int n, int m);

#endif
