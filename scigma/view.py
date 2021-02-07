from . import gui
from . import common
from . import library
from . import options
from . import windowlist
from . import lib
from . import graphs
from . import options
from . import picking

commands={}

# wrappers for set_range;
# unfortunately, they have to use the underscore in the function 
# name, because xrange is a builtin function of Python
def x_range(low,high,win=None):
    """ xrange <low> <high>

    Adjusts the boundaries of the x coordinate.
    (equivalent for yrange, etc.)
    If low is larger than high, they are swapped.
    low and high must be different
    """
    set_range('X',low,high,win)
def y_range(low,high,win=None):
    set_range('Y',low,high,win)
def z_range(low,high,win=None):
    set_range('Z',low,high,win)
def c_range(low,high,win=None):
    set_range('C',low,high,win)
def t_range(low,high,win=None):
    set_range('T',low,high,win)

commands['xr']=commands['xra']=commands['xran']=commands['xrang']=commands['xrange']=x_range
commands['yr']=commands['yra']=commands['yran']=commands['yrang']=commands['yrange']=y_range
commands['zr']=commands['zra']=commands['zran']=commands['zrang']=commands['zrange']=z_range
commands['cr']=commands['cra']=commands['cran']=commands['crang']=commands['crange']=c_range
commands['tr']=commands['tra']=commands['tran']=commands['trang']=commands['trange']=t_range

def axes(axes,win=None):
    """ axes <axes>

    Changes how phase space is displayed (switches between
    2D and 3D, disables/enables color map). 
    The options for the axes argument are
    'xy','xyc','xyt','xytc'
    'xyz','xyzc','xyzt','xyztc'
    """
    win = windowlist.fetch(win)

    try:
        win.glWindow.stall()
        oldaxes=win.options['View']['axes'].label
        axes=axes.lower()
        try:
            for i in range(gui.N_COORDINATES):
                if gui.VIEW_TYPE[axes]&gui.COORDINATE_FLAG[gui.COORDINATE_NAME[i]]:
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i],'visible=true')
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i]+'range','visible=true')
                else:
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i],'visible=false')
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i]+'range','visible=false')
            options.set("View.axes",axes,win)
        except:
            raise Exception(axes+": unknown axes specification")
        try:
            graphs.on_view_change(win)
        except Exception as e:
            for i in range(gui.N_COORDINATES):
                if gui.VIEW_TYPE[oldaxes]&gui.COORDINATE_FLAG[gui.COORDINATE_NAME[i]]:
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i],'visible=true')
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i]+'range','visible=true')
                else:
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i],'visible=false')
                    win.optionPanels['View'].define(gui.COORDINATE_NAME[i]+'range','visible=false')
            options.set("View.axes",oldaxes,win)
            graphs.on_view_change(win)
            raise e
        if 'z' in axes:
            win.glWindow.set_viewing_angle(20)
        else: 
            win.glWindow.set_viewing_angle(0)
            win.glWindow.reset_rotation()
        win.cosy.set_view(gui.VIEW_TYPE[axes])
        win.glWindow.disconnect(win.navigator)
        win.navigator.destroy()
        win.navigator=gui.Navigator(gui.VIEW_TYPE[axes])
        win.glWindow.connect(win.navigator)
        getattr(win,'picker')
        win.glWindow.disconnect(win.picker)
        win.picker.destroy()
        win.picker=gui.Picker(gui.VIEW_TYPE[axes])
        win.glWindow.connect_before(win.picker)
        win.picker.set_callback(lambda ctrl,x,y,z: picking.on_pick(win,ctrl,x,y,z))
    finally:
        win.glWindow.flush()

commands['a']=commands['ax']=commands['axe']=commands['axes']=axes

def twoD(win=None):
    win = windowlist.fetch(win)
    oldview=win.options['View']['axes'].label
    parts=oldview.partition('z')
    newview=parts[0]+parts[2]
    axes(newview,win)

