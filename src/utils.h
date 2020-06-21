#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define abs(a)   (((a) > 0)   ? (a) : (-(a)))

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

void swap(int *a, int *b);

/* Hardcoded factorial of small numbers (n<=12). */
extern int factorial[13];

/* Converts the integer a to its representation in base b (first n digits
 * only) and saves the result in r. */
void int_to_digit_array(int a, int b, int n, int *r);
 
/* Converts the array of n digits a to a integer using base b. */
int digit_array_to_int(int *a, int n, int b);
 
/* Converts a permutation on [0..(n-1)] into the integer i which is the index
 * of the permutation in the sorted list of all n! such permutations.
 * Only works for n<=12. */
int perm_to_index(int *a, int n);

/* Converts a permutation index to the actual permutation as an array
 * (see perm_to_index) and saves the result to r. */
void index_to_perm(int p, int n, int *r);

/* Converts a k-element subset of a set with an element from an array of n
 * elements, of which k are 1 and n-k are 0, to its index in the sorted list
 * of all such subsets.
 * Works only for n <= 12. */
int subset_to_index(int *a, int n, int k);
 
/* Inverse of the above */
void index_to_subset(int s, int n, int k, int *r);

/* Converts the first n-1 digits of a number to an array a of digits in base b;
 * then adds one element to the array, so that the sum of the elements of a is
 * zero modulo b.
 * This is used for determing the edge orientation from an 11-bits integer or
 * the corner orientation from a 7-trits integer. */
void int_to_sum_zero_array(int x, int b, int n, int *a);

