from ctypes import *
from .. import lib
from .constants import *
from . import color

C_CallbackType=CFUNCTYPE(None, c_int, c_int, c_int, c_int,c_int)

class Bundle(object):
    """ Wrapper for Bundle objects """
    lib.scigma_gui_create_bundle.argtypes=[c_int,
                                           c_int,c_int,c_int,
                                           c_int,c_int, 
                                           C_CallbackType]
    lib.scigma_gui_bundle_set_marker_size.argtypes=[c_int,c_float]
    lib.scigma_gui_bundle_set_point_size.argtypes=[c_int,c_float]
    lib.scigma_gui_bundle_set_delay.argtypes=[c_int,c_float]
    lib.scigma_gui_bundle_set_color.argtypes=[c_int,POINTER(c_float)]
    lib.scigma_gui_bundle_set_view.argtypes=[c_int,c_int, POINTER(c_int), c_char_p,c_char_p,c_double]
    lib.scigma_gui_bundle_set_view.restype=c_char_p

    
    def __init__(self, glWindow, length, nRays, nVars,
                 varWave, constWave, cbfun=None):
        self.c_callback=C_CallbackType(lambda double, button, point, x, y:
                                       self.callback(double,button,point,x ,y)) 
        self.py_callback=cbfun
        self.objectID = lib.scigma_gui_create_bundle(glWindow.objectID,length,
                                                     nRays,nVars,varWave.objectID,constWave.objectID,
                                                     self.c_callback)
    def destroy(self):
        if self.objectID != -1:
            lib.scigma_gui_destroy_bundle(self.objectID)
            
    def __str__(self):
        return 'Bundle(id='+str(self.objectID)+')'
        
    def __repr__(self):
        return 'Bundle(id='+str(self.objectID)+')'
    
    def callback(self,double,button,point,x,y):
        if self.py_callback:
            self.py_callback(double,button,point,x,y)

    def set_style(self,style):
        lib.scigma_gui_bundle_set_style(self.objectID,style)
        
    def set_marker_style(self,style):
        lib.scigma_gui_bundle_set_marker_style(self.objectID,style)
        
    def set_marker_size(self,size):
        lib.scigma_gui_bundle_set_marker_size(self.objectID,size)
        
    def set_point_style(self,style):
        lib.scigma_gui_bundle_set_point_style(self.objectID,style)
        
    def set_point_size(self,size):
        lib.scigma_gui_bundle_set_point_size(self.objectID,size)
        
    def set_color(self,color):
        C_FloatArrayType=c_float*4
        colorArray=C_FloatArrayType(*color)
        lib.scigma_gui_bundle_set_color(self.objectID,cast(colorArray,POINTER(c_float)))

    def set_delay(self,delay):
        lib.scigma_gui_bundle_set_delay(self.objectID,delay)
        
    def replay(self):
        lib.scigma_gui_bundle_replay(self.objectID)

    def finalize(self):
        lib.scigma_gui_bundle_finalize(self.objectID)

    def set_view(self,indices,expBuffer,indBuffer,timeStamp):
        C_IntArrayType=c_int*len(indices)
        indexArray=C_IntArrayType(*indices)
        error=lib.scigma_gui_bundle_set_view(self.objectID,len(indices),indexArray,expBuffer,indBuffer,timeStamp)
        if error:
            raise Exception(str(error.decode()))