def threeD(win=None):
    win = windowlist.fetch(win)
    oldview=win.options['View']['axes'].label
    if oldview[:3]!='xyz':
        newview='xyz'+oldview[2:]
        axes(newview,win)

commands['2d']=commands['2D']=twoD
commands['3d']=commands['3D']=threeD

def fit(identifier=None,win=None):
    """ fit      
                                                                                            
    Adjusts the axes that are currently shown such that                                                      
    every object currently drawn fits into the viewing window.                                               
    """
    win=windowlist.fetch(win)
    axes=win.options['View']['axes'].label
    visvar={}
    for i in range(gui.N_COORDINATES):
        if gui.VIEW_TYPE[axes]&gui.COORDINATE_FLAG[gui.COORDINATE_NAME[i]]:
            exp=win.options['View'][gui.COORDINATE_NAME[i]]
            if not (exp=='t' or exp in win.eqsys.var_names()
                    or exp in win.eqsys.par_names()
                    or exp in win.eqsys.func_names()
                    or exp in win.eqsys.const_names()):
                raise Exception("cannot fit while "+gui.COORDINATE_NAME[i]+"-axis shows " + exp)
            visvar[exp]=gui.COORDINATE_NAME[i]

    try:
        win.glWindow.stall()
        for symbol in visvar:
            mi=1e300
            ma=-1e300
            dictionary = win.graphs
            if identifier:
                dictionary={'entry':common.dict_entry(identifier,win.graphs)}
                
            glist=[]
            common.dict_leaves(dictionary,glist,lambda entry: isinstance(entry, dict) and 'cgraph' not in entry,
                               lambda entry:isinstance(entry,dict) and 'cgraph' in entry)
            for g in glist:
                if g['visible']:
                    value = g['min'][symbol] if (('min' in g) and (symbol in g['min'])) else 1e300
                    mi = value if value < mi else mi
                    value = g['max'][symbol] if (('max' in g) and (symbol in g['max'])) else -1e300
                    ma = value if value > ma else ma
            mi=0.0 if mi==1e300 else mi
            ma=0.0 if ma==-1e300 else ma
            if mi==ma:
                mi=mi-1
                ma=ma+1
            set_range(visvar[symbol],mi,ma,win)
    finally:
        win.glWindow.flush()

commands['f']=commands['fi']=commands['fit']=fit
    
def expr(coord,*args,**kwargs):
    """ sets the expression(s) displayed along a coordinate axis
    """
    coord=coord.lower()
    win=windowlist.fetch(None)
    if 'win' in kwargs:
        win = kwargs['win']
    elif len(args) and type(args[-1])==type(win):
        win=args[-1]
        args=args[0:-1]

    if not len(args):
        result = win.options['View'][coord]
        win.console.write_data(result)
        return result
    else:
        # comma-separated lists are passed as one argument,
        # we have to join those first, then join all arguments
        # again to get the complete expression
        noListArgs = [','.join(arg) if isinstance(arg,list) else arg for arg in args]
        expression = ''.join(noListArgs)
        if expression == '':
            win.console.write_data(win.options['View'][coord])
        else:
            oldexp=win.options['View'][coord]=expression
            try:
                options.set('View.'+coord,expression,win)
                win.cosy.set_label(gui.COORDINATE_FLAG[coord],expression)
                graphs.on_view_change(win)
            except Exception as e:
                options.set('View.'+coord,oldexp,win)
                graphs.on_view_change(win)
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
    return expr('x',*args,**kwargs)
def yexpr(*args,**kwargs):
    return expr('y',*args,**kwargs)
def zexpr(*args,**kwargs):
    return expr('z',*args,**kwargs)
def cexpr(*args,**kwargs):
    return expr('c',*args,**kwargs)
#def texpr(*args,**kwargs):
#    expr('t',*args,**kwargs)

commands['x']=commands['xe']=commands['xex']=commands['xexp']=commands['xexpr']=xexpr
commands['y']=commands['ye']=commands['yex']=commands['yexp']=commands['yexpr']=yexpr
commands['z']=commands['ze']=commands['zex']=commands['zexp']=commands['zexpr']=zexpr
commands['c']=commands['ce']=commands['cex']=commands['cexp']=commands['cexpr']=cexpr
#commands['t']=commands['te']=commands['tex']=commands['texp']=commands['texpr']=texpr

