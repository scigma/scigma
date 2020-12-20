import re
from ctypes import *
from time import time
from . import gui
from . import library
from . import dat
from . import common
from . import equations
from . import windowlist
from . import lib

commands={}
independentVariables={}
expBuffer={}
indBuffer={}
timeStamp={}

def show(g=None, win=None):
    win=windowlist.fetch(win)
    g=get(g,win)
    if not g['visible']:
        g['visible']=True
        set_view(g,expBuffer[win], indBuffer[win],timeStamp[win],independentVariables[win])
        win.glWindow.add_drawable(g['cgraph'])        

commands['show']=show
        
def hide(g=None, win=None):
    win=windowlist.fetch(win)
    g=get(g,win)
    if g['visible']:
        g['visible']=False
        win.glWindow.remove_drawable(g['cgraph'])

commands['hide']=hide

def replay(g=None, win=None):
    win=windowlist.fetch(win)
    g=get(g,win)
    try:
        win.glWindow.stall()
        show(g,win)
        g['cgraph'].set_delay(win.options['Drawing']['delay'].value)
    finally:
        win.glWindow.flush()
    g['cgraph'].replay()

commands['rep']=commands['repl']=commands['repla']=commands['replay']=replay

def delete(identifier=None, win=None):
    win=windowlist.fetch(win)

    if not identifier:
        g = get(None,win)
        identifier = g['identifier']

    path=None
    # this deletes named graph, or a node with all graphs below that node
    path=common.dict_single_path(identifier,win.graphs,'graph',lambda entry: isinstance(entry,dict) and 'cgraph' not in entry)
    entry=common.dict_entry(path,win.graphs)
    glist=[]
    common.dict_leaves(entry,glist,lambda entry: isinstance(entry,dict) and 'cgraph' not in entry,
                       lambda entry: isinstance(entry,dict) and 'cgraph' in entry)

    for g in glist:
        if win.selection is g:
            win.selection is None
            show(win.cursor,win)
        destroy(g,win)
    parts=path.rpartition('.')
    parent=common.dict_entry(parts[0],win.graphs)
    child=parts[2]
    del parent[child]

commands['del']=commands['dele']=commands['delet']=commands['delete']=delete

def clear(win=None):
    win=windowlist.fetch(win)
    keys = win.graphs.keys()
    for key in keys:
        delete(key,win)
    win.selection=None
    win.graphIDs = {}
        
commands['cl']=commands['cle']=commands['clea']=commands['clear']=clear


def save(g=None,filename='',win=None):
    win=windowlist.fetch(win)
    g=get(g,win)
    identifier=g['identifier']
    varying=g['varying']
    const=g['const']
    varWave=g['varwave']
    constWave=g['constwave']
    nPoints=g['npoints']
    nParts=g['nparts']
    nVaryings=len(varying)
    nConsts=len(const)

    if filename=='':
        filename=identifier.replace('.','/')+".dat"
    
    with open(filename,"w") as f:
        # print header with trajectory information and constants
        f.write('#'+g['identifier']+': '+str(nParts)+'x'+str(nPoints) +' points\n')
        if nConsts:
            constWave.lock()
            constData=constWave.data()
            f.write('#constant values\n#')
            for i in range(nConsts-1):
                f.write(const[i]+"\t")
            f.write(const[-1]+"\n#")
            for i in range(nConsts-1):
                f.write(str(constData[i])+'\t')
            f.write(str(constData[nConsts-1])+"\n")
            constWave.unlock()
        # print time and variables
        if nVaryings:
            varWave.lock()
            varData=varWave.data()
            f.write('#variable values\n')
            for i in range(nVaryings*nParts-1):
                f.write(varying[i%nVaryings]+"\t")
            f.write(varying[-1]+"\n")
            for i in range(nPoints):
                for j in range(nVaryings*nParts-1):
                    f.write(str(varData[i*nVaryings*nParts+j])+'\t')
                f.write(str(varData[(i+1)*nVaryings*nParts-1])+'\n')
            varWave.unlock()

commands['sav']=commands['save']=save
            
