#!/usr/bin/env python3
import sys,scrypt,ascon
# import itertools

'INCHEONJEJUKANSI31L-13R'

ct=open('../1/enc_blueprint','rb').read()
salt=b'contest2023'
data=scrypt.hash(password=sys.argv[1].encode(),salt=salt,N=0x10000,r=8,p=1,buflen=48)
key,nonce,ad=data[:16],data[16:32],data[32:]

pt=ascon.ascon_decrypt(key=key,nonce=nonce,associateddata=ad,ciphertext=ct)

if pt is not None:
    print('Success')
    open('./blueprint','wb').write(pt)

# iata=[b'ICN',b'CJU',b'KIX',b'JFK']
# icao=[b'RKSI',b'RKPC',b'RJBB',b'KJFK']

# passwords=[b''.join(_) for _ in itertools.permutations(icao)]
# for pw in tqdm(passwords):
    # salt=b'contest2023'
    # data=scrypt.hash(password=pw,salt=salt,N=0x10000,r=8,p=1,buflen=48)
    # key,nonce,ad=data[:16],data[16:32],data[32:]

    # pt=ascon.ascon_decrypt(key=key,nonce=nonce,associateddata=ad,ciphertext=ct)
    # if pt is not None:
        # print('Success')
        # open('./blueprint','wb').write(pt)
