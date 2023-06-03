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

# 주어진 딥러닝 모델 요약 정보 출력
nr6_speck_distinguisher.summary(line_length=128)
plot_model(nr6_speck_distinguisher, to_file='nr6_speck_distinguisher_structure.png', show_shapes=True, show_layer_names=True)

# 문제에서 제시된 SPECK32/64 파라미터 세팅
ROUND = 7
MESSAGE_SIZE = 32
KEY_SIZE = 64
SUBKEY_SIZE = 16
master_key = os.urandom(KEY_SIZE // 8) # 8바이트
subkeys = sp.expand_key(master_key, ROUND)

# 복구해야할 서브키 정보 출력
print(f"[*] Generated master key: k = {master_key.hex()}")
print(f"[*] Scheduled subkey: k1, ..., k7 = {list(map(lambda x: hex(x).zfill(SUBKEY_SIZE//4), subkeys))}")
print(f"[*] Target subkey to recover: k7 = {hex(sp.expand_key(master_key, ROUND)[-1])}")

# 공격에 필요한 암호문 쌍 선택 진행
TEST_SIZE = 128
ciphertext_pairs_r7, _ = sp.make_train_data(TEST_SIZE, ROUND)
