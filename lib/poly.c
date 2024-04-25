#include "poly.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

poly_t *poly_from_array(uint8_t deg, uint8_t *coeff) {
  if (!coeff) {
    return NULL;
  }
  // Assume deg < len(coeff).
  poly_t *poly = malloc(sizeof(*poly));
  uint8_t *tmp = malloc(sizeof(*coeff) * (deg + 1));
  if (!poly || !tmp) {
    free(poly);
    free(tmp);
    return NULL;
  }
  poly->deg = deg;
  memcpy(tmp, coeff, sizeof(*coeff) * (deg + 1));
  poly->coeff = tmp;
  return poly;
}

void poly_destroy(poly_t *poly) {
  if (poly) {
    free(poly->coeff);
    free(poly);
  }
}

bool poly_eq(const poly_t *a, const poly_t *b) {
  if (!a || !b || (a->deg != b->deg)) {
    return false;
  }
  return !memcmp(a->coeff, b->coeff, (a->deg + 1) * sizeof(*a->coeff));
}

poly_t *poly_create_zero(size_t len) {
  if (!len) {
    return NULL;
  }
  poly_t *res = malloc(sizeof(*res));
  uint8_t *tmp = malloc(sizeof(*tmp) * len);
  if (!res || !tmp) {
    free(res);
    free(tmp);
    return NULL;
  }
  res->deg = 0;
  res->coeff = memset(tmp, 0, sizeof(*tmp) * len);
  return res;
}

void poly_normalize_deg(poly_t *a) {
  if (!a) {
    return;
  }
  while ((a->deg > 0) && (a->coeff[a->deg] == 0)) {
    a->deg -= 1;
  }
}

// Set res = a + b, where a and b are polynomials over Fp.
void poly_sum(poly_t *res, poly_t a, poly_t b, uint8_t p) {
  if (!res) {
    return;
  }
  // Using tmp variable w allows a or b to be passed as res.
  uint8_t w;
  size_t max_deg = MAX(a.deg, b.deg);
  for (size_t i = 0; i <= max_deg; ++i) {
    w = 0;
    if (i <= a.deg) {
      w += a.coeff[i];
    }
    if (i <= b.deg) {
      w += b.coeff[i];
    }
    res->coeff[i] = w % p;
  }
  res->deg = max_deg;
  poly_normalize_deg(res);
}

// Calculate res = a mod b, where a and b are polynomials over Fp.
void poly_div(poly_t *res, poly_t a, poly_t b, uint8_t p) {
  if (!res) {
    return;
  }

  memcpy(res->coeff, a.coeff, sizeof(*a.coeff) * (a.deg + 1));
  res->deg = a.deg;

  if (res->deg < b.deg) {
    return;
  }

  // At this point a.deg >= b.deg
  res->deg = b.deg - 1;

  uint8_t n = a.deg;
  uint8_t m = b.deg;

  uint8_t *u = res->coeff;
  uint8_t *v = b.coeff;

  uint8_t q;
  uint8_t w;
  for (size_t k = (n - m) + 1; k > 0; --k) {
    q = u[(k - 1) + m] * inverse(v[m], p);
    for (size_t i = m + (k - 1) + 1; i > (k - 1); --i) {
      w = (q * v[(i - 1) - (k - 1)]) % p;
      w = complement(w, p);
      u[i - 1] = (u[i - 1] + w) % p;
    }
  }
  poly_normalize_deg(res);
}

// Set res = a * b mod p. Must be guaranteed res is niether a nor b.
void poly_mul(poly_t *res, poly_t a, poly_t b, uint8_t p) {
  if (!res) {
    return;
  }
  memset(res->coeff, 0, sizeof(*res->coeff) * (a.deg + b.deg + 1));
  for (size_t i = 0; i <= a.deg; ++i) {
    for (size_t j = 0; j <= b.deg; ++j) {
      res->coeff[i + j] = (res->coeff[i + j] + a.coeff[i] * b.coeff[j]) % p;
    }
  }
  res->deg = a.deg + b.deg;
}

void poly_fpowm(poly_t *res, poly_t a, uint64_t exp, poly_t I, uint8_t p) {
  if (!res) {
    return;
  }

  poly_t *base = poly_create_zero(I.deg + I.deg);
  memcpy(base->coeff, a.coeff, (a.deg + 1) * sizeof(*base->coeff));
  base->deg = a.deg;

  // Temporary buffer
  poly_t *buff = poly_create_zero(I.deg + I.deg);

  // Set prod equal to 1. Prod holds the result.
  poly_t *prod = poly_create_zero(I.deg + I.deg);
  *prod->coeff = 1;

  int8_t *tmp = NULL;
  while (exp > 0) {
    if ((exp % 2) != 0) {
      // Set buff = prod * base
      poly_mul(buff, *prod, *base, p);
      poly_div(buff, *buff, I, p);
      // Swap buff and prod.
      tmp = prod->coeff;
      prod->coeff = buff->coeff;
      prod->deg = buff->deg;
      buff->coeff = tmp;
    }
    // Set buff = base * base;
    poly_mul(buff, *base, *base, p);
    poly_div(buff, *buff, I, p);
    exp = exp / 2;
    // Swap buff and base.
    tmp = base->coeff;
    base->coeff = buff->coeff;
    base->deg = buff->deg;
    buff->coeff = tmp;
  }

  memcpy(res->coeff, prod->coeff, sizeof(*prod->coeff) * (prod->deg + 1));
  res->deg = prod->deg;

  // Clean up.
  poly_destroy(prod);
  poly_destroy(buff);
  poly_destroy(base);
}
