from ctypes import *
from . import gui
from . import common
from . import lib
from . import library
from . import windowlist
from . import graphs
from . import picking
from . import equations
from . import iteration

commands={}

lib.scigma_num_sweep.restype=c_int
lib.scigma_num_sweep.argtypes=[c_char_p,c_int,c_int,c_int,c_int,c_int,c_bool]

def sweep(nSteps,g=None,path=None,win=None,mesh=None):
    win = windowlist.fetch(win)

    try:
        nSteps = int(nSteps)
    except ValueError:
        raise Exception("error: could not read nSteps (usage: sweep <nSteps> [name])")

    if not path:
        path=graphs.gen_ID("mf",win)
    
    if not mesh:
        g=graphs.get(g,win)
        try:
            mesh=g['mesh'].extend()
        except:
            raise Exception(g['identifier']+': has no mesh, cannot sweep from here')

    g=graphs.new(win,1,1,g['varying'],g['const'],[],g['constwave'][:],path)
        
    g['mesh']=mesh
    g['mode']='ode'
    g['callbacks']= {'success':None,
                     'fail':None,
                     'cleanup':None,
                     'minmax':None,
                     'cursor':None}
    g['cgraph'] = gui.Sheet(win.glWindow,mesh,len(g['varying']),g['constwave'])
        
    blob = iteration.blob(win)

    eqsysID=win.eqsys.objectID
    meshID=mesh.objectID
    logID=win.log.objectID
    blobID=blob.objectID
    
    identifier=create_string_buffer(bytes(path.encode("ascii")))
    noThread = (win.options['Global']['threads'].label =='off')
    
    lib.scigma_num_sweep(identifier,eqsysID,logID,nSteps,
                         meshID,blobID,noThread)

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

commands['sw']=commands['swe']=commands['swee']=commands['sweep']=sweep
    
def plug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not load twice into the same window
    if not win.register_plugin('sweeping', lambda:unplug(win), commands):
        return
    
def unplug(win=None):
    win = windowlist.fetch(win)
    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('sweeping'):
        return
    
