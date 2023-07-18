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
    uint32_t i = 0;
    uint32_t t1[8] = {0, 1, 2, 3, 4, 5, 6, 7}, t2[8] = {8, 9, 10, 11, 12, 13, 14, 15};

    a = _mm256_loadu_si256((__m256i*)t1);
    b = _mm256_loadu_si256((__m256i*)t2);

    print(a);
    print(b);
    printf("\n");

    x = _mm256_shuffle_ps(a, b, 0b10001000);
    y = _mm256_shuffle_ps(a, b, 0b11011101);
    print(x);
    print(y);
    printf("\n");

    u = _mm256_shuffle_ps(x, y, 0b01000100);
    v = _mm256_shuffle_ps(x, y, 0b11101110);
    print(u);
    print(v);
    printf("\n");

    x = _mm256_shuffle_epi32(u, 0b11011000);
    y = _mm256_shuffle_epi32(v, 0b11011000);
    print(x);
    print(y);
}