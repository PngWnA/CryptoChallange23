#include <emmintrin.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>
#include <xmmintrin.h>

// round of block cipher
#define NUM_ROUND 80

// size of plaintext and key size
#define BLOCK_SIZE 512
#define P_K_SIZE 2
#define SESSION_KEY_SIZE NUM_ROUND

// basic operation
#define ROR(x, r) ((x >> r) | (x << (32 - r)))
#define ROL(x, r) ((x << r) | (x >> (32 - r)))

int64_t cpucycles(void) {
    unsigned int hi, lo;
    __asm__ __volatile__("rdtsc\n\t"
                         : "=a"(lo), "=d"(hi));
    return ((int64_t)lo) | (((int64_t)hi) << 32);
}

void AVX2_cipher(uint32_t* master_key, uint32_t* input, uint32_t* output) {
    uint32_t i = 0;
    uint32_t k1, k2, tmp;
    uint32_t pt1, pt2, tmp1, tmp2;

    pt1 = input[0];
    pt2 = input[1];

    k1 = master_key[0];
    k2 = master_key[1];

    for (i = 0; i < NUM_ROUND; i++) {
        k1 = ROR(k1, 8);
        k1 = k1 + k2;
        k1 = k1 ^ i;
        k2 = ROL(k2, 3);
        k2 = k1 ^ k2;

        tmp1 = ROL(pt1, 1);
        tmp2 = ROL(pt1, 8);
        tmp2 = tmp1 & tmp2;
        tmp1 = ROL(pt1, 2);
        tmp2 = tmp1 ^ tmp2;
        pt2 = pt2 ^ tmp2;
        pt2 = pt2 ^ k2;

        tmp1 = pt1;
        pt1 = pt2;
        pt2 = tmp1;
    }

    output[0] = pt1;
    output[1] = pt2;
}

int main() {
    long long int kcycles, ecycles, dcycles;
    long long int cycles1, cycles2;
    int32_t i, j;

    // AVX implementation
    uint32_t input_AVX[BLOCK_SIZE][P_K_SIZE] = {0,};
    uint32_t key_AVX[BLOCK_SIZE][P_K_SIZE] = {0,};
    uint32_t session_key_AVX[BLOCK_SIZE][SESSION_KEY_SIZE] = {0,};
    uint32_t output_AVX[BLOCK_SIZE][P_K_SIZE] = {0,};

    // random generation for plaintext and key.
    srand(0);

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < P_K_SIZE; j++) {
            input_AVX[i][j] = rand();
            key_AVX[i][j] = rand();
        }
    }

    // KAT and Benchmark test of AVX implementation
    kcycles = 0;
    cycles1 = cpucycles();
    ///////////////////////////////////////////////////////////////////////////////////////////
    for (i = 0; i < BLOCK_SIZE; i++) {
        AVX2_cipher(key_AVX[i], input_AVX[i], output_AVX[i]);  // this is for testing
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    cycles2 = cpucycles();
    kcycles = cycles2 - cycles1;
    printf("%lld\n", kcycles / BLOCK_SIZE);
}
