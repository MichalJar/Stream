import copy as cp
import numpy as np
import math

class CF:
    def __init__(self):
        self.ls = None
        self.ss = None
        self.n = None

class Node:
    def __init__(self):
        self.cfs = []
        self.childs = []

class Clustree:
    def __init__(self, node_max_size=3, max_distance=3.0):
        self.node_max_size = node_max_size
        self.max_distance = max_distance
        self.root = Node()
    
    def dist(self, cf_l, cf_r):
        diff = cf_l.ls / cf_l.n - cf_r.ls / cf_r.n
        return math.sqrt(np.dot(diff,diff))

    def get_nearest_cf(self, ncf, cfs):
        d = float('inf')
        best_i = 0
        for i,cf in enumerate(cfs):
            if d > self.dist(cf, ncf):
                d = self.dist(cf, ncf)
                best_i = i
        
        return best_i, cfs[i]

    def create_cf_from(self,elem):
        cf = CF()
        cf.ls = elem
        cf.ss = elem*elem
        cf.n = 1
        return cp.deepcopy(cf)

    def add_cfs(self, cf_l, cf_r):
        cf = CF()
        cf.ls = cf_l.ls + cf_r.ls
        cf.ss = cf_l.ss + cf_r.ss
        cf.n = cf_l.n + cf_r.n
        return cf

    def add_elem(self, elem):
        ncf = self.create_cf_from(elem)
        if len(self.root.cfs) == 0:
            self.root.cfs.append(ncf)
        else:
            i, nearest_cf = self.get_nearest_cf(ncf, self.root.cfs)
            if self.dist(ncf, nearest_cf) <= self.max_distance:
                self.root.cfs[i] = self.add_cfs(nearest_cf, ncf)
            elif len(self.root.cfs) < self.node_max_size:
                self.root.cfs.append(ncf)
            else:
                for ic, cf in enumerate(self.root.cfs):
                    leaf = Node()
                    leaf.cfs = [cf]   
                    if ic == i: leaf.cfs.append(ncf)
                    self.root.childs.append(leaf)

                self.root.cfs[i] = self.add_cfs(self.root.cfs[i], ncf)