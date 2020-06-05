import torch
from model import GCNVAE, GMM
from utils import ngsimDataset
from torch.utils.data import DataLoader

import numpy as np
import time
import math
import os

## Network Arguments
args = {}
args['use_cuda'] = True
args['gpu'] = 0
args['obs_len'] = 16
args['pred_len'] = 25
args['in_channels'] = 4
args['h_dim1'] = 128
args['h_dim2'] = 64
args['M'] = 4
args['z_dim'] = 64
args['gmm_emb_dim'] = 128

assert args['h_dim2'] == args['z_dim']

args['device'] = torch.device('cpu')
if args['use_cuda']:
	args['device'] = torch.device('cuda:'+str(args['gpu']))

# Initialize network
net_vae = GCNVAE(args)
net_vae.load_state_dict(torch.load('../saved_models/teacher_vae.tar'))

net_gmm = GMM(args)
net_gmm.load_state_dict(torch.load('../saved_models/teacher_gmm.tar'))

## Initialize data loaders
batch_size = 32
trSet = ngsimDataset('../../data/TrainSet.mat')
trDataloader = DataLoader(trSet,batch_size=batch_size,shuffle=False,num_workers=2,collate_fn=trSet.collate_fn)

## Infer mu_q, sigma_q and dump the dict
gmm_count = 0
gmm_dict = {}
avg_time = 0

net_vae.training = False
for i, data in enumerate(trDataloader):
	st_time = time.time()
	data_list, graph_list = data

	for idx in range(len(data_list)):
		inputs = data_list[idx].to(args['device'])
		As = graph_list[idx][0].to(args['device'])
		
		_, mu, _ = net_vae(inputs, As)
		mu = mu.mean(dim=1).detach()

		_, mu_q, sigma_q = net_gmm(mu)
		mu_q, sigma_q = mu_q.detach().cpu(), sigma_q.detach().cpu()

		gmm_dict[gmm_count] = (mu_q[-1], sigma_q[-1])
		gmm_count += 1

	batch_time = time.time()-st_time
	avg_time += batch_time

	if i % 100 == 99:
		eta = avg_time/100*(len(trSet)/batch_size-i)
		print("gmm ETA(s):", int(eta))
		avg_time = 0

print('****** saving teacher gmm ******')
state = {}
state['gmm_dict'] = gmm_dict

torch.save(state, '../saved_models/teacher_gmm_set.tar')
