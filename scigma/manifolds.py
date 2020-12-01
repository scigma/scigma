import math
from ctypes import *
from . import gui
from . import dat
from . import common
from . import lib
from . import library
from . import windowlist
from . import graphs
from . import picking
from . import equations
from . import iteration
from . import sweeping

commands={}

def manifold(stable,nSteps,g=None,path=None,win=None,showall=False):
    win = windowlist.fetch(win)

    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read nSteps (usage: mu/ms <nSteps> [name])")

    if nSteps<0:
        nSteps=-nSteps

    g=graphs.get(g,win)

    # see if we can use the data in g to create an invariant manifold
    try:
        evreal = g['evreal']
        evimag = g['evimag']
        evecs = g['evecs']
    except:
        raise Exception(g['identifier']+': point has no eigenvalue and/or eigenvector data')

    # check whether we have one or two stable/unstable directions
    mode = g['mode']
    if stable:
        if mode == 'ode':
            idx=[i for i in range(len(evreal)) if evreal[i]<0.0]
        else:
            idx=[i for i in range(len(evreal)) if evreal[i]**2+evimag[i]**2<1.0]
    else:
        if mode == 'ode':
            idx=[i for i in range(len(evreal)) if evreal[i]>0.0]
        else:
            idx=[i for i in range(len(evreal)) if evreal[i]**2+evimag[i]**2>1.0]

    if mode != 'ode':
        if len(idx)!=1:
            win.console.write_warning(g['identifier']+' has not exactly 1 '+ ('stable' if stable else 'unstable')+ ' eigenvalue ('+str(len(idx))+'), doing nothing\n')
            return
    else:
        if len(idx)!=1 and len(idx)!=2:
            win.console.write_warning(g['identifier']+' has not exactly 1 or 2 '+ ('stable' if stable else 'unstable')+ ' eigenvalues ('+str(len(idx))+'), doing nothing\n')
            return

    if mode != win.equationPanel.get("mode"):
        win.console.write_warning('switching mode to '+mode)
        equations.mode(mode)

    if not path:
        path=graphs.gen_ID("mf",win)
         
    picking.select(g['identifier'],-1,win,True)

    if len(idx)==1:
        if stable:
            picking.perts(2,g,win)
            one_d_manifold(-nSteps,g,path,win,showall)
        else:
            picking.pertu(2,g,win)
            one_d_manifold(nSteps,g,path,win,showall)
    else:
        # pick points on a ring with radius eps around the fixed point
        # use the circumference divided by ds as number of initial points
        # but use at least three if ds is too large (otherwise the
        # algorithm for mesh building would fail)
        eps=win.options['Numerical']['Manifolds']['eps']
        arc=win.options['Numerical']['Manifolds']['arc']
        nInitial = win.options['Numerical']['Manifolds']['ninit']

        if stable:
            picking.perts(nInitial,g,win)
            two_d_manifold(-nSteps,g,path,win)
        else:
            picking.pertu(nInitial,g,win)
            two_d_manifold(nSteps,g,path,win)

def munstable(nSteps=1,g=None,path=None,win=None):
    """munstable [nSteps] [origin] [name]                                                                              
                                                                                                                   
    Starting at the fixed point or periodic point with the                                                         
    identifier origin, create the 1-dimensional unstable manifold along                                            
    the eigenvector specified by options['Numerical']['manifolds']['evec1'].                                       
    options['Numerical']['manifolds']['ds'] is used as initial perturbation                                        
    from the fixed point along the specified eigenvector. Plot n time steps                                        
    / arclength                                                                                                    
    The point is stored using the identifier name, or with a generic                                               
    name of the form 'mfN', if no name is given.                                                                   
    The behavior is analog for mstable.                                                                            
    """
    manifold(False,nSteps,g,path,win,False)

commands['mu']=commands['mun']=commands['muns']=commands['munst']=commands['munsta']=commands['munstab']=commands['munstabl']=commands['munstable']=munstable

def mstable(nSteps=1,g=None,path=None,win=None):
    manifold(True,nSteps,g,path,win,False)

commands['ms']=commands['mst']=commands['msta']=commands['mstab']=commands['mstabl']=commands['mstable']=mstable

def munstableall(nSteps=1,g=None,path=None,win=None,):
    manifold(False,nSteps,g,path,win,True)

commands['mu*']=commands['mun*']=commands['muns*']=commands['munst*']=commands['munsta*']=commands['munstab*']=commands['munstabl*']=commands['munstable*']=munstableall

def mstableall(nSteps=1,g=None,path=None,win=None):
    manifold(True,nSteps,g,path,win,True)

commands['ms*']=commands['mst*']=commands['msta*']=commands['mstab*']=commands['mstabl*']=commands['mstable*']=mstableall

