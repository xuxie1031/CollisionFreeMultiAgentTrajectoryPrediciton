from __future__ import print_function, division
from torch.utils.data import Dataset, DataLoader
import scipy.io as scp
import numpy as np
import torch
import torch.nn.functional as F

import os

from graph_full import *

#___________________________________________________________________________________________________________________________

### Dataset class for the NGSIM dataset
class ngsimDataset(Dataset):

    def __init__(self, mat_file, t_h=30, t_f=50, d_s=2, enc_size = 64, grid_size = (13,3)):
        self.D = scp.loadmat(mat_file)['traj']
        self.T = scp.loadmat(mat_file)['tracks']
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

        return hist,fut,neighbors,lat_enc,lon_enc



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
        for _,_,nbrs,_,_ in samples:
            nbr_batch_size += sum([len(nbrs[i])!=0 for i in range(len(nbrs))])
        maxlen = self.t_h//self.d_s + 1
        nbrs_batch = torch.zeros(maxlen, nbr_batch_size, 2)


        # Initialize history, history lengths, future, output mask, lateral maneuver and longitudinal maneuver batches:
        hist_batch = torch.zeros(maxlen, len(samples), 2)
        nbrs_idx_batch = torch.zeros(len(samples)+1).long()

        count = 0

        data_list = []
        graph_list = []
        for sampleId,(hist, fut, nbrs, lat_enc, lon_enc) in enumerate(samples):

            # Set up history, future, lateral maneuver and longitudinal maneuver batches:
            hist_batch[0:len(hist), sampleId, 0] = torch.from_numpy(hist[:, 0])
            hist_batch[0:len(hist), sampleId, 1] = torch.from_numpy(hist[:, 1])

            hist_data = torch.zeros(maxlen, 1, 2)
            hist_data[:, 0, :] = hist_batch[:, sampleId, :]

            # Set up neighbor, neighbor sequence length, and mask batches:
            for id,nbr in enumerate(nbrs):
                if len(nbr)!=0:
                    nbrs_batch[0:len(nbr), count, 0] = torch.from_numpy(nbr[:, 0])
                    nbrs_batch[0:len(nbr), count, 1] = torch.from_numpy(nbr[:, 1])
                    count+=1

            data = hist_data
            nbrs_idx_batch[sampleId+1] = count
            if nbrs_idx_batch[sampleId+1] > nbrs_idx_batch[sampleId]:
                nbrs_data = nbrs_batch[:, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :]
                data = torch.cat((hist_data, nbrs_data), 1)

            T, V, _ = data.size()
            data_batch = torch.zeros(T-1, V, V, 4)
            for i in range(V):
                for j in range(V):
                    data_batch[:, i, j, :2] = data[:(T-1), i, :]
                    data_batch[:, i, j, 2:] = data[:(T-1), j, :]
            data_batch = data_batch.permute(0, 3, 1, 2)

            graph_templates = torch.zeros(T-1, V, 4)
            graph_templates[:, :, :2] = data[:(T-1), :, :]
            graph_templates[:, :, 2:] = data[1:, :, :]-data[:-1, :, :]
            graph = Graph(graph_templates)
            As = graph.normalize_undigraph()
            pos_weights = graph.graph_pos_weights()
            norms = graph.graph_norms()
            targets = graph.graph_As()

            data_list.append(data_batch)
            graph_list.append((As, pos_weights, norms, targets))    

        return data_list, graph_list



### Dataset class for the NGSIM dataset

