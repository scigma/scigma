from sys import version_info
try:
    if version_info.major == 2:
        # We are using Python 2.x
        import tkFileDialog as tkfile
    elif version_info.major == 3:
        # We are using Python 3.x
        import tkinter.filedialog as tkfile
except:
    print("tkinter not found / not using tk")   
import math
from ctypes import *
from . import lib
from . import default
from .util import Float
from . import options
from . import objects
from . import cppwrapper
from .dat import Wave
from .gui import COORDINATE_NAME, COORDINATE_FLAG, COORDINATE_INDEX, N_COORDINATES, C_COORDINATE, VIEW_TYPE
from .gui import graph, Curve, Navigator, Picker, QDOT, QUAD
from .gui import application
from .num import EquationSystem

""" Available commands:

This file contains all available commands with short descriptions.
Also, for each command a couple of abbreviations is stored in the
'alias' dictionary (so that, for example, the command 'guess' can 
be invoked by typing either 'g', 'gu', 'gue', 'gues', or 'guess').
In the descriptions, mandatory arguments are denoted like <this>,
optional arguments are denoted like [this].
 
"""
alias={}

def circle(d,n=100,instance=None):
    """ circle [name]

    creates a circle of initial conditions;
    d is the diameter measured in units of 
    the coordinate system
    """
    d=float(d)
    n=int(n)
    if not instance:
        instance=default.instance
    if n<2:
        raise Exception('circle must have at least two points')

    xexp=instance.options['View']['x']
    yexp=instance.options['View']['y']
    if not (xexp in instance.equationSystem.variable_names()
            or xexp in instance.equationSystem.parameter_names()):
        raise Exception('cannot set initial conditions as long as x-axis shows '+xexp)
    if not (xexp in instance.equationSystem.variable_names()
            or xexp in instance.equationSystem.parameter_names()):
        raise Exception('cannot set initial conditions as long as y-axis shows '+yexp)
    xval=float(instance.equationSystem.parse('$'+xexp))
    yval=float(instance.equationSystem.parse('$'+yexp))

    min,max=instance.glWindow.range()
    dx=d*(max[0]-min[0])*0.5
    dy=d*(max[1]-min[1])*0.5
    dphi=2*math.pi/(n-1)
    phi=0

    instance.equationSystem.parse(xexp+"="+str(xval+dx))
    objects.move_cursor(None,instance)
    for i in range(0,n-1):
        phi=phi+dphi
        instance.equationSystem.stall()
        instance.equationSystem.parse(xexp+"="+str(xval+math.cos(phi)*dx)) 
        instance.equationSystem.parse(yexp+"="+str(yval+math.sin(phi)*dy)) 
        instance.equationSystem.flush()
        objects.add_cursor(instance)

alias['cir']=alias['circ']=alias['circl']=alias['circle']=circle

def fill(n=10,instance=None):
    """ fill 

    fills the gaps between a set of initial conditions
    with a linear interpolation 
    """
    n=int(n)
    if not instance:
        instance=default.instance
    if not n>0:
        raise Exception('n must be at least 1')
    
    N=len(instance.cursorList)
    if N<2:
        raise Exception('need at least two points to interpolate')
    
    nVar=instance.equationSystem.n_variables()
    nPar=instance.equationSystem.n_parameters()
    varnames=instance.equationSystem.variable_names()
    parnames=instance.equationSystem.parameter_names()
    
    vardata=instance.cursorList[0]['__varwave__'].data()
    constdata=instance.cursorList[0]['__constwave__'].data()
    lvars=[]
    for j in range(nVar):
        lvars.append(vardata[j+1])
    for j in range(nPar):
        lvars.append(constdata[j])
    
    for i in range(1,N):
        vardata=instance.cursorList[i]['__varwave__'].data()
        constdata=instance.cursorList[i]['__constwave__'].data()
        vars=[]
        diffs=[]
        for j in range(nVar):
            vars.append(vardata[j+1])
            diffs.append((vars[j]-lvars[j])/float(n+1))
        for j in range(nPar):
            vars.append(constdata[j])
            diffs.append((vars[nVar+j]-lvars[nVar+j])/float(n+1))
        
        for k in range(1,n+1):
            instance.equationSystem.stall()
            for j in range(nVar):
                instance.equationSystem.parse(varnames[j]+"="+str(lvars[j]+float(k)*diffs[j])) 
            for j in range(nPar):
                instance.equationSystem.parse(parnames[j]+"="+str(lvars[j+nVar]+float(k)*diffs[j+nVar])) 
            instance.equationSystem.flush()
            objects.add_cursor(instance)
        
        lvars=vars

alias['fil']=alias['fill']=fill


