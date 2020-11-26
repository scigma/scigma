import os, shutil
from ctypes import *
from . import gui
from . import dat
from . import common
from . import lib
from . import windowlist
from . import graphs
from . import view
from . import picking
from . import equations

AUTO_TYPES={1:'bp', 2:'lp',3:'hb', 0:'fp',10:'fp'}
POINT_STYLES={1:gui.STAR,2:gui.PLUS,3:gui.CROSS,0:gui.RDOT,10:gui.RING}

commands={}

def prepare_continuation(g,parameters,blob,win):
    if win.eqsys.timestamp()!=g['timestamp']:
        raise Exception("structure of equation system has changed: cannot use "+g['identifier']+"for continuation")

    typ = 'fp' if 'autotype' not in g else g['autotype']
    lab = 0 if 'autolabel' not in g else g['autolabel']

    blob.set("IRS",lab)

    if typ == 'fp':
        if len(parameters) != 1:
            raise Exception("need exactly one parameter for steady state continuation")
        blob.set("advanced AUTO.ISW",1)
        blob.set("advanced AUTO.ILP",1)
        
    elif typ in ['bp','lp','hb']:
        if len(parameters) != 2:
            raise Exception("need exactly two parameters for " + "Hopf" if typ =='hb' else "branch point" + " continuation")
        blob.set("advanced AUTO.ISW",2)
        blob.set("advanced AUTO.ILP",0)

def prepare_auto_folder(oldPath, path):
    # guard against removing '/' or "../../usr/lib" and so on by some freak accident
    oldPath=oldPath.replace('/','')
    oldPath=oldPath.replace('\\','')
    path=path.replace('/','')
    path=path.replace('\\','')

    shutil.rmtree("./."+path,True)
    os.makedirs("."+path)
    
    if oldPath:
        shutil.copyfile("./."+oldPath+"/b."+oldPath,"./."+path+"/b."+path)
        shutil.copyfile("./."+oldPath+"/c."+oldPath,"./."+path+"/c."+path)
        shutil.copyfile("./."+oldPath+"/d."+oldPath,"./."+path+"/d."+path)
        shutil.copyfile("./."+oldPath+"/s."+oldPath,"./."+path+"/s."+path)

    
lib.scigma_num_auto.restype=c_int
lib.scigma_num_auto.argtypes=[c_char_p,c_int,c_int,c_int,POINTER(c_int),c_int,c_int,c_bool]

def cont(nSteps=1, parameters=None, g=None, path=None,win=None,noThread=False):
    win = windowlist.fetch(win)

    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read nSteps (usage: plot [nSteps] [name])")
    
    nVars=win.eqsys.n_vars()
    if nVars == 0:
        raise Exception ("error: no variables defined")

    if type(parameters)!= list:
        parameters = [parameters]

    for p in parameters:
        if p not in win.eqsys.par_names():
            raise Exception("error: "+p+" is not a parameter")

    blob=common.Blob(win.options['AUTO'])
    oldPath=None

    if g:
        prepare_continuation(g,parameters,blob,win)
        oldPath=g['identifier'].rpartition('.')[0]
    elif win.selection:
        prepare_continuation(win.selection,parameters,blob,win)
        oldPath=win.selection['identifier'].rpartition('.')[0]

    if not path:
        path=graphs.gen_ID("cont",win)
    prepare_auto_folder(oldPath,path)

    cont={'identifier': path, 'npoints':nSteps,
          'points':{'fp':0,'bp':0,'lp':0,'hb':0},
          'branches':{'fp':0,'bp':0,'lp':0,'hb':0},
          'timestamp':win.eqsys.timestamp()}
    cont['varying']=win.eqsys.var_names()+parameters
    cont['const']=win.eqsys.par_names()
    for p in parameters:
        cont['const'].remove(p)
    cont['callbacks']={'success':lambda args:success(cont,win,args),
                       'minmax': lambda :minmax(cont),
                       'fail':None}

    common.dict_enter(path,win.graphs,cont)

    identifier=create_string_buffer(bytes(path.encode("ascii")))
    C_IntArrayType=c_int*len(parameters)
    indices=[win.eqsys.par_names().index(p) for p in parameters]
    indexArray=C_IntArrayType(*indices)

    cwd = os.getcwd()
    os.chdir("."+path)
    taskID=lib.scigma_num_auto(identifier,win.eqsys.objectID,win.log.objectID,
                               nSteps,indexArray,len(parameters),blob.objectID,True)
    cont['taskID']=taskID
    os.chdir(cwd)

