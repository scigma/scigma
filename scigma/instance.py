from .log import Log
from . import gui
from .gui import GLWindow, ATWPanel, Enum, Navigator, Console, Picker, Cosy
from .gui import VIEW_TYPE, POINT_TYPE, COORDINATE_NAME, COORDINATE_FLAG, Z_INDEX
from .gui import color
from .num import EquationSystem
from .num import MODE
from .dat import Wave
from .util import dict_entry, dict_full_paths, Float
from .commands import alias, parse, axes, set_range, expr, mode, select
from . import options
from . import objects
from . import largeFontsFlag

SUCCESS_PREFIX='__succ__'
FAIL_PREFIX='__fail__'
DATA_PREFIX='__data__'
ERROR_PREFIX='__error_'
PREFIX_LENGTH=8

class Instance(object):
    """ Manages a single instance of Scigma.
    
    This class bundles all information relevant for a Scigma window,
    including both the graphical and the numerical elements. 
    """

    def __init__(self,eqsys=None):
        self.log=Log()
        self.looplambda=lambda:self.loop_callback()
        gui.add_loop_callback(self.looplambda)
        self.glWindow=GLWindow()
        self.cosy=Cosy(self.glWindow,VIEW_TYPE['xy'],largeFontsFlag)
        self.navigator=Navigator(VIEW_TYPE['xy'])
        self.glWindow.connect(self.navigator)
        self.glWindow.set_viewing_angle(0)
        self.console=Console(self.glWindow,largeFontsFlag)
        self.glWindow.connect(self.console)
        self.console.set_callback(lambda line: self.console_callback(line))
        self.picker=Picker(VIEW_TYPE['xy'])
        self.picker.set_callback(lambda ctrl,x,y,z: self.picker_callback(ctrl,x,y,z))
        self.glWindow.connect_before(self.picker)
        self.glWindow.stall()
        self.equationSystem = eqsys if eqsys else EquationSystem()
        self.equationSystemBackup=None
        self.timeStamp=0
        self.equationPanel=ATWPanel(self.glWindow,'Equations')
        self.equationPanel.add('variables.__anchor__',False,True,"visible=false")
        self.equationPanel.add('functions.__anchor__',False,True,"visible=false")
        self.equationPanel.add('constants.__anchor__',False,True,"visible=false")
        self.equationPanel.set_callback(lambda identifier,rhs: self.equation_callback(identifier,rhs))
        
        self.valuePanel=ATWPanel(self.glWindow,'Values')
        self.valuePanel.add('time.t = ','0.0')
        self.valuePanel.add('time.__anchor__',False,True,"visible=false")
        self.valuePanel.add('variables.__anchor__',False,True,"visible=false")
        self.valuePanel.add('parameters.__anchor__',False,True,"visible=false")
        self.valuePanel.set_callback(lambda identifier,rhs: self.equation_callback(identifier,rhs))
        self.numericalPanel=ATWPanel(self.glWindow,'Numerical')
        self.numericalPanel.set_callback(lambda identifier,value: self.numerical_callback(identifier,value))
        enum= Enum(MODE,'ode')
        self.numericalPanel.add('mode',enum)
        self.numericalPanel.add_separator('sep1')
        self.numericalPanel.add('secvar','x',False,"visible=false")
        self.numericalPanel.add('secval',0.0,True,"visible=false")
        self.numericalPanel.add('period','1.0',True,"visible=false")
        self.numericalPanel.add('nperiod',1,True,"visible=false")
        self.numericalPanel.define('nperiod',"min=1")
        enum = Enum({'+':0,'-':1},'+')
        self.numericalPanel.add('secdir',enum, True,"visible=false")
        self.numericalPanel.add('dt',0.001)
        self.numericalPanel.define('dt',"min=0.0")
        self.numericalPanel.add('maxtime',100.0,True,"visible=false") 
        self.numericalPanel.define('maxtime',"min=0.0")
        self.numericalPanel.add('manifolds.eps',0.0001)
        self.numericalPanel.add('manifolds.ds',0.01)
        self.numericalPanel.define('manifolds.ds',"min=0.0")
        self.numericalPanel.add('manifolds.alpha',0.3)
        self.numericalPanel.define('manifolds.alpha',"min=0.0")
        self.numericalPanel.add('manifolds.evec1',int(1))
        self.numericalPanel.define('manifolds.evec1',"min=1")
        self.numericalPanel.add('manifolds.evec2',int(2))
        self.numericalPanel.define('manifolds.evec2',"min=1")
        self.numericalPanel.add('manifolds.arc', True)
        enum = Enum({'non-stiff':0,'stiff':1},'stiff')
        self.numericalPanel.add('odessa.type',enum)
        enum = Enum({'numeric':0,'symbolic':1},'symbolic')
        self.numericalPanel.add('odessa.atol',1e-9)
        self.numericalPanel.define('odessa.atol',"min=0.0")
        self.numericalPanel.add('odessa.rtol',1e-9)
        self.numericalPanel.define('odessa.rtol',"min=0.0")
        self.numericalPanel.add('odessa.mxiter',int(500))
        self.numericalPanel.define('odessa.mxiter',"min=1")
        self.numericalPanel.add('odessa.Jacobian',enum,True,"readonly=true")
        self.numericalPanel.add('Newton.tol',1e-9)
        self.numericalPanel.define('Newton.tol',"min=0.0")
        self.numericalPanel.add('Newton.Jacobian',enum,True,"readonly=true")
        self.viewPanel=ATWPanel(self.glWindow,'View')
        self.viewPanel.set_callback(lambda identifier,value: self.view_callback(identifier,value))
        enum = Enum(VIEW_TYPE,'xy')
        self.viewPanel.add('axes',enum)
        self.viewPanel.add_separator('sep1')
        for coord in COORDINATE_NAME:
            self.viewPanel.add(coord,coord.lower(),False)
        self.viewPanel.add_separator('sep2')