def rtime(objlist=None, instance=None):
    """ rtime [name]

    prints and returns the return time of a set of orbits or
    fixed points of a stroboscopic/Poincare map.
    The orbits specified by objlist; if objlist is not given,
    or the currently selected list of objects is used. 
    Gives an error message if an entry of the specified object
    list does not have a return time
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objlist,instance)
    listrt=[]
    try:
        instance.glWindow.stall()
        for obj in objlist:
            rt=obj['__rtime__']
    except:
        raise Exception('selected object has no return time')
    finally:
        instance.glWindow.flush()
    instance.console.write_data(str(float("{0:.12f}".format(rt)))+'\n')
    listrt.append(rt)
    return listrt



alias['rt']=alias['rti']=alias['rtim']=alias['rtime']=rtime

def evals(objlist=None, instance=None):
    """ evals [name]

    prints and returns the eigenvalues/floquet multipliers
    of the objects specified by objlist, or of the currently 
    selected list of objects, if objlist is omitted.
    Gives an error message if an entry of the specified object
    list does not have eigenvalue information
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objlist,instance)
    try:
        instance.glWindow.stall()
        listreal=[]
        listimag=[]
        for obj in objlist:
            try:
                evreal = obj['__evreal__']
                evimag = obj['__evimag__']
            except:
                raise Exception('selected object has no eigenvalue data')
            for i in range(len(evreal)):
                instance.console.write(str(i+1).rjust(4)+":  ")
                if evimag[i]<0:
                    sign = " - i*" 
                elif evimag[i]>0:
                    sign = " + i*"
                else:
                    sign = None
                instance.console.write_data(str(float("{0:.12f}".format(evreal[i]))))
                if sign==" - i*":
                    instance.console.write_data(sign+str(float("{0:.12f}".format(-evimag[i]))))
                elif sign==" + i*":
                    instance.console.write_data(sign+str(float("{0:.12f}".format(evimag[i]))))
                if obj['__mode__']!= 'ode':
                    modulus = math.sqrt(evreal[i]*evreal[i]+evimag[i]*evimag[i])
                    instance.console.write("   (modulus: ")
                    instance.console.write_data(str(float("{0:.12f}".format(modulus))))
                    instance.console.write(")")
                instance.console.write(' \n')
            listreal.append(evreal)
            listimag.append(evimag)
    finally:
        instance.glWindow.flush()
    return listreal, listimag

alias['ev']=alias['eva']=alias['eval']=alias['evals']=evals 