class gtaDataset(Dataset):

    def __init__(self, dset_path, tag='highway', t_h=30, t_f=50, d_s=2):
        self.data_dir = os.path.join(dset_path, tag)
        self.t_h = t_h
        self.t_f = t_f
        self.d_s = d_s
        self.seq_list = []

        dataPipeline()



    def __len__(self):
        return len(self.seq_list)



    def __getitem__(self, idx):
        hist, fut, nbrs = self.seq_list[idx]

        return hist, fut, nbrs



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

        # Initialize neighbors and neighbors length batches:
        nbr_batch_size = 0
        for _,_,nbrs,_,_ in samples:
            nbr_batch_size += sum([len(nbrs[i])!=0 for i in range(len(nbrs))])
        maxlen = self.t_h//self.d_s + 1
        nbrs_batch = torch.zeros(maxlen, nbr_batch_size, 2)


        # Initialize history, history lengths, future, output mask, lateral maneuver and longitudinal maneuver batches:
        hist_batch = torch.zeros(maxlen, len(samples), 2)
        nbrs_idx_batch = torch.zeros(len(samples)+1).long()

        count = 0

        data_list = []
        graph_list = []
        for sampleId,(hist, fut, nbrs, lat_enc, lon_enc) in enumerate(samples):

            # Set up history, future, lateral maneuver and longitudinal maneuver batches:
            hist_batch[0:len(hist), sampleId, 0] = torch.from_numpy(hist[:, 0])
            hist_batch[0:len(hist), sampleId, 1] = torch.from_numpy(hist[:, 1])

            hist_data = torch.zeros(maxlen, 1, 2)
            hist_data[:, 0, :] = hist_batch[:, sampleId, :]

            # Set up neighbor, neighbor sequence length, and mask batches:
            for id,nbr in enumerate(nbrs):
                if len(nbr)!=0:
                    nbrs_batch[0:len(nbr), count, 0] = torch.from_numpy(nbr[:, 0])
                    nbrs_batch[0:len(nbr), count, 1] = torch.from_numpy(nbr[:, 1])
                    count+=1

            data = hist_data
            nbrs_idx_batch[sampleId+1] = count
            if nbrs_idx_batch[sampleId+1] > nbrs_idx_batch[sampleId]:
                nbrs_data = nbrs_batch[:, nbrs_idx_batch[sampleId]:nbrs_idx_batch[sampleId+1], :]
                data = torch.cat((hist_data, nbrs_data), 1)

            T, V, _ = data.size()
            data_batch = torch.zeros(T-1, V, V, 4)
            for i in range(V):
                for j in range(V):
                    data_batch[:, i, j, :2] = data[:(T-1), i, :]
                    data_batch[:, i, j, 2:] = data[:(T-1), j, :]
            data_batch = data_batch.permute(0, 3, 1, 2)

            graph_templates = torch.zeros(T-1, V, 4)
            graph_templates[:, :, :2] = data[:(T-1), :, :]
            graph_templates[:, :, 2:] = data[1:, :, :]-data[:-1, :, :]
            graph = Graph(graph_templates)
            As = graph.normalize_undigraph()
            pos_weights = graph.graph_pos_weights()
            norms = graph.graph_norms()
            targets = graph.graph_As()

            data_list.append(data_batch)
            graph_list.append((As, pos_weights, norms, targets))    

        return data_list, graph_list

              
#________________________________________________________________________________________________________________________________________


def vae_loss(preds, targets, mu, logvar, n_nodes, norms, pos_weights, device):
    N = preds.size(0)
    costs = torch.zeros(N).to(device)

    for i in range(N):
        costs[i] = norms[i]*F.binary_cross_entropy_with_logits(preds[i], targets[i], pos_weight=pos_weights[i])
    
    KLDs = -0.5 / n_nodes*torch.mean(torch.sum(1+2*logvar-mu.pow(2)-logvar.exp().pow(2), dim=2), dim=1)

    return torch.mean(costs+KLDs)


def gmm_loss(x, mu_q, sigma_q, M, lambd):
    x = x.unsqueeze(1).repeat(1, M, 1)
    prior = F.softmax(lambd, dim=1)
    likelihood = (1/torch.sqrt(2*np.pi*sigma_q))*torch.exp((-1/(2*sigma_q))*torch.norm(x-mu_q, dim=-1)**2)
    loss = -torch.mean(torch.log(torch.sum(prior*likelihood, dim=-1)))

    return loss
