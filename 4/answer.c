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

// example: AVX2 functions; freely remove this code and write what you want in here!
#define INLINE inline __attribute__((always_inline))

#define LOAD(x) _mm256_loadu_si256((__m256i*)x)
#define STORE(x, y) _mm256_storeu_si256((__m256i*)x, y)
#define XOR(x, y) _mm256_xor_si256(x, y)
#define OR(x, y) _mm256_or_si256(x, y)
#define AND(x, y) _mm256_and_si256(x, y)
#define SHUFFLE8(x, y) _mm256_shuffle_epi8(x, y)
#define ADD(x, y) _mm256_add_epi32(x, y)
#define SHIFT_L(x, r) _mm256_slli_epi32(x, r)
#define SHIFT_R(x, r) _mm256_srli_epi32(x, r)

int64_t cpucycles(void) {
    unsigned int hi, lo;
    __asm__ __volatile__("rdtsc\n\t"
                         : "=a"(lo), "=d"(hi));
    return ((int64_t)lo) | (((int64_t)hi) << 32);
}

// 64-bit data
// 64-bit key
// 32-bit x 22 rounds session key
void new_key_gen(uint32_t* master_key, uint32_t* session_key) {
    uint32_t i = 0;
    uint32_t k1, k2;

    k1 = master_key[0];
    k2 = master_key[1];

    for (i = 0; i < NUM_ROUND; i++) {
        k1 = ROR(k1, 8);
        k1 = k1 + k2;
        k1 = k1 ^ i;
        k2 = ROL(k2, 3);
        k2 = k1 ^ k2;
        session_key[i] = k2;
    }
}

void new_block_cipher(uint32_t* input, uint32_t* session_key, uint32_t* output) {
    uint32_t i = 0;
    uint32_t pt1, pt2, tmp1, tmp2;

    pt1 = input[0];
    pt2 = input[1];

    for (i = 0; i < NUM_ROUND; i++) {
        tmp1 = ROL(pt1, 1);
        tmp2 = ROL(pt1, 8);
        tmp2 = tmp1 & tmp2;
        tmp1 = ROL(pt1, 2);
        tmp2 = tmp1 ^ tmp2;
        pt2 = pt2 ^ tmp2;
        pt2 = pt2 ^ session_key[i];

        tmp1 = pt1;
        pt1 = pt2;
        pt2 = tmp1;
    }

    output[0] = pt1;
    output[1] = pt2;
}

///////////////////////////////////////////////////////////////////////////////////////////
#define _ROR(x, r) OR(SHIFT_R(x, r), SHIFT_L(x, 32 - r))
#define _ROL(x, r) OR(SHIFT_L(x, r), SHIFT_R(x, 32 - r))
#define F(x) XOR(AND(_ROL(x, 1), _ROL(x, 8)), _ROL(x, 2))

void new_key_gen_AVX2(uint32_t* master_key, uint32_t* session_key) {
    uint32_t i = 0, x[8];
    __m256i k1, k2;

    k1 = _mm256_set_epi32(master_key[14], master_key[12], master_key[10], master_key[8], master_key[6], master_key[4], master_key[2], master_key[0]);
    k2 = _mm256_set_epi32(master_key[15], master_key[13], master_key[11], master_key[9], master_key[7], master_key[5], master_key[3], master_key[1]);

    for (i = 0; i < NUM_ROUND; i++) {
        k1 = XOR(ADD(_ROR(k1, 8), k2), _mm256_set1_epi32(i));
        k2 = XOR(k1, _ROL(k2, 3));

        STORE(x, k2);
        session_key[i] = x[0];
        session_key[80 + i] = x[1];
        session_key[160 + i] = x[2];
        session_key[240 + i] = x[3];
        session_key[320 + i] = x[4];
        session_key[400 + i] = x[5];
        session_key[480 + i] = x[6];
        session_key[560 + i] = x[7];
    }
}

