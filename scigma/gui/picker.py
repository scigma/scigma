from ctypes import *
from .. import lib
from .constants import *

C_CallbackType=CFUNCTYPE(None, c_bool,c_float, c_float, c_float)

class Picker(object):
    """ Wrapper for Picker objects.
    
    This class exposes the functionality of the Picker class.
    Picker is used to select a point in the viewing volume with
    the mouse. It is an EventSink for mouse button events and 
    must therefore be connected to a GLWindow in order to
    work properly.

    """

    def __init__(self, view):
        self.c_callback=C_CallbackType(lambda ctrl,x,y,z: self.callback(ctrl,x,y,z)) 
        self.objectID = lib.scigma_gui_create_picker(view,self.c_callback)
        self.view=view
        self.py_callback=None

    def destroy(self):
        lib.scigma_gui_destroy_picker(self.objectID)

    def callback(self,ctrl,x,y,z):
        if self.py_callback:
            if self.view&Z_COORDINATE:
                self.py_callback(ctrl,x,y,z)
            else:
                self.py_callback(ctrl,x,y,None)

    def set_callback(self,function):
        self.py_callback=function
   
