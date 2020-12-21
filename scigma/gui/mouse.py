from ctypes import *
from .. import lib

C_CallbackType=CFUNCTYPE(None)

class Mouse(object):
    """ Wrapper for Mouse objects.
    
    This class exposes the functionality of the Mouse class.
    Mouse instances are EventSinks (see 'events.h') for mouse
    events that are not processed by any other EventSink (like Navigator)
    currently, the only event is the context menu right click
    Therefore, they must be connected to a GLWindow after all other
    EventSinks processing mouse events in order to work properly.

    """

    def __init__(self, cbfun):
        self.c_callback=C_CallbackType(cbfun) 
        self.objectID = lib.scigma_gui_create_mouse(self.c_callback)

    def destroy(self):
        lib.scigma_gui_destroy_mouse(self.objectID)
