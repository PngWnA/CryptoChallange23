#!/usr/bin/env python3

xor = lambda x, y: bytearray(a ^ b for a, b in zip(x, y))

enc_3 = open("3.bmp", "rb").read()
org_3 = open("enc_3.bmp", "rb").read()
enc_4 = open("4.png", "rb").read()
org_4 = open("enc_4.png", "rb").read()

keystream_1 = xor(enc_3, org_3)
keystream_2 = xor(enc_4, org_4)

idx = min(len(keystream_1), len(keystream_2))
assert keystream_1[:idx] == keystream_2[:idx]
print("good")

keystream = keystream_1 if len(keystream_1) > len(keystream_2) else keystream_2
enc_5 = open("enc_5.png", "rb").read()
assert len(enc_5) <= len(keystream)
org_5 = xor(enc_5, keystream)
open("5.png", "wb").write(org_5)
