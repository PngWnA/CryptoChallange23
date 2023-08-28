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

#define SCALAR(x) _mm256_set1_epi32(x)
#define SHUFFLE_2(x, y, i) ((__m256i)_mm256_shuffle_ps(x, y, i))
#define SHUFFLE32(x, i) _mm256_shuffle_epi32(x, i)
#define _ROL(x, r) OR(SHIFT_L(x, r), SHIFT_R(x, 32 - r))
#define _ROR(x, r) OR(SHIFT_R(x, r), SHIFT_L(x, 32 - r))

int64_t cpucycles(void) {
    unsigned int hi, lo;
    __asm__ __volatile__("rdtsc\n\t"
                         : "=a"(lo), "=d"(hi));
    return ((int64_t)lo) | (((int64_t)hi) << 32);
}

void AVX2_cipher(uint32_t* master_key, uint32_t* input, uint32_t* output) {
    uint32_t i = 0;
    __m256i k1, k2, pt1, pt2, t1, t2;

    // Load Key
    t1 = LOAD( master_key   );
    t2 = LOAD(&master_key[8]);

    // Shuffle s.t. operands are aligned to be processed together
    // k1: first element of each block
    // k2: second element of each block
    k1 = SHUFFLE_2(t1, t2, 0x88);
    k2 = SHUFFLE_2(t1, t2, 0xdd);

    // Load Plaintext
    t1 = LOAD( input );
    t2 = LOAD(&input[8]);

    // Shuffle s.t. operands are aligned to be processed together
    // k1: first element of each block
    // k2: second element of each block
    pt1 = SHUFFLE_2(t1, t2, 0x88);
    pt2 = SHUFFLE_2(t1, t2, 0xdd);

    for (; i < NUM_ROUND; i++) {
        // Key Scheduler
        k1 = XOR(ADD(_ROR(k1, 8), k2), SCALAR(i));
        k2 = XOR(k1, _ROL(k2, 3));

        // Block Cipher
        pt2 = XOR(k2, XOR(pt2, XOR(_ROL(pt1, 2), AND(_ROL(pt1, 1), _ROL(pt1, 8)))));

        t1 = pt1;
        pt1 = pt2;
        pt2 = t1;
    }

    // Shuffle s.t. elements have same position as one right after Load
    // Store Ciphertext
    STORE( output   , SHUFFLE32(SHUFFLE_2(pt1, pt2, 0x44), 0xd8));
    STORE(&output[8], SHUFFLE32(SHUFFLE_2(pt1, pt2, 0xee), 0xd8));
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
    for (i = 0; i < BLOCK_SIZE; i += 8) {
        AVX2_cipher(key_AVX[i], input_AVX[i], output_AVX[i]);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    cycles2 = cpucycles();
    kcycles = cycles2 - cycles1;
    printf("%lld\n", kcycles / BLOCK_SIZE);
}
