import torch
from model import SocialSTGCNModel
from utils import variationUpdate,ngsimDataset,gtaDataset,klLoss,maskedNLL,maskedMSE,maskedNLLTest
from torch.utils.data import DataLoader
import time
import math


## Network Arguments
args = {}
args['use_cuda'] = True
args['gpu'] = 0
args['obs_len'] = 16
args['pred_len'] = 25
args['in_size'] = 2
args['s_kernel_size'] = 2
args['grid_size'] = (13, 3)
args['soc_conv_depth'] = 64
args['conv_3x1_depth'] = 16
args['dyn_hidden_size'] = 32
args['self_hidden_size'] = 32
args['enc_hidden_size'] = 64
args['dec_hidden_size'] = 128
args['pred_gmm_emb_size'] = 128
args['Mq'] = 4
args['Mp'] = 4
args['var_update'] = 4
args['lr_dist'] = 1e-5
args['lr_path'] = 1e-3
args['grad_clip'] = 10.0
args['out_dim'] = 5
args['gru'] = False
args['z_size'] = 64
args['dset_name'] = 'ngsim'
args['dset_tag'] = 'highway'

args['device'] = torch.device('cpu')
if args['use_cuda']:
	args['device'] = torch.device('cuda:'+str(args['gpu']))


# Initialize network
net =  SocialSTGCNModel(args)

state = torch.load('../saved_models/teacher_gmm_set.tar')
gmm_dict = state['gmm_dict']


# Initialize data loaders
batch_size = 64

trSet = None
if args['dset_name'] == 'ngsim':
	trSet = ngsimDataset('../../data/TrainSetNGSIM.mat',gmm_dict,args['Mq'],args['z_size'])
elif args['dset_name'] == 'gta':
	trSet = gtaDataset('../../data/TrainSetGTA',args['dset_tag'],gmm_dict,args['Mq'],args['z_size'])
assert trSet != None

trDataloader = DataLoader(trSet,batch_size=batch_size,shuffle=True,num_workers=2,collate_fn=trSet.collate_fn)

a = torch.rand(args['Mq'], args['Mp']).to(args['device'])
b = torch.rand(args['Mq'], args['Mp']).to(args['device'])

# Initialize optimizer
pretrainEpochs = 1
trainEpochs = 1

backbone_params = list(net.dyn.parameters())+ \
                  list(net.graph_conv.parameters())+ \
                  list(net.enc.parameters())+ \
                  list(net.hidden.parameters())+ \
                  list(net.soc_conv.parameters())+ \
                  list(net.conv_3x1.parameters())+ \
                  list(net.soc_maxpool.parameters())+ \
                  list(net.leaky_relu.parameters())
dist_params = backbone_params+list(net.pred_gmm.parameters())
path_params = backbone_params+list(net.dec.parameters())+list(net.output.parameters())

optim_dist = torch.optim.Adam(dist_params, lr=args['lr_dist'])
optim_path = torch.optim.Adam(path_params, lr=args['lr_path'])

# Variables holding train loss values:
train_dist_loss = []
train_path_loss = []
pre_val_loss = math.inf

dist_count = 0
for epoch_num in range(pretrainEpochs+trainEpochs):
	if epoch_num < pretrainEpochs:
		print('Training with KL and NLL loss')
	else:
		print('Training with NLL loss')

	## Train:_________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________

    # Variables to track training performance:
	avg_tr_dist_loss = 0
	avg_tr_path_loss = 0
	avg_tr_time = 0

	for ii, data in enumerate(trDataloader):

		st_time = time.time()
		hist, nbrs, mask, lat_enc, lon_enc, fut, op_mask, graph_list, mu_q, sigma_q, nbrs_idx = data

		if args['use_cuda']:
			hist = hist.to(args['device'])
			nbrs = nbrs.to(args['device'])
			mask = mask.to(args['device'])
			fut = fut.to(args['device'])
			op_mask = op_mask.to(args['device'])
			nbrs_idx = nbrs_idx.to(args['device'])
			mu_q = mu_q.to(args['device'])
			sigma_q = sigma_q.to(args['device'])

		# Forward pass

		if epoch_num < pretrainEpochs:

			# dist update
			mu_p, sigma_p = net(hist, nbrs, mask, graph_list, nbrs_idx, mode='dist')

			dist_count += 1
			l_dist, kldist_pd = klLoss(mu_p, sigma_p, mu_q, sigma_q, args['Mp'], args['Mq'], args['z_size'], a.clone())

			if dist_count % args['var_update'] == 0:
				variationUpdate(a, b, kldist_pd[-1], args['Mp'], args['Mq'])

			optim_dist.zero_grad()
			l_dist.backward()
			torch.nn.utils.clip_grad_norm_(dist_params, args['grad_clip'])
			optim_dist.step()

		# path update
		fut_pred = net(hist, nbrs, mask, graph_list, nbrs_idx, mode='path')
		l_path = maskedNLL(fut_pred, fut, op_mask)

		optim_path.zero_grad()
		l_path.backward()
		torch.nn.utils.clip_grad_norm_(path_params, args['grad_clip'])
		optim_path.step()

		# Track average train loss and average train time:
		batch_time = time.time()-st_time
		avg_tr_dist_loss += l_dist.item()
		avg_tr_path_loss += l_path.item()
		avg_tr_time += batch_time

		if ii%100 == 99:
			eta = avg_tr_time/100*(len(trSet)/batch_size-ii)
			print("Epoch no:",epoch_num+1,"| Epoch progress(%):",format(ii/(len(trSet)/batch_size)*100,'0.2f'), "| Avg train loss dist:",format(avg_tr_dist_loss/100,'0.4f'), "| Avg train loss path:",format(avg_tr_path_loss/100,'0.4f'), "| ETA(s):",int(eta))
			avg_tr_dist_loss = 0
			avg_tr_path_loss = 0
			avg_tr_time = 0

torch.save(net.state_dict(), '../saved_models/student_stgcn.tar')
