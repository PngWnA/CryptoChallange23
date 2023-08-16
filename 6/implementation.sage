import random
from hashlib import sha256

from Crypto.Util.number import long_to_bytes

print(f"[*] 파라미터 세팅...")
# SIKEp434 파라미터 선언
(lA,eA), (lB,eB) = (2,216), (3,137)
p = lA^eA * lB^eB - 1  # SIKEp434 spec.
assert p.is_prime()

# E : y^2 = x^3 + 6x^2 + x over Fp^2
Fp2.<i> = GF(p^2, modulus=x^2+1)
E = EllipticCurve(Fp2, [0,6,0,1,0])

assert E.is_supersingular()

# Ps
Ps_x_Re = 0x00003CCFC5E1F050030363E6920A0F7A4C6C71E63DE63A0E6475AF621995705F7C84500CB2BB61E950E19EAB8661D25C4A50ED279646CB48
Ps_x_Im = 0x0001AD1C1CAE7840EDDA6D8A924520F60E573D3B9DFAC6D189941CB22326D284A8816CC4249410FE80D68047D823C97D705246F869E3EA50
Ps_y_Re = 0x0001AB066B84949582E3F66688452B9255E72A017C45B148D719D9A63CDB7BE6F48C812E33B68161D5AB3A0A36906F04A6A6957E6F4FB2E0
Ps_y_Im = 0x0000FD87F67EA576CE97FF65BF9F4F7688C4C752DCE9F8BD2B36AD66E04249AAF8337C01E6E4E1A844267BA1A1887B433729E1DD90C7DD2F
Ps_x = Ps_x_Re + Ps_x_Im * i
Ps_y = Ps_y_Re + Ps_y_Im * i
Ps = E(Ps_x, Ps_y)

# Qs
Qs_x_Re = 0x0000C7461738340EFCF09CE388F666EB38F7F3AFD42DC0B664D9F461F31AA2EDC6B4AB71BD42F4D7C058E13F64B237EF7DDD2ABC0DEB0C6C
Qs_x_Im = 0x000025DE37157F50D75D320DD0682AB4A67E471586FBC2D31AA32E6957FA2B2614C4CD40A1E27283EAAF4272AE517847197432E2D61C85F5
Qs_y_Re = 0x0001D407B70B01E4AEE172EDF491F4EF32144F03F5E054CEF9FDE5A35EFA3642A11817905ED0D4F193F31124264924A5F64EFE14B6EC97E5
Qs_y_Im = 0x0000E7DEC8C32F50A4E735A839DCDB89FE0763A184C525F7B7D0EBC0E84E9D83E9AC53A572A25D19E1464B509D97272AE761657B4765B3D6
Qs_x = Qs_x_Re + Qs_x_Im * i
Qs_y = Qs_y_Re + Qs_y_Im * i
Qs = E(Qs_x, Qs_y)

print(f"[*] 공개키, 개인키 생성...")
# S 연산
ns = random.randrange(1, lA^eA - 1)
S = Ps + ns * Qs

# 공개키, 개인키 연산
phiS = E.isogeny(kernel=S, algorithm="factored")
Es = phiS.codomain()

print(f"[+] 공개키: {Es}")
print(f"[+] 개인키: {ns} ({phiS})")

print(f"[*] 서명 생성...")
# 서명생성
m = b"This is my message"
R = list()
Curves = list()

# Pr
Pr_x_Re = 0x00008664865EA7D816F03B31E223C26D406A2C6CD0C3D667466056AAE85895EC37368BFC009DFAFCB3D97E639F65E9E45F46573B0637B7A9
Pr_x_Im = 0x0
Pr_y_Re = 0x00006AE515593E73976091978DFBD70BDA0DD6BCAEEBFDD4FB1E748DDD9ED3FDCF679726C67A3B2CC12B39805B32B612E058A4280764443B
Pr_y_Im = 0x0
Pr_x = Pr_x_Re + Pr_x_Im * i
Pr_y = Pr_y_Re + Pr_y_Im * i
Pr = E(Pr_x, Pr_y)

# Qr
Qr_x_Re = 0x00012E84D7652558E694BF84C1FBDAAF99B83B4266C32EC65B10457BCAF94C63EB063681E8B1E7398C0B241C19B9665FDB9E1406DA3D3846
Qr_x_Im = 0x0
Qr_y_Re = 0x0
Qr_y_Im = 0x0000EBAAA6C731271673BEECE467FD5ED9CC29AB564BDED7BDEAA86DD1E0FDDF399EDCC9B49C829EF53C7D7A35C3A0745D73C424FB4A5FD2
Qr_x = Qr_x_Re + Qr_x_Im * i
Qr_y = Qr_y_Re + Qr_y_Im * i
Qr = E(Qr_x, Qr_y)

# 랜덤 점 추출
print(f"[*] Ri 추출중...")
while len(R) != 256:
    print(". ", end='', flush=True)
    Ri = Pr * random.randrange(1, lB^eB - 1) + Qr * random.randrange(1, lB^eB - 1)
    R.append(Ri)
print()

# 각 Ri에 대한 Es -> Ei isogeny 연산
G = list()
print(f"[*] beta 연산중...")
for Ri in R:
    print(". ", end='', flush=True)
    Gi = S + Ri
    G.append(Gi)
    betai = Es.isogeny(kernel=phiS(Ri), algorithm="factored")
    Curves.append(betai.codomain())
print()

# r, e 계산
print(f"[*] r, e 계산중...")
r = b""
for Ei in Curves:
    print(". ", end='', flush=True)
    re, im = Ei.j_invariant()
    r += long_to_bytes(re.lift()) + long_to_bytes(im.lift())
print()

e = sha256()
e.update(r+m)
e = e.hexdigest()
print(f"[+] e = {e}")


# s 계산
print(f"[*] s 계산중...")
s = list()
b = [int(bit) for byte in bytes.fromhex(e) for bit in format(byte, '08b')]
for i in range(len(b)):
    print(". ", end='', flush=True)
    if b[i] == 0:
        s.append(G[i])
    elif b[i] == 1:
        s.append(phiS(R[i]))
print()

print(f"[+] s: {s}") # 448bit 잘라 저장하는 부분은 생략