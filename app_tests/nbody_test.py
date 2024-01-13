import random
import dis

def nbody():
    random.seed(42)
    N = 3
    ITERATIONS = 10
    
    mass = []
    pos = []
    for i in range(N):
        mass.append(random.random())
        pos.append([random.random(), random.random(), random.random()])
    
    for i in range(ITERATIONS):
        for j in range(N):
            for k in range(N):
                if j != k:
                    dx = pos[j][0] - pos[k][0]
                    dy = pos[j][1] - pos[k][1]
                    dz = pos[j][2] - pos[k][2]
                    d = (dx ** 2 + dy ** 2 + dz ** 2) ** 1.5
                    mass_j = mass[j]
                    mass_k = mass[k]
                    pos[j][0] -= mass_k * dx / d
                    pos[j][1] -= mass_k * dy / d
                    pos[j][2] -= mass_k * dz / d
                    pos[k][0] += mass_j * dx / d
                    pos[k][1] += mass_j * dy / d
                    pos[k][2] += mass_j * dz / d
    return pos

dis.dis(nbody)

pos1 = nbody()

import raijit
raijit.enable()
pos2 = nbody()
print(pos1)
print(pos2)

assert pos1 == pos2
