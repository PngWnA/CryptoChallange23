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
# 주어진 점이 E0 위에 있는 점이면 Gi, 아니면 phi(Ri)로 판단

e = ""
for idx in range(len(s)):
    x = s[idx][0] + s[idx][1] * i
    yy = x^3 + E0.a2() * x ^ 2 + E0.a4() * x
    if yy.is_square():
        e += "0"
        print(f"[*] Point s{idx} is on E0 => b{idx} = 0")
    else:
        e += "1"
        print(f"[*] Point s{idx} is not on E0 => b{idx} = 1")

print(f"[*] Recovered e => {e}")

    
