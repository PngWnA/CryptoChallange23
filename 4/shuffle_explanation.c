#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>
#include <xmmintrin.h>

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
    b = _mm256_set_epi32(15, 14, 13, 12, 11, 10, 9, 8);

    print(a);
    print(b);

    x = _mm256_shuffle_ps(a, b, 0x88);
    y = _mm256_shuffle_ps(a, b, 0xdd);
    print(x);
    print(y);

    u = _mm256_shuffle_ps(x, y, 0x44);
    v = _mm256_shuffle_ps(x, y, 0xee);
    print(u);
    print(v);
    
    x = _mm256_shuffle_epi32(u, 0xd8);
    y = _mm256_shuffle_epi32(v, 0xd8);
    print(x);
    print(y);

    // u = _mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0);
    // v = _mm256_set_epi32(15, 13, 11, 9, 7, 5, 3, 1);

    // print(u);
    // print(v);
}