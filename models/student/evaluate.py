import torch
from model import SocialSTGCNModel
from utils import ngsimDataset,gtaDataset,maskedNLL,maskedMSETest,maskedNLLTest
from torch.utils.data import DataLoader
import time


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
args['out_dim'] = 5
args['gru'] = False
args['z_size'] = 64
args['dset_name'] = 'ngsim'
args['dset_tag'] = 'highway'

args['device'] = torch.device('cpu')
if args['use_cuda']:
    args['device'] = torch.device('cuda:'+str(args['gpu']))


# Evaluation metric:
# metric = 'nll'  #or rmse
metric = 'rmse'


# Initialize network
net = SocialSTGCNModel(args)
net.load_state_dict(torch.load('../saved_models/student_stgcn.tar'))

state = torch.load('../saved_models/teacher_gmm_set.tar')
gmm_dict = state['gmm_dict']

tsSet = None
if args['dset_name'] == 'ngsim':
    tsSet = ngsimDataset('../../data/TestSetNGSIM.mat',gmm_dict,args['Mq'],args['z_size'])
elif args['dset_name'] == 'gta':
    tsSet = gtaDataset('../../data/TestSetGTA',args['dset_tag'],gmm_dict,args['Mq'],args['z_size'])
assert tsSet != None

tsDataloader = DataLoader(tsSet,batch_size=64,shuffle=True,num_workers=8,collate_fn=tsSet.collate_fn)

lossVals = torch.zeros(25).cuda()
counts = torch.zeros(25).cuda()


for i, data in enumerate(tsDataloader):
    st_time = time.time()
    hist, nbrs, mask, lat_enc, lon_enc, fut, op_mask, graph_list, _, _, nbrs_idx = data

    # Initialize Variables
    if args['use_cuda']:
        hist = hist.to(args['device'])
        nbrs = nbrs.to(args['device'])
        mask = mask.to(args['device'])
        fut = fut.to(args['device'])
        op_mask = op_mask.to(args['device'])
        nbrs_idx = nbrs_idx.to(args['device'])

    if metric == 'nll':
        # Forward pass
        fut_pred = net(hist, nbrs, mask, graph_list, nbrs_idx, mode='path')
        l, c = maskedNLLTest(fut_pred, 0, 0, fut, op_mask, use_maneuvers=False)
    else:
        # Forward pass
        fut_pred = net(hist, nbrs, mask, graph_list, nbrs_idx, mode='path')
        l, c = maskedMSETest(fut_pred, fut, op_mask)


    lossVals +=l.detach()
    counts += c.detach()

if metric == 'nll':
    print(lossVals / counts)
else:
    print(torch.pow(lossVals / counts,0.5)*0.3048)   # Calculate RMSE and convert from feet to meters


