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
#define GATHER(x, i, s) _mm256_i32gather_epi32(x, i, s)
#define _ROL(x, r) OR(SHIFT_L(x, r), SHIFT_R(x, 32 - r))
#define _ROR(x, r) OR(SHIFT_R(x, r), SHIFT_L(x, 32 - r))

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
    uint32_t k1, k2, tmp;

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
void AVX2_cipher(uint32_t* master_key, uint32_t* input, uint32_t* output) {
    uint32_t i = 0;
    __m256i k1, k2, pt1, pt2, t1, t2;
    __m256i _i = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
    uint32_t tmp1[8], tmp2[8];

    // Load Key
    k1 = GATHER( master_key   , _i, 8);
    k2 = GATHER(&master_key[1], _i, 8);

    // Load Plaintext
    pt1 = GATHER( input   , _i, 8);
    pt2 = GATHER(&input[1], _i, 8);

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
    STORE(tmp1, pt1);
    STORE(tmp2, pt2);

    for (int i = 0; i < 16; i += 2) output[i] = tmp1[i >> 1];
    for (int i = 1; i < 16; i += 2) output[i] = tmp2[i >> 1];
}
///////////////////////////////////////////////////////////////////////////////////////////

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
