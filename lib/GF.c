#include "GF.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "poly.h"
#include "utils.h"

// x^8 + x^4 + x^3 + x^2 + 1
uint8_t IGF2_8_coeff[9] = {1, 0, 1, 1, 1, 0, 0, 0, 1};
poly_t IGF2_8 = {.deg = 8, .coeff = IGF2_8_coeff};
GF_t GF2_8 = {.p = 2, .I = &IGF2_8};

// x^16 + x^9 + x^8 + x^7 + x^6 + x^4 + x^3 + x^2 + 1
uint8_t IGF2_16_coeff[17] = {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1};
poly_t IGF2_16 = {.deg = 16, .coeff = IGF2_16_coeff};
GF_t GF2_16 = {.p = 2, .I = &IGF2_16};

// x^32 + x^22 + x^2 + x^1 + 1
uint8_t IGF2_32_coeff[33] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
poly_t IGF2_32 = {.deg = 32, .coeff = IGF2_32_coeff};
GF_t GF2_32 = {.p = 2, .I = &IGF2_32};

GF_t *GF_init_field(uint8_t p, poly_t I) {
  GF_t *GF = malloc(sizeof(*GF));
  poly_t *a = poly_from_array(I.deg, I.coeff);
  if (!GF || !a) {
    free(GF);
    poly_destroy(a);
    return NULL;
  }
  GF->p = p;
  GF->I = a;
  return GF;
}

void GF_destroy_field(GF_t *GF) {
  if (GF) {
    poly_destroy(GF->I);
    free(GF);
  }
}

void GF_elem_destroy(GF_elem_t *a) {
  if (a) {
    poly_destroy(a->poly);
    free(a);
  }
}

bool GF_eq(const GF_t *F, const GF_t *K) {
  // Irreducible polynomials must match.
  bool ret = poly_eq(F->I, K->I);
  // Characteristics of fields and dimensions of extensions must match.
  if (F->p != K->p) {
    ret = false;
  }
  return ret;
}

GF_elem_t *GF_elem_from_array(uint8_t deg, uint8_t *coeff, GF_t *GF) {
  if (!coeff || (!deg) || !GF) {
    return NULL;
  }
  if (!GF->I || !GF->I->coeff || (GF->I->deg < 2)) {
    return NULL;
  }

  GF_elem_t *a = malloc(sizeof(*a));
  poly_t *poly = poly_from_array(deg, coeff);
  if (!a || !poly) {
    free(a);
    poly_destroy(poly);
    return NULL;
  }

  // Set coefficients mod p and normalize degree.
  for (size_t i = 0; i <= poly->deg; ++i) {
    poly->coeff[i] %= GF->p;
  }
  poly_normalize_deg(poly);

  // Redduce polynomial over GF(p)[X]/(I).
  if (poly->deg >= GF->I->deg) {
    poly_div(poly, *poly, *GF->I, GF->p);
  }

  uint8_t *buff = calloc(GF->I->deg, sizeof(*coeff));
  memcpy(buff, poly->coeff, sizeof(*poly->coeff) * (poly->deg + 1));
  free(poly->coeff);

  poly->coeff = buff;
  a->GF = GF;
  a->poly = poly;
  return a;
}

GF_elem_t *GF_elem_get_neutral(GF_t *GF) {
  if (!GF) {
    return NULL;
  }
  GF_elem_t *neutral = malloc(sizeof(*neutral));
  poly_t *poly = poly_create_zero(GF->I->deg);
  if (!neutral || !poly) {
    free(neutral);
    poly_destroy(poly);
    return NULL;
  }
  neutral->GF = GF;
  neutral->poly = poly;
  return neutral;
}

GF_elem_t *GF_elem_get_unity(GF_t *GF) {
  /* Get neutral and set the least significant digit to one. */
  GF_elem_t *unity = GF_elem_get_neutral(GF);
  if (!unity || !GF) {
    GF_elem_destroy(unity);
    return NULL;
  }
  *unity->poly->coeff = 1;
  return unity;
}

GF_elem_t *GF_elem_get_complement(GF_elem_t a) {
  GF_elem_t *res = GF_elem_get_neutral(a.GF);
  if (!res) {
    return NULL;
  }
  for (size_t i = 0; i < a.GF->I->deg; ++i) {
    res->poly->coeff[i] = complement(a.poly->coeff[i], a.GF->p);
  }
  res->poly->deg = a.poly->deg;
  return res;
}

