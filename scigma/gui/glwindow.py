from ctypes import *
from .. import lib
from .navigator import Navigator
from .picker import Picker
from .console import Console
from .atwpanel import ATWPanel
from .curve import Curve
from .constants import N_COORDINATES

class GLWindow(object):
    """ Wrapper for GLWindow objects.
    
    This class exposes the functionality of the GLWindow class,
    as well as the functionality of its ViewingArea, ViewingVolume
    and GLContext members.

    """
    lib.scigma_gui_gl_window_set_title.argtypes=[c_int,c_char_p]
    lib.scigma_gui_gl_window_set_margin.argtypes=[c_int,c_int,c_float]
    lib.scigma_gui_gl_window_set_margins.argtypes=[c_int,c_float,c_float,c_float,c_float]
    lib.scigma_gui_gl_window_blow_up.argtypes=[c_int,c_float,c_float,c_float,c_float]
    lib.scigma_gui_gl_window_set_z_camera.argtypes=[c_int,c_float]
    lib.scigma_gui_gl_window_set_viewing_angle.argtypes=[c_int,c_float]
    lib.scigma_gui_gl_window_set_range.argtypes=[c_int,c_int,c_float,c_float]
    lib.scigma_gui_gl_window_shift.argtypes=[c_int,c_float,c_float,c_float]
    lib.scigma_gui_gl_window_shiftII.argtypes=[c_int,c_int,c_float]
    lib.scigma_gui_gl_window_scale.argtypes=[c_int,c_int,c_float]
    lib.scigma_gui_gl_window_rotate.argtypes=[c_int,c_float,c_float,c_float,c_float]
    lib.scigma_gui_gl_window_rotateII.argtypes=[c_int,c_int,c_float]
    lib.scigma_gui_gl_window_min.restype=POINTER(c_float)
    lib.scigma_gui_gl_window_max.restype=POINTER(c_float)

    def __init__(self):
        self.objectID = lib.scigma_gui_create_gl_window()

    def destroy(self):
        lib.scigma_gui_destroy_gl_window(self.objectID)

    def set_title(self,title):
        title = create_string_buffer(bytes(title.encode("ascii")))
        lib.scigma_gui_gl_window_set_title(self.objectID,title)

    def connect(self, eventSink):
        if(isinstance(eventSink,Navigator)):
            lib.scigma_gui_gl_window_connect_navigator(self.objectID, eventSink.objectID)
        elif(isinstance(eventSink,Console)):
            lib.scigma_gui_gl_window_connect_console(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,ATWPanel)):
            lib.scigma_gui_gl_window_connect_atw_panel(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,Picker)):
            lib.scigma_gui_gl_window_connect_picker(self.objectID,eventSink.objectID)


    def connect_before(self, eventSink):
        if(isinstance(eventSink,Navigator)):
            lib.scigma_gui_gl_window_connect_navigator_before(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,Console)):
            lib.scigma_gui_gl_window_connect_console_before(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,ATWPanel)):
            lib.scigma_gui_gl_window_connect_atw_panel_before(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,Picker)):
            lib.scigma_gui_gl_window_connect_picker_before(self.objectID,eventSink.objectID)


    def disconnect(self, eventSink):
        if(isinstance(eventSink,Navigator)):
            lib.scigma_gui_gl_window_disconnect_navigator(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,Console)):
            lib.scigma_gui_gl_window_disconnect_console(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,ATWPanel)):
            lib.scigma_gui_gl_window_disconnect_atw_panel(self.objectID,eventSink.objectID)
        elif(isinstance(eventSink,Picker)):
            lib.scigma_gui_gl_window_disconnect_picker(self.objectID,eventSink.objectID)


    def set_size(self,width,height):
        lib.scigma_gui_gl_window_set_size(self.objectID,width,height)

    def set_margin(self,whichMargin,pixels):
        lib.scigma_gui_gl_window_set_margin(self.objectID,whichMargin,pixels)

    def set_margins(self,left, right, bottom, top):
        lib.scigma_gui_gl_window_set_margins(self.objectID,left,right,bottom,top)

    def set_visible_region(self,left,right,bottom,top):
        lib.scigma_gui_gl_window_blow_up(self.objectID,left,right,bottom,top)

    def set_z_camera(self,z):
        lib.scigma_gui_gl_window_set_z_camera(self.objectID,z)
 
    def set_viewing_angle(self,angle):
        lib.scigma_gui_gl_window_set_viewing_angle(self.objectID,angle)

    def set_range(self,coordinate,low,high):
        lib.scigma_gui_gl_window_set_range(self.objectID,coordinate,low,high)

    def range(self):
        min=[]
        max=[]
        cmin=cast(lib.scigma_gui_gl_window_min(self.objectID),POINTER(c_float))
        cmax=cast(lib.scigma_gui_gl_window_max(self.objectID),POINTER(c_float))
        for i in range(0,N_COORDINATES):
            min.append(cmin[i])
            max.append(cmax[i])
        return min,max

    def shift(self,first,second,third=None):
        if(None==third):
            lib.scigma_gui_gl_window_shiftII(self.objectID,first,second)
        else:
            lib.scigma_gui_gl_window_shift(self.objectID,first,second,third)

    def scale(self,coordinate,factor):
        lib.scigma_gui_gl_window_scale(self.objectID,coordinate,factor)
  
    def rotate(self,first,second,third=None,fourth=None):
        if(None==third):
            lib.scigma_gui_gl_window_rotateII(self.objectID,first,second)
        else:
            lib.scigma_gui_gl_window_rotate(self.objectID,first,second,third,fourth)
  
    def reset(self):
        lib.scigma_gui_gl_window_reset(self.objectID)

    def reset_rotation(self):
        lib.scigma_gui_gl_window_reset_rotation(self.objectID)

    def continuous_refresh_needed(self):
        lib.scigma_gui_gl_window_continuous_refresh_needed()

    def continuous_refresh_not_needed(self):
        lib.scigma_gui_gl_window_continuous_refresh_not_needed()

    def stall(self):
        lib.scigma_gui_gl_window_stall(self.objectID)

    def draw_frame(self):
        lib.scigma_gui_gl_window_draw_frame(self.objectID)

    def request_redraw(self):
        lib.scigma_gui_gl_window_request_redraw(self.objectID)
  
    def flush(self):
        lib.scigma_gui_gl_window_flush(self.objectID)

    def add_drawable(self, drawable):
        if isinstance(drawable,Curve):
            lib.scigma_gui_gl_window_add_curve(self.objectID,drawable.objectID)
        elif isinstance(drawable,Picker):
            lib.scigma_gui_gl_window_add_picker(self.objectID,drawable.objectID)

    def remove_drawable(self, drawable):
        if isinstance(drawable,Curve):
            lib.scigma_gui_gl_window_remove_curve(self.objectID,drawable.objectID)
        if isinstance(drawable,Picker):
            lib.scigma_gui_gl_window_remove_picker(self.objectID,drawable.objectID)



