from ctypes import *
from . import lib, largeFontsFlag
from . import default
from .util import *
from .dat import Wave
from .num import N_EIGEN_TYPES, N_FLOQUET_TYPES, EIGEN_TYPE, FLOQUET_TYPE
from .gui import RING, RDOT, QUAD, QDOT
from .gui import Curve
from .gui import RCROSS,NONE

lib.scigma_num_destroy_thread.argtypes=[c_void_p]

def new_identifier(prefix,instance):
    """generates a unique name with given prefix
    
    This generates a name of the form 'TR_00001' (if prefix is 'TR').
    The counter is increased for each prefix, to ensure that all 
    names are unique (as long as no more than 100000 objects have
    the same prefix)
    """
    if prefix not in instance.identifiers:
        instance.identifiers[prefix]=1
    count = instance.identifiers[prefix]
    identifier = prefix + str(count)
    instance.identifiers[prefix]=count+1
    return identifier

def get(objlist, instance):
    if not objlist:
        objlist=instance.selectedObjectList
        if not objlist:
            raise Exception('no object selected')
    elif isinstance(objlist,str):
        objstr=objlist.partition('|')[0]
        objlist=dict_entry(objstr,instance.objects)
        if not objlist:
            raise Exception('unknown object: '+objstr)
    return objlist

def newobj(identifier,nEntry,nPoints,varying,const,varValues,constValues,instance):
    obj={'__no__':instance.runningObjectNumber,'__id__':identifier+'['+str(nEntry)+']', 
         '__mode__':instance.options['Numerical']['mode'].label, '__varying__':varying,
         '__varwave__':Wave(varValues,lines=nPoints), '__const__':const,
         '__constwave__':Wave(constValues,lines=1), '__visible__':False,
         '__timestamp__':instance.timeStamp}
    instance.runningObjectNumber=instance.runningObjectNumber+1
    return obj

def newlist(identifier, n, instance):
    """creates a new list of data objects (points, curves, etc.)
    
    If an object list is currently selected, the list is created from 
    the positions stored in that list. If there is no currently selected
    list of objects, or if the structure of the equation system has 
    changed since the selection, a list with 1 object in the current
    state of the equation system is created.
    """
    identifier=(instance.base+'.'+identifier).strip('.')
    path=identifier.split('.')
    parent = instance.objects
    alreadyDefined=True
    for child in path:
        if not child in parent:
            alreadyDefined=False
            parent[child]={}
        d = parent   
        parent = parent[child]
    key = child
    if alreadyDefined:
        raise Exception("identifier "+ identifier + " is already in use")
    
    varying=['t']+instance.equationSystem.variable_names()+instance.equationSystem.function_names()
    const=instance.equationSystem.parameter_names()+instance.equationSystem.constant_names()
    
    newlist=[]
    
    index=0
    for obj in instance.cursorList:
        offset=(obj['__varwave__'].rows()-1)*obj['__varwave__'].columns()
        vardata=obj['__varwave__'].data()
        constdata=obj['__constwave__'].data()
        varValues=[]
        constValues=[]
        for i in range(obj['__varwave__'].columns()):
            varValues.append(vardata[offset+i])
        for i in range(obj['__constwave__'].columns()):
            constValues.append(constdata[i])
        newlist.append(newobj(identifier,index,n,varying,const,varValues,constValues,instance))
        index=index+1
    
    d[key]=newlist
    return d[key]

def values(obj,instance):
    timeStamp=obj['__timestamp__']
    if timeStamp!=instance.timeStamp:
        raise Exception("structure of equation system has changed - cannot extract values from object "+ obj['__id__'])
    vvalues=[]
    pvalues=[]
    offset=(obj['__varwave__'].rows()-1)*obj['__varwave__'].columns()
    vardata=obj['__varwave__'].data()
    constdata=obj['__constwave__'].data()
    for i in range (instance.equationSystem.n_variables()):
        vvalues.append(vardata[offset+i+1])
    for i in range (instance.equationSystem.n_parameters()):
        pvalues.append(constdata[i])
    return vvalues, pvalues

def move_to(obj,instance):
    offset=(obj['__varwave__'].rows()-1)*obj['__varwave__'].columns()
    vardata=obj['__varwave__'].data()
    constdata=obj['__constwave__'].data()
    varnames=obj['__varying__']
    constnames=obj['__const__']

    instance.equationSystem.stall()
    for i in range (instance.equationSystem.n_variables()):
        instance.equationSystem.parse(varnames[i+1]+'='+str(vardata[offset+i+1])+'\n')
    for i in range (instance.equationSystem.n_parameters()):
        instance.equationSystem.parse(constnames[i]+'='+str(constdata[i])+'\n')
    instance.equationSystem.flush()

