from ctypes import *
from .. import lib
from .constants import *
from . import color

C_CallbackType=CFUNCTYPE(None, c_char_p)

class Curve(object):
    """ Wrapper for Curve objects """
    lib.scigma_gui_create_curve.argtypes=[c_int,c_char_p,c_int,c_int, c_int, 
                                          c_int,c_int, c_float,c_float,POINTER(c_float),c_float,
                                          C_CallbackType]
    lib.scigma_gui_curve_set_marker_size.argtypes=[c_int,c_float]
    lib.scigma_gui_curve_set_point_size.argtypes=[c_int,c_float]
    lib.scigma_gui_curve_set_color.argtypes=[c_int,POINTER(c_float)]
    lib.scigma_gui_curve_set_view.argtypes=[c_int,c_int,POINTER(c_int)]
    
    def __init__(self, glWindow,identifier, nPoints, varWave, constWave, 
                 marker=STAR, point=NONE, markerSize=16.0,
                 pointSize=8.0, col=color.values['green'], delay=0.0,cbfun=None):
        """ This creates a polygon line from the data given in waves

        waves is expected to be a list of lists of waves representing
        all independent variables of the current view(s). 
        """
        C_FloatArrayType=c_float*4
        colorArray=C_FloatArrayType(*col)
        identifier=bytes(str(identifier).encode("ascii"))
        self.c_callback=C_CallbackType(lambda id_ptr: self.callback(id_ptr)) 
        self.py_callback=cbfun
        self.objectID = lib.scigma_gui_create_curve(glWindow.objectID,identifier,nPoints,
                                                    varWave.objectID,constWave.objectID,
                                                    marker,point,markerSize,pointSize,
                                                    cast(colorArray,POINTER(c_float)),
                                                    delay,self.c_callback)
    
    def destroy(self):
        if self.objectID != -1:
            lib.scigma_gui_destroy_curve(self.objectID)
            
    def __str__(self):
        return 'Curve(id='+str(self.objectID)+')'
        
    def __repr__(self):
        return 'Curve(id='+str(self.objectID)+')'
    
    def callback(self,id_ptr):
        if self.py_callback:
            identifier=str(string_at(id_ptr).decode())
            self.py_callback(identifier)
    
    def set_marker_style(self,style):
        lib.scigma_gui_curve_set_marker_style(self.objectID,style)
        
    def set_marker_size(self,size):
        lib.scigma_gui_curve_set_marker_size(self.objectID,size)
        
    def set_point_style(self,style):
        lib.scigma_gui_curve_set_point_style(self.objectID,style)
        
    def set_point_size(self,size):
        lib.scigma_gui_curve_set_point_size(self.objectID,size)
        
    def set_color(self,color):
        C_FloatArrayType=c_float*4
        colorArray=C_FloatArrayType(*col)
        lib.scigma_gui_curve_set_color(self.objectID,cast(colorArray,POINTER(c_float)))
        
    def set_view(self, indices):
        length=len(indices)
        C_IntArrayType=c_int*length
        indexArray=C_IntArrayType(*indices)
        lib.scigma_gui_curve_set_view(self.objectID,length,cast(indexArray,POINTER(c_int)))
        
    def set_n_points(self,nPoints):
        lib.scigma_gui_curve_set_n_points(self.objectID,nPoints)