def evecs(objlist=None, instance=None):
    """ evecs [name]

    prints and returns the eigenvectors of the objects specified
    by objlist, or of the currently selected list of objects, if 
    objlist is omitted. 
    Gives an error message if an entry of the specified object
    list does not have eigenvector information
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objlist,instance)
    try:
        instance.glWindow.stall()
        listevecs=[]
        for obj in objlist:
            try:
                evecs = obj['__evecs__']
                evimag = obj['__evimag__']
                nVar = len(evecs)
            except:
                raise Exception('selected object has no eigenvector data')
            impart=False
            for i in range(nVar):
                instance.console.write(str(i+1).rjust(4)+":  ")
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
                                line+=(sign+str(float("{0:.12f}".format(-evecs[i][j]))))
                            elif sign==" + i*":
                                line+=(sign+str(float("{0:.12f}".format(evecs[i][j]))))
                            line+=' , '
                instance.console.write_data(line.strip(' , ')+')\n')
            listevecs.append(evecs)
    finally:
        instance.glWindow.flush()
    return listevecs

alias['eve']=alias['evec']=alias['evecs']=evecs 

def where(instance=None):
    """ where
    
    Prints the current values of variables and
    parameters at the console.
    """
    if not instance:
        instance=default.instance
    objlist=instance.selectedObjectList
    vnames=instance.equationSystem.variable_names()
    pnames=instance.equationSystem.parameter_names()
    try:
        instance.glWindow.stall()
        if not objlist or (objlist[0]['__timestamp__']!=instance.timeStamp):
            vvalues=instance.equationSystem.variable_values()
            pvalues=instance.equationSystem.parameter_values()
            instance.console.write("variables:\n")
            for i in range(len(vnames)):
                instance.console.write(vnames[i]+" = ")
                instance.console.write_data(str(float("{0:.10f}".format(vvalues[i])))+'\n')
                instance.console.write("parameters:\n")
            for i in range(len(pnames)):
                instance.console.write(pnames[i]+" = ")
                instance.console.write_data(str(float("{0:.10f}".format(pvalues[i])))+'\n')
            return [vvalues],[pvalues]
        
        vvaluelist=[]
        pvaluelist=[]
        if len(objlist)>1:
            instance.console.write("you have picked "+str(len(objlist))+" points:\n")
        for obj in objlist:
            vvalues,pvalues=objects.values(obj,instance)
            vvaluelist.append(vvalues)
            pvaluelist.append(pvalues)
            instance.console.write("variables:\n")
            for i in range(len(vnames)):
                instance.console.write(vnames[i]+" = ")
                instance.console.write_data(str(float("{0:.10f}".format(vvalues[i])))+'\n')
            instance.console.write("parameters:\n")
            for i in range(len(pnames)):
                instance.console.write(pnames[i]+" = ")
                instance.console.write_data(str(float("{0:.10f}".format(pvalues[i])))+'\n')
         
        return vvaluelist,pvaluelist
    
    finally:
        instance.glWindow.flush()

alias['w']=alias['wh']=alias['whe']=alias['wher']=alias['where']=where

def delete(objstring, instance=None):
    """delete <object>

    Deletes the object with the specified identifier.
    """
    if not instance:
        instance=default.instance
    try:
        instance.glWindow.stall()
        objlist=objects.get(objstring,instance)
        for obj in objlist:
            objects.delete(obj, instance)
        id = objlist[0]['__id__'].partition('[')[0]
        del instance.objects[id]
    finally:
        instance.glWindow.flush()

alias['del']=alias['dele']=alias['delet']=alias['delete']=delete

def clear(instance=None):
    """ clear

    Deletes all graphical objects.
    """
    if not instance:
        instance=default.instance
    instance.glWindow.stall()
    keys=[key for key in instance.objects]
    for key in keys:
        delete(key,instance)
    instance.glWindow.flush()
    instance.identifiers={}

alias['cl']=alias['cle']=alias['clea']=alias['clear']=clear

def fit(instance=None):
    """ fit

    Adjusts the axes that are currently shown such that
    every object currently drawn fits into the viewing window.
    """
    if not instance:
        instance=default.instance
        
    axes=options.get_string('View.axes',instance)
    visvar={}
    for i in range(N_COORDINATES):
        if VIEW_TYPE[axes]&COORDINATE_FLAG[COORDINATE_NAME[i]]:
            exp=instance.options['View'][COORDINATE_NAME[i]]
            if not (exp=='t' or exp in instance.equationSystem.variable_names()
                    or exp in instance.equationSystem.parameter_names()
                    or exp in instance.equationSystem.function_names()
                    or exp in instance.equationSystem.constant_names()):
                raise Exception("cannot fit while "+COORDINATE_NAME[i]+"-axis shows " + exp)
            visvar[exp]=COORDINATE_NAME[i]
    instance.glWindow.stall()
    for symbol in visvar:
        mi=1e300
        ma=-1e300
        for objkey in instance.objects:
            objlist=instance.objects[objkey]
            for obj in objlist:
                if obj['__visible__']:
                    value = obj['__min__'][symbol] if (('__min__' in obj) and (symbol in obj['__min__'])) else 1e300 
                    mi = value if value < mi else mi
                    value = obj['__max__'][symbol] if (('__max__' in obj) and (symbol in obj['__max__'])) else -1e300 
                    ma = value if value > ma else ma
        mi=0.0 if mi==1e300 else mi
        ma=0.0 if ma==-1e300 else ma
        if mi==ma:
            mi=mi-1
            ma=ma+1
        set_range(visvar[symbol],mi,ma,instance)
    instance.glWindow.flush()

alias['f']=alias['fi']=alias['fit']=fit

def mark(instance=None):
    "marks a periodic point with period > 1"
    if not instance:
        instance=default.instance
        
    
    nperiod=instance.options['Numerical']['nperiod']
    mode=instance.options['Numerical']['mode'].label
    
    origlist=objects.get(None,instance)
    objlist=objects.newlist(objects.new_identifier("ppp",instance),nperiod,instance)
    if origlist[0]['__type__']!='pt' or origlist[0]['__mode__'] == 'ode':
        raise Exception("Can only mark periodic points")
    
    marker = instance.options['Style']['marker']['style']
    marker = marker.definition['none']
    markerSize = 1.0
    pointSize = instance.options['Style']['marker']['size'].value
    color=instance.options['Style']['color']
    delay=0.0
    
    instance.options['Numerical']['nperiod']=1
    instance.options['Numerical']['mode'].label=objlist[0]['__mode__']
    
    # start plotting
    cppwrapper.plot(nperiod-1,objlist,instance)
    # now create the curve
    i = 0
    for obj in objlist:   
        obj['__type__']='pt'
        point = QDOT if origlist[i]['__stable__'] else QUAD
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],
                               nperiod,obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,delay,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
        i=i+1
    
    instance.options['Numerical']['nperiod']=nperiod
    instance.options['Numerical']['mode'].label=mode
    
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

alias['ma']=alias['mar']=alias['mark']=mark

def plot(nSteps=1,name=None,instance=None,showall=False,noThread=False):
    """ plot [n] [name]
    
    Performs and plots n iterations of the current map or n time steps
    of the ODE integration, starting at the current position in phase 
    space and parameter space. Makes a single step if n is not given.
    The resulting trajectory is displayed in the current line style 
    (see 'linestyle' and 'linewidth' and 'color' commands). The data
    is stored in the dictionary obj, which is also returned. name is
    used as identifier, or a generic name of the form 'tr<N>' if
    name is not given.     
    """
    if not instance:
        instance=default.instance
    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read N (usage: plot [n] [name])")
    
    if instance.equationSystem.n_variables() == 0:
        raise Exception ("error: no variables defined")
    
    n = nSteps if nSteps>0 else -nSteps
    mode=instance.options['Numerical']['mode'].label
    nperiod=instance.options['Numerical']['nperiod']   
    
    n = n*nperiod if (showall and mode!='ode') else n 
    
    if name:
        objlist=objects.newlist(name,n+1,instance)
    else:
        objlist=objects.newlist(objects.new_identifier("tr",instance),n+1,instance)
        
    marker = instance.options['Style']['marker']['style']
    marker = marker.definition[marker.label]
    point = instance.options['Style']['point']['style']
    point = point.definition[point.label]
    markerSize = instance.options['Style']['marker']['size'].value
    pointSize = instance.options['Style']['point']['size'].value
    color=instance.options['Style']['color']
    delay=instance.options['Style']['delay'].value
    
    # start plotting
    cppwrapper.plot(nSteps,objlist,showall,noThread,instance)
    # now create the curve
    for obj in objlist:   
        obj['__type__']='tr'
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],
                               n+1,obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,delay,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
    
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

alias['p']=alias['pl']=alias['plo']=alias['plot']=plot

def plotall(nSteps=1,name=None,instance=None,noThread=False):
    return plot(nSteps,name,instance,True,noThread)

alias['p*']=alias['pl*']=alias['plo*']=alias['plot*']=plotall

def guess(name=None,instance=None,showall=False):
    """ guess [name]
    Starting at the current position in phase space and parameter 
    space, search for a steady state of the current map/ODE with
    a Newton iteration. If successful, the newly found fixed point
    is displayed with a marker
    The point is stored using the identifier name, or with a generic 
    name of the form 'fpN' (for odes) or 'ppN' (for maps), if no 
    name is given. 
    """
    if not instance:
        instance=default.instance
    
    nVar=instance.equationSystem.n_variables()
    
    if nVar == 0:
        raise Exception ("error: no variables defined")
    
    mode=instance.options['Numerical']['mode'].label
    if mode=='ode':
        defname='fp'
    else:
        defname='pp'
    
    mode=instance.options['Numerical']['mode'].label
    nperiod=instance.options['Numerical']['nperiod']   
    
    n = nperiod if (showall and mode!='ode') else 1 
    
        # get a dictionary to store data and graph handles
    if name:
        objlist=objects.newlist(name,n,instance)
    else:
        objlist=objects.newlist(objects.new_identifier(defname,instance),n,instance)
    
    marker = instance.options['Style']['marker']['style']
    marker = marker.definition['none']
    markerSize = 1.0
    point = instance.options['Style']['point']['style']
    point = point.definition[point.label]
    pointSize = instance.options['Style']['marker']['size'].value
    color=instance.options['Style']['color']
    delay=0.0
    nperiod=instance.options['Numerical']['nperiod']   
    
    for obj in objlist:
        objects.move_to(obj,instance)
        obj['__nVar__']=nVar
        cppwrapper.guess(obj,showall,instance)
        obj['__type__']='pt'
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],n,
                               obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,0.0,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
    
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

alias['g']=alias['gu']=alias['gue']=alias['gues']=alias['guess']=guess

def guessall(name=None,instance=None):
    return guess(name, instance, True)

alias['g*']=alias['gu*']=alias['gue*']=alias['gues*']=alias['guess*']=guessall

def manifold(stable,n=1,originlist=None,name=None,instance=None,showall=False,noThread=False):
    if not instance:
        instance=default.instance
    try:
        n = int(n)
    except ValueError:
        raise Exception("error: could not read N (usage: mu/ms [n] [name])")
    if n<0:
        n=-n
    
    # the object from which to generate the manifold
    origobjlist=objects.get(originlist,instance)
    
    # check, whether the structure of the equation system is still the same
    if origobjlist[0]['__timestamp__']!=instance.timeStamp:
        id=origobjlist[0]['__id__'].partition('[')[0]
        raise Exception("structure of equation system has changed - cannot compute invariant manifold of object "+ id)
    
    # get a dictionary to store data and graph handles
    if name:
        objlist=objects.newlist(name,1,instance)
    else:
        objlist=objects.newlist(objects.new_identifier("mf",instance),1,instance)
    
    eps=instance.options['Numerical']['manifolds']['eps']
    marker = instance.options['Style']['marker']['style']
    marker = marker.definition[marker.label]
    point = instance.options['Style']['point']['style']
    point = point.definition[point.label]
    markerSize = instance.options['Style']['marker']['size'].value
    pointSize = instance.options['Style']['point']['size'].value
    color=instance.options['Style']['color']
    delay=instance.options['Style']['delay'].value
    evindex=instance.options['Numerical']['manifolds']['evec1']
    
    mode=instance.options['Numerical']['mode'].label
    nperiod=instance.options['Numerical']['nperiod']
    nPoints = ((n+1)*nperiod) if (showall and mode !='ode') else (n+1)
    
    for origobj,obj in zip(origobjlist,objlist):
        # see if we can use the specified origin to create an unstable manifold
        try:
            evreal = origobj['__evreal__']
            evimag = origobj['__evimag__']
            evecs = origobj['__evecs__']
        except:
            raise Exception(origobj['__id__']+': origin has no eigenvalue and/or eigenvector data')
        
        if stable and evimag[evindex-1]!=0:
            raise Exception(origobj['__id__']+': eigenvector number '+ str(len(evreal)-evindex+1) +' has complex eigenvalue (evec1='+str(evindex)+')')
        elif not stable and evimag[-evindex]!=0:
            raise Exception(origobj['__id__']+': eigenvector number '+ str(len(evreal)-evindex+1) +' has complex eigenvalue (evec1='+str(evindex)+')')
        
        if origobj['__mode__']=='ode':
            if stable and evreal[evindex-1]>=0:
                raise Exception(origobj['__id__']+': real part of eigenvalue number ' + str(len(evreal)-evindex+1) +' is not negative (evec1='+str(evindex)+')')
            elif not stable and evreal[-evindex]<=0:
                raise Exception(origobj['__id__']+': real part of eigenvalue number ' + str(len(evreal)-evindex+1) +' is not positive (evec1='+str(evindex)+')')
        else:
            if stable and evreal[evindex-1]*evreal[evindex-1]+evimag[evindex-1]*evimag[evindex-1]>=1:
                raise Exception(origobj['__id__']+': modulus of multiplier number ' + str(len(evreal)-evindex+1) +' is not < 1 (evec1='+str(evindex)+')')
            elif not stable and evreal[-evindex]*evreal[-evindex]+evimag[-evindex]*evimag[-evindex]<=1:
                raise Exception(origobj['__id__']+': modulus of multiplier number ' + str(len(evreal)-evindex+1) +' is not > 1 (evec1='+str(evindex)+')')
        
        # print information about selected epsilon, eigenvalue and eigenvector
        instance.glWindow.stall()
        instance.console.write("eps: ")
        instance.console.write_data(str(instance.options['Numerical']['manifolds']['eps'])+'\n')
        instance.console.write("eigenvalue: ")
        if stable:
            instance.console.write_data(str(evreal[evindex-1])+'\n')
        else:
            instance.console.write_data(str(evreal[-evindex])+'\n')
        instance.console.write("eigenvector: ")
        line ='('
        for j in range(len(evreal)):
            if stable:
                line+=str(float("{0:.12f}".format(evecs[evindex-1][j])))+","
            else:
                line+=str(float("{0:.12f}".format(evecs[-evindex][j])))+","
        line=line.strip(',')+')\n'
        instance.console.write_data(line)
        instance.glWindow.flush()
        
        fp = origobj['__varwave__'].data()
        rows = origobj['__varwave__'].rows()
        columns = origobj['__varwave__'].columns()
        init=[]
        for i in range(columns):
            init.append(fp[i+columns*(rows-1)])
        init.append(fp[columns*(rows-1)])
        if stable:
            for i in range(len(evreal)):
                init.append(fp[i+1+columns*(rows-1)]+eps*evecs[evindex-1][i])
        else:
            for i in range(len(evreal)):
                init.append(fp[i+1+columns*(rows-1)]+eps*evecs[-evindex][i])
        
        # for the next bit, we are cheating, because we do not
        # actually evaluate the values of dependent functions
        # for the initial segment - instead, we copy the values
        # for the fixed point
        for i in range(1+len(evreal),columns):
            init.append(fp[i+columns*(rows-1)])
            
        obj['__varwave__'].destroy()
        obj['__varwave__']=Wave(init,columns,lines=nPoints)


        objects.move_to(obj,instance)
        
    if n>1:
        if origobjlist[0]['__mode__']=='ode':
            nSteps= 1-n if stable else n-1
            cppwrapper.plot(nSteps,objlist,showall,noThread,instance)
        else:
            for obj in objlist:
                eival = 1/evreal[evindex-1] if stable else evreal[-evindex]
                nSteps = 1-n if stable else n-1
                cppwrapper.map_manifold(nSteps,eival,obj,showall,noThread,instance)
        
    # create the curves
    for obj in objlist:
        obj['__type__']='mf' if stable else 'mu'
        obj['__graph__']=Curve(instance.glWindow,obj['__id__'],nPoints,
                               obj['__varwave__'],obj['__constwave__'],
                               marker,point,markerSize,pointSize,color,delay,
                               lambda identifier:instance.select_callback(identifier))
        objects.show(obj,instance)
        
    instance.pendingTasks=instance.pendingTasks+len(objlist)
    return objlist

def munstable(n=1,origin=None,name=None,instance=None,noThread=False):
    """ munstable [n] [origin] [name]
    
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
    return manifold(False,n,origin,name,instance,False,noThread)
