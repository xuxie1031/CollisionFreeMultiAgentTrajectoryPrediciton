from __future__ import print_function, division
from torch.utils.data import Dataset, DataLoader
import scipy.io as scp
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.distributions as tdist

import os

from graph import Graph

#___________________________________________________________________________________________________________________________

### Dataset class for the NGSIM dataset
class ngsimDataset(Dataset):


    def __init__(self, mat_file, gmm_dict, M, z_dim, t_h=30, t_f=50, d_s=2, enc_size = 64, grid_size = (13,3)):
        self.D = scp.loadmat(mat_file)['traj']
        self.T = scp.loadmat(mat_file)['tracks']
        self.gmm_dict = gmm_dict
        self.M = M
        self.z_dim = z_dim
        self.t_h = t_h  # length of track history
        self.t_f = t_f  # length of predicted trajectory
        self.d_s = d_s  # down sampling rate of all sequences
        self.enc_size = enc_size # size of encoder LSTM
        self.grid_size = grid_size # size of social context grid


    def __len__(self):
        return len(self.D)



    def __getitem__(self, idx):

        dsId = self.D[idx, 0].astype(int)
        vehId = self.D[idx, 1].astype(int)
        t = self.D[idx, 2]
        grid = self.D[idx,8:]
        neighbors = []

        # Get track history 'hist' = ndarray, and future track 'fut' = ndarray
        hist = self.getHistory(vehId,t,vehId,dsId)
        fut = self.getFuture(vehId,t,dsId)

        # Get track histories of all neighbours 'neighbors' = [ndarray,[],ndarray,ndarray]
        for i in grid:
            neighbors.append(self.getHistory(i.astype(int), t,vehId,dsId))

        # Maneuvers 'lon_enc' = one-hot vector, 'lat_enc = one-hot vector
        lon_enc = np.zeros([2])
        lon_enc[int(self.D[idx, 7] - 1)] = 1
        lat_enc = np.zeros([3])
        lat_enc[int(self.D[idx, 6] - 1)] = 1

        mu_q, sigma_q = self.gmm_dict[idx]

        return hist,fut,neighbors,lat_enc,lon_enc,mu_q,sigma_q


    ## Helper function to get track history
    def getHistory(self,vehId,t,refVehId,dsId):
        if vehId == 0:
            return np.empty([0,2])
        else:
            if self.T.shape[1]<=vehId-1:
                return np.empty([0,2])
            refTrack = self.T[dsId-1][refVehId-1].transpose()
            vehTrack = self.T[dsId-1][vehId-1].transpose()
            refPos = refTrack[np.where(refTrack[:,0]==t)][0,1:3]

            if vehTrack.size==0 or np.argwhere(vehTrack[:, 0] == t).size==0:
                 return np.empty([0,2])
            else:
                stpt = np.maximum(0, np.argwhere(vehTrack[:, 0] == t).item() - self.t_h)
                enpt = np.argwhere(vehTrack[:, 0] == t).item() + 1
                hist = vehTrack[stpt:enpt:self.d_s,1:3]-refPos

            if len(hist) < self.t_h//self.d_s + 1:
                return np.empty([0,2])
            return hist



    ## Helper function to get track future
    def getFuture(self, vehId, t,dsId):
        vehTrack = self.T[dsId-1][vehId-1].transpose()
        refPos = vehTrack[np.where(vehTrack[:, 0] == t)][0, 1:3]
        stpt = np.argwhere(vehTrack[:, 0] == t).item() + self.d_s
        enpt = np.minimum(len(vehTrack), np.argwhere(vehTrack[:, 0] == t).item() + self.t_f + 1)
        fut = vehTrack[stpt:enpt:self.d_s,1:3]-refPos
        return fut



    ## Collate function for dataloader
    def collate_fn(self, samples):

        # Initialize neighbors and neighbors length batches:
        nbr_batch_size = 0
        for _,_,nbrs,_,_,_,_ in samples:
            nbr_batch_size += sum([len(nbrs[i])!=0 for i in range(len(nbrs))])
        maxlen = self.t_h//self.d_s + 1
        nbrs_batch = torch.zeros(maxlen,nbr_batch_size,2)


        # Initialize social mask batch:
        pos = [0, 0]
        mask_batch = torch.zeros(len(samples), self.grid_size[1],self.grid_size[0],self.enc_size)
        mask_batch = mask_batch.bool()


        # Initialize history, history lengths, future, output mask, lateral maneuver and longitudinal maneuver batches:
        hist_batch = torch.zeros(maxlen,len(samples),2)
        fut_batch = torch.zeros(self.t_f//self.d_s,len(samples),2)
        op_mask_batch = torch.zeros(self.t_f//self.d_s,len(samples),2)
        lat_enc_batch = torch.zeros(len(samples),3)
        lon_enc_batch = torch.zeros(len(samples), 2)
        nbrs_idx_batch = torch.zeros(len(samples)+1).long()
        mu_q_batch = torch.zeros(len(samples), self.M, self.z_dim)
        sigma_q_batch = torch.zeros(len(samples), self.M)

        count = 0
        graph_list = []
        for sampleId,(hist, fut, nbrs, lat_enc, lon_enc, mu_q, sigma_q) in enumerate(samples):

            # Set up history, future, lateral maneuver and longitudinal maneuver batches:
            hist_batch[0:len(hist),sampleId,0] = torch.from_numpy(hist[:, 0])
            hist_batch[0:len(hist), sampleId, 1] = torch.from_numpy(hist[:, 1])
            fut_batch[0:len(fut), sampleId, 0] = torch.from_numpy(fut[:, 0])
            fut_batch[0:len(fut), sampleId, 1] = torch.from_numpy(fut[:, 1])
            op_mask_batch[0:len(fut),sampleId,:] = 1
            lat_enc_batch[sampleId,:] = torch.from_numpy(lat_enc)
            lon_enc_batch[sampleId, :] = torch.from_numpy(lon_enc)
            mu_q_batch[sampleId, :] = mu_q
            sigma_q_batch[sampleId, :] = sigma_q

            hist_data = torch.zeros(len(hist), 1, 2)
            hist_data[:, 0, :] = hist_batch[:, sampleId, :]

            # Set up neighbor, neighbor sequence length, and mask batches:
            for id,nbr in enumerate(nbrs):
                if len(nbr)!=0:
                    nbrs_batch[0:len(nbr),count,0] = torch.from_numpy(nbr[:, 0])
                    nbrs_batch[0:len(nbr), count, 1] = torch.from_numpy(nbr[:, 1])
                    pos[0] = id % self.grid_size[0]
                    pos[1] = id // self.grid_size[0]
                    mask_batch[sampleId,pos[1],pos[0],:] = torch.ones(self.enc_size).bool()
                    count+=1

            nbrs_idx_batch[sampleId+1] = count

            # graph
            graph_templates = torch.zeros(nbrs_idx_batch[sampleId+1]-nbrs_idx_batch[sampleId], 4)
            graph_templates[:, :2] = nbrs_batch[0, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :]
            graph_templates[:, 2:] = nbrs_batch[1, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :] - \
                                     nbrs_batch[0, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :]
            
            graph = Graph(graph_templates)
            graph_list.append(graph.normalize_undigraph())

        return hist_batch, nbrs_batch, mask_batch, lat_enc_batch, lon_enc_batch, fut_batch, op_mask_batch, graph_list, mu_q_batch, sigma_q_batch, nbrs_idx_batch



class gtaDataset(Dataset):

    def __init__(self, dset_path, tag='highway', gmm_dict, M, z_dim, t_h=30, t_f=50, d_s=2, enc_size=64, grid_size=(13, 3)):
        self.data_dir = os.path.join(dset_path, tag)
        self.t_h = t_h
        self.t_f = t_f
        self.d_s = d_s
        self.gmm_dict = gmm_dict
        self.M = M
        self.z_dim = z_dim
        self.enc_size = enc_size
        self.grid_size = grid_size
        
        self.seq_list = []

        dataPipeline()


    
    def __len__(self):
        return len(self.seq_list)



    def __getitem__(self, idx):
        hist, fut, nbrs = self.seq_list[idx]
        mu_q, sigma_q = self.gmm_dict[idx]

        return hist, fut, nbrs, mu_q, sigma_q



    def readFile(self, path, delim=','):
        data = []
        with open(path, 'r') as f:
            for line in f:
                line = line.strip().split(delim)
                line = [float(i) for i in line]
                data.append(line)

        return np.asarray(data)



    def getHistory(vehTrack, t, refTrack):
        refPos = refTrack[np.where(refTrack[:, 0] == t)][0, 2:4]

        if vehTrack.size == 0 or np.argwhere(vehTrack[:, 0] == t).size == 0:
            return np.empty([0, 2])
        else:
            stpt = np.maximum(0, np.argwhere(vehTrack[:, 0] == t).item() - self.t_h)
            enpt = np.argwhere(vehTrack[:, 0] == t).item()+1
            hist = vehTrack[stpt:enpt:self.d_s, 2:4]-refPos

        if len(hist) < self.t_h//self.d_s + 1:
            return np.empty([0, 2])
        return hist



    def getFuture(vehTrack, t):
        refPos = vehTrack[np.where(vehTrack[:, 0] == t)][0, 2:4]

        stpt = np.argwhere(vehTrack[:, 0] == t).item() + self.d_s
        enpt = np.minimum(len(vehTrack), np.argwhere(vehTrack[:, 0] == t).item() + self.t_f + 1)
        fut = vehTrack[stpt:enpt:self.d_s, 2:4]-refPos

        return fut



    def dataPipeline(self):
        all_files = os.listdir(self.data_dir)
        all_files = [os.path.join(self.data_dir, path) for path in all_files]

        # each file represents a trial
        for path in all_files:
            raw_data = readFile(path)
            
            data = raw_data[np.argsort(raw_data[:, 1])]
            vids = np.unique(data[:, 1])

            for vi in vids:
                track_data = data[data[:, 1] == vi, :]

                for idx in range(len(track_data)):
                    t = track_data[idx, 0]
                    hist = getHistory(track_data, t, track_data)
                    fut = getFuture(track_data, t)

                    nbrs = []
                    for vj in vids:
                        if vi == vj: pass
                        
                        ref_data = data[data[:, 1] == vj, :]
                        nbr = getHistory(track_data, t, ref_data)
                        nbrs.append(nbr)
                    
                    self.seq_list.append(hist, fut, nbrs)



    def collate_fn(self, samples):
        nbr_batch_size = 0
        for _,_,nbrs,_,_,_,_ in samples:
            nbr_batch_size += sum([len(nbrs[i])!=0 for i in range(len(nbrs))])
        maxlen = self.t_h//self.d_s + 1
        nbrs_batch = torch.zeros(maxlen,nbr_batch_size,2)

        pos = [0, 0]
        mask_batch = torch.zeros(len(samples), self.grid_size[1],self.grid_size[0],self.enc_size)
        mask_batch = mask_batch.bool()

        hist_batch = torch.zeros(maxlen,len(samples),2)
        fut_batch = torch.zeros(self.t_f//self.d_s,len(samples),2)
        op_mask_batch = torch.zeros(self.t_f//self.d_s,len(samples),2)        
        nbrs_idx_batch = torch.zeros(len(samples)+1).long()
        mu_q_batch = torch.zeros(len(samples), self.M, self.z_dim)
        sigma_q_batch = torch.zeros(len(samples), self.M)

        count = 0
        graph_list = []
        for sampleId, (hist, fut, nbrs, mu_q, sigma_q) in enumerate(samples):
            hist_batch[0:len(hist),sampleId,0] = torch.from_numpy(hist[:, 0])
            hist_batch[0:len(hist), sampleId, 1] = torch.from_numpy(hist[:, 1])
            fut_batch[0:len(fut), sampleId, 0] = torch.from_numpy(fut[:, 0])
            fut_batch[0:len(fut), sampleId, 1] = torch.from_numpy(fut[:, 1])
            op_mask_batch[0:len(fut),sampleId,:] = 1
            mu_q_batch[sampleId, :] = mu_q
            sigma_q_batch[sampleId, :] = sigma_q

            hist_data = torch.zeros(len(hist), 1, 2)
            hist_data[:, 0, :] = hist_batch[:, sampleId, :]

            for id,nbr in enumerate(nbrs):
                if len(nbr)!=0:
                    nbrs_batch[0:len(nbr),count,0] = torch.from_numpy(nbr[:, 0])
                    nbrs_batch[0:len(nbr), count, 1] = torch.from_numpy(nbr[:, 1])
                    pos[0] = id % self.grid_size[0]
                    pos[1] = id // self.grid_size[0]
                    mask_batch[sampleId,pos[1],pos[0],:] = torch.ones(self.enc_size).bool()
                    count+=1

            nbrs_idx_batch[sampleId+1] = count

            # graph
            graph_templates = torch.zeros(nbrs_idx_batch[sampleId+1]-nbrs_idx_batch[sampleId], 4)
            graph_templates[:, :2] = nbrs_batch[0, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :]
            graph_templates[:, 2:] = nbrs_batch[1, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :] - \
                                     nbrs_batch[0, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :]
            
            graph = Graph(graph_templates)
            graph_list.append(graph.normalize_undigraph())

        return hist_batch, nbrs_batch, mask_batch, None, None, fut_batch, op_mask_batch, graph_list, mu_q_batch, sigma_q_batch, nbrs_idx_batch
        

#________________________________________________________________________________________________________________________________________


## Custom activation for output layer (Graves, 2015)

def outputActivation(x):
    muX = x[:,:,0:1]
    muY = x[:,:,1:2]
    sigX = x[:,:,2:3]
    sigY = x[:,:,3:4]
    rho = x[:,:,4:5]
    sigX = torch.exp(sigX)
    sigY = torch.exp(sigY)
    rho = torch.tanh(rho)
    out = torch.cat([muX, muY, sigX, sigY, rho],dim=2)
    return out


def variationUpdate(a, b, kldist_pd, k_p, k_d):
    kldist_pd_no_grad = kldist_pd.detach()

    for i in range(k_d):
        for j in range(k_p):
            v = torch.sum(b, dim=1)
            w = torch.sum(a, dim=0)

            b[i, j] = v[i]*a[i, j]/torch.sum(a[i, :])
            a[i, j] = w[j]*b[i, j]*torch.exp(-kldist_pd_no_grad[i, j]) / \
                      torch.sum(b[:, j]*torch.exp(-kldist_pd_no_grad[:, j]))
    
    return a, b


def klLoss(mu_dist, sigma_dist, mu_demo, sigma_demo, k_p, k_d, feat_len, a):
    tmp_d = sigma_demo.flatten().repeat(k_p).unsqueeze(1).repeat(1, feat_len)
    tmp_p = sigma_dist.flatten().repeat(k_d).unsqueeze(1).repeat(1, feat_len)
    sigma_d = torch.zeros(tmp_d.size(0), feat_len, feat_len).to(tmp_d)
    sigma_p = torch.zeros(tmp_p.size(0), feat_len, feat_len).to(tmp_p)
    sigma_d.as_strided(tmp_d.size(), [sigma_d.stride(0), sigma_d.size(2)+1]).copy_(tmp_d)
    sigma_p.as_strided(tmp_p.size(), [sigma_p.stride(0), sigma_p.size(2)+1]).copy_(tmp_p)
    sigma_d = sigma_d.view(-1, k_d*k_p, feat_len, feat_len)
    sigma_p = sigma_p.view(-1, k_d*k_p, feat_len, feat_len)

    mu_d = mu_demo.repeat(1, k_p, 1)
    mu_p = mu_dist.repeat(1, k_d, 1)

    dist_p = tdist.multivariate_normal.MultivariateNormal(mu_p, sigma_p)
    dist_d = tdist.multivariate_normal.MultivariateNormal(mu_d, sigma_d)
    kldist_pd = tdist.kl.kl_divergence(dist_p, dist_d).view(-1, k_d, k_p)

    a = a.repeat(kldist_pd.size(0), 1, 1)
    loss = torch.mean(torch.sum(a*kldist_pd, dim=(1, 2)))

    return loss, kldist_pd


## Batchwise NLL loss, uses mask for variable output lengths
def maskedNLL(y_pred, y_gt, mask):
    acc = torch.zeros_like(mask)
    muX = y_pred[:,:,0]
    muY = y_pred[:,:,1]
    sigX = y_pred[:,:,2]
    sigY = y_pred[:,:,3]
    rho = y_pred[:,:,4]
    ohr = torch.pow(1-torch.pow(rho,2),-0.5)
    x = y_gt[:,:, 0]
    y = y_gt[:,:, 1]
    # If we represent likelihood in feet^(-1):
    out = 0.5*torch.pow(ohr, 2)*(torch.pow(sigX, 2)*torch.pow(x-muX, 2) + torch.pow(sigY, 2)*torch.pow(y-muY, 2) - 2*rho*torch.pow(sigX, 1)*torch.pow(sigY, 1)*(x-muX)*(y-muY)) - torch.log(sigX*sigY*ohr) + 1.8379
    # If we represent likelihood in m^(-1):
    # out = 0.5 * torch.pow(ohr, 2) * (torch.pow(sigX, 2) * torch.pow(x - muX, 2) + torch.pow(sigY, 2) * torch.pow(y - muY, 2) - 2 * rho * torch.pow(sigX, 1) * torch.pow(sigY, 1) * (x - muX) * (y - muY)) - torch.log(sigX * sigY * ohr) + 1.8379 - 0.5160
    acc[:,:,0] = out
    acc[:,:,1] = out
    acc = acc*mask
    lossVal = torch.sum(acc)/torch.sum(mask)
    return lossVal

## NLL for sequence, outputs sequence of NLL values for each time-step, uses mask for variable output lengths, used for evaluation
def maskedNLLTest(fut_pred, lat_pred, lon_pred, fut, op_mask, num_lat_classes=3, num_lon_classes = 2,use_maneuvers = True, avg_along_time = False):
    if use_maneuvers:
        acc = torch.zeros(op_mask.shape[0],op_mask.shape[1],num_lon_classes*num_lat_classes).cuda()
        count = 0
        for k in range(num_lon_classes):
            for l in range(num_lat_classes):
                wts = lat_pred[:,l]*lon_pred[:,k]
                wts = wts.repeat(len(fut_pred[0]),1)
                y_pred = fut_pred[k*num_lat_classes + l]
                y_gt = fut
                muX = y_pred[:, :, 0]
                muY = y_pred[:, :, 1]
                sigX = y_pred[:, :, 2]
                sigY = y_pred[:, :, 3]
                rho = y_pred[:, :, 4]
                ohr = torch.pow(1 - torch.pow(rho, 2), -0.5)
                x = y_gt[:, :, 0]
                y = y_gt[:, :, 1]
                # If we represent likelihood in feet^(-1):
                out = -(0.5*torch.pow(ohr, 2)*(torch.pow(sigX, 2)*torch.pow(x-muX, 2) + 0.5*torch.pow(sigY, 2)*torch.pow(y-muY, 2) - rho*torch.pow(sigX, 1)*torch.pow(sigY, 1)*(x-muX)*(y-muY)) - torch.log(sigX*sigY*ohr) + 1.8379)
                # If we represent likelihood in m^(-1):
                # out = -(0.5 * torch.pow(ohr, 2) * (torch.pow(sigX, 2) * torch.pow(x - muX, 2) + torch.pow(sigY, 2) * torch.pow(y - muY, 2) - 2 * rho * torch.pow(sigX, 1) * torch.pow(sigY, 1) * (x - muX) * (y - muY)) - torch.log(sigX * sigY * ohr) + 1.8379 - 0.5160)
                acc[:, :, count] =  out + torch.log(wts)
                count+=1
        acc = -logsumexp(acc, dim = 2)
        acc = acc * op_mask[:,:,0]
        if avg_along_time:
            lossVal = torch.sum(acc) / torch.sum(op_mask[:, :, 0])
            return lossVal
        else:
            lossVal = torch.sum(acc,dim=1)
            counts = torch.sum(op_mask[:,:,0],dim=1)
            return lossVal,counts
    else:
        acc = torch.zeros(op_mask.shape[0], op_mask.shape[1], 1).cuda()
        y_pred = fut_pred
        y_gt = fut
        muX = y_pred[:, :, 0]
        muY = y_pred[:, :, 1]
        sigX = y_pred[:, :, 2]
        sigY = y_pred[:, :, 3]
        rho = y_pred[:, :, 4]
        ohr = torch.pow(1 - torch.pow(rho, 2), -0.5)
        x = y_gt[:, :, 0]
        y = y_gt[:, :, 1]
        # If we represent likelihood in feet^(-1):
        out = 0.5*torch.pow(ohr, 2)*(torch.pow(sigX, 2)*torch.pow(x-muX, 2) + torch.pow(sigY, 2)*torch.pow(y-muY, 2) - 2 * rho*torch.pow(sigX, 1)*torch.pow(sigY, 1)*(x-muX)*(y-muY)) - torch.log(sigX*sigY*ohr) + 1.8379
        # If we represent likelihood in m^(-1):
        # out = 0.5 * torch.pow(ohr, 2) * (torch.pow(sigX, 2) * torch.pow(x - muX, 2) + torch.pow(sigY, 2) * torch.pow(y - muY, 2) - 2 * rho * torch.pow(sigX, 1) * torch.pow(sigY, 1) * (x - muX) * (y - muY)) - torch.log(sigX * sigY * ohr) + 1.8379 - 0.5160
        acc[:, :, 0] = out
        acc = acc * op_mask[:, :, 0:1]
        if avg_along_time:
            lossVal = torch.sum(acc[:, :, 0]) / torch.sum(op_mask[:, :, 0])
            return lossVal
        else:
            lossVal = torch.sum(acc[:,:,0], dim=1)
            counts = torch.sum(op_mask[:, :, 0], dim=1)
            return lossVal,counts

## Batchwise MSE loss, uses mask for variable output lengths
def maskedMSE(y_pred, y_gt, mask):
    acc = torch.zeros_like(mask)
    muX = y_pred[:,:,0]
    muY = y_pred[:,:,1]
    x = y_gt[:,:, 0]
    y = y_gt[:,:, 1]
    out = torch.pow(x-muX, 2) + torch.pow(y-muY, 2)
    acc[:,:,0] = out
    acc[:,:,1] = out
    acc = acc*mask
    lossVal = torch.sum(acc)/torch.sum(mask)
    return lossVal

## MSE loss for complete sequence, outputs a sequence of MSE values, uses mask for variable output lengths, used for evaluation
def maskedMSETest(y_pred, y_gt, mask):
    acc = torch.zeros_like(mask)
    muX = y_pred[:, :, 0]
    muY = y_pred[:, :, 1]
    x = y_gt[:, :, 0]
    y = y_gt[:, :, 1]
    out = torch.pow(x - muX, 2) + torch.pow(y - muY, 2)
    acc[:, :, 0] = out
    acc[:, :, 1] = out
    acc = acc * mask
    lossVal = torch.sum(acc[:,:,0],dim=1)
    counts = torch.sum(mask[:,:,0],dim=1)
    return lossVal, counts

## Helper function for log sum exp calculation:
def logsumexp(inputs, dim=None, keepdim=False):
    if dim is None:
        inputs = inputs.view(-1)
        dim = 0
    s, _ = torch.max(inputs, dim=dim, keepdim=True)
    outputs = s + (inputs - s).exp().sum(dim=dim, keepdim=True).log()
    if not keepdim:
        outputs = outputs.squeeze(dim)
    return outputs