def move_cursor(objlist,instance):
    if not objlist:
        instance.glWindow.stall()
        for cursor in instance.cursorList:
            delete(cursor,instance)
        instance.cursorList=[]
        add_cursor(instance)
        instance.glWindow.flush()
        return
        
    timeStamp=objlist[0]['__timestamp__']
    if timeStamp!=instance.timeStamp:
        instance.console.write("structure of equation system has changed since creation of")
        id=objlist[0]['__id__'].partition('[')[0]
        instance.console.write_data(id)
        instance.console.write(": cannot move cursor there\n")
        return
    instance.glWindow.stall()
    for cursor in instance.cursorList:
        delete(cursor,instance)
    instance.cursorList=[]
    index=0
    varying=objlist[0]['__varying__']
    const=objlist[0]['__const__']
    for obj in objlist:
        offset=(obj['__varwave__'].rows()-1)*obj['__varwave__'].columns()
        vardata=obj['__varwave__'].data()
        constdata=obj['__constwave__'].data()
        varValues=[]
        constValues=[]
        for i in range(obj['__varwave__'].columns()):
            varValues.append(vardata[offset+i])
        for i in range(obj['__constwave__'].columns()):
            constValues.append(constdata[i])
        instance.cursorList.append(newobj('',index,1,varying,const,varValues,constValues,instance))
        instance.cursorList[index]['__graph__']=Curve(instance.glWindow,
                                                      instance.cursorList[index]['__id__'],1,
                                                      instance.cursorList[index]['__varwave__'],
                                                      instance.cursorList[index]['__constwave__'],
                                                      NONE,RCROSS,0.0,50 if largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0,
                                                      lambda identifier:instance.select_callback(identifier))
#        show(instance.cursorList[index],instance) 
        index=index+1
    move_to(objlist[0],instance)
    instance.rebuild_panels()
    instance.glWindow.flush()

def add_cursor(instance):
    varying=['t']+instance.equationSystem.variable_names()+instance.equationSystem.function_names()
    const=instance.equationSystem.parameter_names()+instance.equationSystem.constant_names()
    varValues=[instance.equationSystem.time()]+instance.equationSystem.variable_values()+instance.equationSystem.function_values()
    constValues=instance.equationSystem.parameter_values()+instance.equationSystem.constant_values()
    newCursor=newobj('',0,1,varying,const,varValues,constValues,instance)
    newCursor['__graph__']=Curve(instance.glWindow,
                                 newCursor['__id__'],1,newCursor['__varwave__'],newCursor['__constwave__'],
                                 NONE,RCROSS,0.0,50 if largeFontsFlag else 25,[1.0,1.0,1.0,1.0],0.0,
                                 lambda identifier:instance.select_callback(identifier))
    instance.cursorList.append(newCursor)
    for obj in instance.cursorList:
        show(obj,instance)

def select(objlist, instance):
    instance.selectedObjectList=objlist
    move_cursor(objlist, instance)

def set_view(obj, plotVariables):
    indices=[]
    varying=obj['__varying__']
    const=obj['__const__']
    for var in plotVariables:
        try: 
            index=varying.index(var)
            indices.append(index+1)
        except:
            try:
                index=const.index(var)
                indices.append(-index-1)
            except:
                indices.append(0)
    
    obj['__graph__'].set_view(indices)

def show(obj, instance):
    if not obj['__visible__']:
        obj['__visible__']=True
        set_view(obj,instance.plotVariables)
        instance.glWindow.add_drawable(obj['__graph__'])

def hide(obj, instance):
    if obj['__visible__']:
        obj['__visible__']=False
        instance.glWindow.remove_drawable(obj['__graph__'])

def delete(obj, instance):
    if obj['__visible__']:
        instance.glWindow.remove_drawable(obj['__graph__'])
    if '__graph__' in obj:
        obj['__graph__'].destroy()
    obj['__varwave__'].destroy()
    obj['__constwave__'].destroy()
    if '__evwave__' in obj:
        obj['__evwave__'].destroy()
    if '__thread__' in obj:
        if obj['__thread__']:
            lib.scigma_num_destroy_thread(obj['__thread__'])
            obj['__thread__']=None

def min_max(obj):
    obj['__min__']={}
    obj['__max__']={}
    mi=obj['__min__']
    ma=obj['__max__']
    vw = obj['__varwave__']
    d=vw.data()
    minima=[1e300]*vw.columns()
    maxima=[-1e300]*vw.columns()
    columns=vw.columns()
    rows=vw.rows()
    try:
        xrange
    except NameError:
        xrange=range
    for i in xrange(rows):
        for j in xrange(columns):
            value=d[i*columns+j]
            if value<minima[j]:
                minima[j]=value
            if value>maxima[j]:
                maxima[j]=value
    varying=obj['__varying__']
    for i in xrange(columns):
        mi[varying[i]]=minima[i]
        ma[varying[i]]=maxima[i]
    cw=obj['__constwave__']
    const=obj['__const__']
    for i in xrange(cw.columns()):
        mi[const[i]]=cw.data()[i]
        ma[const[i]]=cw.data()[i]