alias['mu']=alias['mun']=alias['muns']=alias['munst']=alias['munsta']=alias['munstab']=alias['munstabl']=alias['munstable']=munstable
def mstable(n=1,origin=None,name=None,instance=None,noThread=False):
    return manifold(True,n,origin,name,instance,False,noThread)
alias['ms']=alias['mst']=alias['msta']=alias['mstab']=alias['mstabl']=alias['mstable']=mstable
def munstableall(n=1,origin=None,name=None,instance=None,noThread=False):
    return manifold(False,n,origin,name,instance,True,noThread)
alias['mu*']=alias['mun*']=alias['muns*']=alias['munst*']=alias['munsta*']=alias['munstab*']=alias['munstabl*']=alias['munstable*']=munstableall
def mstableall(n=1,origin=None,name=None,instance=None,noThread=False):
    return manifold(True,n,origin,name,instance,True,noThread)
alias['ms*']=alias['mst*']=alias['msta*']=alias['mstab*']=alias['mstabl*']=alias['mstable*']=mstableall

def show(objstring, instance=None):
    """show <object>

    Displays the object with the specified identifier.
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objstring,instance)
    for obj in objlist:
        objects.show(obj, instance)

alias['show']=show

def hide(objstring, instance=None):
    """hide <object>

    Hides the object with the specified identifier.
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objstring,instance)
    for obj in objlist:
        objects.hide(obj, instance)

