import math
from . import gui
from . import num
from . import common
from . import library
from . import options
from . import graphs
from . import windowlist
from . import equations
from . import maxpts

commands={}

def pert(n,idx,g=None,win=None):
    win = windowlist.fetch(win)
    g = graphs.get(g)

    n=int(n)
    if type(idx)!=list:
        idx=[int(idx)-1]
    else:
        idx=[int(i)-1 for i in idx]

    evecs=[g['evecs'][i] for i in idx]

    eps=win.options['Numerical']['Manifolds']['eps']

    evecs=num.gsortho(evecs)

    nDim = len(evecs)-1

    if n > maxpts.nMax[nDim][0]:
        win.console.write_warning("n="+str(n)+ " is too large for "+str(nDim)+"-dimensional grid;" +
                                  " capping to n="+str(maxpts.nMax[nDim][0])+" for "+str(maxpts.nMax[nDim][1])+ " grid points\n")
        n=maxpts.nMax[nDim][0];

    varying=win.cursor['varying']
    varVals=win.cursor['varwave'][:]
    const = win.cursor['const']
    constVals=win.cursor['constwave'][:]

    nVar=len(evecs[0])
    # the line below adds the second point along opposite directions of the eigenvector for nDim=0 and n=2
    # for dependent functions this is slightly inaccurate: we assign the values at the selected point to all
    # varying values that are not variables to the perturbed point as well
    newVarVals = [] if (nDim > 0 or n==1 ) else [varVals[0]]+[varVals[i+1]-eps*evecs[0][i] for i in range(nVar)]+varVals[1+nVar:]

    # build up a regular grid on the surface of the n-sphere with radius eps in
    # the selected subspace; use n points for the largest circle on this surface
    # example:
    # for a 3-sphere with n=100, we use 100 points around the equator
    # and 51 layers from pole to pole, for a total of 3144 points
    for phiset in num.angles(n,nDim):
        coeffs=[eps*math.cos(phi) for phi in phiset]+[eps]
        for i in range(nDim+1):
            for j in range(i):
                coeffs[i]*=math.sin(phiset[j])
                                
        # again, this is slightly inaccurate: we assign the values at the selected point all varying values that are not variables ... 
        newVarVals+=[varVals[0]]+[varVals[i+1]+sum([coeffs[j]*evecs[j][i] for j in range(nDim+1)]) for i in range(nVar)]+varVals[1+nVar:]
        
    graphs.move_cursor(win,varying,const,newVarVals,constVals)
    win.selection=None
    
commands['pert']=pert

def pertu(n,g=None,win=None):
    win = windowlist.fetch(win)
    g = graphs.get(g)

    mode = g['mode']

    evreal=g['evreal']
    evimag=g['evimag']

    eps=win.options['Numerical']['Manifolds']['eps']

    if mode == 'ode':
        idx=[i+1 for i in range(len(evreal)) if evreal[i]>0.0]
    else:
        idx=[i+1 for i in range(len(evreal)) if evreal[i]**2+evimag[i]**2>1.0]

    pert(n,idx,g,win)

commands['pertu']=pertu

def perts(n,g=None,win=None):
    win = windowlist.fetch(win)
    g = graphs.get(g)

    mode = g['mode']

    evreal=g['evreal']
    evimag=g['evimag']

    eps=win.options['Numerical']['Manifolds']['eps']

    if mode == 'ode':
        idx=[i+1 for i in range(len(evreal)) if evreal[i]<0.0]
    else:
        idx=[i+1 for i in range(len(evreal)) if evreal[i]**2+evimag[i]**2<1.0]

    pert(n,idx,g,win)

commands['perts']=perts

def circle(d,n=100,win=None):
    win = windowlist.fetch(win)
    
    d=float(d)
    n=int(n)

    if n<2:
        raise Exception('circle must have at least two points')
    
    
    xexp=win.options['View']['x']
    yexp=win.options['View']['y']
    if not (xexp in win.eqsys.var_names()
            or xexp in win.eqsys.par_names()):
        raise Exception('cannot set initial conditions as long as x-axis shows '+xexp)
    if not (yexp in win.eqsys.var_names()
            or yexp in win.eqsys.par_names()):
        raise Exception('cannot set initial conditions as long as y-axis shows '+yexp)
    xval=float(win.eqsys.parse('$'+xexp))
    yval=float(win.eqsys.parse('$'+yexp))

    min,max=win.glWindow.range()
    dx=d*(max[0]-min[0])*0.5
    dy=d*(max[1]-min[1])*0.5
    dphi=2*math.pi/n
    phi=0
    varying=[]
    varVals=[]
    win.eqsys.parse(xexp+"="+str(xval+math.cos(phi)*dx))
    const, constVals = equations.point(win)
    for i in range(0,n-1):
        phi=phi+dphi
        win.eqsys.stall()
        win.eqsys.parse(xexp+"="+str(xval+math.cos(phi)*dx))
        win.eqsys.parse(yexp+"="+str(yval+math.sin(phi)*dy))
        win.eqsys.flush()
        names, vals = equations.point(win)
        update_varying_const(i+1,varying,varVals,const,constVals,names,vals)

    graphs.move_cursor(win,varying,const,varVals,constVals)

    win.selection=None
        
