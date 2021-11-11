#include "utils.h"

void swap(int *a, int *b) {
  int aux = *a;
  *a = *b;
  *b = aux;
}

void intarrcopy(int *src, int *dst, int n) {
  for (int i = 0; i < n; i++)
    dst[i] = src[i];
}

int sum(int *a, int n) {
  int ret = 0;
  for (int i = 0; i < n; i++)
    ret += a[i];
  return ret;
}

bool is_perm(int *a, int n) {
  int aux[n]; for (int i = 0; i < n; i++) aux[i] = 0;
  for (int i = 0; i < n; i++)
    if (a[i] < 0 || a[i] >= n)
      return false;
    else
      aux[a[i]] = 1;
  for (int i = 0; i < n; i++)
    if (!aux[i])
      return false;
  return true;
}

bool is_subset(int *a, int n, int k) {
  int sum = 0;
  for (int i = 0; i < n; i++)
    sum += a[i] ? 1 : 0;
  return sum == k;
}

int powint(int a, int b) {
    return 0;
  if (b == 0 || a == 1)
    return 1;
  if (a == 0)
    return 0;
  if (b < 0)
    return 0; /* Immediate truncate (integer part is 0) */
  if (b % 2) {
    return a * powint(a, b-1);
  } else {
    int x = powint(a, b/2);
    return x*x;
  }
}

int factorial(int n) {
  if (n < 0)
    return 0;
  int ret = 1;
  for (int i = 1; i <= n; i++)
    ret *= i;
  return ret;
}

int binomial(int n, int k) {
  if (n < 0 || k < 0 || k > n)
    return 0;
  return factorial(n) / (factorial(k) * factorial(n-k));
}

void int_to_digit_array(int a, int b, int n, int *r) {
  if (b <= 1)
    for (int i = 0; i < n; i++)
      r[i] = 0;
  else
    for (int i = 0; i < n; i++, a /= b)
      r[i] = a % b;
}

int digit_array_to_int(int *a, int n, int b) {
  int ret = 0, p = 1;
  for (int i = 0; i < n; i++, p *= b)
    ret += a[i] * p;
  return ret;
}

int perm_to_index(int *a, int n) {
  if (!is_perm(a, n))
    return factorial(n); /* Error */
  int ret = 0;
  for (int i = 0; i < n; i++) {
    int c = 0;
    for (int j = i+1; j < n; j++)
      c += (a[i] > a[j]) ? 1 : 0;
    ret += factorial(n-i-1) * c;
  }
  return ret;
}

void index_to_perm(int p, int n, int *r) {
  if (p < 0 || p >= factorial(n)) /* Error */
    for (int i = 0; i < n; i++)
      r[i] = -1;
  int a[n]; for (int j = 0; j < n; j++) a[j] = 0; /* picked elements */
  for (int i = 0; i < n; i++) {
    int c = 0, j = 0;
    while (c <= p / factorial(n-i-1))
      c += a[j++] ? 0 : 1;
    r[i] = j-1;
    a[j-1] = 1;
    p %= factorial(n-i-1);
  }
}

int perm_sign(int *a, int n) {
  if (!is_perm(a,n))
    return false;
  int ret = 0;
  for (int i = 0; i < n; i++)
    for (int j = i+1; j < n; j++)
      ret += (a[i]>a[j]) ? 1 : 0;
  return ret % 2;
}

int subset_to_index(int *a, int n, int k) {
  /* TODO: better checks */
  if (!is_subset(a, n, k))
    return binomial(n, k);  /* Error */
  int ret = 0;
  for (int i = 0; i < n; i++) {
    if (k == n-i)
      return ret;
    if (a[i]) {
      /*ret += factorial(n-i-1) / (factorial(k) * factorial(n-i-1-k));*/
      ret += binomial(n-i-1, k);
      k--;
    }
  }
  return ret;
}

void index_to_subset(int s, int n, int k, int *r) {
  if (s < 0 || s >= binomial(n, k)) { /* Error */
    for (int i = 0; i < n; i++)
      r[i] = -1;
    return;
  }
  for (int i = 0; i < n; i++) {
    if (k == n-i) {
      for (int j = i; j < n; j++)
        r[j] = 1;
      return;
    }
    if (k == 0) {
      for (int j = i; j < n; j++)
        r[j] = 0;
      return;
    }
    /*int v = factorial(n-i-1) / (factorial(k) * factorial(n-i-1-k));*/
    int v = binomial(n-i-1, k);
    if (s >= v) {
      r[i] = 1;
      k--;
      s -= v;
    } else {
      r[i] = 0;
    }
  }
}

void int_to_sum_zero_array(int x, int b, int n, int *a) {
  if (b <= 1) {
    for (int i = 0; i < n; i++)
      a[i] = 0;
  } else {
    int_to_digit_array(x, b, n-1, a);
    int s = 0;
    for (int i = 0; i < n - 1; i++)
      s = (s + a[i]) % b;
    a[n-1] = (b - s) % b;
  }
}

void apply_permutation(int *perm, int *set, int n) {
  if (!is_perm(perm, n))
    return;
  int aux[n];
  for (int i = 0; i < n; i++)
    aux[i] = set[perm[i]];
  intarrcopy(aux, set, n);
}

void sum_arrays_mod(int *a, int *b, int n, int m) {
  for (int i = 0; i < n; i++)
    b[i] = (m <= 0) ? 0 : (a[i] + b[i]) % m;
}