alias['hide']=hide

def select(objstring, instance=None):
    """select <object>

    Selects the object with the specified identifier.
    """
    if not instance:
        instance=default.instance
    objlist=objects.get(objstring,instance)
    objects.select(objlist, instance)

alias['sel']=alias['sele']=alias['selec']=alias['select']=select

def equations(filename=None,instance=None):
    """ equations [filename]

    Loads a python file with equations
    """
    if not instance:
        instance=default.instance
    if not filename:
        filename=tkfile.askopenfilename()
    if filename=='internal':
        instance.equationSystem=instance.equationSystemBackup
        instance.equationPanel.define("","visible=true")
        instance.source='internal'
    elif filename:
        instance.equationSystemBackup=instance.equationSystem
        instance.equationSystem=EquationSystem(filename)
        instance.equationPanel.define("","visible=false")
        instance.source=filename
    instance.glWindow.set_title("SCIGMA - script: "+instance.script+" - equations: "+instance.source)
    instance.rebuild_panels()

alias['eq']=alias['equ']=alias['equa']=alias['equat']=alias['equati']=alias['equatio']=alias['equation']=alias['equations']=equations

def load(filename=None,instance=None):
    """ load <filename>

    Loads a script file and executes the content;
    opens a file dialog if filename is not given.
    """
    if not instance:
        instance=default.instance
    if not filename:
        filename=tkfile.askopenfilename()
    if filename[-2:]=='py':
        execfile(filename)
        return
    try:
        with open(filename) as f:
            script = f.readlines()
        for line in script:
            try:
                instance.console_callback(line)
                # process notifications in between commands!
                instance.loop_callback()
            except:
                pass # exceptions have already been caught in console_callback()
    except IOError:
        raise Exception(filename+": file not found")
    instance.script=filename
    instance.glWindow.set_title("SCIGMA - script: "+instance.script+" - equations: "+instance.source)