def get(g, win=None, isnode=lambda entry: isinstance(entry,dict) and 'cgraph' not in entry):
    win=windowlist.fetch(win)
    if isinstance(g,str):
        return common.dict_single_entry(g,win.graphs,'graph',isnode)
    elif not g:
        if win.selection:
            return win.selection
        else:
            raise Exception("no object selected to perform this operation")
    return g

def destroy(g, win):
    if g['cgraph']:
        if g['visible']:
            win.glWindow.remove_drawable(g['cgraph'])
        g['cgraph'].destroy()

    g['varwave'].destroy()
    g['constwave'].destroy()

    if g['callbacks']['cleanup']:
        g['callbacks']['cleanup']()

def gen_ID(prefix,win):
    """generates a unique name with given prefix
    
    This generates a name of the form 'tr_01' (if prefix is 'tr').
    The counter is increased for each prefix, to ensure that tje 
    names are unique 
    """
    if prefix not in win.graphIDs:
        win.graphIDs[prefix]=1
    count = win.graphIDs[prefix]
    ID = prefix + str(count).zfill(2)
    win.graphIDs[prefix]=count+1
    return ID

def new(win,nPoints,nParts,varying,const,varVals,constVals,path=None):
    """ adds a new graph to the window 
    """    

    callbacks={'cleanup':None,'success':None,'fail':None,'minmax':None,'select':None}
        
    g={'varying':varying,'const':const,'visible':False,
       'npoints':nPoints,'nparts':nParts,'callbacks':callbacks,
       'timestamp': win.eqsys.timestamp()}

    if path:
        common.dict_enter(path,win.graphs,g)
        g['identifier']=path
          
    # only initizialize Waves here, in case
    # something goes wrong with the new dict entry
    g['varwave']=dat.Wave(capacity=len(varying)*nPoints*nParts)
    g['constwave']=dat.Wave(capacity=len(constVals))
    g['varwave'].push_back(varVals)
    g['constwave'].push_back(constVals)
    return g

def move_cursor(win,varying=None,const=None,varVals=None,constVals=None,nParts=None):

    # use current point if no data given
    if varying is None and const is None and varVals is None and constVals is None:
        const, constVals = equations.point(win)
        varying=[]
        varVals=[]

    if win.cursor:
        destroy(win.cursor,win)

    if not nParts:
        nParts=1 if len(varying)== 0 else len(varVals)//len(varying)
    
    win.cursor = new(win,1,nParts,varying,const,varVals,constVals)
    win.cursor['cgraph']=gui.Bundle(win.glWindow,1,win.cursor['nparts'],len(win.cursor['varying']),
                                        win.cursor['varwave'],win.cursor['constwave'])
    win.cursor['cgraph'].set_point_style(gui.RCROSS)
    win.cursor['cgraph'].set_point_size(50 if library.largeFontsFlag else 25)
    win.cursor['cgraph'].set_color([1.0,1.0,1.0,1.0])
    win.cursor['cgraph'].finalize()
    #gui.application.idle(0.05)
    show(win.cursor,win)
    
GLSL_BUILT_IN_FUNCTIONS=["sin","cos","tan","asin","acos","atan","abs",
                           "mod","sign","step","pow","exp","log","sqrt"]

def is_variable(x):
    if x[0]>'0'and x[0]<='9':
        return False
    if x in GLSL_BUILT_IN_FUNCTIONS:
        return False
    return True

def coordinate_expressions(win):
    expressions=[]
    for i in range(gui.N_COORDINATES):
        expressions.append(win.options['View'][gui.COORDINATE_NAME[i]])

    # if an expression is empty, assume that it's value is zero, except for
    # colors (where an empty expression string means that no color map is used)
    axes=win.options['View']['axes'].label
    for i in range(gui.N_COORDINATES):
        if gui.VIEW_TYPE[axes]&gui.COORDINATE_FLAG[gui.COORDINATE_NAME[i]]:
            if expressions[i]=='':
                expressions[i]='0.0'
        else:
            if i==gui.C_INDEX:
                expressions[i]=''
            else:
                expressions[i]='0.0'
    return expressions

def independent_variables(expressions):
    skip = (expressions[gui.C_INDEX]=='')
    variables=[]
    for j in range(gui.N_COORDINATES):
        if j==gui.C_INDEX and skip:
            continue
        exprlist = re.sub('[^0-9_.a-zA-Z]+', ' ', expressions[j]).split()   
        for expr in exprlist:
            try:
                x=float(expr)
            except:
                if is_variable(expr):
                    variables.append(expr)
    return sorted(list(set(variables)))

