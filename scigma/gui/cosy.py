from ctypes import *
from .. import lib
from .constants import X_COORDINATE, Y_COORDINATE, Z_COORDINATE

class Cosy(object):
    """ Wrapper for Cosy objects.
    
    This class exposes the functionality of the Cosy class.
    Cosy stands for 'coordinate system'. 
    """
    lib.scigma_gui_create_cosy.argtypes=[c_int,c_int,c_bool]

    def __init__(self,glWindow,view,largeFontsFlag):
        self.objectID = lib.scigma_gui_create_cosy(glWindow.objectID,view,largeFontsFlag)

    def destroy(self):
        lib.scigma_gui_destroy_cosy(self.objectID)

    def set_view(self,view):
        lib.scigma_gui_cosy_set_view(self.objectID,view)

    def set_label(self,coord,label):
        label=bytes(label.encode("ascii"))
        lib.scigma_gui_cosy_set_label(self.objectID,coord,label)
