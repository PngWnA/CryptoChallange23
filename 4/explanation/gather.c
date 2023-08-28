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
    uint32_t t3[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint32_t t4[16] = { 0, };
    uint32_t t5[8] = { 0, };
    uint32_t t6[8] = { 0, };

    a = _mm256_loadu_si256((__m256i *)t1);
    b = _mm256_loadu_si256((__m256i *)t2);

    print(a);
    print(b);
    printf("\n");

    x = _mm256_i32gather_epi32(t3, _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), 8);
    y = _mm256_i32gather_epi32(&t3[1], _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), 8);
    print(x);
    print(y);
    printf("\n");

    _mm256_storeu_si256((__m256i *) t5, x);
    _mm256_storeu_si256((__m256i *) t6, y);
    for (int i = 0; i < 16; i += 2) t4[i] = t5[i >> 1];
    for (int i = 1; i < 16; i += 2) t4[i] = t6[i >> 1];

    for (int i = 0; i < 16; i++) printf("%d ", t4[i]);
}