commands['cir']=commands['circ']=commands['circl']=commands['circle']=circle

def fill(n=10, win=None):
    win = windowlist.fetch(win)

    nFill=int(n)

    if nFill<2:
        raise Exception('must create at least two new segments')

    nParts=win.cursor['nparts']
    if nParts<2:
        raise Exception('need at least two points to interpolate')


    varying = win.cursor['varying']
    varWave=win.cursor['varwave']
    varWave.lock()
    varVals=[]
    varCVals=win.cursor['varwave'].data()
    nVars=len(varying)
    
    for k in range(nParts-1):
        for j in range(nFill):
            for i in range(nVars):
                varVals.append(varCVals[k*nVars+i]+(varCVals[(k+1)*nVars+i]-varCVals[k*nVars+i])*float(j)/float(nFill))
    for i in range(nVars):
        varVals.append(varCVals[(nParts-1)*nVars+i])
    varWave.unlock()           

    const = win.cursor['const']
    constVals=win.cursor['constwave'][:]

    graphs.move_cursor(win,varying,const,varVals,constVals)

    win.selection=None
    
commands['fil']=commands['fill']=fill    

def add(win=None):
    win = windowlist.fetch(win)

    names, vals = equations.point(win)

    varVals=win.cursor['varwave'][:]
    constVals=win.cursor['constwave'][:]

    varying=win.cursor['varying']
    const=win.cursor['const']

    nParts=win.cursor['nparts']+1
    update_varying_const(nParts-1,varying,varVals,const,constVals,names,vals)

    graphs.move_cursor(win,varying,const,varVals,constVals)

    win.selection=None

def keep(win=None):
    win = windowlist.fetch(win)

    varying=win.cursor['varying']
    const=win.cursor['const']
    
    varVals=win.cursor['varwave'][:]+win.cursor['varwave'][-len(varying):]
    constVals=win.cursor['constwave'][:]

    graphs.move_cursor(win,varying,const,varVals,constVals,win.cursor['nparts']+1)

    win.selection=None

commands['keep'] = keep    

def pick(win=None):
    win = windowlist.fetch(win)
    graphs.move_cursor(win)

commands['pick'] = pick    
    
def where(win=None):
    """ where
    
    Prints the current values of variables and
    parameters at the console.
    """
    win=windowlist.fetch(win)

    varNames=win.eqsys.var_names()
    parNames=win.eqsys.par_names()
    try:
        win.glWindow.stall()
        nParts=win.cursor['nparts']
        if nParts>1:
            win.console.write("currently, "+str(nParts)+" points are selected. First point at:\n")

        pt = 0
        for tVal,varVals,parVals in points(win,['t'],varNames,parNames):
            if pt == 0:
                win.console.write('t = ')
                win.console.write_data(str(float("{0:.10f}".format(tVal[0])))+'\n')
                if len(varNames): win.console.write("variables:\n")
                for i in range(len(varNames)):
                    win.console.write(varNames[i]+" = ")
                    win.console.write_data(str(float("{0:.10f}".format(varVals[i])))+'\n')
                if len(parNames): win.console.write("parameters:\n")
                for i in range(len(parNames)):
                    win.console.write(parNames[i]+" = ")
                    win.console.write_data(str(float("{0:.10f}".format(parVals[i])))+'\n')
            pt = pt+1      
    finally:
        win.glWindow.flush()

commands['w']=commands['wh']=commands['whe']=commands['wher']=commands['where']=where 

def which(win=None):
    """ which
    
    Prints the currently selected object
    """
    win=windowlist.fetch(win)
    if win.selection:
        win.console.write(win.selection['identifier']+'\n')
    else:
        win.console.write("currently, no object is selected\n")
        
commands['which']=which 

def select(identifier,point=-1,win=None,silent=False):
    win=windowlist.fetch(win)
    g=graphs.get(identifier,win)

    if point >= g['npoints']*g['nparts']:
        raise Exception("cannot select point with index "+str(point)+": not enough points")
    
    if win.eqsys.timestamp()!=g['timestamp']:
        raise Exception("structure of equation system has changed: cannot move to "+g['identifier'])

    if point==-1:
        win.selection=g
        if not silent:
            win.console.write("selecting ")
            win.console.write_data(identifier+"\n")
    
    if g['callbacks']['cursor']:
        varying, const, varVals, constVals = g['callbacks']['cursor'](point)
    else:
        return

    # t is never reset to the end value of an orbit
    # because this causes too much trouble with
    # stroboscopic maps due to inaccuracies for large t
    # => do not include 't' in list of names
    names=win.eqsys.var_names()+win.eqsys.par_names()
    
    try:
        win.eqsys.stall()
        nVarying=len(varying)
        nConst=len(const)
        for i in range(nVarying):
            if g['varying'][i] in names:
                win.eqsys.parse(g['varying'][i]+"="+str(varVals[i-nVarying]))
                win.invsys.parse(g['varying'][i]+"="+str(varVals[i-nVarying]))
                if not silent and point != -1:
                    win.console.write("setting "+g['varying'][i]+" to ")
                    win.console.write_data(str(varVals[i-nVarying])+"\n")
        for i in range(nConst):
            if g['const'][i] in names:
                win.eqsys.parse(g['const'][i]+"="+str(constVals[i]))
                win.invsys.parse(g['const'][i]+"="+str(constVals[i]))
                if not silent and point != -1:
                    win.console.write("setting "+g['const'][i]+" to ")
                    win.console.write_data(str(constVals[i])+"\n")
    finally:
        win.eqsys.flush()

    try:
        win.glWindow.stall()
        equations.rebuild_panels(win)
        if point >= 0:
            if win.selection:
                win.selection=None
                graphs.move_cursor(win)
            else:
                add(win)
        else:
            graphs.move_cursor(win,varying,const,varVals,constVals)
            graphs.hide(win.cursor)
    finally:
        win.glWindow.flush()

