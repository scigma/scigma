v_names = ['x', 'y']; v_values = [0, 0]
p_names = ['k', 'l']; p_values = [2.0, 0.0]

def f(x, p, xdot):
    xdot[0] =-(x[0]**3-3*x[0]+p[1])-p[0]*x[1]
    xdot[1] = x[0]-x[1]
