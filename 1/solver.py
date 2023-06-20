
import sys

import torch
import torch.nn as nn

sys.path.insert(0, '..')


from torchvision import models #loading model
from utils import get_imagenet_data #loading model
from robustbench.utils import clean_accuracy #loading model: A collection of robust models for RobustBench: a standardized adversarial robustness benchmark
import numpy as np 
from utils import imshow, get_pred

# def vmni_ditisi_fgsm(attack_type, model, x, y, target_label=-1, num_iter=20, max_epsilon=16, step_size=1, mu=1.0,
#                      number_of_v_samples=5, beta=1.5,
#                      number_of_si_scales=4, constraint_img=None, di_prob=0.5, di_pad_amount=31, di_pad_value=0,
#                      ti_kernel_size=7, every_step_controller=None):
#     """
#     Perform momentum ifgsm attack with respect to model on images
#     x with labels y
#     Args:
#         attack_type: string containing 'M'(momentum) or 'N'(Nesterov momentum) / 'V'(variance tuning)
#         model: torch model with respect to which attacks will be computed
#         x: batch of torch images. in range [-1.0, 1.0]
#         target_label : used for targeted attack. For untargeted attack, set this to -1.
#         y: true labels corresponding to the batch of images
#         num_iter: T. number of iterations of ifgsm to perform.
#         max_epsilon: Linf norm of resulting perturbation (in pixels)
#         step_size: step size (in pixels)
#         mu: mu. decay of momentum. If mu==0, then momentum is not used.
#         number_of_v_samples: N. # samples to calculate V
#         beta: the bound for variance tuning. If beta==0, then gradient variance is not used.
#         number_of_si_scales: m. (in scale-invariance paper)
#         constraint_img: used only for special cases. This image becomes the mid-point of Lp bound.
#         di_prob: p (in diverse-input paper). Probability of applying diverse-input method.
#         di_pad_amount: (in diverse-input paper) Image will be enlarged by di_pad pixels in width and height.
#         di_pad_value: (in diverse-input paper) Value used for padding. Note: Padding will be done after input transform.
#         ti_kernel_size: (in translation-invariance paper) k. The kernel will be a k by k Gaussian kernel.

#     Returns:
#         The batch of adversarial examples corresponding to the original images
#     """

#     if "M" not in attack_type and "N" not in attack_type:
#         mu = 0

#     if target_label >= 0:
#         y = target_label

#     if 'T' in attack_type:  # Create smoothing kernel for translation-invariance.
#         kernel = gkern(ti_kernel_size, 3).astype(np.float32)
#         stack_kernel = np.stack([kernel, kernel, kernel])
#         stack_kernel = np.expand_dims(stack_kernel, 1)
#         ti_conv = torch.nn.Conv2d(in_channels=3, out_channels=3, kernel_size=(ti_kernel_size, ti_kernel_size), padding=ti_kernel_size//2, groups=3, bias=False,)
#         with torch.no_grad():
#             ti_conv.weight = nn.Parameter(torch.from_numpy(stack_kernel).float().cuda())
#             ti_conv.requires_grad_(False)
#     model.eval()

#     eps = 2.0 * max_epsilon / 255.0  # epsilon in scale [-1, 1]
#     alpha = 2.0 * step_size / 255.0  # alpha in scale [-1, 1]

#     if constraint_img is not None:
#         x_min = torch.clamp(constraint_img - eps, -1.0, 1.0)
#         x_max = torch.clamp(constraint_img + eps, -1.0, 1.0)
#     else:
#         x_min = torch.clamp(x - eps, -1.0, 1.0)
#         x_max = torch.clamp(x + eps, -1.0, 1.0)

#     x_adv = x.clone()

#     elastic_prob = 0.5

#     g = 0
#     v = 0
#     for t in range(num_iter):
#         # every step function
#         if every_step_controller is not None:
#             if not hasattr(every_step_controller, "__iter__"):
#                 every_step_controller = [every_step_controller]
#             for controller in every_step_controller:
#                 controller.next_step_modification()

#         # Calculate ghat
#         if 'N' in attack_type:  # Nesterov momentum
#             x_nes = x_adv.detach() + alpha * mu * g
#             x_nes.requires_grad = True
#             x_adv_or_nes = x_nes
#         else:  # usual momentum
#             x_adv = x_adv.detach()
#             x_adv.requires_grad = True
#             x_adv_or_nes = x_adv


#         if 'S' in attack_type:  # Scale-Invariance
#             ghat = calculate_si_ghat(model, x_adv_or_nes, y, number_of_si_scales, target_label, attack_type, di_prob, di_pad_amount, di_pad_value, elastic_prob)
#         else:
#             if 'E' in attack_type:  # Elastic transformation
#                 output2 = model(elastic_input(x_adv_or_nes, elastic_prob))

#             elif 'D' in attack_type:  # Diverse-Input
#                 output2 = model(diverse_input(x_adv_or_nes, di_prob, di_pad_amount, di_pad_value))
#             else:
#                 output2 = model(x_adv_or_nes)

#             loss = get_loss(output2, y)
#             if target_label >= 0:
#                 loss = -loss
#             ghat = torch.autograd.grad(loss, x_adv_or_nes,
#                 retain_graph=False, create_graph=False)[0]

#         # Update g
#         grad_plus_v = ghat + v
#         if 'T' in attack_type:  # Translation-invariance
#             grad_plus_v = ti_conv(grad_plus_v)
#         g = mu * g + grad_plus_v / torch.sum(torch.abs(grad_plus_v), dim=[1,2,3], keepdim=True)

#         # Update v
#         if 'V' in attack_type:
#             v = calculate_v(model, x_adv_or_nes, y, eps, number_of_v_samples, beta, target_label, attack_type, number_of_si_scales, di_prob, di_pad_amount, di_pad_value) - ghat

#         # Update x_adv
#         pert = alpha * g.sign()
#         x_adv = x_adv.detach() + pert
#         x_adv = torch.clamp(x_adv, x_min, x_max)

#     return x_adv.detach()


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

# vmni_ditisi_fgsm("M", model, images, labels)
attack = PGD(model, attack_config)
print(attack)
adversarial_image = attack(images, labels)
idx = 0
pre = get_pred(model, adversarial_image[idx:idx+1], device)
imshow(adversarial_image[idx:idx+1], title="True:%d, Pre:%d"%(labels[idx], pre))