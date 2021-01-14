import math
from ctypes import *
from . import gui
from . import dat
from . import common
from . import lib
from . import windowlist
from . import graphs
from . import picking
from . import popups
from . import iteration

commands={}

N_EIGEN_TYPES=6

REAL_STABLE=0
REAL_NEUTRAL=1
REAL_UNSTABLE=2
COMPLEX_STABLE=3
COMPLEX_NEUTRAL=4
COMPLEX_UNSTABLE=5

EIGEN_INDEX={'RS':REAL_STABLE,'RN':REAL_NEUTRAL,'RU':REAL_UNSTABLE,
             'CS':COMPLEX_STABLE,'CN':COMPLEX_NEUTRAL,'CU':COMPLEX_UNSTABLE}
EIGEN_TYPE=['RS','RN','RU','CS','CN','CU']

N_FLOQUET_TYPES=9

REAL_POSITIVE_STABLE=0
REAL_POSITIVE_NEUTRAL=1
REAL_POSITIVE_UNSTABLE=2
COMPLEX_STABLE=3
COMPLEX_NEUTRAL=4
COMPLEX_UNSTABLE=5
REAL_NEGATIVE_STABLE=6
REAL_NEGATIVE_NEUTRAL=7
REAL_NEGATIVE_UNSTABLE=8

FLOQUET_INDEX={'RPS':REAL_POSITIVE_STABLE,'RPN':REAL_POSITIVE_NEUTRAL,'RPU':REAL_POSITIVE_UNSTABLE,
               'CS':COMPLEX_STABLE,'CN':COMPLEX_NEUTRAL,'CU':COMPLEX_UNSTABLE,
               'RNS':REAL_NEGATIVE_STABLE,'RNN':REAL_NEGATIVE_NEUTRAL,'RNU':REAL_NEGATIVE_UNSTABLE}
FLOQUET_TYPE=['RPS','RPN','RPU','CS','CN','CU','RNS','RNN','RNU']

def guess(path=None,win=None,showall=False):
    """ guess [path]

    Starting at the current position in phase space and parameter 
    space, search for a steady state of the current map/ODE with
    a Newton iteration. If successful, the newly found fixed point
    is displayed with a marker
    The point is stored using the identifier path, or with a generic 
    name of the form 'fpN' (for odes) or 'ppN' (for maps), if no 
    path is given.
    """

    win = windowlist.fetch(win)
    
    nVars=win.eqsys.n_vars()
    
    if nVars == 0:
        raise Exception ("error: no variables defined")

    mode=win.equationPanel.get('mode')
    blob=iteration.blob(win)
    
    if not path:
        rootpath=""
        if mode =='ode':
            prefix='fp'
        else:
            prefix='pp'
    else:
        rootpath=path.rpartition('.')[0]+path.rpartition('.')[1]
        prefix=path.rpartition('.')[2]

    varying=['t']+win.eqsys.var_names()+win.eqsys.func_names()
    const=win.eqsys.par_names()+win.eqsys.const_names()

    nParts = win.cursor['nparts']

    result = []
    
    for varVals, constVals in picking.points(win, varying, const):
        if nParts>1 or not path:
            adjpath=rootpath+graphs.gen_ID(prefix,win)
        else:
            adjpath=rootpath+prefix

        result.append(guess_single(adjpath,blob,varying,const,varVals,constVals,win,showall)) 

    if len(result) == 1:
        return result[0]
    else:
        return result

commands['g']=commands['gu']=commands['gue']=commands['gues']=commands['guess']=guess

def guessall(path=None,win=None):
    return guess(path, win, True)

commands['g*']=commands['gu*']=commands['gue*']=commands['gues*']=commands['guess*']=guessall


def rtime(identifier=None, win=None):
    """ rtime [name]

    prints and returns the return time of a set of orbits or
    fixed points of a stroboscopic/Poincare map.
    The orbits specified by objlist; if objlist is not given,
    or the currently selected list of objects is used. 
    Gives an error message if an entry of the specified object
    list does not have a return time
    """
    win = windowlist.fetch(win)
    g = graphs.get(identifier,win) if identifier else win.selection

    try:
        win.glWindow.stall()
        rt=g['rtime']
    except:
        raise Exception('selected object has no return time')
    finally:
        win.glWindow.flush()
    win.console.write_data(str(float("{0:.12f}".format(rt)))+'\n')
    return rt

commands['rt']=commands['rti']=commands['rtim']=commands['rtime']=rtime

