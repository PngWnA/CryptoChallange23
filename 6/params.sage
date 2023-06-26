# 문제에서 주어진 파라미터 세팅
(lA,eA), (lB,eB) = (2,216), (3,137)
p = lA^eA * lB^eB - 1  # SIKEp434에서 사용하는 p와 동일

# y^2 = x^3 + 6*x^2 + x over Fp^2, i^2 + 1 = 0
Fp2.<i> = GF(p^2, modulus=x^2+1)
E0 = EllipticCurve(Fp2, [0,6,0,1,0])

# Ps
Ps_x_Re = 0x00003CCFC5E1F050030363E6920A0F7A4C6C71E63DE63A0E6475AF621995705F7C84500CB2BB61E950E19EAB8661D25C4A50ED279646CB48
Ps_x_Im = 0x0001AD1C1CAE7840EDDA6D8A924520F60E573D3B9DFAC6D189941CB22326D284A8816CC4249410FE80D68047D823C97D705246F869E3EA50
Ps_y_Re = 0x0001AB066B84949582E3F66688452B9255E72A017C45B148D719D9A63CDB7BE6F48C812E33B68161D5AB3A0A36906F04A6A6957E6F4FB2E0
Ps_y_Im = 0x0000FD87F67EA576CE97FF65BF9F4F7688C4C752DCE9F8BD2B36AD66E04249AAF8337C01E6E4E1A844267BA1A1887B433729E1DD90C7DD2F
Ps_x = Ps_x_Re + Ps_x_Im * i
Ps_y = Ps_y_Re + Ps_y_Im * i
Ps = E0(Ps_x, Ps_y)

# Qs
Qs_x_Re = 0x0000C7461738340EFCF09CE388F666EB38F7F3AFD42DC0B664D9F461F31AA2EDC6B4AB71BD42F4D7C058E13F64B237EF7DDD2ABC0DEB0C6C
Qs_x_Im = 0x000025DE37157F50D75D320DD0682AB4A67E471586FBC2D31AA32E6957FA2B2614C4CD40A1E27283EAAF4272AE517847197432E2D61C85F5
Qs_y_Re = 0x0001D407B70B01E4AEE172EDF491F4EF32144F03F5E054CEF9FDE5A35EFA3642A11817905ED0D4F193F31124264924A5F64EFE14B6EC97E5
Qs_y_Im = 0x0000E7DEC8C32F50A4E735A839DCDB89FE0763A184C525F7B7D0EBC0E84E9D83E9AC53A572A25D19E1464B509D97272AE761657B4765B3D6
Qs_x = Qs_x_Re + Qs_x_Im * i
Qs_y = Qs_y_Re + Qs_y_Im * i
Qs = E0(Qs_x, Qs_y)

# Ps - Qs
Ds_x_Re = 0x0000F37AB34BA0CEAD94F43CDC50DE06AD19C67CE4928346E829CB92580DA84D7C36506A2516696BBE3AEB523AD7172A6D239513C5FD2516
Ds_x_Im = 0x000196CA2ED06A657E90A73543F3902C208F410895B49CF84CD89BE9ED6E4EE7E8DF90B05F3FDB8BDFE489D1B3558E987013F9806036C5AC
Ds_y_Re = 0x00007F65B303A50EF1B4192237611E226A3D13384EF608A6B117365A16E0EB5112156F2012CB029C819F3330F69BD5C73CCC9A1F1C06CD15
Ds_y_Im = 0x0000749095AB8A36C841FBF25A5671A67FDE5023131C73F0EC6031C7E472DAE138FBED0A0BE63C6706CD893EF88D32CC766EC67EC056ED33
Ds_x = Ds_x_Re + Ds_x_Im * i
Ds_y = Ds_y_Re + Ds_y_Im * i
Ds = E0(Ds_x, Ds_y)

assert Ps - Qs == Ds

# Pr
Pr_x_Re = 0x00008664865EA7D816F03B31E223C26D406A2C6CD0C3D667466056AAE85895EC37368BFC009DFAFCB3D97E639F65E9E45F46573B0637B7A9
Pr_x_Im = 0x0
Pr_y_Re = 0x00006AE515593E73976091978DFBD70BDA0DD6BCAEEBFDD4FB1E748DDD9ED3FDCF679726C67A3B2CC12B39805B32B612E058A4280764443B
Pr_y_Im = 0x0
Pr_x = Pr_x_Re + Pr_x_Im * i
Pr_y = Pr_y_Re + Pr_y_Im * i
Pr = E0(Pr_x, Pr_y)

# Qr
Qr_x_Re = 0x00012E84D7652558E694BF84C1FBDAAF99B83B4266C32EC65B10457BCAF94C63EB063681E8B1E7398C0B241C19B9665FDB9E1406DA3D3846
Qr_x_Im = 0x0
Qr_y_Re = 0x0
Qr_y_Im = 0x0000EBAAA6C731271673BEECE467FD5ED9CC29AB564BDED7BDEAA86DD1E0FDDF399EDCC9B49C829EF53C7D7A35C3A0745D73C424FB4A5FD2
Qr_x = Qr_x_Re + Qr_x_Im * i
Qr_y = Qr_y_Re + Qr_y_Im * i
Qr = E0(Qr_x, Qr_y)

# Pr - Qr
Dr_x_Re = 0x0001CD28597256D4FFE7E002E87870752A8F8A64A1CC78B5A2122074783F51B4FDE90E89C48ED91A8F4A0CCBACBFA7F51A89CE518A52B76C
Dr_x_Im = 0x000147073290D78DD0CC8420B1188187D1A49DBFA24F26AAD46B2D9BB547DBB6F63A760ECB0C2B20BE52FB77BD2776C3D14BCBC404736AE4
Dr_y_Re = 0x0000DA7A98EA26469B843EBF8D1EE0F00E6786E680AC535F5FF26D25819549C959497D8E8FB14B1BF6764BD27BAE970D0791AF091E344F22
Dr_y_Im = 0x000048704FEC03D05B06D8A8197DF08D4946E465099F31B75C63A865A23CA2AD41A74F05074E9DC3F45C5A26F741A0EA1F3C2E6CDA0BB344
Dr_x = Dr_x_Re + Dr_x_Im * i
Dr_y = Dr_y_Re + Dr_y_Im * i
Dr = E0(Dr_x, Dr_y)

assert Pr - Qr == Dr

# Es
Es_A_Re = 0x0000BC39A8C22AFDCAC43EFDD3AB05B45AF0A795D823CD1EC0931D386BFDE2D477DFFFBF2C8463460DE0586510E99F24323AB8E54BD0026B
Es_A_Im = 0x0000045E901E3BAA12BA1A2D0A37634DEF74A6791039D723962496EB9C4C368FD50BD06BC7D7EF0B2582ADF73577537BDAA9A064C9AB0DA5

Es = EllipticCurve(Fp2, [0, Es_A_Re + Es_A_Im * i, 0, 1, 0])

# s
s = open("./2023pqc_s.txt", "r").readlines()
s = list(map(lambda x: int(x, 16), s))
sx = s[0::2]
sy = s[1::2]
s = list(zip(sx, sy))