from sage.schemes.elliptic_curves.hom_composite import EllipticCurveHom_composite

# 문제에서 주어진 환경 세팅
load("params.sage")

# 문제 파라미터 확인
print(f"[*] E0: {E0}")
print(f"[*] Generator of E0[2^216]")
print(f"    Ps.x: {Ps.xy()[0]}")
print(f"    Ps.y: {Ps.xy()[1]}")
print(f"    Qs.x: {Qs.xy()[0]}")
print(f"    Qs.y: {Qs.xy()[1]}")
print(f"[*] Generator of E0[3^137]")
print(f"    Pr.x: {Pr.xy()[0]}")
print(f"    Pr.y: {Pr.xy()[1]}")
print(f"    Qr.x: {Qr.xy()[0]}")
print(f"    Qr.y: {Qr.xy()[1]}")
print(f"[*] Es: {Es}")

# e값 복구
# 주어진 점이 E0 위에 있는 점이면 Gi, Es에 있는 점이면 phi(Ri)로 판단
Gi = list()
phiSRi = list()
e = str()
for idx in range(len(s)):
    x = s[idx][0] + s[idx][1] * i
    try:
        P = E0.lift_x(x)
        e += "0"
        Gi.append(P)
    except:
        P = Es.lift_x(x)
        e += "1"
        phiSRi.append(P)

print(f"[*] Recovered e => {e} ({len(e)})")

# 풀이 1
# Gi에서 ns 복구 시도
print("[*] Recovering ns...")
for gi in Gi:
    try:
        ns = Qs.discrete_log(gi * (3^137) * inverse_mod(3^137, 2^216) - Ps)
        print(f"[+] Recovered S => {gi * (3^137) * inverse_mod(3^137, 2^216)}")
        print(f"[+] Recovered ns => {ns}")
        break
    except ValueError:
        print("[-] Trying with another Gi ...")
        pass



# 풀이 2
# Gi를 kernel로 하는 isogeny 분해
print(f"[*] Recovering phiS...")
for gi in Gi:
    try:
        iso = E0.isogeny(kernel=gi, algorithm="factored")
        factors = list(filter(lambda i: i.degree() == 2, iso.factors()))
        assert len(factors) == 216
        phiS = EllipticCurveHom_composite.from_factors(factors)
        print(f"[+] Recovered phiS => {phiS}")
        mapX, mapY = phiS.factors()[0].rational_maps()
        for phis in phiS.factors()[1:]:
            mapX = phis.rational_maps()[0]
            mapY = phis.rational_maps()[1]
        print(f"[+] Rational map of phiS: (x, y) -> (x', y') is as follows")
        print(f"x' = {mapX}")
        print(f"y' = {mapY}")
        break
    except AssertionError:
        print("[-] Trying with another Gi ...")
        pass
    
