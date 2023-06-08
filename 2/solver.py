import os

from keras.models import model_from_json
from keras.utils import plot_model
import nr6.speck as sp
import numpy as np

def convert_to_uint16(X):
    x, y = np.packbits(X)
    z = np.uint16(x) << 8 | y
    return np.array([z], dtype=np.uint16)

# 주어진 딥러닝 모델 로드
print("[*] Loading 6-round neural distinguisher... ", end='', flush=True)
arch = open('./nr6/arch_neural_distinguisher.json')
json_arch = arch.read()
nr6_speck_distinguisher = model_from_json(json_arch) 
nr6_speck_distinguisher.load_weights('./nr6/weights_nr6_speck.h5') 
nr6_speck_distinguisher.compile(optimizer='adam',loss='mse',metrics=['acc']) 
print("Done.")

# 주어진 딥러닝 모델 요약 정보 출력
nr6_speck_distinguisher.summary(line_length=128)
plot_model(nr6_speck_distinguisher, to_file='nr6_speck_distinguisher_structure.png', show_shapes=True, show_layer_names=True)

# 문제에서 제시된 SPECK32/64 파라미터 세팅
ROUND = 7
MESSAGE_SIZE = 32
KEY_SIZE = 64
SUBKEY_SIZE = 16
master_key = np.frombuffer(os.urandom(KEY_SIZE // 8), dtype=np.uint16).reshape(4,-1); # 8바이트
subkeys = sp.expand_key(master_key, ROUND)

# 복구해야할 서브키 정보 출력
print(f"[*] Generated master key: k = 0x{''.join(list(map(lambda x: hex(x[0])[2:], master_key)))}")
print(f"[*] Scheduled subkeys: k1, ..., k7 = {list(map(lambda x: hex(x[0]), subkeys))}")
print(f"[*] Target subkey to recover: k7 = {hex(subkeys[-1][0])}")

# 공격에 필요한 암호문 쌍 선택 진행
TEST_SIZE = 1
ciphertext_pairs_r7, _ = sp.make_train_data(TEST_SIZE, ROUND)
if _[0] != 1:
    exit()

for ciphertext_pair_r7 in ciphertext_pairs_r7:
    C0, C1 = ciphertext_pair_r7[:32], ciphertext_pair_r7[32:]
    C0l, C0r = convert_to_uint16(C0[:16]), convert_to_uint16(C0[16:])
    C1l, C1r = convert_to_uint16(C1[:16]), convert_to_uint16(C1[16:])

    print(f"[*] Target ciphertext pair info")
    print(f"[*] {C0l} {C0r} | {C1l} {C1r}")
    
    # 가능한 모든 subkey 공간에 대해 1라운드 복호화 진행
    possible_ciphertext_pairs_r6 = np.ndarray(shape=(0, 64),dtype=np.uint16)
    for possible_subkey_r7 in range(2**SUBKEY_SIZE):
        
        D0l, D0r = sp.dec_one_round((C0l, C0r), possible_subkey_r7)
        D1l, D1r = sp.dec_one_round((C1l, C1r), possible_subkey_r7)
        print(f"[*] -> {D0l} {D0r} | {D1l} {D1r} (using subkey={bin(possible_subkey_r7)[2:].zfill(SUBKEY_SIZE)})")
        possible_ciphertext_pairs_r6 = np.append(possible_ciphertext_pairs_r6, sp.convert_to_binary([D0l, D0r, D1l, D1r]), axis=0)
    prediction = nr6_speck_distinguisher.predict(possible_ciphertext_pairs_r6)