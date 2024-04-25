#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Polynomial.
typedef struct {
  uint8_t deg;     // Degree of polynomial.
  uint8_t *coeff;  // Array of coefficients.
} poly_t;

/* Initialize a polynomial. */
poly_t *poly_from_array(uint8_t deg, uint8_t *coeff);

/* Destroy a given polynomial. */
void poly_destroy(poly_t *a);

/* Return true if the degree and corresponding coefficients match. */
bool poly_eq(const poly_t *a, const poly_t *b);

/* Return a zero polynomial of the given length.*/
poly_t *poly_create_zero(size_t len);

/* Set res = a + b, where a and b are polynomials over Fp. */
void poly_sum(poly_t *res, poly_t a, poly_t b, uint8_t p);

/* Set res = a mod b, where a and b are polynomials over Fp. */
void poly_div(poly_t *res, poly_t a, poly_t b, uint8_t p);

/* Calculate res = a * b. */
void poly_mul(poly_t *res, poly_t a, poly_t b, uint8_t p);

/* Calculate res = a^exp mod (I) */
void poly_fpowm(poly_t *res, poly_t a, uint64_t exp, poly_t I, uint8_t p);

/* Normalize the degree of the given polynomial. */
void poly_normalize_deg(poly_t *a);
