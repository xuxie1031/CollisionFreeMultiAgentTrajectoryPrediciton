import torch
import numpy as np

class Graph:
    # batch_templates size: (N, V, 4), 4 dims are: x, y, vx, vy
    def __init__(self, batch_templates):
        N, V, C = batch_templates.size()

        self.A = torch.zeros(N, V, V)
        self.A.requires_grad = False

        self.edges(batch_templates)


    def edges(self, batch_templates):
        N = self.A.size(0)
        V = self.A.size(-1)

        for num in range(N):
            for i in range(V):
                for j in range(i+1, V):
                    xi, yi, vxi, vyi = batch_templates[num, i]
                    xj, yj, vxj, vyj = batch_templates[num, j]
                    a, b, c, d = (xi-xj), (yi-yj), (vxi-vxj), (vyi-vyj)
                    tmin = -(a*c+b*d)/(c**2+d**2)
                    tmin = np.ceil(tmin.item())
                    self.A[num, i, j] = 1.0 / tmin if tmin > 0.0 else 0.0
                    self.A[num, j, i] = self.A[num, i, j]
        
        self.A = self.A + torch.eye(V).repeat(N, 1, 1)
    

    def normalize_undigraph(self, alpha=1e-3):
        DADs = torch.zeros(self.A.size())
        DADs.requires_grad = False
    
        N = self.A.size(0)
        V = self.A.size(-1)

        for num in range(N):
            A = self.A[num]
            D1 = torch.sum(A, dim=0)+alpha
            Dn = torch.zeros(V, V)
            for i in range(V):
                Dn[i, i] = D1[i]**(-0.5)
            DAD = Dn.mm(A).mm(Dn)
            DADs[num] = DAD

        return DADs


    def graph_pos_weights(self, alpha=1e-3):
        N = self.A.size(0)
        V = self.A.size(-1)
        pos_weights = torch.zeros(N)

        for i in range(N):
            A_sum = self.A[i].sum()-1.0*V+alpha
            pos_weights[i] = (V**2-A_sum)/A_sum
        
        return pos_weights

    
    def graph_norms(self, alpha=1e-3):
        N = self.A.size(0)
        V = self.A.size(-1)
        norms = torch.zeros(N)

        for i in range(N):
            A_sum = self.A[i].sum()-1.0*V+alpha
            norms[i] = (V**2)/(2*(V**2-A_sum))

        return norms

    def graph_As(self):
        return self.A