GF_elem_t *GF_elem_get_inverse(GF_elem_t a) {
  if ((a.poly->deg == 0) && (*a.poly->coeff == 0)) {
    return NULL;
  }
  uint64_t mul_group_ord = fpow(a.GF->p, a.GF->I->deg) - 2;
  GF_elem_t *res = GF_elem_get_neutral(a.GF);
  if (!res) {
    return NULL;
  }
  poly_fpowm(res->poly, *a.poly, mul_group_ord, *a.GF->I, a.GF->p);
  return res;
}

GF_elem_t *GF_elem_from_uint8(uint8_t x) {
  GF_elem_t *res = GF_elem_get_neutral(&GF2_8);
  uint8_t d;
  uint8_t deg = 0;
  size_t i = 0;
  while (x > 0) {
    d = x % 2;
    if (d == 1) {
      deg++;
    }
    res->poly->coeff[i] = d;
    x /= 2;
    i++;
  }
  return res;
}

uint8_t GF_elem_to_uint8(GF_elem_t *a) {
  uint8_t res = 0;
  uint8_t factor = 1;
  for (size_t i = 0; i < a->GF->I->deg; ++i) {
    res += factor * a->poly->coeff[i];
    factor *= 2;
  }
  return res;
}

GF_elem_t *GF_elem_from_uint16(uint16_t x) {
  GF_elem_t *res = GF_elem_get_neutral(&GF2_16);
  uint8_t d;
  uint8_t deg = 0;
  size_t i = 0;
  while (x > 0) {
    d = x % 2;
    if (d == 1) {
      deg++;
    }
    res->poly->coeff[i] = d;
    x /= 2;
    i++;
  }
  return res;
}

uint16_t GF_elem_to_uint16(GF_elem_t *a) {
  uint8_t res = 0;
  uint8_t factor = 1;
  for (size_t i = 0; i < a->GF->I->deg; ++i) {
    res += factor * a->poly->coeff[i];
    factor *= 2;
  }
  return res;
}

GF_elem_t *GF_elem_from_uint32(uint32_t x) {
  GF_elem_t *res = GF_elem_get_neutral(&GF2_32);
  uint8_t d;
  uint8_t deg = 0;
  size_t i = 0;
  while (x > 0) {
    d = x % 2;
    if (d == 1) {
      deg++;
    }
    res->poly->coeff[i] = d;
    x /= 2;
    i++;
  }
  return res;
}

uint32_t GF_elem_to_uint32(GF_elem_t *a) {
  uint8_t res = 0;
  uint8_t factor = 1;
  for (size_t i = 0; i < a->GF->I->deg; ++i) {
    res += factor * a->poly->coeff[i];
    factor *= 2;
  }
  return res;
}

void GF_elem_sum(GF_elem_t *res, GF_elem_t a, GF_elem_t b) {
  if (!res) {
    return;
  }
  if (!GF_eq(res->GF, a.GF) && !GF_eq(res->GF, b.GF)) {
    return;
  }
  poly_sum(res->poly, *a.poly, *b.poly, res->GF->p);
}

void GF_elem_prod(GF_elem_t *res, GF_elem_t a, GF_elem_t b) {
  if (!res) {
    return;
  }

  // Different fields.
  if (!GF_eq(res->GF, a.GF) && !GF_eq(res->GF, b.GF)) {
    return;
  }

  poly_t *tmp = poly_create_zero(a.poly->deg + b.poly->deg + 1);
  if (!tmp) {
    return;
  }

  poly_mul(tmp, *a.poly, *b.poly, res->GF->p);
  poly_div(tmp, *tmp, *res->GF->I, res->GF->p);

  memcpy(res->poly->coeff, tmp->coeff, sizeof(*tmp->coeff) * (tmp->deg + 1));
  res->poly->deg = tmp->deg;

  poly_destroy(tmp);
}

void GF_elem_div(GF_elem_t *res, GF_elem_t a, GF_elem_t b) {
  if (!res || ((b.poly->deg == 0) && (*b.poly->coeff == 0))) {
    return;
  }
  if (!GF_eq(res->GF, a.GF) && !GF_eq(res->GF, b.GF)) {
    return;
  }
  GF_elem_t *inv_b = GF_elem_get_inverse(b);
  if (!inv_b) {
    return;
  }
  GF_elem_prod(res, a, *inv_b);
  GF_elem_destroy(inv_b);
}

void GF_elem_diff(GF_elem_t *res, GF_elem_t a, GF_elem_t b) {
  if (!res) {
    return;
  }
  GF_elem_t *negb = GF_elem_get_complement(b);
  GF_elem_sum(res, a, *negb);
  GF_elem_destroy(negb);
}