commands['c']=commands['co']=commands['con']=commands['cont']=cont

def success(cont,win,args):
    if len(args)==0:
        finalize_branch(cont)
    elif len(args)==1:
        lib.scigma_num_finish_task(cont['taskID'])   
    else:
        varWave=dat.Wave(objectID=int(args[0]))
        constWave=dat.Wave(objectID=int(args[1]))
        stab=int(args[2])
        typ=int(args[3])
        lab=int(args[4])

        if typ==0:
            add_branch(cont,varWave,constWave,stab,typ//10,win)
        else:
            add_point(cont,varWave,constWave,stab,typ//10 if typ%10==9 else typ,lab,win)

def minmax(cont):
    glist=[]
    common.dict_leaves(cont,glist,isleaf=lambda entry:isinstance(entry,dict) and 'cgraph' in entry)
    for g in glist:
        if 'min' not in g:
            graphs.stdminmax(g)
            
def add_branch(cont,varWave,constWave,stab,typ,win):
    # create the visible object        
    if typ == 0: # 1-parameter continuation
        typ = 0 if stab <0 else 10
        drawingStyle= gui.POINTS if stab <0 else gui.LINES
        pointStyle= gui.DOT
        pointSize = 4
    else: # two parameter continuation of special points
        drawingStyle = gui.DRAWING_TYPE[win.options['Drawing']['style'].label]
        pointStyle = gui.POINT_TYPE[win.options['Drawing']['style'].label]
        pointSize = win.options['Drawing']['point']['size'].value

    color = win.options['Drawing']['color']

    typeName=AUTO_TYPES[typ]
    cont['branches'][typeName]=cont['branches'][typeName]+1
    suffix = str(cont['branches'][typeName]).zfill(2)
    g={'identifier':cont['identifier']+'.'+typeName+'b'+suffix,'visible':False,
       'varying':cont['varying'],'const':cont['const'],'timestamp':cont['timestamp'],
       'varwave':varWave,'constwave':constWave, 'autotype':typeName}
    
    g['callbacks']={'cleanup':None,'cursor':lambda point:graphs.stdcursor(g,point,win)}
        
    g['cgraph']=gui.Bundle(win.glWindow,cont['npoints'],1,len(g['varying']),
                           g['varwave'],g['constwave'],
                           lambda double,button,point,x,y:
                           mouse_callback(g,double,button,point,x,y,win))
    g['cgraph'].set_point_style(pointStyle)
    g['cgraph'].set_point_size(pointSize)
    g['cgraph'].set_style(drawingStyle)
    g['cgraph'].set_color(color)

    common.dict_enter(g['identifier'],win.graphs,g)
    
    graphs.show(g,win)

    cont['lastbranch']=g
    
def finalize_branch(cont):
    cont['lastbranch']['cgraph'].finalize()
    nVarying=len(cont['lastbranch']['varying'])
    nPoints=cont['lastbranch']['varwave'].size()//nVarying
    cont['lastbranch']['npoints']=nPoints
    cont['lastbranch']['nparts']=1
    del cont['lastbranch']

def add_point(cont,varWave,constWave,stab,typ,lab,win):
    if typ==0: # end point
        typ = 0 if stab <0 else 10
        
    pointStyle = POINT_STYLES[typ]
    pointSize=win.options['Drawing']['marker']['size'].value
    color=win.options['Drawing']['color']

    typeName=AUTO_TYPES[typ]
    
    cont['points'][typeName]=cont['points'][typeName]+1
    suffix = str(cont['points'][typeName]).zfill(2)

    g={'identifier':cont['identifier']+'.'+typeName+suffix,'visible':False,
       'npoints':1,'nparts':1,
       'varying':cont['varying'],'const':cont['const'],'timestamp':cont['timestamp'],
       'varwave':varWave,'constwave':constWave, 'autolabel':lab, 'autotype':typeName}

    if typ == 9:
        g['stable']=True
    elif typ == 10:
        g['stable']=False
    
    g['callbacks']={'cleanup':None,'cursor':lambda point:graphs.stdcursor(g,point,win)}

    g['cgraph']=gui.Bundle(win.glWindow,1,1,
                           len(g['varying']),g['varwave'],g['constwave'],
                           lambda double,button,point,x,y:
                           mouse_callback(g,double,button,point,x,y,win))
    g['cgraph'].set_point_style(pointStyle)
    g['cgraph'].set_point_size(pointSize)
    g['cgraph'].set_color(color)
    g['cgraph'].finalize()

    common.dict_enter(g['identifier'],win.graphs,g)
    
    graphs.show(g,win)

def mouse_callback(g,double,button,point,x,y,win):
    identifier = g['identifier']
    if(double):
        picking.select(identifier,point,win)
    elif button==1 and gui.tk:
        x=gui.tkroot.winfo_pointerx()#-gui.tkroot.winfo_rootx()
        y=gui.tkroot.winfo_pointery()#-gui.tkroot.winfo_rooty()
        menu=gui.tk.Menu(gui.tkroot, tearoff=0)
        menu.add_command(label='fit', command=lambda:view.fit(identifier,win))
        menu.add_command(label='fit all',
                         command=lambda:view.fit(identifier.rpartition('.')[0],win))
        menu.add_command(label='delete', command=lambda:graphs.delete(identifier,win))
        menu.add_command(label='delete all',
                         command=lambda:graphs.delete(identifier.rpartition('.')[0],win))
        menu.tk_popup(x,y)
    
def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('Continuation', lambda:unplug(win), commands):
        return
    
    # fill option panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('AUTO')
    panel.add('NPR',0)
    panel.add('stepsize.ds', 0.01)
    panel.add('stepsize.dsmin', 1e-6)
    panel.add('stepsize.dsmax', 1e0)
    panel.add('tolerances.epsl',1e-7)
    panel.add('tolerances.epsu',1e-7)
    panel.add('tolerances.epss',1e-6)
    panel.add('bounds.rl0',-1e300)
    panel.add('bounds.rl1',1e300)
    panel.add('bounds.a0',-1e300)
    panel.add('bounds.a1',1e300)
    panel.add('advanced AUTO.NTST',100)
    panel.add('advanced AUTO.NCOL',4)
    panel.add('advanced AUTO.IAD',3)
    panel.add('advanced AUTO.IADS',1)
    panel.add('advanced AUTO.ITMX',8)
    panel.add('advanced AUTO.NWTN',3)
    panel.add('advanced AUTO.ITNW',5)
    panel.add('advanced AUTO.NMX',0)
    panel.add('advanced AUTO.ILP',1)
    panel.add('advanced AUTO.ISP',2)
    panel.add('advanced AUTO.ISW',1)
    panel.add('advanced AUTO.MXBF',1)
    panel.add('advanced AUTO.IPS',1)
    panel.add('advanced AUTO.IIS',3)
    panel.add('advanced AUTO.IID',3)
    panel.add('advanced AUTO.IPLT',0)
    panel.define('advanced AUTO', 'opened=false')
    panel.add('HOMCONT.NUNSTAB',-1)
    panel.add('HOMCONT.NSTAB',-1)
    panel.add('HOMCONT.IEQUIB',1)
    panel.add('HOMCONT.ITWIST',0)
    panel.add('HOMCONT.ISTART',5)
    panel.define('HOMCONT', 'opened=false')

    win.glWindow.flush()
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('Continuation'):
        return
    
    # remove options from panels
    win.glWindow.stall()
    panel=win.acquire_option_panel('AUTO')
    panel.remove('NPR')
    panel.remove('stepsize.ds')
    panel.remove('stepsize.dsmin')
    panel.remove('stepsize.dsmax')
    panel.remove('tolerances.epsl')
    panel.remove('tolerances.epsu')
    panel.remove('tolerances.epss')
    panel.remove('bounds.rl0')
    panel.remove('bounds.rl1')
    panel.remove('bounds.a0')
    panel.remove('bounds.a1')
    panel.remove('advanced AUTO.NTST')
    panel.remove('advanced AUTO.NCOL')
    panel.remove('advanced AUTO.IAD')
    panel.remove('advanced AUTO.IADS')
    panel.remove('advanced AUTO.ITMX')
    panel.remove('advanced AUTO.NWTN')
    panel.remove('advanced AUTO.ITNW')
    panel.remove('advanced AUTO.NMX')
    panel.remove('advanced AUTO.ILP')
    panel.remove('advanced AUTO.ISP')
    panel.remove('advanced AUTO.ISW')
    panel.remove('advanced AUTO.MXBF')
    panel.remove('advanced AUTO.IPS')
    panel.remove('advanced AUTO.IIS')
    panel.remove('advanced AUTO.IID')
    panel.remove('advanced AUTO.IPLT')
    panel.remove('HOMCONT.NUNSTAB')
    panel.remove('HOMCONT.NSTAB')
    panel.remove('HOMCONT.IEQUIB')
    panel.remove('HOMCONT.ITWIST')
    panel.remove('HOMCONT.ISTART')

    win.release_option_panel('AUTO')
    win.glWindow.flush()