alias['l']=alias['lo']=alias['loa']=alias['load']=load

def set_range(coord,low,high,instance=None):
    """ sets the range of the specified coordinate
    """
    coord = coord.lower()
    low = float(low)
    high = float(high)
    if not instance:
        instance=default.instance
    if low>=high:
        raise Exception("low must be smaller than high")
    instance.glWindow.set_range(COORDINATE_FLAG[coord],low,high)

# wrappers for set_range;
# unfortunately, they have to use the underscore in the function 
# name, because xrange is a builtin function of Python
def x_range(low,high,instance=None):
    """ xrange <low> <high>

    Adjusts the boundaries of the x coordinate.
    (equivalent for yrange, etc.)
    If low is larger than high, they are swapped.
    low and high must be different
    """
    set_range('X',low,high,instance)
def y_range(low,high,instance=None):
    set_range('Y',low,high,instance)
def z_range(low,high,instance=None):
    set_range('Z',low,high,instance)
def c_range(low,high,instance=None):
    set_range('C',low,high,instance)
def t_range(low,high,instance=None):
    set_range('T',low,high,instance)

alias['xr']=alias['xra']=alias['xran']=alias['xrang']=alias['xrange']=x_range
alias['yr']=alias['yra']=alias['yran']=alias['yrang']=alias['yrange']=y_range
alias['zr']=alias['zra']=alias['zran']=alias['zrang']=alias['zrange']=z_range
alias['cr']=alias['cra']=alias['cran']=alias['crang']=alias['crange']=c_range
alias['tr']=alias['tra']=alias['tran']=alias['trang']=alias['trange']=t_range

def expr(coord,*args,**kwargs):
    """ sets the expression(s) displayed along a coordinate axis
    """
    coord=coord.lower()
    if 'instance' in kwargs:
        instance = kwargs['instance']
    elif len(args) and type(args[-1])==type(default.instance):
        instance=args[-1]
        args=args[0:-1]
    else:
        instance = default.instance
    if not len(args):
        instance.console.write_data(instance.options['View'][coord])
    else:
        expression = ''.join(args)
        if expression == '':
            instance.console.write_data(instance.options['View'][coord])
        else:
            oldexp=instance.options['View'][coord]=expression
            try:
                options.set('View.'+coord,expression,instance)
                instance.cosy.set_label(COORDINATE_FLAG[coord],expression)
                graph.rebuild_shaders(instance)
            except Exception as e:
                options.set('View.'+coord,oldexp,instance)
                graph.rebuild_shaders(instance)
                raise e

#wrappers for expr;
def xexpr(*args,**kwargs):
    """ x <expr1> [|<expr2>|...|<exprN>]

    Sets the expression(s) displayed along the x axis.
    For example, to have the variables x and y/10 
    displayed along the x axis, call the function like this:
    x x| y/10
    from the console, or xexpr('x|y/10') from python. Do
    NOT use xexpr('x', 'y/10'), as this will be interpreted
    the same as xexpr('xy/10').
    You can use an arbitrary number of expressions, separated
    by a ','; if you do not want to associate the x dimension
    with an expression, call xexpr('!')
    To specify another than the default instance, call either
    xexpr(myExpression,myInstance) or xexpr(myExpression,instance=myInstance)
    yexpr etc. work analogously.
    """
    expr('x',*args,**kwargs)
def yexpr(*args,**kwargs):
    expr('y',*args,**kwargs)
def zexpr(*args,**kwargs):
    expr('z',*args,**kwargs)
def cexpr(*args,**kwargs):
    expr('c',*args,**kwargs)
#def texpr(*args,**kwargs):
#    expr('t',*args,**kwargs)

alias['x']=alias['xe']=alias['xex']=alias['xexp']=alias['xexpr']=xexpr
alias['y']=alias['ye']=alias['yex']=alias['yexp']=alias['yexpr']=yexpr
alias['z']=alias['ze']=alias['zex']=alias['zexp']=alias['zexpr']=zexpr
alias['c']=alias['ce']=alias['cex']=alias['cexp']=alias['cexpr']=cexpr
#alias['t']=alias['te']=alias['tex']=alias['texp']=alias['texpr']=texpr