#        for coord in COORDINATE_NAME:
#            self.viewPanel.add(coord+'range.min',Float(-1.0),False)
#            self.viewPanel.add(coord+'range.max',Float(1.0),False)
#        self.viewPanel.add('trange.duration', Float(10.0))
#        self.viewPanel.define('trange.duration', 'min=0.0')
        for coord in COORDINATE_NAME[Z_INDEX:]:
            self.viewPanel.set(coord,'')
            self.viewPanel.define(coord,'visible=false')
#            self.viewPanel.define(coord+'range','visible=false')
        self.stylePanel=ATWPanel(self.glWindow,'Style')
        self.stylePanel.set_callback(lambda identifier,value: self.style_callback(identifier,value))
        #enum = Enum({'dark':0,'light':1},'dark')
        #self.stylePanel.add('theme',enum)
        #self.stylePanel.add_separator('sep1')
        self.stylePanel.add("color",color.from_string('red'))
        self.stylePanel.define("color","colormode=hls");
        enum = Enum(POINT_TYPE,'none')
        self.stylePanel.add('marker.style',enum)
        self.stylePanel.add('marker.size',Float(32.0 if largeFontsFlag else 16.0))
        self.stylePanel.define('marker.size', 'min=1.0')
        self.stylePanel.define('marker.size', 'max=64.0')
        enum = Enum(POINT_TYPE,'none')
        self.stylePanel.add('point.style',enum)
        self.stylePanel.add('point.size',Float(16.0 if largeFontsFlag else 8.0))
        self.stylePanel.define('point.size', 'min=1.0')
        self.stylePanel.define('point.size', 'max=64.0')
        self.stylePanel.add('delay',Float(0.0))
        self.stylePanel.define('delay', 'min=0.0')
       
        self.optionPanels={'Numerical':self.numericalPanel,'View':self.viewPanel,
                           'Style':self.stylePanel}
        self.options={'Numerical':self.numericalPanel.data,
                      'View':self.viewPanel.data, 'Style':self.stylePanel.data}
        self.glWindow.flush()
        self.base=""
        self.identifiers={}
        self.objects={}
        self.runningObjectNumber=0
        self.selectedObjectList=None
        self.cursorList=[]
        self.plotVariables=None
        self.pendingTasks=0
        self.script="none"
        self.source="internal"
        self.glWindow.set_title("SCIGMA - script: "+self.script+" - equations: "+self.source)


    def destroy(self):
        self.numericalPanel.destroy()
        self.viewPanel.destroy()
        self.valuePanel.destroy()
        self.equationPanel.destroy()
        self.stylePanel.destroy()
        self.glWindow.remove_drawable(self.picker)
        self.glWindow.disconnect(self.console)
        self.glWindow.disconnect(self.navigator)
        self.glWindow.disconnect(self.picker)
        self.picker.destroy()
        self.console.destroy()
        self.navigator.destroy()
        self.cosy.destroy()
        self.glWindow.destroy()
        if self.equationSystemBackup:
            if not self.equationSystemBackup is self.equationSystem:
                self.equationSystemBackup.destroy()
        self.equationSystem.destroy()
        gui.remove_loop_callback(self.looplambda)
        self.log.destroy()
    
    def loop_callback(self): 
        message=self.log.pop()
        while message is not "":
            if message[0:PREFIX_LENGTH]==DATA_PREFIX:
                self.console.write_data(message)
            elif message[0:PREFIX_LENGTH]==ERROR_PREFIX:
                self.console.write_error(message[PREFIX_LENGTH:])
            elif message[0:PREFIX_LENGTH]==SUCCESS_PREFIX:
                identifier=message[PREFIX_LENGTH:]
                listid=identifier.partition('[')[0]
                listindex=int(identifier.strip(']').partition('[')[2])
                objects.success(objects.get(listid,self)[listindex],self)
            elif message[0:PREFIX_LENGTH]==FAIL_PREFIX:
                identifier=message[PREFIX_LENGTH:]
                listid=identifier.partition('[')[0]
                listindex=int(identifier.strip(']').partition('[')[2])
                objects.fail(objects.get(listid,self)[listindex],self)
            else:
                self.console.write(message)
            message=self.log.pop()
    
    def console_callback(self,line):
        line=line.partition('#')[0]
        list=line.split()
        if len(list)==0:
            return
        cmd=list[0]
        args=list[1:]  
        
        if cmd in alias:
            try:
                alias[cmd](*args,instance=self)
            except Exception as e:
                self.console.write_error(str(e.args[0])+'\n')
                raise
        else:
            try:
                parse(line,self)
                self.timeStamp=self.equationSystem.timestamp()
                objects.move_cursor(None,self)
            except Exception as e:
                # this may happen if user is trying to set/get an option,
                # do not give up yet
                try:
                    if len(args)==0:
                        result=options.get_string(cmd,self)
                        self.console.write_data(result)
                    else:
                        options.set(cmd,' '.join(args),self)
                except Exception as ee:
                    if e.args[0]=='syntax error':
                        # a generic failure in parse() indicates that we were not
                        # trying to parse an equation, therefore forward ee as well
                        error =ee.args[0]+', or syntax error'
                        self.console.write_error(error+'\n')
                        raise Exception(error)
                    else:
                        # if the input was sufficiently well formed to trigger a
                        # specific warning, assume that not get/set option was tried
                        # and ignore ee
                        self.console.write_error(e.args[0]+'\n')
                        raise e
        return 0

    def equation_callback(self,identifier,rhs):
        try:
            line = identifier.rpartition('.')[2]+rhs
            parse(line,self)
        except Exception as e:
            self.console.write_error(e.args[0]+'\n')
            raise

    def numerical_callback(self, identifier,value):
        if identifier == 'mode':
            mode(value.label, instance=self)
        elif identifier == 'secvar':
            secvar=None
            varnames=self.equationSystem.variable_names()
            for i in range(len(varnames)):
                if value==varnames[i]:
                    options.set('Numerical.secvar',value,self)
                    return
            self.console.write_error(value + " cannot be used for a Poincare section (only variables)\n")

    def picker_callback(self,ctrl,*args):
        allow=True
        for i in range(Z_INDEX+1):
            var = self.options['View'][COORDINATE_NAME[i]]
            if (not (var in self.equationSystem.variable_names() or 
            var in self.equationSystem.parameter_names()) and
            args[i]):
                allow = False
        if not allow:
            self.console.write_error("picking is deactivated\n")
        else:
            for i in range(Z_INDEX+1):
                if(args[i]):
                    var=self.options['View'][COORDINATE_NAME[i]]
                    parse(var+"="+str(args[i]))
                    self.console.write("setting "+var+" to ")
                    self.console.write_data(str(args[i])+"\n")
            self.selectedObject=None
            if ctrl:
                objects.add_cursor(self)
            else:
                objects.move_cursor(None,self)
    
    def style_callback(self, identifier, value):
        pass
    
    def view_callback(self, identifier, value):
        if identifier == 'axes':
            axes(value.label,instance=self)
        if identifier in COORDINATE_NAME:
            expr(identifier,value,instance=self)
        if identifier in [x+'range.min' for x in COORDINATE_NAME]:
            coord=identifier[0]
            min,max = value.value, self.options['View'][coord+'range']['max'].value
            set_range(coord,min,max,self)
        if identifier in [x+'range.max' for x in COORDINATE_NAME]:
            coord=identifier[0]
            min,max = self.options['View'][coord+'range']['min'].value,value.value
            set_range(coord,min,max,self)
    
    def select_callback(self, identifier):
        identifier=identifier.partition('[')[0]
        select(identifier,self)
        self.console.write("selecting ")
        self.console.write_data(identifier+"\n")
    
    def rebuild_panels(self):
        self.glWindow.stall()
        
        self.valuePanel.remove('time.t = ')
        ids=[]
        for group in self.equationPanel.data:
            for entry in self.equationPanel.data[group]:
                if not entry == '__anchor__':
                    ids.append(group+'.'+entry)
        for identifier in ids:
            self.equationPanel.remove(identifier)   
        ids=[]
        for group in self.valuePanel.data:
            for entry in self.valuePanel.data[group]:
                if not entry == '__anchor__':
                    ids.append(group+'.'+entry)
        for identifier in ids:
            self.valuePanel.remove(identifier)   
        
        self.valuePanel.add('time.t = ',str(self.equationSystem.time()))
        
        self.equationSystem.variable_definitions()
        groups = ['variables','functions','constants','variables','parameters']
        defs = [self.equationSystem.variable_definitions(),
                self.equationSystem.function_definitions(),
                self.equationSystem.constant_definitions(),
                self.equationSystem.variables(),
                self.equationSystem.parameters()]
        panels = [self.equationPanel,self.equationPanel,self.equationPanel,
                  self.valuePanel,self.valuePanel]
        for i in range(len(groups)):
            for definition in defs[i]:
                parts=definition.partition('=')
                var=parts[0].strip()
                rhs=parts[2].strip()
                panels[i].add(groups[i]+'.'+var+' = ',rhs, False)
        self.glWindow.flush()
        
        nVarStr=str(self.equationSystem.n_variables())
        self.numericalPanel.define('manifolds.evec1',"max="+nVarStr)
        self.numericalPanel.define('manifolds.evec2',"max="+nVarStr)


