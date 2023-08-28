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

# ImageNet 데이터 로드
# 아래 폴더에서 임의의 사진 1개 로드
TARGET_DIR = "dataset"
files = os.listdir(TARGET_DIR)
file = random.choice(files)
img = Image.open(os.path.join(TARGET_DIR, file))
print(f"[*] Loaded random image from ImageNet: {file}")


# 이미지 형태를 4차원 텐서로 변환
# 이미지 normalization 후 공격 진행하고 싶은 경우 아래 변수 수정
DO_NORMALIZE = False
if DO_NORMALIZE:
    MEAN = [0.485, 0.456, 0.406]
    STD = [0.229, 0.224, 0.225]
    transform = transforms.Compose(
        [
            transforms.ToTensor(),
            transforms.Normalize(MEAN, STD)]
        )
    print(f"[*] Preprocessing with Image -> ToTensor -> Normalize -> Tensor")
else:
    transform = transforms.Compose(
        [transforms.ToTensor()]
        )
    print(f"[*] Preprocessing with Image -> ToTensor -> Tensor")
image = transform(img).unsqueeze(0)

# 공격 전 분류값 추출
prediction = model(image)
values, indexes = torch.topk(prediction, 5)
classifications = [labels[index] for index in indexes[0].tolist()]
origin_top1 = classifications[0]
print(f"[*] Original image is classified as '{origin_top1}'")

# PGD 공격 파라미터
eps = 0.01
alpha = 2/255
random_start = True

# PGD 공격 수행
# 데이터 로드
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
model = model.to(device)
image = image.to(device)
_, target = torch.max(prediction, 1)
target = target.to(device)

loss = torch.nn.CrossEntropyLoss()

if random_start:
    adv_image = image + torch.empty_like(image).uniform_(-eps, eps)
    adv_image = torch.clamp(adv_image, 0, 1)
    print(f"[*] Start image' = image + ε, (|ε| < {eps})")
else:
    adv_image = torch.clamp(image, 0, 1)
    print(f"[*] Start image' = image + ε, (ε = 0)")

ITER = 40
for i in range(0, ITER):
    adv_image.requires_grad = True
    output = model(adv_image)

    model.zero_grad()
    cost = loss(output, target).to(device)
    cost.backward()

    attack_images = adv_image + alpha*adv_image.grad.sign()
    eta = torch.clamp(attack_images - image, min=-eps, max=eps)
    print(f"[*] η = ε + α * sign(image')")
    adv_image = torch.clamp(image + eta, min=0, max=1).detach_()
    print(f"[*] image* = image + η")

perturbation = adv_image - image

# 공격 후 분류값 추출
prediction = model(adv_image)
values, indexes = torch.topk(prediction, 5)
classifications = [labels[index] for index in indexes[0].tolist()]
adv_top1 = classifications[0]
print(f"[*] Adversarial image is classified as '{adv_top1}'")

# 공격 성공 검증
if origin_top1 != adv_top1:
    print(f"[+] PGD perturbation attack successful! ({origin_top1} != {adv_top1})")
else:
    print(f"[+] PGD perturbation attack failed! ({origin_top1} == {adv_top1})")

# 원본, 노이즈, 공격 이미지 출력
_, figures = plt.subplots(1, 3, figsize=(15, 5))

figures[0].imshow(image.squeeze(0).permute(1, 2, 0).clamp(0, 1))
figures[0].set_title(f"Original image: classified as {origin_top1}")

MULTIPLIER = 50
figures[1].imshow(perturbation.squeeze(0).permute(1, 2, 0).clip(0, 255) * MULTIPLIER)
figures[1].set_title(f"Perturbation: amplified by x{MULTIPLIER}")

figures[2].imshow(adv_image.squeeze(0).permute(1, 2, 0).clamp(0, 1))
figures[2].set_title(f"Adversarial image: classified as {adv_top1}")

plt.show()