def axes(axes,instance=None):
    """ axes <axes>

    Changes how phase space is displayed (switches between
    2D and 3D, disables/enables color map). 
    The options for the axes argument are
    'xy','xyc','xyt','xytc'
    'xyz','xyzc','xyzt','xyztc'
    """
    if not instance:
        instance=default.instance
    instance.glWindow.stall()
    oldaxes=options.get_string('View.axes',instance)
    axes=axes.lower()
    try:
        for i in range(N_COORDINATES):
            if VIEW_TYPE[axes]&COORDINATE_FLAG[COORDINATE_NAME[i]]:
                instance.viewPanel.define(COORDINATE_NAME[i],'visible=true')
                #instance.viewPanel.define(COORDINATE_NAME[i]+'range','visible=true')
            else:
                instance.viewPanel.define(COORDINATE_NAME[i],'visible=false')
                #instance.viewPanel.define(COORDINATE_NAME[i]+'range','visible=false')
        options.set("View.axes",axes,instance)
    except:
        instance.glWindow.flush()
        raise Exception(axes+": unknown axes specification")
    try:
        graph.rebuild_shaders(instance)
    except Exception as e:
        for i in range(N_COORDINATES):
            if VIEW_TYPE[oldaxes]&COORDINATE_FLAG[COORDINATE_NAME[i]]:
                instance.viewPanel.define(COORDINATE_NAME[i],'visible=true')
                #instance.viewPanel.define(COORDINATE_NAME[i]+'range','visible=true')
            else:
                instance.viewPanel.define(COORDINATE_NAME[i],'visible=false')
                #instance.viewPanexprel.define(COORDINATE_NAME[i]+'range','visible=false')
        options.set("View.axes",oldaxes,instance)
        graph.rebuild_shaders(instance)
        instance.glWindow.flush()
        raise e
    if 'z' in axes:
        instance.glWindow.set_viewing_angle(20)
    else: 
        instance.glWindow.set_viewing_angle(0)
        instance.glWindow.reset_rotation()
    instance.cosy.set_view(VIEW_TYPE[axes])
    instance.glWindow.disconnect(instance.navigator)
    instance.navigator.destroy()
    instance.navigator=Navigator(VIEW_TYPE[axes])
    instance.glWindow.connect(instance.navigator)
    instance.glWindow.disconnect(instance.picker)
    instance.picker.destroy()
    instance.picker=Picker(VIEW_TYPE[axes])
    instance.glWindow.connect_before(instance.picker)
    instance.picker.set_callback(lambda ctrl,x,y,z: instance.picker_callback(ctrl,x,y,z))
    instance.glWindow.flush()

alias['a']=alias['ax']=alias['axe']=alias['axes']=axes

def twoD(instance=None):
    if not instance:
        instance = default.instance
    oldview=options.get_string('axes',instance)
    parts=oldview.partition('z')
    newview=parts[0]+parts[2]
    axes(newview,instance)

def threeD(instance=None):
    if not instance:
        instance = default.instance
    oldview=options.get_string('axes',instance)
    if oldview[:3]!='xyz':
        newview='xyz'+oldview[2:]
        axes(newview,instance)

alias['2d']=alias['2D']=twoD
alias['3d']=alias['3D']=threeD

def reset(instance=None):
    """ reset
    Deletes all graphical objects and all equations, and
    resets the viewing volumme
    """
    if not instance:
        instance=default.instance
    clear(instance)
    xexpr('x',instance)
    yexpr('y',instance)
    zexpr('',instance)
    axes('xy', instance)
    x_range(-1,1,instance)
    y_range(-1,1,instance)
    z_range(-1,1,instance)
    c_range(0,1,instance)
    instance.glWindow.reset_rotation()
    instance.script='none'
    instance.source='internal'
    instance.equationSystemBackup=EquationSystem()
    equations('internal',instance)

alias['res']=alias['rese']=alias['reset']=reset

