#include <stdio.h> 
#include <string.h> 
#include <stdint.h> 
#include <xmmintrin.h> 
#include <emmintrin.h> 
#include <immintrin.h> 
#include <x86intrin.h>
#include <stdlib.h> 
#include <time.h>

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

int64_t cpucycles(void) {
    unsigned int hi, lo;
    __asm__ __volatile__("rdtsc\n\t"
                         : "=a"(lo), "=d"(hi));
    return ((int64_t)lo) | (((int64_t)hi) << 32);
}

uint32_t _i;
__m256i _k1, _k2, _pt1, _pt2, _t1, _t2;
__m256i r8, l8;

void AVX2_cipher(uint32_t* master_key, uint32_t* input, uint32_t* output) {
    // Load Key
    _t1 = LOAD( master_key   );
    _t2 = LOAD(&master_key[8]);

    // Shuffle s.t. operands are aligned to be processed together
    // k1: first element of each block
    // k2: second element of each block
    _k1 = SHUFFLE_2(_t1, _t2, 0x88);
    _k2 = SHUFFLE_2(_t1, _t2, 0xdd);

    // Load Plaintext
    _t1 = LOAD( input );
    _t2 = LOAD(&input[8]);

    // Shuffle s.t. operands are aligned to be processed together
    // k1: first element of each block
    // k2: second element of each block
    _pt1 = SHUFFLE_2(_t1, _t2, 0x88);
    _pt2 = SHUFFLE_2(_t1, _t2, 0xdd);

    for (_i = 0; _i < NUM_ROUND; _i += 2) {
        // Key Scheduler
        _k1 = XOR(ADD(SHUFFLE8(_k1, r8), _k2), SCALAR(_i));
        _k2 = XOR(_k1, _ROL(_k2, 3));

        // Block Cipher
        _pt2 = XOR(_k2, XOR(_pt2, XOR(_ROL(_pt1, 2), AND(_ROL(_pt1, 1), SHUFFLE8(_pt1, l8)))));

         // Key Scheduler
        _k1 = XOR(ADD(SHUFFLE8(_k1, r8), _k2), SCALAR(_i | 1));
        _k2 = XOR(_k1, _ROL(_k2, 3));

        // Block Cipher
        _pt1 = XOR(_k2, XOR(_pt1, XOR(_ROL(_pt2, 2), AND(_ROL(_pt2, 1), SHUFFLE8(_pt2, l8)))));
    }

    // Shuffle s.t. elements have same position as one right after Load
    // Store Ciphertext
    STORE( output   , SHUFFLE32(SHUFFLE_2(_pt1, _pt2, 0x44), 0xd8));
    STORE(&output[8], SHUFFLE32(SHUFFLE_2(_pt1, _pt2, 0xee), 0xd8));
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
    r8 = _mm256_set_epi32(
        0x0c0f0e0d, 0x080b0a09, 0x04070605, 0x00030201, 
        0x0c0f0e0d, 0x080b0a09, 0x04070605, 0x00030201);
    l8 = _mm256_set_epi32(
        0x0e0d0c0f, 0x0a09080b, 0x06050407, 0x02010003, 
        0x0e0d0c0f, 0x0a09080b, 0x06050407, 0x02010003);

    for (i = 0; i < BLOCK_SIZE; i += 8) {
        AVX2_cipher(key_AVX[i], input_AVX[i], output_AVX[i]);  // this is for testing
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    cycles2 = cpucycles();
    kcycles = cycles2 - cycles1;
    printf("%lld\n", kcycles / BLOCK_SIZE);
}
