#pragma once

#include <stddef.h>
#include <stdint.h>

#include "poly.h"

// Galois field.
typedef struct {
  uint8_t p;  // Characteristic of the field GF(p).
  poly_t *I;  // Irreducible polynomial over GF(p)[x].
} GF_t;

// Element of the Galois field.
typedef struct {
  GF_t *GF;      // Galois field.
  poly_t *poly;  // Element of the GF(p)[x]/(I).
} GF_elem_t;

// x^8 + x^4 + x^3 + x^2 + 1
extern GF_t GF2_8;

// x^16 + x^9 + x^8 + x^7 + x^6 + x^4 + x^3 + x^2 + 1
extern GF_t GF2_16;

// x^32 + x^22 + x^2 + x^1 + 1
extern GF_t GF2_32;

/* Initialize GF(P)[x]/(I) */
GF_t *GF_init_field(uint8_t p, poly_t I);

void GF_destroy_field(GF_t *GF);

/* Destroy a given element of the Galois Field.
   The field itself is left untouched. */
void GF_elem_destroy(GF_elem_t *a);

/* Given an array of polynomial coefficients of any length and GF(p)[x]/(I),
   Return an element over that field. */
GF_elem_t *GF_elem_from_array(uint8_t deg, uint8_t *coeff, GF_t *GF);

/* Convert to and from uint8, uint16, uint32 to the element of the GF(2). */
GF_elem_t *GF_elem_from_uint8(uint8_t x);

GF_elem_t *GF_elem_from_uint16(uint16_t x);

GF_elem_t *GF_elem_from_uint32(uint32_t x);

uint8_t GF_elem_to_uint8(GF_elem_t *a);

uint16_t GF_elem_to_uint16(GF_elem_t *a);

uint32_t GF_elem_to_uint32(GF_elem_t *a);

/* Return 1 if the given fields are equal.
   Meaning they have the same characteristic and irreducible polynomials. */
bool GF_eq(const GF_t *F, const GF_t *K);

/* res = a + b mod (I). */
void GF_elem_sum(GF_elem_t *res, GF_elem_t a, GF_elem_t b);

/* res = a - b mod (I). */
void GF_elem_diff(GF_elem_t *res, GF_elem_t a, GF_elem_t b);

/* res = a * b mod (I). */
void GF_elem_prod(GF_elem_t *res, GF_elem_t a, GF_elem_t b);

/* Calculate res: a = b * res mod (I). */
void GF_elem_div(GF_elem_t *res, GF_elem_t a, GF_elem_t b);

/* Return res = -a mod p. */
GF_elem_t *GF_elem_get_complement(GF_elem_t a);

/* Calculate res: res * a = 1 mod (I). */
GF_elem_t *GF_elem_get_inverse(GF_elem_t a);

/* Return neutral element of the given finite field. */
GF_elem_t *GF_elem_get_neutral(GF_t *GF);

/* Return unity element of the given finite field. */
GF_elem_t *GF_elem_get_unity(GF_t *GF);

