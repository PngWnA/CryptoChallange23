import os
import math
import json
import datetime

from itertools import repeat

import numpy as np

from keras.models import model_from_json
from keras.utils import plot_model

import multiprocessing as mp
from multiprocessing import Pool, freeze_support

import nr6.speck as sp

# convert_to_binary의 역연산
def convert_to_uint16(X):
    x, y = np.packbits(X)
    z = np.uint16(x) << 8 | y
    return np.array([z], dtype=np.uint16)

# multiprocessing API 사용을 위한 함수 분리
def decrypt_ciphertext_pair_1round(C0C1, subkey):
    C0l, C0r, C1l, C1r = C0C1
    D0l, D0r = sp.dec_one_round((C0l, C0r), subkey)
    D1l, D1r = sp.dec_one_round((C1l, C1r), subkey)
    return sp.convert_to_binary([D0l, D0r, D1l, D1r])[0]

def attack():
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
    BLOCK_SIZE = MESSAGE_SIZE // 2
    KEY_SIZE = 64
    SUBKEY_SIZE = 16
    master_key = np.frombuffer(os.urandom(KEY_SIZE // 8), dtype=np.uint16).reshape(4,-1); # 8바이트
    subkeys = sp.expand_key(master_key, ROUND)

    # 복구해야할 서브키 정보 출력
    print(f"[*] Generated master key: k = 0x{''.join(list(map(lambda x: hex(x[0])[2:], master_key)))}")
    print(f"[*] Scheduled subkeys: k1, ..., k7 = {list(map(lambda x: hex(x[0]), subkeys))}")
    print(f"[*] Target subkey to recover: k7 = {hex(subkeys[-1][0])}")

    # 공격에 필요한 데이터 생성
    TEST_SIZE = 32  # 생성할 암호문 쌍의 개수
    
    ciphertext_pairs_r7, _ = sp.make_train_data(TEST_SIZE, ROUND, inject_key=master_key, force_diff=True)  # 암호문 쌍 생성

    # 공격 진행
    test_round = 0
    predictions = []
    for ciphertext_pair_r7 in ciphertext_pairs_r7:
        C0, C1 = ciphertext_pair_r7[:MESSAGE_SIZE], ciphertext_pair_r7[MESSAGE_SIZE:]
        C0l, C0r = convert_to_uint16(C0[:BLOCK_SIZE]), convert_to_uint16(C0[BLOCK_SIZE:])
        C1l, C1r = convert_to_uint16(C1[:BLOCK_SIZE]), convert_to_uint16(C1[BLOCK_SIZE:])

        print(f"[*] ({test_round+1}/{len(ciphertext_pairs_r7)}) Target ciphertext pair info : {C0l} {C0r} | {C1l} {C1r}")
        
        # 가능한 모든 subkey 공간에 대해 1라운드 복호화 진행
        # multiprocessing 모듈 사용하여 공격 가속
        with Pool(mp.cpu_count() - 2) as p:
            possible_ciphertext_pairs_r6 = p.starmap(decrypt_ciphertext_pair_1round, zip(repeat([C0l, C0r, C1l, C1r]), range(2**SUBKEY_SIZE)))
        
        # 예측 및 예측값 저장
        prediction = nr6_speck_distinguisher.predict(np.array(possible_ciphertext_pairs_r6))
        predictions.append(prediction)
        test_round += 1

    # key scoring 진행
    result = dict()
    for index in range(2**16):
        result[index] = 0
        
    for prediction in predictions:
        for index in range(2**SUBKEY_SIZE):
            z = prediction[index][0]
            result[index] += math.log2(z / (1 - z))
            
    # 공격 결과 저장
    report = dict()
    report["master_key"] = master_key.tobytes().hex()
    report["subkeys"] = list(map(lambda x: int(x[0]), subkeys))
    report["attack_size"] = TEST_SIZE
    report["score"] = sorted(result.items(), key=lambda x: x[1], reverse=True)
    
    now = datetime.datetime.now()
    open(f"report_{now.month}.{now.day}.{now.hour}.{now.minute}.{now.second}.json", "w").write(json.dumps(report, indent=2))
    
    # score가 가장 높은 키가 실제 7라운드 서브키인지 확인
    if report["subkeys"][-1] == report["score"][0][0]:
        print(f"[+] Successfully recovered 7th round subkey: {report['subkeys'][-1]} (real subkey) == {report['score'][0][0]} (expected))")
    else:
        print(f"[-] Recovered unsuccessful: {report['subkeys'][-1]} (real subkey) != {report['score'][0][0]} (expected))")

# main process 확인 안 할 경우 multiprocessing 모듈 제대로 작동 안함
if __name__ == '__main__':
    freeze_support()
    attack()