def two_d_manifold(nSteps,g,path,win):
    varying=win.cursor['varying']
    varVals=win.cursor['varwave'][:]
    const = win.cursor['const']
    constVals=win.cursor['constwave'][:]

    # add fixed point itself to the initial mesh of the cursor
    varVals = g['varwave'][:]+varVals

    mesh = dat.Mesh(len(varying),varVals)

    sweeping.sweep(nSteps,g,path,win,mesh)
    
lib.scigma_num_map_manifold.restype=c_int
lib.scigma_num_map_manifold.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_int,c_bool,c_bool]
lib.scigma_num_ode_manifold.restype=c_int
lib.scigma_num_ode_manifold.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_int,c_bool]

def one_d_manifold(nSteps,g,path,win,showall):

    varying=win.cursor['varying']
    varVals=win.cursor['varwave'][:]
    const = win.cursor['const']
    constVals=win.cursor['constwave'][:]

    # add steadyState point itself to the initial segment(s) of the cursor point(s)
    nVarying = len(varying)
    steadyState = g['varwave'][-nVarying:]   # if g is result of guess* command, pick last point (this is what pert acts upon)
    nSegments = win.cursor['nparts']
    segments=steadyState*nSegments
    for i in range(nSegments):
        firstPoint = varVals[i*nVarying:(i+1)*nVarying]
        segments = segments + firstPoint
    
    eqsysID=win.eqsys.objectID
    mode = win.equationPanel.get("mode")
    if mode == 'map' and nSteps<0:
        eqsysID=win.invsys.objectID
        if win.invsys.var_names() != win.eqsys.var_names():
            raise Exception("map and inverse map have different variables")

    g=graphs.new(win,abs(nSteps)+1,nSegments,varying,const,segments,constVals,path)

    g['mode']=mode
    g['callbacks']= {'success':lambda args:iteration.success(g,win,args),
                     'fail':lambda args:iteration.fail(g,win,args),
                     'cleanup':lambda:iteration.cleanup(g),
                     'minmax':lambda:graphs.stdminmax(g),
                     'cursor':lambda point:iteration.cursor(g,point,win)}

    identifier=create_string_buffer(bytes(path.encode("ascii")))
    
    varWaveID=g['varwave'].objectID
    logID=win.log.objectID
    blob = iteration.blob(win)
    blobID=blob.objectID

    noThread = (win.options['Global']['threads'].label =='off')

    if mode !='ode':
        g['taskID']=lib.scigma_num_map_manifold(identifier,eqsysID,logID,nSteps - abs(nSteps)//nSteps,
                                                nSegments,varWaveID,blobID,showall,noThread)
    else:
        print segments
        print g['varwave'][:]
        g['taskID']=lib.scigma_num_ode_manifold(identifier,eqsysID,logID,nSteps - abs(nSteps)//nSteps,
                                                nSegments,varWaveID,blobID,noThread)

    nperiod = win.equationPanel.get("nperiod")
    nPoints = nSegments*nperiod*abs(nSteps+1) if (showall and mode!='ode') else nSegments*abs(nSteps+1)
        
    g['cgraph']=gui.Bundle(win.glWindow,g['npoints'],g['nparts'],
                           len(g['varying']),g['varwave'],g['constwave'],
                           lambda double,button,point,x,y:
                           mouse_callback(g,double,button,point,x,y,win))
    g['cgraph'].set_marker_style(gui.POINT_TYPE[win.options['Drawing']['marker']['style'].label])
    g['cgraph'].set_marker_size(win.options['Drawing']['marker']['size'].value)
    g['cgraph'].set_point_style(gui.POINT_TYPE[win.options['Drawing']['point']['style'].label])
    g['cgraph'].set_point_size(win.options['Drawing']['point']['size'].value)
    g['cgraph'].set_style(gui.DRAWING_TYPE[win.options['Drawing']['style'].label])
    g['cgraph'].set_color(win.options['Drawing']['color'])
    g['cgraph'].set_delay(win.options['Drawing']['delay'].value)
    graphs.show(g,win)
    g['cgraph'].replay()
    graphs.hide(win.cursor)

def mouse_callback(g,double,button,point,x,y,win):
    identifier = g['identifier']
    if(double):
        picking.select(identifier,point,win)
    else:
        pass
    
def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('Manifolds', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Numerical')
    panel.add('Manifolds.eps',1e-5)
    panel.add('Manifolds.arc',0.1)
    panel.add('Manifolds.alpha',0.3)
    panel.add('Manifolds.ninit',int(20))
    panel.add('Manifolds.fudge',1.05)
    win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('Manifolds'):
        return
    
    # remove options from panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('Numerical')
    panel.remove('Manifolds.eps')
    panel.remove('Manifolds.arc')
    panel.remove('Manifolds.alpha')
    panel.remove('Manifolds.ninit')
    panel.remove('Manifolds.fudge')
    win.release_option_panel('Numerical')
    win.glWindow.flush()