commands['sel']=commands['sele']=commands['selec']=commands['select']=select

def points(win,*allnames):
    varying, const = win.cursor['varying'], win.cursor['const']
    varVals, constVals = win.cursor['varwave'][:], win.cursor['constwave'][:]
    
    nVars = len(varying)
    
    for i in range(win.cursor['nparts']):
        result = ()
        for names in allnames:
            values = []
            for name in names:
                try:
                    values.append(varVals[varying.index(name)+i*nVars])
                except:
                    try:
                        values.append(constVals[const.index(name)])
                    except:
                        values.append(0)
            result = result + (values,)
        yield result

def on_pick(win,ctrl,*args):
    allow=True
    for i in range(gui.Z_INDEX+1):
        var = win.options['View'][gui.COORDINATE_NAME[i]]
        if (not (var in win.eqsys.var_names() or 
                 var in win.eqsys.par_names()) and
        args[i]):
            allow = False
    if not allow:
        win.console.write_warning("mouse picking is deactivated\n")
    else:
        for i in range(gui.Z_INDEX+1):
            if(args[i]):
                var=win.options['View'][gui.COORDINATE_NAME[i]]
                win.eqsys.parse(var+"="+str(args[i]))
                win.invsys.parse(var+"="+str(args[i]))
                win.console.write("setting "+var+" to ")
                win.console.write_data(str(args[i])+"\n")
        equations.rebuild_panels(win)
        if ctrl:
            add(win)
        else:
            graphs.move_cursor(win)

        win.selection=None

def on_parse(win):
    # if the structure has not changed, replace the last point of the cursor with
    # the current point; if the structure has changed, create a new single cursor
    if not (win.eqsys.timestamp() == win.cursor['timestamp'] and win.cursor['nparts'] > 1):
        graphs.move_cursor(win)
    else:
        names, vals = equations.point(win)

        varWave=win.cursor['varwave']
        varVals=varWave[:-len(win.cursor['varying'])]
        constVals=win.cursor['constwave'][:]
            
        varying=win.cursor['varying']
        const=win.cursor['const']

        nParts=win.cursor['nparts']

        update_varying_const(nParts-1,varying,varVals,const,constVals,names,vals)
        
        graphs.move_cursor(win,varying,const,varVals,constVals)

        win.selection=None


def update_varying_const(nParts,varying,varVals,const,constVals,names,vals):
    nVars=len(varying)

    for i in range(len(names)):
        if names[i] in varying:
            varVals.append(vals[i])
        else:
            isConst = False
            for j in range(len(const)):
                if names[i] == const[j]:
                    if vals[i] == constVals[j]:
                        isConst=True
                    break
            if not isConst:
                # delete label and value from constants
                idx=const.index(names[i])
                value=constVals[idx]
                del const[idx]
                del constVals[idx]

                # insert label and both old and new const values into varyings
                varying.append(names[i])
                for j in range(nParts):
                    varVals.insert(j*(nVars+1)+nVars,value)
                varVals.append(vals[i])
                nVars=nVars+1

def set(name,value,win=None):
    win=windowlist.fetch(win)

    if name in win.eqsys.var_names() or name in win.eqsys.par_names() or name == 't':
        equations.parse(name+'='+str(value),win)
        on_parse(win)
    else:
        raise Exception(name + "is not in equation system")
                
def plug(win):
    if not win.register_plugin('picking', lambda:unplug(win),commands):
        return

    picker=gui.Picker(gui.VIEW_TYPE[win.options['View']['axes'].label])
    picker.set_callback(lambda ctrl,x,y,z: on_pick(win,ctrl,x,y,z))
    win.glWindow.connect_before(picker)

    setattr(win,'picker',picker)
    setattr(win,'selection',None)
    setattr(win,'cursor',None)
    
    graphs.move_cursor(win)

def unplug(win):
    if not win.unregister_plugin('picking'):
        return

    win.glWindow.remove_drawable(win.picker)
    win.glWindow.disconnect(win.picker)
    win.picker.destroy()
    

    delattr(win,'picker')
    delattr(win,'selection')

    graphs.destroy(win.cursor,win)
    delattr(win,'cursor')