def mode(mode,instance=None):
    """ mode <mode>

    Changes the computation mode of Scigma.
    Options are 'map', 'ode', 'strobe' and 'Poincare'
    Depending on the selected mode, other options
    (such as 'secval' and 'secvar' for Poincare mode)
    become available
    """
    if not instance:
        instance=default.instance
    instance.glWindow.stall()
    abbrev={'m':'map','o':'ode','s':'strobe','p':'Poincare','P':'Poincare'}
    if mode in abbrev:
        mode = abbrev[mode]
    mode=mode.lower()
    if mode == 'map':
        instance.numericalPanel.define('dt', 'visible=false')
        instance.numericalPanel.define('manifolds.alpha','visible=true')
        instance.numericalPanel.define('manifolds.evec2','visible=false')
        instance.numericalPanel.define('manifolds.arc','visible=false')
        instance.numericalPanel.define('odessa', 'visible=false')
        instance.numericalPanel.define('secvar', 'visible=false')
        instance.numericalPanel.define('secval', 'visible=false')
        instance.numericalPanel.define('period', 'visible=false')
        instance.numericalPanel.define('nperiod', 'visible=true')
        instance.numericalPanel.define('secdir', 'visible=false')
        instance.numericalPanel.define('maxtime', 'visible=false')
    elif mode == 'ode':
        instance.numericalPanel.define('dt', 'visible=true')
        instance.numericalPanel.define('manifolds.alpha','visible=false')
        instance.numericalPanel.define('manifolds.evec2','visible=true')
        instance.numericalPanel.define('manifolds.arc','visible=true')
        instance.numericalPanel.define('odessa', 'visible=true')
        instance.numericalPanel.define('secvar', 'visible=false')
        instance.numericalPanel.define('secval', 'visible=false')
        instance.numericalPanel.define('period', 'visible=false')
        instance.numericalPanel.define('nperiod', 'visible=false')
        instance.numericalPanel.define('secdir', 'visible=false')
        instance.numericalPanel.define('maxtime', 'visible=false')
    elif mode == 'strobe':
        instance.numericalPanel.define('dt', 'visible=true')
        instance.numericalPanel.define('manifolds.alpha','visible=true')
        instance.numericalPanel.define('manifolds.evec2','visible=true')
        instance.numericalPanel.define('manifolds.arc','visible=true')
        instance.numericalPanel.define('odessa', 'visible=true')
        instance.numericalPanel.define('secvar', 'visible=false')
        instance.numericalPanel.define('secval', 'visible=false')
        instance.numericalPanel.define('period', 'visible=true')
        instance.numericalPanel.define('nperiod', 'visible=true')
        instance.numericalPanel.define('secdir', 'visible=false')
        instance.numericalPanel.define('maxtime', 'visible=false')
    elif mode == 'poincare':
        instance.numericalPanel.define('dt', 'visible=true')
        instance.numericalPanel.define('manifolds.alpha','visible=true')
        instance.numericalPanel.define('manifolds.evec2','visible=true')
        instance.numericalPanel.define('manifolds.arc','visible=true')
        instance.numericalPanel.define('odessa', 'visible=true')
        instance.numericalPanel.define('secvar', 'visible=true')
        instance.numericalPanel.define('secval', 'visible=true')
        instance.numericalPanel.define('period', 'visible=false')
        instance.numericalPanel.define('nperiod', 'visible=true')
        instance.numericalPanel.define('secdir', 'visible=true')
        instance.numericalPanel.define('maxtime', 'visible=true')
        mode = 'Poincare'
    else:
        instance.glWindow.flush()
        raise Exception(mode+": unknown mode specification")
    options.set("Numerical.mode",mode,instance)
    instance.rebuild_panels()
    instance.glWindow.flush()

alias['m']=alias['mo']=alias['mode']=alias['mode']=mode

def bye(instance=None):
    """ quit

    closes the instance window and frees all resources
    """
    if not instance:
        instance=default.instance
        default.instance = None
    clear(instance)
    instance.destroy()
    
alias['q']=alias['qu']=alias['qui']=alias['quit']=alias['end']=alias['bye']=bye

def parse(line,instance=None):
    """Used to parse differential equations and maps.

    This function is called automatically if no other command is entered.

    The syntax is as follows:
    * define differential equation:
    x' = x*y+a
    * define map:
    x+ = x**2+a
    * define inverse map:
    x- = sqrt(x-a)
    * set the value of a variable or parameter:
    a = sin(3.1415)+2
    * define a dependent function:
    f = x**a 
    * define a function with arguments:
    f(x,y)=x**2+y**2
    * use a function with arguments:
    a = f(2,3) (a will be equal to 13)
    * print the value of a variable or parameter
    $x
    * remove a variable, parameter or dependent function
    !x

    supported mathematical operations are:
    sin,cos,tan,asin,acos,atan,sinh,cosh,tanh,
    asinh,acosh,atanh,sqrt,exp,ln,log10,atan2,
    sigmoid,pulse,abs,sign,step,mod,pow +,-,*,/,**(or ^)
    (the last two are aliases for pow)

    """
    if not instance:
        instance = default.instance
    result=instance.equationSystem.parse(line)
    if(result[0:6]== "error:"):
        raise Exception(result[6:])
    else:
        instance.rebuild_panels()
        if(result != ''):
            instance.console.write_data(result+'\n')
            return float(result)

def inverse(line,instance=None):
    """Used to parse inverse maps.
    """
    if not instance:
        instance = default.instance
    if instance.options['Numerical']['mode'].label !='map':
        raise Exception("must be in 'map' mode to define inverse equations")
    result=instance.inverseEquationSystem.parse(line)
    if(result[0:6]== "error:"):
        raise Exception(result[6:])
    else:
        names=instance.equationSystem.variable_names()+instance.equationSystem.parameter_names()
        inames=instance.inverseEquationSystem.variable_names()+instance.inverseEquationSystem.parameter_names()
        instance.inverseConsistent=(names==inames)
        instance.rebuild_panels()
        if(result != ''):
            instance.console.write_data(result+'\n')
            return float(result)

def set(function, value, instance=None):
    if not instance:
        instance = default.instance
    result=instance.equationSystem.parse(function+"="+str(value))
    if(result[0:6]== "error:"):
        raise Exception(result[6:])
    instance.timeStamp=instance.equationSystem.timestamp()
    objects.move_cursor(None,instance)

def wait(seconds):
    application.loop(seconds)