def evals(identifier=None, win=None):
    """ evals [name]

    prints and returns the eigenvalues/floquet multipliers
    of the object specified by identifier, or of the current 
    selection , if identifier is omitted.
    Gives an error message if the specified object does not 
    have eigenvalue information
    """
    win = windowlist.fetch(win)
    g = graphs.get(identifier,win) if identifier else win.selection

    try:
        win.glWindow.stall()
        try:
            evreal = g['evreal']
            evimag = g['evimag']
        except:
            raise Exception('selected object has no eigenvalue data')
        for i in range(len(evreal)):
            win.console.write(str(i+1).rjust(4)+":  ")
            if evimag[i]<0:
                sign = " - i*" 
            elif evimag[i]>0:
                sign = " + i*"
            else:
                sign = None
            win.console.write_data(str(float("{0:.12f}".format(evreal[i]))))
            if sign==" - i*":
                win.console.write_data(sign+str(float("{0:.12f}".format(-evimag[i]))))
            elif sign==" + i*":
                win.console.write_data(sign+str(float("{0:.12f}".format(evimag[i]))))
            if g['mode']!= 'ode':
                modulus = math.sqrt(evreal[i]*evreal[i]+evimag[i]*evimag[i])
                win.console.write("   (modulus: ")
                win.console.write_data(str(float("{0:.12f}".format(modulus))))
                win.console.write(")")
            win.console.write(' \n')
    finally:
        win.glWindow.flush()
    return evreal, evimag

commands['ev']=commands['eva']=commands['eval']=commands['evals']=evals 

def evecs(identifier=None, win=None):
    """ evecs [name]

    prints and returns the eigenvectors
    of the object specified by identifier, or of the current 
    selection , if identifier is omitted.
    Gives an error message if the specified object does not 
    have eigenvector information
    """
    win = windowlist.fetch(win)
    g = graphs.get(identifier,win) if identifier else win.selection

    try:
        win.glWindow.stall()
        try:
            evecs = g['evecs']
            evimag = g['evimag']
            nVar = len(evecs)
        except:
            raise Exception('selected object has no eigenvector data')
        impart=False
        for i in range(nVar):
            win.console.write(str(i+1).rjust(4)+":  ")
            line ='('
            if(evimag[i]==0):
                for j in range(nVar):
                    line+=str(float("{0:.12f}".format(evecs[i][j])))+","
            else:
                if not impart:
                    impart=True
                    for j in range(nVar):
                        if evecs[i+1][j]<0:
                            sign = " - i*" 
                        elif evecs[i+1][j]>0:
                            sign = " + i*"
                        else :
                            sign = None
                        line+=str(float("{0:.12f}".format(evecs[i][j])))
                        if sign==" - i*":
                            line+=(sign+str(float("{0:.12f}".format(-evecs[i+1][j]))))
                        elif sign==" + i*":
                            line+=(sign+str(float("{0:.12f}".format(evecs[i+1][j]))))
                        line+=' , '
                else:
                    impart=False
                    for j in range(nVar):
                        if evecs[i][j]<0:
                            sign = " + i*" 
                        elif evecs[i][j]>0:
                            sign = " - i*"
                        else :
                            sign = None
                        line+=str(float("{0:.12f}".format(evecs[i-1][j])))
                        if sign==" - i*":
                            line+=(sign+str(float("{0:.12f}".format(evecs[i][j]))))
                        elif sign==" + i*":
                            line+=(sign+str(float("{0:.12f}".format(-evecs[i][j]))))
                        line+=' , '
            win.console.write_data(line.strip(' , ')+')\n')
    finally:
        win.glWindow.flush()
    return evecs

commands['eve']=commands['evec']=commands['evecs']=evecs 


lib.scigma_num_guess.restype=c_int
lib.scigma_num_guess.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_bool,c_bool]

def guess_single(path,blob,varying,const,varVals,constVals,win,showall):
    mode = win.equationPanel.get("mode")
    nperiod = win.equationPanel.get("nperiod")
    nPoints = nperiod if (showall and mode!='ode') else 1

    graph=graphs.new(win,nPoints,1,varying,const,varVals,constVals,path)
    graph['mode']=mode
    graph['callbacks']= {'success':lambda args:success(graph,win,args),
                         'fail':lambda args:fail(graph,win,args),
                         'cleanup':lambda:cleanup(graph),
                         'minmax':lambda:graphs.stdminmax(graph),
                         'cursor':lambda point:graphs.stdcursor(graph,point,win)}
    
    nVars=win.eqsys.n_vars()
    eqsysID=win.eqsys.objectID
    logID=win.log.objectID
    blobID=blob.objectID

    identifier=create_string_buffer(bytes(path.encode("ascii")))
    varWaveID=graph['varwave'].objectID

    if mode=='ode':
        graph['evwave']=dat.Wave(nVars*(nVars+2)+N_EIGEN_TYPES)
    else:
        graph['evwave']=dat.Wave(nVars*(nVars+2)+N_FLOQUET_TYPES)

    evWaveID=graph['evwave'].objectID

    graph['taskID']=lib.scigma_num_guess(identifier,eqsysID,logID,varWaveID,evWaveID,blobID,showall,True)
    graph['cgraph']=None

    return graph
    
