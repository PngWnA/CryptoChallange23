from sage.schemes.elliptic_curves.hom_composite import EllipticCurveHom_composite

load("implementation.sage")

'''
# 분석 1
만약 S를 알수 있는 경우 다음과 같은 관계가 성립
S-Ps = ns*Qs
주어진 Curve가 supersingular하기 때문에 discrete logarithm을 효율적으로 수행할 수 있는 알고리즘이 존재하며,
따라서 개인키인 ns를 효율적으로 복구할 수 있음

(참고자료: MOV attack)
https://www.dima.unige.it/~morafe/MaterialeCTC/p80-menezes.pdf
https://crypto.stackexchange.com/questions/1871/how-does-the-mov-attack-work

S+Ri로 계산되는 Gi에서 Ri를 다음과 같이 제거할 수 있음
Gi * 3^137
= (S+Ri) * 3^137: Gi=S+Ri 대입
= S*3^137 + Ri*3^137
= S*3^137: Ri의 order가 3^137임

양 변을 정리하면 다음과 같은 관계가 성립함
S = Gi * 3^137 * (3^-137 mod 2^216): S의 order가 2^216임

최종적으로 S=Ps+ns*Qs 를 대입하면 아래 관계가 성립하며 discrete logarithm을 해결하여 ns를 복구할 수 있음
ns*Qs = Gi * 3^137 * (3^-137 mod 2^216) - Ps
'''

print("[*] 분석1: ns 복구 공격 검증")
for Gi in G:
    print(". ", end='', flush=True)
    assert ns == Qs.discrete_log(Gi * (3^137) * inverse_mod(3^137, 2^216) - Ps)
print()
print("[+] 검증 통과")

'''
# 분석 2
Isogeny ai: E -> E_i'의 경우 다음과 같은 합성함수로 분해할 수 있음
ai = bi o phiS'
(phiS': E -> Es, bi: Es -> E_i')

이때 phiS'는 키 생성 과정의 phiS와 동일하기 때문에 phiS를 복구할 수 있음
'''
print("[*] 분석2: phiS 복구 공격 검증")
for Gi in G:
    print(". ", end='', flush=True)
    ai = E.isogeny(kernel=Gi, algorithm="factored")
    phis = EllipticCurveHom_composite.from_factors(ai.factors()[:216])
    assert phiS == phis
print()
print("[+] 검증 통과")
print("[+] 분석 1, 2 검증 완료")