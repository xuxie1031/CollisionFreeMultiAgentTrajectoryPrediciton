import torch
import torch.nn as nn
from utils import *

class CPM(nn.Module):
    def __init__(self, h_dim, emb_dim, mu_dim, M=4, activation='tanh', batch_norm=False, dropout=0.0):
        super(CPM, self).__init__()
        self.M = M
        self.mu_dim = mu_dim

        self.enc_h = nn.Linear(h_dim, emb_dim)
        self.enc_mu = nn.Linear(emb_dim, M*mu_dim)
        self.enc_sigma = nn.Linear(emb_dim, M)

        if activation == 'tanh':
            self.activation = nn.Tanh()
        elif activation == 'relu':
            self.activation = nn.ReLU()
        elif activation == 'leakyrelu':
            self.activation = nn.LeakyReLU()


    def forward(self, x):
        x = self.activation(self.enc_h(x))

        mu = self.enc_mu(x).view(-1, self.M, self.mu_dim)
        sigma = torch.exp(self.enc_sigma(x))

        return mu, sigma


class GraphConv(nn.Module):
    def __init__(self, args):
        super(GraphConv, self).__init__()

        self.s_kernel_size = args['s_kernel_size']
        self.s_conv = torch.nn.Conv1d(args['dyn_hidden_size'], args['dyn_hidden_size']*args['s_kernel_size'], 1)

    def forward(self, ngbr, graph):
        ngbr = ngbr.permute(0, 2, 1)
        ngbr = self.s_conv(ngbr)
        n, kc, v = ngbr.size()
        ngbr = ngbr.view(n, self.s_kernel_size, kc // self.s_kernel_size, v)
        ngbr = torch.einsum('nkcv,kvw->ncw', (ngbr, graph))
        ngbr = ngbr.permute(0, 2, 1)

        return ngbr


class SocialSTGCNModel(nn.Module):
    def __init__(self, args):
        super(SocialSTGCNModel, self).__init__()

        self.s_kernel_size = args['s_kernel_size']
        self.enc_dim = args['enc_hidden_size']
        self.pred_len = args['pred_len']
        self.out_dim = args['out_dim']
        self.device = args['device']

        self.soc_emb_size = (((args['grid_size'][0]-4)+1)//2)*args['conv_3x1_depth']
        self.soc_conv = torch.nn.Conv2d(args['enc_hidden_size'], args['soc_conv_depth'], 3)
        self.conv_3x1 = torch.nn.Conv2d(args['soc_conv_depth'], args['conv_3x1_depth'], (3, 1))
        self.soc_maxpool = torch.nn.MaxPool2d((2, 1), padding=(1, 0))

        self.graph_conv = GraphConv(args)

        self.pred_gmm = CPM(
            self.soc_emb_size+args['self_hidden_size'],
            args['pred_gmm_emb_size'],
            args['z_size'],
            M=args['Mp'],
        )
        
        self.dyn = nn.Linear(args['in_size'], args['dyn_hidden_size'])

        self.enc = nn.LSTM(args['dyn_hidden_size'], args['enc_hidden_size'])
        if args['gru']:
            self.enc = nn.GRU(args['dyn_hidden_size'], args['enc_hidden_size'])

        self.hidden = nn.Linear(args['enc_hidden_size'], args['self_hidden_size'])

        self.dec = nn.LSTM(self.soc_emb_size+args['self_hidden_size'], args['dec_hidden_size'])
        if args['gru']:
            self.dec = nn.GRU(self.soc_emb_size+args['self_hidden_size'], args['dec_hidden_size'])

        self.output = nn.Linear(args['dec_hidden_size'], args['out_dim'])

        self.leaky_relu = nn.LeakyReLU(0.1)

        if args['use_cuda']:
            self.to(args['device'])


    def forward(self, x, ngbrs, masks, graph_list, ngbrs_idx, mode='path'):
        N = x.size(1)

        x = self.leaky_relu(self.dyn(x))
        ngbrs = self.leaky_relu(self.dyn(ngbrs))

        _, tup_enc = self.enc(x)
        x = tup_enc[0].view(N, self.enc_dim)

        ngbrs_list = []
        for num, graph in enumerate(graph_list):
            if ngbrs_idx[num] == ngbrs_idx[num+1]: continue
            graph = graph.to(self.device)
            ngbr = ngbrs[:, ngbrs_idx[num]:ngbrs_idx[num+1], :]
            ngbr = self.graph_conv(ngbr, graph)
            ngbrs_list.append(ngbr)
        ngbrs = torch.cat(ngbrs_list, 1)

        _, tup_enc = self.enc(ngbrs)
        ngbrs = tup_enc[0].view(-1, self.enc_dim)

        soc_enc = torch.zeros_like(masks).float()
        soc_enc = soc_enc.masked_scatter_(masks, ngbrs)
        soc_enc = soc_enc.permute(0, 3, 2, 1)
        soc_enc = self.soc_maxpool(self.leaky_relu(self.conv_3x1(self.leaky_relu(self.soc_conv(soc_enc)))))		
        soc_enc = soc_enc.view(-1, self.soc_emb_size)

        x = self.leaky_relu(self.hidden(x))

        x = torch.cat((x, soc_enc), 1)

        if mode == 'dist':
            mu_p, sigma_p = self.pred_gmm(x)
            return mu_p, sigma_p

        elif mode == 'path':
            x = x.repeat(self.pred_len, 1, 1)
            h_dec, _ = self.dec(x)
            h_dec = h_dec.permute(1, 0, 2)
            o = self.output(h_dec)
            o = o.permute(1, 0, 2)

            return outputActivation(o)
