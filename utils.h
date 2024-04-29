#pragma once

#include <stdint.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))

// Return complement(a) : a + complement(a) = 0 mod p.
uint8_t complement(uint8_t a, uint8_t p);

int8_t inverse(int8_t a, int8_t p);

// Fast base to the exponent.
uint64_t fpow(uint8_t base, uint8_t exp);