def on_entry_change(identifier,value,win):
    if identifier == 'axes':
        axes(value.label,win)
    elif identifier in gui.COORDINATE_NAME:
        expr(identifier,value,win)
        if identifier in [x+'range.min' for x in gui.COORDINATE_NAME]:
            coord=identifier[0]
            min,max = value.value, self.options['View'][coord+'range']['max'].value
            set_range(coord,min,max,win)
        if identifier in [x+'range.max' for x in gui.COORDINATE_NAME]:
            coord=identifier[0]
            min,max = self.options['View'][coord+'range']['min'].value,value.value
            set_range(coord,min,max,win)
    
def set_range(coord,low,high,win):
    """ sets the range of the specified coordinate
    """
    win = windowlist.fetch(win)
    coord = coord.lower()
    low = float(low)
    high = float(high)
    if low>=high:
        raise Exception("lower bound must be smaller than upper bound")
    if low<-3e38 or high > 3e38:
        raise Exception("will not set range, boundaries too large")        
    win.glWindow.set_range(gui.COORDINATE_FLAG[coord],low,high)

def plug(win=None):
    win = windowlist.fetch(win)
    
    # make sure that we do not load twice into the same window
    if not win.register_plugin('view', lambda:unplug(win),commands):
        return

    # only redraw once after load is complete
    try:
        win.glWindow.stall()
        
        # add coordinate system, navigator and picking
        cosy=gui.Cosy(win.glWindow,gui.VIEW_TYPE['xy'],library.largeFontsFlag)
        navigator=gui.Navigator(gui.VIEW_TYPE['xy'])
        setattr(win,'cosy',cosy)
        setattr(win,'navigator',navigator)
        
        win.glWindow.set_viewing_angle(0)
        win.glWindow.connect(navigator)
        
        # add viewing option panel
        panel=win.acquire_option_panel('View')
        panel.define('','iconified=true')
        panel.set_callback(lambda identifier,value: on_entry_change(identifier,value,win))
        enum = common.Enum(gui.VIEW_TYPE,'xy')
        panel.add('axes',enum)
        panel.add('sep1',gui.Separator())
        for coord in gui.COORDINATE_NAME:
            panel.add(coord,coord.lower(),False)
        panel.add('sep2',gui.Separator())
        for coord in gui.COORDINATE_NAME:
            panel.add(coord+'range.min',common.Float(-1.0),False)
            panel.add(coord+'range.max',common.Float(1.0),False)
        panel.add('trange.duration', common.Float(10.0))
        panel.define('trange.duration', 'min=0.0')
        for coord in gui.COORDINATE_NAME[gui.Z_INDEX:]:
            panel.set(coord,'')
            panel.define(coord,'visible=false')
            panel.define(coord+'range','visible=false')
            
        # redraw here
    finally:
        win.glWindow.flush()
        
def unplug(win=None):
    win = windowlist.fetch(win)

    # make sure that we do not unload twice from the same window
    if not win.unregister_plugin('view'):
        return

    # redraw only once after unload is complete
    try:
        win.glWindow.stall()
        
        # remove options from panels
        panel=win.acquire_option_panel('View')
        panel.remove('axes')
        panel.remove('sep1')
        for coord in gui.COORDINATE_NAME:
            panel.remove(coord)
        panel.remove('sep2')
        for coord in gui.COORDINATE_NAME:
            panel.remove(coord+'range.min')
            panel.remove(coord+'range.max')
        panel.remove('trange.duration')
        
        win.release_option_panel('View')
        
        win.glWindow.disconnect(win.navigator)
        win.navigator.destroy()
        delattr(win,'navigator')
        win.cosy.destroy()
        delattr(win,'cosy')
        
        # redraw here
    finally:
        win.glWindow.flush()
