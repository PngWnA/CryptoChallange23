
import sys

import torch
import torch.nn as nn

sys.path.insert(0, '..')


from torchvision import models #loading model
from utils import get_imagenet_data #loading model
from robustbench.utils import clean_accuracy #loading model: A collection of robust models for RobustBench: a standardized adversarial robustness benchmark
import numpy as np 
from utils import imshow, get_pred


import torch
import torch.nn.functional as F

import torch
import torch.nn as nn
import torch.nn.functional as F
from abc import ABC, abstractmethod

class Attacker(ABC):
    def __init__(self, model, config):
        """
        ## initialization ##
        :param model: Network to attack
        :param config : configuration to init the attack
        """
        self.config = config
        self.model = model
        self.clamp = (0,1)
    
    def _random_init(self, x):
        x = x + (torch.rand(x.size(), dtype=x.dtype, device=x.device) - 0.5) * 2 * self.config['eps']
        x = torch.clamp(x,*self.clamp)
        return x

    def __call__(self, x,y):
        x_adv = self.forward(x,y)
        return x_adv


class PGD(Attacker):
    def __init__(self, model, config, target=None):
        super(PGD, self).__init__(model, config)
        self.target = target

    def forward(self, x, y):
        """
        :param x: Inputs to perturb
        :param y: Ground-truth label
        :param target : Target label 
        :return adversarial image
        """
        x_adv = x.detach().clone()
        if self.config['random_init'] :
            x_adv = self._random_init(x_adv)
        for _ in range(self.config['attack_steps']):
            x_adv.requires_grad = True
            self.model.zero_grad()
            logits = self.model(x_adv) #f(T((x))
            if self.target is None:
                # Untargeted attacks - gradient ascent
                
                loss = F.cross_entropy(logits, y,  reduction="sum")
                loss.backward()                      
                grad = x_adv.grad.detach()
                grad = grad.sign()
                x_adv = x_adv + self.config['attack_lr'] * grad
            else:
                # Targeted attacks - gradient descent
                assert self.target.size() == y.size()           
                loss = F.cross_entropy(logits, self.target)
                loss.backward()
                grad = x_adv.grad.detach()
                grad = grad.sign()
                x_adv = x_adv - self.config['attack_lr'] * grad

            # Projection
            x_adv = x + torch.clamp(x_adv - x, min=-self.config['eps'], max=self.config['eps'])
            x_adv = x_adv.detach()
            x_adv = torch.clamp(x_adv, *self.clamp)

        return x_adv

attack_config = {
    'eps' : 8.0/255.0, 
    'attack_steps': 10,
    'attack_lr': 2.0 / 255.0, 
    'random_init': True, 
}

images, labels = get_imagenet_data()
print('[Data loaded]') #loading data

device = "cuda:0" if torch.cuda.is_available() else "cpu" #change device from 'cuda' to 'cuda or cpu'
model = models.resnet18(pretrained=True).to(device).eval() #loads pretrained model 
acc = clean_accuracy(model, images.to(device), labels.to(device))
print('[Model loaded]') #loading model
print('Acc: %2.2f %%'%(acc*100))

attack = PGD(model, attack_config)
print(attack)
adversarial_image = attack(images, labels)
idx = 0
pre = get_pred(model, adversarial_image[idx:idx+1], device)
imshow(adversarial_image[idx:idx+1], title="True:%d, Pre:%d"%(labels[idx], pre))