def fail(obj, instance):
    # the next two lines are needed to stop drawing if
    # delay is set and the computation ended prematurely
    obj['__graph__'].set_n_points(0)
    instance.glWindow.request_redraw()
    
    delete(obj,instance)
    instance.pendingTasks=instance.pendingTasks-1
    
    #if this is the last running task, select the list this object belongs to,
    #but only if it is not empty; if it is empty, delete the list from instance.objects
    if instance.pendingTasks==0:
        id=obj['__id__'].partition('[')[0]
        objlist=get(id,instance)
        index=obj['__no__']-objlist[0]['__no__']
        del objlist[index]
        if objlist ==[]:
            del instance.objects[id]
        else:
            select(objlist,instance)

def success(obj, instance):
    instance.glWindow.stall()
    
    # get minima and maxima
    min_max(obj)
    
    # display some additional information for fixed points and periodic points    
    if '__evwave__' in obj:
        evdata=obj['__evwave__'].data()
        nVar=obj['__nVar__']
        evreal=[]
        evimag=[]
        evecs=[]
        for i in range(nVar):
            evreal.append(evdata[i])
            evimag.append(evdata[i+nVar])
            evecs.append([])
            for j in range(nVar):
                evecs[i].append(evdata[2*nVar+i*nVar+j])
        
        instance.console.write("found steady state:\n")
        vardata=obj['__varwave__'].data()
        names=obj['__varying__']
        for i in range(nVar):
            instance.console.write(names[i+1]+' = ')
            instance.console.write_data(str(float("{0:.10f}".format(vardata[i+1])))+'\n')
        
        evtypes=[]
        instance.console.write("stability: ")
        info=''
        if obj['__mode__']=='ode':
            if evreal[-1]<=0:
                obj['__stable__']=True
                obj['__graph__'].set_point_style(RDOT)
            else:
                obj['__stable__']=False
                obj['__graph__'].set_point_style(RING)
            for i in range(N_EIGEN_TYPES):
                evtypes.append(int(evdata[nVar*(nVar+2)+i]))
                if evtypes[i]>0:
                    info+=(str(evtypes[i])+EIGEN_TYPE[i]+' / ')
            instance.console.write_data(info.strip(' / ')+'\n')
        else:
            if evreal[-1]*evreal[-1]+evimag[-1]*evimag[-1]<=1:
                obj['__stable__']=True
                obj['__graph__'].set_point_style(QDOT)
            else:
                obj['__stable__']=False
                obj['__graph__'].set_point_style(QUAD)
            for i in range(N_FLOQUET_TYPES):
                evtypes.append(int(evdata[nVar*(nVar+2)+i]))
                if evtypes[i]>0:
                    info+=(str(evtypes[i])+FLOQUET_TYPE[i]+' / ')
            instance.console.write_data(info.strip(' / ')+'\n')
        
        obj['__evwave__'].destroy()
        del obj['__evwave__']
        del obj['__nVar__']
        obj['__evreal__']=evreal
        obj['__evimag__']=evimag
        obj['__evecs__']=evecs
        obj['__evtypes__']=evtypes
        obj['__graph__'].set_point_size(instance.options['Style']['marker']['size'].value)   
    
    # if we plotted a stroboscopic map/ Poincare map or guessed steady state of one of those, store the return time
    if obj['__mode__']=='strobe' or obj['__mode__']=='Poincare':
        varwave=obj['__varwave__']
        if varwave.rows()==1:
            obj['__rtime__']=varwave.data()[0]
        else:
            obj['__rtime__']=varwave.data()[(varwave.rows()-1)*varwave.columns()]-varwave.data()[(varwave.rows()-2)*varwave.columns()]
    
    # the next line is needed to stop drawing if delay is set and the computation ended prematurely
    obj['__graph__'].set_n_points(obj['__varwave__'].rows())
    
    instance.glWindow.flush()
    
    lib.scigma_num_destroy_thread(obj['__thread__'])
    obj['__thread__']=None
    instance.pendingTasks=instance.pendingTasks-1
    
    #if this is the last running task, select the list this object belongs to
    if instance.pendingTasks==0:
        id=obj['__id__'].partition('[')[0]
        objlist=get(id,instance)
        select(objlist,instance)

