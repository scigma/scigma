from . import num
from . import gui
from . import common
from . import windowlist

commands={}

MAP=0
ODE=1
STROBE=2
POINCARE=3

MODE={'map':MAP,'ode':ODE,'strobe':STROBE, 'Poincare':POINCARE}

def mode(mode=None,win=None,silent=False):
    """ mode <mode>
    Changes scigma's mode of operation.
    Options are '[m]ap', '[o]de', '[s]trobe' and '[P]oincare'.
    """
    win=windowlist.fetch(win)
    if not mode:
        mode=win.equationPanel.get('mode')
        if not silent:
            win.console.write_data(mode+"\n")
        return mode
    abbrev={'m':'map','o':'ode','s':'strobe','p':'Poincare','poincare':'Poincare'}
    if mode.lower() in abbrev:
        mode = abbrev[mode.lower()]
    if mode not in MODE:
        raise Exception(mode+": unknown mode specification")
    win.glWindow.stall()
    win.equationPanel.set('mode',mode)
    if mode == 'map':
        win.equationPanel.define('nperiod','visible=true')
        win.equationPanel.define('inverse','visible=true')
        win.valuePanel.define('time','visible=false')
        win.valuePanel.define('step','visible=true')
    else:
        win.equationPanel.define('inverse','visible=false')
        win.valuePanel.define('time','visible=true')
        win.valuePanel.define('step','visible=false')
    if mode == 'ode':
        win.equationPanel.define('nperiod','visible=false')
    if mode == 'strobe':
        win.equationPanel.define('nperiod','visible=true')
        win.equationPanel.define('period','visible=true')
    else:
        win.equationPanel.define('period','visible=false')
    if mode == 'Poincare':
        win.equationPanel.define('nperiod','visible=true')
        win.equationPanel.define('secvar','visible=true')
        win.equationPanel.define('secval','visible=true')
        win.equationPanel.define('secdir','visible=true')
        win.equationPanel.define('maxtime','visible=true')
    else:
        win.equationPanel.define('secvar','visible=false')
        win.equationPanel.define('secval','visible=false')
        win.equationPanel.define('secdir','visible=false')
        win.equationPanel.define('maxtime','visible=false')
        
    win.glWindow.flush()

commands['m']=commands['mo']=commands['mode']=commands['mode']=mode

def period(period=None,win=None,silent=False):
    """ period <period>
    Changes the period used for stroboscopic maps.
    """
    win=windowlist.fetch(win)
    if not period:
        period=win.equationPanel.get('period')
        if not silent:
            win.console.write_data(period+"\n")
        return period
    win.equationPanel.set('period',period)

commands['period']=period
    
def nperiod(nperiod=None,win=None,silent=False):
    """ nperiod <nperiod>
    Changes the number of periods used for maps.
    """
    win=windowlist.fetch(win)
    if not nperiod:
        nperiod=win.equationPanel.get('nperiod')
        if not silent:
            win.console.write_data(str(nperiod)+"\n")
        return nperiod
    win.equationPanel.set('nperiod',int(nperiod))
    
commands['nperiod']=nperiod

def secvar(secvar=None,win=None,silent=False):
    """ secvar <secvar>
    Changes the variable used for Poincare sections.
    """
    win=windowlist.fetch(win)
    if not secvar:
        secvar=win.equationPanel.get('secvar')
        if not silent:
            win.console.write_data(secvar+"\n")
        return secvar
    win.equationPanel.set('secvar',secvar)
    
commands['secvar']=secvar
    
def secval(secval=None,win=None,silent=False):
    """ secval <secval>
    Changes the value used for Poincare sections.
    """
    win=windowlist.fetch(win)
    if not secval:
        secval=win.equationPanel.get('secval')
        if not silent:
            win.console.write_data(str(secval)+"\n")
        return secval
    win.equationPanel.set('secval',float(secval))

commands['secval']=secval
    
def secdir(secdir=None,win=None,silent=False):
    """ secdir <secdir>
    Changes the direction for Poincare sections.
    """
    win=windowlist.fetch(win)
    if not secdir:
        secdir=win.equationPanel.get('secdir')
        if not silent:
            win.console.write_data(secdir+"\n")
        return secdir
    if secdir not in ['+','-']:
        raise Exception("secdir must either be '+' or '-'")   
    win.equationPanel.set('secdir',secdir)

commands['secdir']=secdir

def maxtime(maxtime=None,win=None,silent=False):
    """ secval <secval>
    Changes the value used for Poincare sections.
    """
    win=windowlist.fetch(win)
    if not maxtime:
        secval=win.equationPanel.get('maxtime')
        if not silent:
            win.console.write_data(str(maxtime)+"\n")
        return secval
    win.equationPanel.set('maxtime',float(maxtime))

commands['maxtime']=maxtime
    
