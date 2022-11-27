import numpy as np

EPSILON = .00000000001

def DeMoivre(z: complex, n: int) -> np.array:
    length = np.sqrt(z * z.conjugate())**(1/n)
    o = np.angle(z)
    k = np.arange(0, n)
    return length * np.exp(1j*(o + 2*k*np.pi)/n)

def minum(array : np.array) -> complex:
    m = 0
    for i, z in enumerate(array):
        if abs(z.imag) < abs(array[m].imag) and z.real > array[m].real:
            m = i
    return array[m]

class Polynom:
    def __init__(self, coef: np.array):
        # coef in order: a*x**5 + b*x**4 ...
        self.coef = coef

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        size = len(self.coef)
        rep = ''
        for i, j in enumerate(self.coef):
            rep += f'{j}x^{size-i-1}+'
        return rep[:-4]

    def __call__(self, x: float) -> float:
        size = len(self.coef)
        return np.sum([j * x**(size-i-1) for i, j in enumerate(self.coef)], axis=0)

    def derivate(self, x: float) -> float:
        size = len(self.coef)
        return np.sum([(size-i-1) * j * x**(size-i-2) for i, j in enumerate(self.coef)], axis=0)

class Quadratic(Polynom):
    def __init__(self, a, b, c):
        super().__init__([a, b, c])

    def roots(self):
        a, b, c = self.coef
        delta = b**2 - 4*a*c
        if delta == 0:
            return -b/(2*a)
        delta = DeMoivre(delta, 2)
        return (-b+delta)/(2*a)

class Cubic(Polynom):
    def __init__(self, a, b, c, d):
        super().__init__([a, b, c, d])

    def roots(self) -> np.array:
        a, b, c, d = self.coef

        b /= a
        c /= a
        d /= a
        a = 1

        p = (3*c - b**2)/3
        q = (9*c*b - 27*d - 2 * b**3)/27

        R = q/2
        Q = p/3

        w3 = R + np.emath.sqrt(R**2 + Q**3)
        w = DeMoivre(w3, 3)
        x = w - p/(3*w)
        z = x - b/3
        return z

class Quartic(Polynom):
    def __init__(self, a, b, c, d, e):
        super().__init__([a, b, c, d, e])

    def roots(self) -> np.array:
        a, b, c, d, e = self.coef

        b /= a #a_3
        c /= a #a_2
        d /= a #a_1
        e /= a #a_0
        a = 1

        rc = Cubic(1, -c, d*b - 4*e, 4*c*e - d**2 - b**2*e)
        y = minum(rc.roots()).real #one of real root of the resolvent cubic equation root(s)

        R = np.emath.sqrt(1/4*b**2 - c + y)
        if abs(R) <= 0.1:
            R = 0
            D = np.emath.sqrt(3/4 * b**2 - 2 * c + 2 * np.emath.sqrt(y**2 - 4 * e))
            E = np.emath.sqrt(3/4 * b**2 - 2 * c - 2 * np.emath.sqrt(y**2 - 4 * e))
        else :
            D = np.emath.sqrt(3/4 * b**2 - R**2 - 2 * c + 1/4*(4*b*c - 8 * d - b**3)/R)
            E = np.emath.sqrt(3/4 * b**2 - R**2 - 2 * c - 1/4*(4*b*c - 8 * d - b**3)/R)

        return np.array([
            -1/4 * b + 1/2 * R + 1/2 * D,
            -1/4 * b + 1/2 * R - 1/2 * D,
            -1/4 * b - 1/2 * R + 1/2 * E,
            -1/4 * b - 1/2 * R - 1/2 * E,
        ])

class Quintic(Polynom):
    def __init__(self, a, b, c, d, e, f):
        super().__init__([a, b, c, d, e, f])

    def roots(self) -> np.array:
        a, b, c, d, e, f = self.coef

        b /= a
        c /= a
        d /= a
        e /= a
        f /= a
        a = 1
        r = 1/2

        while abs(self(r)) > EPSILON:
            r = r - self(r)/self.derivate(r)
        
        k = b+r
        l = c+r*k
        m = d+r*l
        n = -f/r

        fq = Quartic(a, k, l, m, n)
        # print(f'({fq})(x-{r})')
        return np.array([r, *fq.roots()])

