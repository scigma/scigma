import sys,os
from sys import version_info
try:
    if version_info.major == 2:
        # We are using Python 2.x
        import Tkinter as tk
    elif version_info.major == 3:
        # We are using Python 3.x
        import tkinter as tk
except:
    tk=None
    print("tkinter not found / not using tk")
import ctypes
from .constants import *
from .application import loop, break_loop
from .application import add_loop_callback, add_idle_callback
from .application import remove_loop_callback, remove_idle_callback
from .glwindow import GLWindow
from .atwpanel import ATWPanel, Enum
from .navigator import Navigator
from .console import Console
from .curve import Curve
from .picker import Picker
from .cosy import Cosy
from . import color
from . import graph

if os.name== 'posix':
    import select
    def stdin_ready():
        """ 
        A function to determine whether the Python
        interpreter is ready to receive input on
        Unix systems.
        """ 
        infds, outfds, erfds = select.select([sys.stdin],[],[],0)
        if infds:
            return True
        else:
            return False
elif sys.platform=='win32':
    import msvcrt
    def stdin_ready():
        """ 
        A function to determine whether the Python
        interpreter is ready to receive input on
        Windows.
        """
        return msvcrt.kbhit()

if tk:
    tkroot=tk.Tk()
    tkroot.withdraw()

KEY_INTERVAL=0.05 # wait for events for KEY_INTERVAL seconds,
# between checks whether a key has been hit at the Python interface

def hook():       
    """ 
    If the Python interpreter is idle, the scigma.gui event 
    loop is called. The argument KEY_INTERVAL given to
    scigma_gui_loop() specifies the time to wait for gui
    events in seconds, before returning control to the 
    interpreter. Note that this is not a busy wait - a large
    KEY_INTERVAL therefore will reduce system load, but make
    the python console less responsive
    """

    while not stdin_ready():
        loop(KEY_INTERVAL)
    return 0

"""
This installs hook() as PyOS_InputHook, calling
the scigma event loop whenever the interpreter
is idle.
"""
c_hook=ctypes.PYFUNCTYPE(ctypes.c_int)(hook)
ctypes.c_void_p.in_dll(ctypes.pythonapi,"PyOS_InputHook").value=ctypes.cast(c_hook,ctypes.c_void_p).value

colorList=[color.values[name] for name in color.names]
graph.set_indexed_colors(colorList)
