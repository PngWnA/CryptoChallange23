import os

from keras.models import model_from_json
from keras.utils import plot_model
import nr6.speck as sp

# 주어진 딥러닝 모델 로드
print("[*] Loading 6-round neural distinguisher... ", end='', flush=True)
arch = open('./nr6/arch_neural_distinguisher.json')
json_arch = arch.read()
nr6_speck_distinguisher = model_from_json(json_arch) 
nr6_speck_distinguisher.load_weights('./nr6/weights_nr6_speck.h5') 
nr6_speck_distinguisher.compile(optimizer='adam',loss='mse',metrics=['acc']) 
print("Done.")

# 딥러닝 모델 요약 출력
nr6_speck_distinguisher.summary(line_length=128)
plot_model(nr6_speck_distinguisher, to_file='nr6_speck_distinguisher_structure.png', show_shapes=True, show_layer_names=True)

# 문제에서 제시된 SPECK32/64 파라미터 세팅
ROUND = 7
MESSAGE_SIZE = 32
KEY_SIZE = 64
SUBKEY_SIZE = 16
master_key = os.urandom(KEY_SIZE // 8) # 8바이트
subkeys = sp.expand_key(master_key, ROUND)

# 예측 대상 서브키 출력
print(f"[*] Generated master key: k = {master_key.hex()}")
print(f"[*] Scheduled subkey: k1, ..., k7 = {list(map(lambda x: hex(x).zfill(SUBKEY_SIZE//4), subkeys))}")
print(f"[*] Target subkey to recover: k7 = {hex(sp.expand_key(master_key, ROUND)[-1])}")

# 임의로 암호문 쌍 선택 진행
TEST_SIZE = 1
ciphertext_pairs_r7, _ = sp.make_train_data(TEST_SIZE, ROUND, fixed_key = master_key, gen_real_ciphertext_pair = True)

# 키 복구 진행
for ciphertext_pair_r7 in ciphertext_pairs_r7:
    possible_ciphertext_pairs_r6 = sp.sp_list()
    print(possible_ciphertext_pairs_r6)
    C0, C1 = ciphertext_pair_r7[:32], ciphertext_pair_r7[32:]
    print(f"[*] Target ciphertext pair: {C0} | {C1})")
    for candidate_subkey_k7 in range(2**SUBKEY_SIZE)[:2]:
        (D0l, D0r), (D1l, D1r) = sp.dec_one_round(C0, candidate_subkey_k7), sp.dec_one_round(C1, candidate_subkey_k7)
        D0l, D0r, D1l, D1r = sp.int64_array_to_binary([D0l, D0r, D1l, D1r])
        D0, D1 = sp.concat(D0l, D0r), sp.concat(D1l, D1r)
        possible_ciphertext_pair_r6 = sp.concat(D0, D1)
        print(f"[*] -> {sp.concat(D0l, D0r)} | {sp.concat(D1l, D1r)} (using k7 = {bin(candidate_subkey_k7)[2:].zfill(SUBKEY_SIZE)})")
        possible_ciphertext_pairs_r6 = sp.concat(possible_ciphertext_pairs_r6, possible_ciphertext_pair_r6)


possible_ciphertext_pairs_r6 = possible_ciphertext_pairs_r6.reshape(-1, MESSAGE_SIZE * 2)
print(possible_ciphertext_pairs_r6)
predictions = nr6_speck_distinguisher.predict(possible_ciphertext_pairs_r6, verbose=1)

with open("./result.txt", "w") as result:
    for z in predictions:
        if z > 0.5:
            print(z, predictions.index(z))
    