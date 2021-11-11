#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>

/* Constants and macros *****************************************************/

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

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

/* Generic utility functions *************************************************/

void apply_permutation(int *perm, int *set, int n);
void intarrcopy(int *src, int *dst, int n);
bool is_perm(int *a, int n);
bool is_subset(int *a, int n, int k);
int  sum(int *a, int n);
void sum_arrays_mod(int *src, int *dst, int n, int m);
void swap(int *a, int *b);

/* Standard mathematical functions *******************************************/

int binomial(int n, int k);
int factorial(int n);
int perm_sign(int a[], int n);
int powint(int a, int b);

/* Conversions to and from int (base b digits, permutations...) **************/

int  digit_array_to_int(int *a, int n, int b);
void int_to_digit_array(int a, int b, int n, int *r);
void int_to_sum_zero_array(int x, int b, int n, int *a);
int  invert_digits(int a, int b, int n);

void index_to_perm(int p, int n, int *r);
int  perm_to_index(int *a, int n);

void index_to_subset(int s, int n, int k, int *r);
int  subset_to_index(int *a, int n, int k);

/* Am I not using these two?
void index_to_ordered_subset(int s, int n, int k, int *r);
int  ordered_subset_to_index(int *a, int n, int k);
*/

#endif