def parse(line,win=None):
    """Used to parse differential equations and maps.

    This function is called automatically if no other command is entered.

    The syntax is as follows:
    * define differential equation/define map
    x' = x*y+a
    * define inverse map:
    x, = sqrt(x-a)
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
    win=windowlist.fetch(win)

    oldts=win.eqsys.timestamp()
    
    # use invsys if it's an inverse equation
    result = win.invsys.parse(line.replace(',=',"'=")) if ',=' in line else win.eqsys.parse(line)
    
    # if we are redefining a function or parameter, do it in the inverse equation system as well
    if not ',=' in line and not "'" in line:
        win.invsys.parse(line)

    if(result[0:6]== "error:"):
        raise Exception(result[6:])
    else:
        ts=win.eqsys.timestamp()
        if not ts == oldts:
            win.console.write_warning('structure of equation system has changed\n')
            
        rebuild_panels(win)

        if(result != ''):
            win.console.write_data(result+'\n')
            return float(result)

        
def point(win=None):
    win=windowlist.fetch(win)
    
    names=['t']+win.eqsys.var_names()+win.eqsys.func_names()+win.eqsys.par_names()+win.eqsys.const_names()
    values=[win.eqsys.time()]+win.eqsys.var_vals()+win.eqsys.func_vals()+win.eqsys.par_vals()+win.eqsys.const_vals()
    
    return names, values

def on_panel_change(identifier,rhs,win):
    if identifier == 'mode':
        mode(rhs.label, win)
    elif identifier == "period":
        period(rhs,win)
    elif identifier == "nperiod":
        nperiod(rhs,win)
    elif identifier == 'secvar':
        secvar(rhs,win)
    elif identifier == 'secval':
        secval(rhs,win)
    elif identifier == 'secdir':
        secdir(rhs.label,win)
    else:
        on_equation_change(identifier,rhs,win)
        
def on_equation_change(identifier,rhs, win):
    line = identifier.rpartition('.')[2]+rhs
    line = line.replace("'",',') if "inverse" in identifier else line
    try:
        parse(line,win)
    except Exception as e:
        win.console.write_error(e.args[0]+'\n')
        raise

def rebuild_panels(win):
    win.glWindow.stall()
        
    win.valuePanel.remove('time.t = ')
    win.valuePanel.remove('step.t = ')
    ids=[]
    for group in win.equationPanel.data:
        if group in ['mode','period','nperiod','secvar','secval','secdir','maxtime']:
            continue
        for entry in win.equationPanel.data[group]:
            if not entry == '__anchor__':
                ids.append(group+'.'+entry)
    for identifier in ids:
        win.equationPanel.remove(identifier)   
    ids=[]
    for group in win.valuePanel.data:
        for entry in win.valuePanel.data[group]:
            if not entry == '__anchor__':
                ids.append(group+'.'+entry)
    for identifier in ids:
        win.valuePanel.remove(identifier)   
        
    win.valuePanel.add('time.t = ',str(win.eqsys.time()))
    win.valuePanel.add('step.t = ',str(win.eqsys.time()))
        
    groups = ['variables','inverse','functions','constants','variables','parameters']
    defs = [win.eqsys.var_defs(),win.invsys.var_defs()]
    panels = [win.equationPanel,win.equationPanel,win.equationPanel]
        
    defs +=[win.eqsys.func_defs(),win.eqsys.const_defs(),win.eqsys.vars(),win.eqsys.pars()]
        
    panels += [win.equationPanel,win.valuePanel,win.valuePanel]
        
    for i in range(len(groups)):
        for definition in defs[i]:
            parts=definition.partition('=')
            var=parts[0].strip()
            rhs=parts[2].strip()
            panels[i].add(groups[i]+'.'+var+' = ',rhs, False)
    win.glWindow.flush()
        
def plug(win):
    if not win.register_plugin('equations', lambda:unplug(win),commands):
        return

    setattr(win,'eqsys',num.EquationSystem())
    setattr(win,'invsys',num.EquationSystem())
    
    setattr(win,'equationPanel',gui.ATWPanel(win.glWindow,'Equations'))

    win.glWindow.stall()

    enum=common.Enum(MODE,'ode')
    win.equationPanel.add('mode',enum)
    win.equationPanel.add('period','1.0',True,'visible=false')
    win.equationPanel.add('nperiod',1,True,'visible=false')
    win.equationPanel.define('nperiod','min=1')
    win.equationPanel.add('secvar','x',True,'visible=false')
    win.equationPanel.add('secval', 0.0,True,'visible=false')
    win.equationPanel.add('maxtime',100.0,True,'visible=false')
    enum=common.Enum({'+':0,'-':1},'+')
    win.equationPanel.add('secdir',enum,True,'visible=false')
    win.equationPanel.add('variables.__anchor__',False,True,"visible=false")
    win.equationPanel.add('inverse.__anchor__',False,True,"visible=false")
    win.equationPanel.define('inverse','visible=false')
    win.equationPanel.add('functions.__anchor__',False,True,"visible=false")
    win.equationPanel.add('constants.__anchor__',False,True,"visible=false")
    win.equationPanel.set_callback(lambda identifier,rhs: on_panel_change(identifier,rhs,win))

    setattr(win,'valuePanel',gui.ATWPanel(win.glWindow,'Values'))
    win.valuePanel.add('time.t = ','0.0')
    win.valuePanel.add('step.t = ','0')
    win.valuePanel.define('step', 'visible=false')
    win.valuePanel.add('time.__anchor__',False,True,"visible=false")
    win.valuePanel.add('step.__anchor__',False,True,"visible=false")
    win.valuePanel.add('variables.__anchor__',False,True,"visible=false")
    win.valuePanel.add('parameters.__anchor__',False,True,"visible=false")
    win.valuePanel.set_callback(lambda identifier,rhs: on_panel_change(identifier,rhs,win))

    win.glWindow.flush()
    
def unplug(win):
    if not win.unregister_plugin('equations'):
        return
    win.equationPanel.destroy()
    win.valuePanel.destroy()
    delattr(win,'equationPanel')
    delattr(win,'valuePanel')
    delattr(win,'eqsys')
    delattr(win,'invsys')
