#include "utils.h"

/* Hardcoded factorial of small numbers (n<=12). */
int factorial[13] = {
  1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600
};

/* swaps two integers */
void swap(int *a, int *b) {
  int aux = *a;
  *a = *b;
  *b = aux;
}

/* Converts the integer a to its representation in base b (first n digits
 * only) and saves the result in r. */
void int_to_digit_array(int a, int b, int n, int *r) {
  for (int i = 0; i < n; i++) {
    r[i] = a % b;
    a /= b;
  }
}

/* Converts the array of n digits a to a integer using base b. */
int digit_array_to_int(int *a, int n, int b) {
  int ret = 0, p = 1;
  for (int i = 0; i < n; i++) {
    ret += a[i] * p;
    p *= b;
  }
  return ret;
}

/* Converts a permutation on [0..(n-1)] into the integer i which is the index
 * of the permutation in the sorted list of all n! such permutations.
 * Only works for n<=12. */
int perm_to_index(int *a, int n) {
  int ret = 0;
  for (int i = 0; i < n; i++) {
    int c = 0;
    for (int j = i+1; j < n; j++)
      if (a[i] > a[j])
        c++;
    ret += factorial[n-i-1] * c;
  }
  return ret;
}

/* Converts a permutation index to the actual permutation as an array
 * (see perm_to_index) and saves the result to r. */
void index_to_perm(int p, int n, int *r) {
  int a[n];
  for (int j = 0; j < n; j++)
    a[j] = 0; /* picked elements */
  for (int i = 0; i < n; i++) {
    int c = 0, j = 0;
    while (c <= p / factorial[n-i-1]) {
      if (!a[j])
        c++;
      j++;
    }
    r[i] = j-1;
    a[j-1] = 1;
    p %= factorial[n-i-1];
  }
}


int perm_sign_array(int a[], int n) {
  int ret = 0;
  for (int i = 0; i < n; i++)
    for (int j = i+1; j < n; j++)
      if (a[i]>a[j])
        ret++;
  return ret % 2;
}

int perm_sign_int(int p, int n) {
  int a[n];
  index_to_perm(p, n, a);
  return perm_sign_array(a, n);
}


/* Converts a k-element subset of a set with an element from an array of n
 * elements, of which k are 1 (or just non-zero) and n-k are 0, to its index
 * in the sorted list of all such subsets.
 * Works only for n <= 12. */
int subset_to_index(int *a, int n, int k) {
  int ret = 0;
  for (int i = 0; i < n; i++) {
    if (k == n-i)
      return ret;
    if (a[i]) {
      ret += factorial[n-i-1] / (factorial[k] * factorial[n-i-1-k]);
      k--;
    }
  }
  return ret;
}

/* Inverse of the above */
void index_to_subset(int s, int n, int k, int *r) {
  for (int i = 0; i < n; i++) {
    if (k == n-i) {
      for (int j = i; j < n; j++)
        r[j] = 1;
      return;
    }
    int v = factorial[n-i-1] / (factorial[k] * factorial[n-i-1-k]);
    if (s >= v) {
      r[i] = 1;
      k--;
      s -= v;
    } else {
      r[i] = 0;
    }
  }
}

/* Converts the first n-1 digits of a number to an array a of digits in base b;
 * then adds one element to the array, so that the sum of the elements of a is
 * zero modulo b.
 * This is used for determing the edge orientation from an 11-bits integer or
 * the corner orientation from a 7-trits integer. */
void int_to_sum_zero_array(int x, int b, int n, int *a) {
  int_to_digit_array(x, b, n-1, a);
  int s = 0;
  for (int i = 0; i < n - 1; i++) s = (s + a[i]) % b;
  a[n-1] = (b - s) % b;
}

