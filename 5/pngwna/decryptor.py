import sys

from Crypto.Cipher import AES
from Crypto.Util.Counter import new

FILE = open(sys.argv[1], "rb").read()
KEY = b"RKSIRKPCRJBBKJFK"
NONCE = b"id:#59mmk35nasd82@2023#"

counter = new(128, initial_value=int.from_bytes(NONCE, byteorder='big'))
cipher = AES.new(KEY, AES.MODE_CTR, counter=counter)

plaintext = cipher.decrypt(FILE)

open(f"dec_{sys.argv[1]}", "wb").write(plaintext)