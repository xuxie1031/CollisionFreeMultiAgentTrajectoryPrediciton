import torch
from model import GCNVAE, GMM
from utils import ngsimDataset, gtaDataset, vae_loss, gmm_loss
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
args['lr_vae'] = 1e-4
args['lr_gmm'] = 1e-6
args['grad_clip'] = 10.0
args['dset_name'] = 'ngsim'
args['dset_tag'] = 'highway'

assert args['h_dim2'] == args['z_dim']

args['device'] = torch.device('cpu')
if args['use_cuda']:
	args['device'] = torch.device('cuda:'+str(args['gpu']))

# Initialize network
net_vae = GCNVAE(args)
net_gmm = GMM(args)

## Initialize optimizer
pretrainEpochs = 0
trainEpochs = 1
optim_vae = torch.optim.Adam(net_vae.parameters(), lr=args['lr_vae'])
optim_gmm = torch.optim.Adam(net_gmm.parameters(), lr=args['lr_gmm'])
batch_size = 32

## Initialize data loaders
trSet = None
if args['dset_name'] == 'ngsim':
	trSet = ngsimDataset('../../data/TeacherSetNGSIM.mat')
elif args['dset_name'] == 'gta':
	trSet = gtaDataset('../../data/TeacherSetGTA', args['dset_tag'])
assert trSet != None

trDataloader = DataLoader(trSet,batch_size=batch_size,shuffle=True,num_workers=2,collate_fn=trSet.collate_fn)

## Variables holding train and validation loss values:
vae_losses = []
gmm_losses = []

for epoch_num in range(pretrainEpochs+trainEpochs):
	print('Training with vae loss')
	num_batch = 0

	## Train:_________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________

	# Variables to track training performance:

	avg_tr_loss_vae = 0
	avg_tr_loss_gmm = 0
	avg_tr_time = 0

	for i, data in enumerate(trDataloader):
		st_time = time.time()
		data_list, graph_list = data

		for idx in range(len(data_list)):
			inputs = data_list[idx]
			num_nodes = inputs.size(-1)

			As, pos_weights, norms, targets = graph_list[idx]

			if args['use_cuda']:
				inputs = inputs.to(args['device'])
				As = As.to(args['device'])
				pos_weights = pos_weights.to(args['device'])
				norms = norms.to(args['device'])
				targets = targets.to(args['device'])

			# gcn-vae 
			net_vae.training = True
			recovered, mu, logvar = net_vae(inputs, As)
			l_vae = vae_loss(recovered, targets, mu, logvar, num_nodes, norms, pos_weights, args['device'])
			avg_tr_loss_vae += l_vae.item() / len(data_list)

			optim_vae.zero_grad()
			l_vae.backward()
			torch.nn.utils.clip_grad_norm_(net_vae.parameters(), args['grad_clip'])
			optim_vae.step()

			# gmm
			net_vae.training = False
			_, mu, _ = net_vae(inputs, As)
			mu = mu.mean(dim=1).detach()

			lambd, mu_q, sigma_q = net_gmm(mu)
			l_gmm = gmm_loss(mu, mu_q, sigma_q, args['M'], lambd)
			avg_tr_loss_gmm += l_gmm.item() / len(data_list)

			optim_gmm.zero_grad()
			l_gmm.backward()
			torch.nn.utils.clip_grad_norm_(net_gmm.parameters(), args['grad_clip'])
			optim_gmm.step()

		batch_time = time.time()-st_time
		avg_tr_time += batch_time

		num_batch += 1

		if i%100 == 99:
			eta = avg_tr_time/100*(len(trSet)/batch_size-i)
			print("Epoch no:",epoch_num+1,"| Epoch progress(%):",format(i/(len(trSet)/batch_size)*100,'0.2f'), "| Avg train loss vae:",format(avg_tr_loss_vae/100,'0.4f'), "| Avg train loss gmm:",format(avg_tr_loss_gmm/100,'0.4f'),"| ETA(s):",int(eta))
			avg_tr_loss_vae = 0
			avg_tr_loss_gmm = 0
			avg_tr_time = 0


print('****** saving model ******')
if not os.path.exists('../saved_models'): os.makedirs('../saved_models')
torch.save(net_vae.state_dict(), '../saved_models/teacher_vae.tar')
torch.save(net_gmm.state_dict(), '../saved_models/teacher_gmm.tar')
