#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>
#include <xmmintrin.h>

#define _L8 _mm256_set_epi32(0x0e0d0c0f, 0x0a09080b, 0x06050407, 0x02010003, 0x0e0d0c0f, 0x0a09080b, 0x06050407, 0x02010003)
#define _R8 _mm256_set_epi32(0x0c0f0e0d, 0x080b0a09, 0x04070605, 0x00030201, 0x0c0f0e0d, 0x080b0a09, 0x04070605, 0x00030201)
#define _ROL8(x) _mm256_shuffle_epi8(x, _L8)
#define _ROR8(x) _mm256_shuffle_epi8(x, _R8)
#define I   _mm256_set_epi32(0x0f0e0d0c, 0x0b0a0908, 0x07060504, 0x03020100, 0x0f0e0d0c, 0x0b0a0908, 0x07060504, 0x03020100)

void print(__m256i x) {
    uint32_t i, t[8];
    _mm256_storeu_si256((__m256i_u *)t, x);
    for (i = 0; i < 8; i++) printf("%u ", t[i]);
    printf("\n");
}

int main() {
    __m256i a, b, x, y, u, v;
    uint32_t i = 0, t1[8], t2[8];
    a = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);

    print(a);

    a = _ROL8(a);

    print(a);
}
