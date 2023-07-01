pt = open("../4/4.png", "rb").read()
ct = open("./enc_4.png", "rb").read()

keystream = bytearray(x ^ y for x, y in zip(pt, ct))

ct_5 = open("./enc_5.png", "rb").read()

pt_5 = bytearray(x ^ y for x, y in zip(keystream, ct_5))

open("5.png", "wb").write(pt_5)