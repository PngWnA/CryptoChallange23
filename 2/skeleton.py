# Model import
from keras.models import model_from_json
arch = open('./nr6/arch_neural_distinguisher.json')
json_arch = arch.read()
nr6_speck_distinguisher = model_from_json(json_arch) 
nr6_speck_distinguisher.load_weights('./nr6/weights_nr6_speck.h5') 
nr6_speck_distinguisher.compile(optimizer='adam',loss='mse',metrics=['acc']) 
# Model test
import nr6.speck as sp
x_test, y_test = sp.make_train_data(10**6, 6)
results = nr6_speck_distinguisher.evaluate(x_test, y_test, batch_size=10000) 
print('test loss, test_acc: ', results)