def set_view(g, expBuffer,indBuffer,timeStamp,independentVariables):
    indices=[]
    varying=g['varying']
    const=g['const']

    for var in independentVariables:
        try: 
            index=varying.index(var)
            indices.append(index+1)
        except:
            try:
                index=const.index(var)
                indices.append(-index-1)
            except:
                indices.append(0)

    g['cgraph'].set_view(indices,expBuffer,indBuffer,timeStamp)

def on_view_change(win):
    expressions=coordinate_expressions(win)
    independentVariables[win]=independent_variables(expressions)
    
    arg1=''
    for x in expressions:
        arg1+='|'+x
    arg1=bytes(arg1.strip('|').encode("ascii"))
    
    arg2=''
    for x in independentVariables[win]:
        arg2+='|'+x
    arg2=bytes(arg2.strip('|').encode("ascii"))
    
    glist=[]

    common.dict_leaves(win.graphs,glist,lambda entry: isinstance(entry,dict) and 'cgraph' not in entry,
                       lambda entry: isinstance(entry,dict) and 'cgraph' in entry)
    
    timeStamp[win]=time()
    expBuffer[win]=create_string_buffer(arg1)
    indBuffer[win]=create_string_buffer(arg2)

    for g in glist:
        set_view(g,expBuffer[win],indBuffer[win],timeStamp[win],independentVariables[win])

    if win.has_plugin('picking'):
        set_view(win.cursor,expBuffer[win],indBuffer[win],timeStamp[win],independentVariables[win])
        
    win.glWindow.request_redraw()

def fail(identifier,args,win):
    g=get(identifier,win, lambda entry: isinstance(entry,dict) and 'callbacks' not in entry)
    # the next two lines are needed to stop drawing if
    # delay is set and the computation ended prematurely
    # g['cgraph'].set_n_points(0)
    # win.glWindow.request_redraw()

    if g['callbacks']['fail']:
        g['callbacks']['fail'](args)

    delete(identifier,win)
    
def success(identifier,args,win):
    g=get(identifier,win, lambda entry: isinstance(entry,dict) and 'cgraph' not in entry)

    # get minima and maxima
    if g['callbacks']['minmax']:
        g['callbacks']['minmax']()

    # perform any other initialization tasks
    if g['callbacks']['success']:
        g['callbacks']['success'](args)

def stdcursor(g,point,win):
    nVarying=len(g['varying'])

    if point == -1:
        varVals=g['varwave'][-nVarying:]
    else:
        varVals=g['varwave'][nVarying*point:nVarying*(point+1)]
    constVals=g['constwave'][:]
   
    return g['varying'], g['const'], varVals, constVals

def stdminmax(g):
    g['min']={}
    g['max']={}
    mi=g['min']
    ma=g['max']

    varying=g['varying']
    varWave=g['varwave']
    rows=varWave.size()//len(varying)
    columns=len(varying)
    minima=[1e300]*columns
    maxima=[-1e300]*columns

    for i in range(rows):
        varWave.lock()
        d=varWave.data()
        for j in range(columns):
            value=d[i*columns+j]
            if value<minima[j]:
                minima[j]=value
            if value>maxima[j]:
                maxima[j]=value
        varWave.unlock()
    for i in range(columns):
        mi[varying[i]]=minima[i]
        ma[varying[i]]=maxima[i]

    constWave=g['constwave']
    const=g['const']
    for i in range(constWave.size()):
        mi[const[i]]=constWave[i]
        ma[const[i]]=constWave[i]

        
def plug(win):
    if not win.register_plugin('graphs', lambda:unplug(win),commands):
        return
    setattr(win,'graphs',{})
    setattr(win,'graphIDs',{})

    independentVariables[win]=None
    expBuffer[win]=None
    indBuffer[win]=None
    timeStamp[win]=None
    
    #initial build of GL shaders occurs here
    on_view_change(win)
    
def unplug(win):
    if not win.unregister_plugin('graphs'):
        return
    delattr(win,'graphs')
    delattr(win,'graphIDs')

    del independentVariables[win]

    
