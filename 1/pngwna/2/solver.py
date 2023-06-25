import os
import random

import torch
import torchvision
import torchvision.transforms as transforms

import requests

from PIL import Image

import matplotlib.pyplot as plt

# 모델, 테스트셋 준비
# ResNet-18 준비
model = torchvision.models.resnet18(weights='DEFAULT').eval()
print(f"[*] Loaded pretrained ResNet-18.")

# ResNet-18 분류 라벨
url = 'https://raw.githubusercontent.com/anishathalye/imagenet-simple-labels/master/imagenet-simple-labels.json'
labels = requests.get(url).json()
print(f"[*] Loaded human readable output label mapping (total {len(labels)} labels)")

# 원본, 공격 이미지 로드
TARGET_DIR = "eval"
ORIGIN = "orig_image.png"
ADV = "adv_image.png"
PATCH = "patch.png"
orig_img = Image.open(os.path.join(TARGET_DIR, ORIGIN)).convert("RGB")
adv_img = Image.open(os.path.join(TARGET_DIR, ADV)).convert("RGB")
patch = Image.open(PATCH).convert("RGB")
print(f"[*] Loaded original, adversarial image, and patch image")

# 이미지 형태를 4차원 텐서로 변환
# 이미지 normalization 후 공격 진행하고 싶은 경우 아래 변수 수정
MEAN = [0.485, 0.456, 0.406]
STD = [0.229, 0.224, 0.225]
transform = transforms.Compose(
    [
        transforms.ToTensor(),
        transforms.Normalize(MEAN, STD)]
    )
print(f"[*] Preprocessing with Image -> ToTensor -> Normalize -> Tensor")

orig_image = transform(orig_img).unsqueeze(0)
adv_image = transform(adv_img).unsqueeze(0)

# 공격 전 분류값 추출
prediction = model(orig_image)
values, indexes = torch.topk(prediction, 5)
classifications = [labels[index] for index in indexes[0].tolist()]
origin_top1 = classifications[0]
print(f"[*] Original image is classified as '{origin_top1}'")

# 공격 전 분류값 추출
prediction = model(adv_image)
values, indexes = torch.topk(prediction, 5)
classifications = [labels[index] for index in indexes[0].tolist()]
adv_top1 = classifications[0]

# 공격 성공 검증
if origin_top1 != adv_top1:
    print(f"[+] Patch attack successful! ({origin_top1} != {adv_top1})")
else:
    print(f"[+] Patch attack failed! ({origin_top1} == {adv_top1})")

# 이미지 denormalize 진행
for t, m, s in zip(orig_image, MEAN, STD):
    t.mul_(s).add_(m)
orig_image = orig_image.squeeze(0).permute(1, 2, 0).clip(0, 1)

for t, m, s in zip(adv_image, MEAN, STD):
    t.mul_(s).add_(m)
adv_image = adv_image.squeeze(0).permute(1, 2, 0).clip(0, 1)
    

# 원본, 노이즈, 공격 이미지 출력
_, figures = plt.subplots(1, 3, figsize=(15, 5))

figures[0].imshow(orig_image)
figures[0].set_title(f"Original image: classified as {origin_top1}")

figures[1].imshow(patch)
figures[1].set_title(f"Adversarial patch")

figures[2].imshow(adv_image)
figures[2].set_title(f"Adversarial image: classified as {adv_top1}")

plt.show()