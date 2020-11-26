import re, os
from . import num
from . import gui
from . import common
from . import library
from . import options
from . import equations
from . import picking
from . import graphs
from .windowlist import windows

class Window(object):
    """ Manages a single Scigma window.
    
    """
    
    def __init__(self):
        windows.append(self)
        self.log=common.Log()
        self.looplambda=lambda:self.loop_callback()
        gui.add_loop_callback(self.looplambda)
        
        self.glWindow=gui.GLWindow()
        self.console=gui.Console(self.glWindow,library.largeFontsFlag)
        self.glWindow.connect(self.console)
        self.console.set_callback(lambda line: self.on_console(line))

        self.unplugFunctions={}
        
        self.options={}
        self.optionPanels={}
        self.commands={"sleep":gui.sleep}

        self.queue=[]
        self.sleeping=False
        
    def destroy(self):
        for key in self.unplugFunctions.keys():
            self.unplugFunctions[key]()
            
        for key in self.optionPanels.keys():
            self.optionPanels[key].destroy()
            
        self.glWindow.destroy()
        gui.remove_loop_callback(self.looplambda)
        self.log.destroy()
        self.log=None
        windows.remove(self)
        
    def register_plugin(self, name, funplug, commands=None):
        if self.has_plugin(name):
            return False
        self.unplugFunctions[name]=funplug
        self.commands[name]=commands
        return True

    def unregister_plugin(self, name):
        if not self.has_plugin(name):
            return False
        del self.unplugFunctions[name]
        del self.commands[name]
        return True

    def has_plugin(self, name):
        return name in self.unplugFunctions
    
    def acquire_option_panel(self, identifier):
        if not identifier in self.options:
            self.optionPanels[identifier]=gui.ATWPanel(self.glWindow,identifier)
            self.options[identifier]=self.optionPanels[identifier].data
            self.glWindow.request_redraw()
        return self.optionPanels[identifier] 

    def release_option_panel(self, identifier):
        if not self.options[identifier]:
            del self.options[identifier]
            self.optionPanels[identifier].destroy()
            del self.optionPanels[identifier]
            self.glWindow.request_redraw()

    def loop_callback(self):
        if not gui.application.is_sleeping():
            self.process_messages()
            if self.sleeping:
                self.sleeping=False
                self.console.write(" continuing!\n")
                print("... continuing!")
       
        while self.queue and not gui.application.is_sleeping():
            line=self.queue[0]            
            self.queue=self.queue[1:]
            if (self.options['Global']['echo'].label=='on' and
                line[:5] != 'write' and line[:7]!='writeln' and
                line[:4] != 'data' and line[:6]!='dataln' and
                line[:7] != 'comment' and line[:9]!='commentln' and
                line[:4] != 'warn' and line[:6]!='warnln' and
                line[:5] != 'error' and line[:7]!='errorln' and
                line[:5]!='sleep'):
                self.console.write(line+'\n')
            self.process_command(line)
            if not self in windows: #return if the last command was 'quit'
                return
            self.process_messages()

        if gui.application.is_sleeping():  
            if not self.sleeping:
                self.sleeping=True
                self.console.write("Press ESC to continue ...")
                print("Press ESC in the graphics window to continue ...")
        
        self.glWindow.request_redraw()
                
    def on_console(self,line):
        self.queue.append(line)

    def process_command(self,line):
        line=line.partition('#')[0]         # remove any comment
        if len(line) and line[-1]=='"':     # string command (write/writeln)
            clist=line.partition('"')
            clist=[clist[0].strip(),clist[2].strip('"')]
        else:
            line=re.sub("\s*=\s*","=",line)     # turn 'x = y' into 'x=y' (avoids interpretation as x('=','y'))
            line=re.sub("\s*,\s*",",",line)     # turn 'pert 1, 3, 4' into 'pert 1,3,4' (comma-separated lists are one argument) 
            clist=line.split()                  # separate command and arguments
            clist=[item if len(item.split(','))==1 else item.split(',') for item in clist] # turn comma-separated lists into Python lists

        if len(clist)==0:
            return
     
        cmd=clist[0]
        args=clist[1:]

        paths=[]
        try:
            common.dict_full_paths(cmd,self.commands,paths) # search for command in dictionary
        except:
            paths=[]

        if len(paths) == 0: # this is possibly an equation or the attempt to set/query an option
            try:
                equations.parse(line,self)
                if line[0]!='$':
                    picking.on_parse(self)
            except Exception as e: # if parsing fails, check if we are trying to set/query option
                try:
                    if len(args)==0:
                        result=options.get_string(cmd,self)
                        self.console.write_data(result+"\n")
                    else:
                        options.set(cmd,' '.join(args),self)
                except Exception as ee:
                    if ee.args[0][:9] == 'ambiguous':
                        error=ee.args[0]
                    elif e.args[0]=='syntax error':
                        # a generic failure in parse() suggests that we were not
                        # trying to parse an equation, therefore forward ee as well
                        error =ee.args[0]+', or syntax error'
                    else:
                        # if the input was sufficiently well formed to trigger a
                        # specific parser warning, assume that no set/query option was tried
                        # and ignore ee
                        error=e.args[0]
                    
                    self.console.write_error(error+'\n')
                    raise Exception(error)
                   
        elif not len(paths) == 1: # there is more than one command with the same name or path
            error = "ambiguous command; use qualified name, e.g. " + paths[0] + " or " + paths[1]
            self.console.write_error(error+'\n')
            raise Exception(error)
        else: # run command with the arguments
            try:
                cfunc=common.dict_entry(paths[0],self.commands)
                cfunc(*args,win=self)
            except Exception as e:
                self.console.write_error(str(e.args[0])+'\n')
                raise e

    def process_messages(self):
        mtype, message=self.log.pop()
        while message != "":
            if mtype==common.Log.DATA:
                self.console.write_data(message)
            elif mtype==common.Log.WARNING:
                self.console.write_warning(message)
            elif mtype==common.Log.ERROR:
                self.console.write_error(message)
            elif mtype==common.Log.SUCCESS:
                args=message.split("|")
                graphs.success(args[0],args[1:],self)
            elif mtype==common.Log.FAIL:
                args=message.split("|")
                graphs.fail(args[0],args[1:],self)
            else:
                self.console.write(message)
            mtype,message=self.log.pop()
