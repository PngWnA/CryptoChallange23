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
void new_key_gen_AVX2(uint32_t* master_key, uint32_t* session_key) {
    // example: AVX2 codes; freely remove this code and write what you want in here! 
    __m256i x0, x1, tmp;
    x0 = LOAD(master_key);
    x1 = LOAD(&master_key[8]);
    tmp = ADD(x0, x1);
    STORE(&session_key[0], tmp);
}

// check input_length --> multiple of 8 * 64-bit
void new_block_cipher_AVX2(uint32_t* input, uint32_t* session_key, uint32_t* output) {
    // example: AVX2 codes; freely remove this code and write what you want in here! 
    __m256i x0, x1, tmp;
    x0 = LOAD(input);
    x1 = LOAD(&input[8]);
    tmp = ADD(x0, x1);
    STORE(&output[0], tmp);
}
///////////////////////////////////////////////////////////////////////////////////////////

int main() {
    long long int kcycles, ecycles, dcycles;
    long long int cycles1, cycles2;
    int32_t i, j;

    // C implementation
    uint32_t input_C[BLOCK_SIZE][P_K_SIZE] = { 0, };
    uint32_t key_C[BLOCK_SIZE][P_K_SIZE] = { 0, };
    uint32_t session_key_C[BLOCK_SIZE][SESSION_KEY_SIZE] = { 0, };
    uint32_t output_C[BLOCK_SIZE][P_K_SIZE] = { 0, };

    // AVX implementation
    uint32_t input_AVX[BLOCK_SIZE][P_K_SIZE] = { 0, };
    uint32_t key_AVX[BLOCK_SIZE][P_K_SIZE] = { 0, };
    uint32_t session_key_AVX[BLOCK_SIZE][SESSION_KEY_SIZE] = { 0, };
    uint32_t output_AVX[BLOCK_SIZE][P_K_SIZE] = { 0, };

    // random generation for plaintext and key. 
    srand(0);

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < P_K_SIZE; j++) {
            input_AVX[i][j] = input_C[i][j] = rand();
            key_AVX[i][j] = key_C[i][j] = rand();
        }
    }

    // execution of C implementation 
    kcycles=0;
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
    for (i = 0; i < BLOCK_SIZE; i++) {
        new_key_gen(key_AVX[i], session_key_AVX[i]);                        // this is for testing
        new_block_cipher(input_AVX[i], session_key_AVX[i], output_AVX[i]);  // this is for testing
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