def success(g,win,args):
    # finish the cpp Task
    lib.scigma_num_finish_task(g['taskID'])

    # gather and display some additional information for the fixed/periodic point:
    nVar=win.eqsys.n_vars()
    nVarying=len(g['varying'])
    evreal=g['evwave'][:nVar]
    evimag=g['evwave'][nVar:2*nVar]
    evecs=[g['evwave'][2*nVar+j*nVar:2*nVar+(j+1)*nVar] for j in range(nVar)]

    win.console.write("found steady state ")
    win.console.write_data(g['identifier'])
    win.console.write(":\n")
    vardata=g['varwave'].data()
    nPeriod=g['varwave'].size()//nVarying
    names=g['varying']
    for i in range(nVar):
        win.console.write(names[i+1]+' = ')
        win.console.write_data(str(float("{0:.10f}".format(vardata[i+1+nVarying*(nPeriod-1)])))+'\n')
        
    evtypes=[int(t) for t in g['evwave'][nVar*(nVar+2):]]
    win.console.write("stability: ")
    info=''
    if g['mode']=='ode':
        g['stable']=True if evreal[-1]<=0 else False
        for i in range(N_EIGEN_TYPES):
            if evtypes[i]>0:
                info+=(str(evtypes[i])+EIGEN_TYPE[i]+' / ')
    else:
        g['stable']=True if evreal[-1]*evreal[-1]+evimag[-1]*evimag[-1]<=1 else False
        for i in range(N_FLOQUET_TYPES):
            if evtypes[i]>0:
                info+=(str(evtypes[i])+FLOQUET_TYPE[i]+' / ')
    win.console.write_data(info.strip(' / ')+'\n')

    g['evreal']=evreal
    g['evimag']=evimag
    g['evecs']=evecs
    g['evtypes']=evtypes

    # if we guessed the steady state of a stroboscopic map/ Poincare map, store the return time
    if g['mode']=='strobe' or g['mode']=='Poincare':
        if nPeriod==1:
            g['rtime']=vardata[0]
        else:
            g['rtime']=vardata[nVarying*(nPeriod-1)]
    
    # create the visible object        
    if g['mode']=='ode':
        pointStyle=gui.RDOT if g['stable'] else gui.RING
    else:
        pointStyle=gui.QDOT if g['stable'] else gui.QUAD

    pointSize=win.options['Drawing']['marker']['size'].value
    color=win.options['Drawing']['color']
    
    g['cgraph']=gui.Bundle(win.glWindow,g['npoints'],g['nparts'],
                           len(g['varying']),g['varwave'],g['constwave'],
                           lambda double, button, point,x, y:
                           mouse_callback(g,double,button,point,x,y,win))
    g['cgraph'].set_point_style(pointStyle)
    g['cgraph'].set_point_size(pointSize)
    g['cgraph'].set_color(color)
    g['cgraph'].finalize()

    graphs.show(g,win)
    picking.select(g['identifier'],-1,win,True)

def fail(g,win,args):
    # finish the cpp Task
    lib.scigma_num_finish_task(g['taskID'])

    g['cgraph']=None
        
def cleanup(g):
    g['evwave'].destroy()

def mouse_callback(g,double,button,point,x,y,win):
    identifier = g['identifier']
    if(double):
        picking.select(identifier,point,win)
    elif button==1 and gui.tk:
        extra_labels = ['evals', 'evecs']
        extra_commands = [lambda:evals(identifier, win), lambda:evecs(identifier, win)]
        extra_separators = [0]
        popups.graph(g, extra_labels, extra_commands, extra_separators, win)
        
def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('guessing', lambda:unplug(win), commands):
        return
    
    # fill option panels
    try:
        win.glWindow.stall()
        panel=win.acquire_option_panel('Numerical')
        panel.add('Newton.tol',1e-9)
        panel.define('Newton.tol',"min=0.0")
        enum = common.Enum({'numeric':0,'symbolic':1},'symbolic')
        panel.add('Newton.Jacobian',enum,True,"readonly=true")
    finally:
        win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('guessing'):
        return
    
    # remove options from panels
    try:
        win.glWindow.stall()
        panel=win.acquire_option_panel('Numerical')
        panel.remove('Newton.tol')
        panel.remove('Newton.Jacobian')
        win.release_option_panel('Numerical')
    finally:
        win.glWindow.flush()
