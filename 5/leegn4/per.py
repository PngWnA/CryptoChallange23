#!/usr/bin/env python3
from ascon import *

def rotl(x, r):
    return ((x << r) | (x >> (64 - r))) & 0xFFFFFFFFFFFFFFFF

def state_to_bytes(state):
    return b''.join([x.to_bytes(8, byteorder="little") for x in state])

def ascon_inverse_permutation(S, rounds=1):
    """
    Inverse of Ascon core permutation for the sponge construction - internal helper function.
    S: Ascon state, a list of 5 64-bit integers
    rounds: number of rounds to perform
    returns nothing, updates S
    """
    assert(rounds <= 12)
    if debugpermutation: printwords(S, "inverse permutation input:")
    for r in range(rounds-1, -1, -1):
        # --- inverse linear diffusion layer ---
        S[4] = rotl(S[4] ^ (S[4] >> 7) ^ (S[4] >> 41), 64-7)
        S[3] = rotl(S[3] ^ (S[3] >> 10) ^ (S[3] >> 17), 64-10)
        S[2] = rotl(S[2] ^ (S[2] << 1) ^ (S[2] << 6), 64-1)
        S[1] = rotl(S[1] ^ (S[1] >> 61) ^ (S[1] >> 39), 64-61)
        S[0] = rotl(S[0] ^ (S[0] >> 19) ^ (S[0] >> 28), 64-19)
        if debugpermutation: printwords(S, "inverse linear diffusion layer:")
        # --- inverse substitution layer ---
        S[2] ^= 0XFFFFFFFFFFFFFFFF
        S[3] ^= S[2]
        S[0] ^= S[4]
        S[1] ^= S[0]
        T = [(S[i] ^ 0xFFFFFFFFFFFFFFFF) & S[(i+1)%5] for i in range(5)]
        for i in range(5):
            S[i] ^= T[(i+1)%5]
        S[2] ^= S[1]
        S[4] ^= S[3]
        S[0] ^= S[4]
        if debugpermutation: printwords(S, "inverse substitution layer:")
        # --- subtract round constants ---
        S[2] ^= (0xf0 - r*0x10 + r*0x1)
        if debugpermutation: printwords(S, "round constant subtraction:")


a = open("enc_blueprint", "rb").read()
b = 0
output = bytearray()
print(len(a))
while b < len(a):
    _=bytes_to_state(a[b:b+40])
    # ascon_permutation(_)
    ascon_inverse_permutation(_)
    output.extend(state_to_bytes(_))
    b += 40
print(len(output))
open("blueprint", "wb").write(output)
