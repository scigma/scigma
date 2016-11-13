from ctypes import *
from .. import lib

C_CallbackType=CFUNCTYPE(None, c_char_p)

class Console(object):
    """ Wrapper for Console objects.
    
    This class exposes the functionality of the Console class.
    Console instances are EventSinks (see 'events.h') for keyboard
    events. Therefore, they must be connected to a to a GLWindow
    in order to work. 
    """

    lib.scigma_gui_create_console.argtypes=[c_int,c_bool,C_CallbackType]
    lib.scigma_gui_console_write.argtypes=[c_int,c_char_p]
    lib.scigma_gui_console_write_data.argtypes=[c_int,c_char_p]
    lib.scigma_gui_console_write_warning.argtypes=[c_int,c_char_p]
    lib.scigma_gui_console_write_comment.argtypes=[c_int,c_char_p]
    lib.scigma_gui_console_write_error.argtypes=[c_int,c_char_p]
    lib.scigma_gui_console_set_theme.argtypes=[c_int,c_int]
    
    def __init__(self,glWindow,largeFontsFlag):
        self.c_callback=C_CallbackType(lambda line_ptr: self.callback(line_ptr)) 
        self.py_callback=None
        self.objectID = lib.scigma_gui_create_console(glWindow.objectID,largeFontsFlag,self.c_callback)

    def destroy(self):
        lib.scigma_gui_destroy_console(self.objectID)

    def callback(self,line_ptr):
        if self.py_callback:
            line=str(string_at(line_ptr).decode())
            self.py_callback(line)

    def set_callback(self,function):
        self.py_callback=function
 
    def lines(self,l):
        lib.scigma_gui_console_set_displayed_screen_lines(self.objectID,l)

    def fadeout(self,yesNo):
        lib.scigma_gui_console_set_history_fadeout(self.objectID, 1 if yesNo else 0)

    def write(self,text):
        text=bytes(str(text).encode("ascii"))
        lib.scigma_gui_console_write(self.objectID, create_string_buffer(text))

    def write_data(self,text):
        text=bytes(str(text).encode("ascii"))
        lib.scigma_gui_console_write_data(self.objectID, create_string_buffer(text))

    def write_warning(self,text):
        text=bytes(str(text).encode("ascii"))
        lib.scigma_gui_console_write_warning(self.objectID, create_string_buffer(text))

    def write_comment(self,text):
        text=bytes(str(text).encode("ascii"))
        lib.scigma_gui_console_write_comment(self.objectID, create_string_buffer(text))
        
    def write_error(self,text):
        text=bytes(str(text).encode("ascii"))
        lib.scigma_gui_console_write_error(self.objectID, create_string_buffer(text))

    def set_theme(self,theme):
        lib.scigma_gui_console_set_theme(self.objectID,theme)
