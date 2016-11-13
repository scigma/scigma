import math

def n_phi(n,phi=[]):
    for f in phi:
        n*=math.sin(f)
    return n

def next_phi(n,d,phi=[]):
    n=n_phi(n,phi)
    if(len(phi)==d-1): # make full circle for the last angle
        m=math.ceil(2*n) if n>0 else 1.0
        for i in range(int(m)):
            phi_i=phi+[2*math.pi/m*float(i)]
            yield phi_i
    else:
        m=math.ceil(n) if n>0 else 1.0
        for i in range(int(m)):
            phi_i=phi+[math.pi/(m-1)*float(i)] if m > 1 else phi+[0.0]
            for p in next_phi(n,d,phi_i):
                yield p
                    
def angles(n,d):
    if d==0:
        yield []
        return
    for phi in next_phi(n/2.,d):
        yield phi    

def dot(v1,v2):
    result=0
    for i in range(len(v1)):
        result+=v1[i]*v2[i]
    return result
    
def gs_coefficient(v1, v2):
    return dot(v2, v1)/ dot(v1, v1)

def multiply(coefficient, v):
    return map((lambda x : x * coefficient), v)

def proj(v1, v2):
    return multiply(gs_coefficient(v1, v2) , v1)

def gsortho(X):
    Y = []
    for i in range(len(X)):
        temp_vec = X[i]
        for inY in Y :
            proj_vec = proj(inY, X[i])
            temp_vec = list(map(lambda x, y : x - y, temp_vec, proj_vec))
        Y.append(temp_vec)
    for i in range(len(X)):
        l = math.sqrt(dot(Y[i],Y[i]))
        for j in range(len(Y[i])):
            Y[i][j]=Y[i][j]/l
    return Y