// check input_length --> multiple of 8 * 64-bit
void new_block_cipher_AVX2(uint32_t* input, uint32_t* session_key, uint32_t* output) {
    uint32_t i = 0, x1[8], x2[8];
    __m256i pt1, pt2, t;

    pt1 = _mm256_set_epi32(input[14], input[12], input[10], input[8], input[6], input[4], input[2], input[0]);
    pt2 = _mm256_set_epi32(input[15], input[13], input[11], input[9], input[7], input[5], input[3], input[1]);

    for (i = 0; i < NUM_ROUND; i++) {
        t = XOR(_mm256_set_epi32(session_key[560 + i],
                                    session_key[480 + i],
                                    session_key[400 + i],
                                    session_key[320 + i],
                                    session_key[240 + i],
                                    session_key[160 + i],
                                    session_key[80 + i],
                                    session_key[i]),
                   XOR(XOR(_ROL(pt1, 2), AND(_ROL(pt1, 1), _ROL(pt1, 8))), pt2));
        pt2 = pt1;
        pt1 = t;
    }

    STORE(x1, pt1);
    STORE(x2, pt2);

    output[0] = x1[0];
    output[2] = x1[1];
    output[4] = x1[2];
    output[6] = x1[3];

    output[8] = x1[4];
    output[10] = x1[5];
    output[12] = x1[6];
    output[14] = x1[7];

    output[1] = x2[0];
    output[3] = x2[1];
    output[5] = x2[2];
    output[7] = x2[3];

    output[9] = x2[4];
    output[11] = x2[5];
    output[13] = x2[6];
    output[15] = x2[7];
}
///////////////////////////////////////////////////////////////////////////////////////////

int main() {
    long long int kcycles, ecycles, dcycles;
    long long int cycles1, cycles2;
    int32_t i, j;

    // C implementation
    uint32_t input_C[BLOCK_SIZE][P_K_SIZE] = {
        0,
    };
    uint32_t key_C[BLOCK_SIZE][P_K_SIZE] = {
        0,
    };
    uint32_t session_key_C[BLOCK_SIZE][SESSION_KEY_SIZE] = {
        0,
    };
    uint32_t output_C[BLOCK_SIZE][P_K_SIZE] = {
        0,
    };

    // AVX implementation
    uint32_t input_AVX[BLOCK_SIZE][P_K_SIZE] = {
        0,
    };
    uint32_t key_AVX[BLOCK_SIZE][P_K_SIZE] = {
        0,
    };
    uint32_t session_key_AVX[BLOCK_SIZE][SESSION_KEY_SIZE] = {
        0,
    };
    uint32_t output_AVX[BLOCK_SIZE][P_K_SIZE] = {
        0,
    };

    // random generation for plaintext and key.
    srand(0);

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < P_K_SIZE; j++) {
            input_AVX[i][j] = input_C[i][j] = rand();
            key_AVX[i][j] = key_C[i][j] = rand();
        }
    }

    // execution of C implementation
    kcycles = 0;
    cycles1 = cpucycles();
    for (i = 0; i < BLOCK_SIZE; i++) {
        new_key_gen(key_C[i], session_key_C[i]);
        new_block_cipher(input_C[i], session_key_C[i], output_C[i]);
    }
    cycles2 = cpucycles();
    kcycles = cycles2 - cycles1;
    printf("C implementation runs in ................. %8lld cycles", kcycles / BLOCK_SIZE);
    printf("\n");

    // KAT and Benchmark test of AVX implementation
    kcycles = 0;
    cycles1 = cpucycles();
    ///////////////////////////////////////////////////////////////////////////////////////////
    // These functions (new_key_gen, new_block_cipher) should be replaced to "new_key_gen_AVX2" and "new_block_cipher_AVX2".
    for (i = 0; i < BLOCK_SIZE; i += 8) {
        new_key_gen_AVX2(key_AVX[i], session_key_AVX[i]);
        new_block_cipher_AVX2(input_AVX[i], session_key_AVX[i], output_AVX[i]);  // this is for testing

    }
    ///////////////////////////////////////////////////////////////////////////////////////////

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < P_K_SIZE; j++) {
            if (output_C[i][j] != output_AVX[i][j]) {
                printf("Test failed!!!\n");
                return 0;
            }
        }
    }
    cycles2 = cpucycles();
    kcycles = cycles2 - cycles1;
    printf("AVX implementation runs in ................. %8lld cycles", kcycles / BLOCK_SIZE);
    printf("\n");
}
