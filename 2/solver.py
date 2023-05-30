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

# 작성중
print("[*] Generating sample plaintext pair")
x_test, y_test = sp.make_train_data(10**6, 6)
print(x_test, y_test)
results = nr6_speck_distinguisher.evaluate(x_test, y_test, batch_size=10000)
print('test loss, test_acc: ', results)
