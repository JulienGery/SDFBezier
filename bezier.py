import numpy as np 
from math import comb as choose
import polynom

class Bezier:
    def __init__(self, ancors: np.array):
        self.ancors = ancors

    def __call__(self, t):
        n = len(self.ancors) -1
        return sum(
            (1-t)**(n-k) * t**k * choose(n, k) * p for k, p in enumerate(self.ancors)
        )
    
    def derivate(self, t):
        n = len(self.ancors) -1
        return sum(
            (-(n-k)*(1-t)**(n-k-1) * t**k + (1-t)**(n-k) * k * t**(k-1)) * choose(n, k) * p for k, p in enumerate(self.ancors)
        )
    
    def findClosestPoint(self, P: np.array):
        pass