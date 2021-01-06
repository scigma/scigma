import string
from ctypes import *
from .. import common
from .. import lib

C_Color = c_float*4
C_Direction = c_float*3
C_CallbackType=CFUNCTYPE(None, c_char_p, c_void_p)

class Button(object):
    pass

class Separator(object):
    pass

class ATWPanel(object):
    """ Wrapper for ATWPanel objects.
    
    This class exposes the functionality of the ATWPanel class.
    To instatiate a ATWPanel object, use the create() method, to
    destroy it use destroy(), which is necessary to avoid memory
    leaks. 
    """

    lib.scigma_gui_create_atw_panel.argtypes=[c_int,c_char_p,C_CallbackType]
    lib.scigma_gui_atw_panel_add_bool.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_add_int.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_add_float.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_add_double.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_add_color.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_add_direction.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_add_string.argtypes=[c_int,c_char_p,c_bool,c_char_p]
    lib.scigma_gui_atw_panel_remove.argtypes=[c_int,c_char_p]
    lib.scigma_gui_atw_panel_set_bool.argtypes=[c_int,c_char_p,c_int]
    lib.scigma_gui_atw_panel_set_int.argtypes=[c_int,c_char_p,c_int]
    lib.scigma_gui_atw_panel_set_float.argtypes=[c_int,c_char_p,c_float]
    lib.scigma_gui_atw_panel_set_double.argtypes=[c_int,c_char_p,c_double]
    lib.scigma_gui_atw_panel_set_color.argtypes=[c_int,c_char_p,POINTER(c_float)]
    lib.scigma_gui_atw_panel_set_direction.argtypes=[c_int,c_char_p,POINTER(c_float)]
    lib.scigma_gui_atw_panel_set_string.argtypes=[c_int,c_char_p,c_char_p]
    lib.scigma_gui_atw_panel_add_separator.argtypes=[c_int,c_char_p,c_char_p]
    lib.scigma_gui_atw_panel_define.argtypes=[c_int,c_char_p]
    lib.scigma_gui_atw_panel_define_entry.argtypes=[c_int,c_char_p,c_char_p]

    def __init__(self,glWindow, title):
        self.c_callback=C_CallbackType(lambda id_ptr,entry_ptr: self.callback(id_ptr,entry_ptr)) 
        self.py_callback=None
        self.data={}
        self.objectID = lib.scigma_gui_create_atw_panel(glWindow.objectID,create_string_buffer(bytes(title.encode("ascii"))),self.c_callback)

    def destroy(self):
        lib.scigma_gui_destroy_atw_panel(self.objectID)

    def callback(self,id_ptr,entry_ptr):
        identifier=str(string_at(id_ptr).decode())
        parts=identifier.rpartition('.')
        d=common.dict_entry(parts[0],self.data)
        key=parts[2]
        oldval=d[key]
        if isinstance(oldval, bool):
            value=cast(entry_ptr,POINTER(c_int))
            d[key]=True if value[0]==1 else False
        elif isinstance(oldval,int):
            value=cast(entry_ptr,POINTER(c_int))
            d[key]=value[0]
        elif isinstance(oldval,common.Float):
            value=cast(entry_ptr,POINTER(c_float))
            d[key]=common.Float(value[0])
        elif isinstance(oldval,float):
            value=cast(entry_ptr,POINTER(c_double))
            d[key]=value[0]
        elif type(oldval) is list:
            value=cast(entry_ptr,POINTER(c_float))
            if len(oldval) == 3:   # direction
                d[key]=[value[0],value[1],value[2]]
            else:   # color
                d[key]=[value[0],value[1],value[2],value[3]]
        elif isinstance(oldval,str):
            value=str(cast(entry_ptr,c_char_p).value.decode())
            d[key]=value
        elif isinstance(oldval,common.Enum):
            newval=cast(entry_ptr,POINTER(c_int))
            for entry, value in oldval.definition.items():
                if value==newval[0]:
                    d[key].label=entry
        if self.py_callback:
            self.py_callback(identifier,d[key])

    def change(self,identifier,d,key,value):
        # this method does not perform any checks
        # whether identifier and/or d and key exist
        # and value has the correct type -> caller
        # is responsible for all that
        # it checks, however, if a numerical value
        # is within the min/max boundaries set by the user
        identifier=bytes(identifier.encode("ascii"))
        d[key]=value
        if isinstance(value,bool):
            if value:
                b=1
            else:
                b=0
            lib.scigma_gui_atw_panel_set_bool(self.objectID,create_string_buffer(identifier),b)
        elif isinstance(value,int):
            lib.scigma_gui_atw_panel_set_int(self.objectID,create_string_buffer(identifier),value)
        elif isinstance(value,common.Float):
            lib.scigma_gui_atw_panel_set_float(self.objectID,create_string_buffer(identifier),
                                                  c_float(value.value))
        elif isinstance(value,float):
            lib.scigma_gui_atw_panel_set_double(self.objectID,create_string_buffer(identifier),c_double(value))
        elif isinstance(value,str):
            lib.scigma_gui_atw_panel_set_string(self.objectID,create_string_buffer(identifier),
                                                   create_string_buffer(bytes(value.encode("ascii"))))
        elif isinstance(value,common.Enum):
            lib.scigma_gui_atw_panel_set_enum(self.objectID,create_string_buffer(identifier),value.definition[value.label])
        elif isinstance(value,list):
            if len(value)==3:
                d=C_Direction(value[0],value[1],value[2])
                lib.scigma_gui_atw_panel_set_direction(self.objectID,create_string_buffer(identifier),
                                                          cast(d,POINTER(c_float)))
            elif len(value)==4:
                c=C_Color(value[0],value[1],value[2],value[3])
                lib.scigma_gui_atw_panel_set_color(self.objectID,create_string_buffer(identifier),
                                                      cast(c,POINTER(c_float)))

    def add(self,identifier,value,forward=True,defs=""):
        path=identifier.split('.')
        parent = self.data
        alreadyDefined=True
        for child in path:
            if not child in parent:
                alreadyDefined=False
                parent[child]={}
            d = parent   
            parent = parent[child]
        if alreadyDefined:
            raise Exception("identifier " + identifier + " already in use")
        key = child
        idascii=bytes(identifier.encode("ascii"))
        defs=bytes(defs.encode("ascii"))   
        if isinstance(value,bool):
            lib.scigma_gui_atw_panel_add_bool(self.objectID,create_string_buffer(idascii),
                                                 forward, create_string_buffer(defs))
        elif isinstance(value,int):
            lib.scigma_gui_atw_panel_add_int(self.objectID,create_string_buffer(idascii),
                                                forward, create_string_buffer(defs))
        elif isinstance(value,common.Float):
            lib.scigma_gui_atw_panel_add_float(self.objectID,create_string_buffer(idascii),
                                                  forward, create_string_buffer(defs))
        elif isinstance(value,float):
            lib.scigma_gui_atw_panel_add_double(self.objectID,create_string_buffer(idascii),
                                                   forward, create_string_buffer(defs))
        elif isinstance(value,str):
            lib.scigma_gui_atw_panel_add_string(self.objectID,create_string_buffer(idascii),
                                                   forward, create_string_buffer(defs))
        elif isinstance(value,common.Enum):
            labels = ''
            n=0
            for entry in value.definition:
                labels=labels+'|'+entry
                n=n+1
            labels=labels.strip('|')
            values=(c_int*n)()
            i=0
            for entry in value.definition:
                values[i]=value.definition[entry]
                i=i+1
            lib.scigma_gui_atw_panel_add_enum(self.objectID,create_string_buffer(idascii),
                                              create_string_buffer(labels.encode("ascii")),
                                              cast(values,POINTER(c_int)),
                                              forward, create_string_buffer(defs))
        elif isinstance(value,list):
            if len(value)==3:
                try:
                    value=[float(value[0]),float(value[1]),float(value[2])]
                except TypeError:
                    raise Exception("direction must be of type [float,float,float]")
                lib.scigma_gui_atw_panel_add_direction(self.objectID,create_string_buffer(idascii),
                                                          forward,create_string_buffer(defs))
            elif len(value)==4:
                try:
                    value=[float(value[0]),float(value[1]),float(value[2]),float(value[3])]
                except TypeError:
                    raise Exception("color must be of type [float,float,float,float]")
                lib.scigma_gui_atw_panel_add_color(self.objectID,create_string_buffer(idascii),
                                                      forward,create_string_buffer(defs))
            else:
                raise Exception("expected list with 3 (direction) or 4 (color) elements")
        elif isinstance(value,Button):
            lib.scigma_gui_atw_panel_add_button(self.objectID,create_string_buffer(idascii),
                                                   create_string_buffer(defs))
        elif isinstance(value,Separator):
            lib.scigma_gui_atw_panel_add_separator(self.objectID,create_string_buffer(idascii),
                                                   create_string_buffer(defs))
        else:
            raise Exception("expected bool, int, common.Float, float, Direction, Color, str, Enum, Button or Separator")
        self.change(identifier,d,key,value)

    def get(self, identifier):
        entry=common.dict_single_entry(identifier,self.data,"entry")
        if isinstance(entry,common.Float):
            return entry.value
        elif isinstance(entry,common.Enum):
            return entry.label
        elif isinstance(entry,Button) or isinstance(entry,Separator):
            return None
        else:
            return entry

    def set(self, identifier, value):
        path=common.dict_single_path(identifier,self.data)

        parts=path.rpartition('.')
        d=common.dict_entry(parts[0],self.data)
        key=parts[2]
        oldval=d[key]
        if not type(oldval) == type(value):
            if type(oldval) == common.Float:
                if isinstance(value,int) or isinstance(value,float):
                    value=common.Float(value)
                else:
                    raise Exception(key+": excpected " + str(type(oldval)))
            elif type(oldval) == common.Enum:
                if isinstance(value,str):
                    if value in oldval.definition:
                        value = common.Enum(oldval.definition,value)
                    else:
                        raise Exception(key+": value "+value+" not in Enum")
        if isinstance(oldval,list):
            if len(oldval)==3:
                try:
                    value=[float(value[0]),float(value[1]),float(value[2])]
                except TypeError:
                    raise Exception(key+": direction must be of type [float,float,float]")
            elif len(oldval)==4:
                try:
                    if len(value)==4:
                        value=[float(value[0]),float(value[1]),float(value[2]),float(value[3])]
                    else:
                        value=[float(value[0]),float(value[1]),float(value[2]),1.0]
                except TypeError:
                    raise Exception(key+": color must be of type [float,float,float,float]")
        elif isinstance(value,common.Enum):
            if not oldval.definition is value.definition:
                raise Exception("wrong type of Enum for "+key)
        self.change(path,d,key,value)

    def remove(self, identifier):
        path=common.dict_single_path(identifier,self.data)
        entry=common.dict_entry(path,self.data)

        parts=path.rpartition('.')
        dname=None
        name=parts[2]
        d0=None # node two levels above
        d1=common.dict_entry(parts[0],self.data) # node one level above
        if not parts[0] == '':
            parts=parts[0].rpartition('.')
            dname=parts[2]
            d0=common.dict_entry(parts[0],self.data)

        if isinstance(entry,dict): # if entry is a node, 
            for key in list(entry): # recursively delete subnodes
                self.remove(identifier+'.'+key)
        else: # if entry is not a node, remove it from the TWBar
            lib.scigma_gui_atw_panel_remove(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))))
        #remove entry from dictionary one level higher
        del d1[name]
        #if dictionary one level higher is empty, delete the whole group
        if not d1 and dname:
            del d0[dname]

    def add_separator(self, identifier,defs=''):
        lib.scigma_gui_atw_panel_add_separator(self.objectID,
                                               create_string_buffer(bytes(identifier.encode("ascii"))),
                                               create_string_buffer(bytes(defs.encode("ascii"))))

    def define(self, identifier, defs):
        if(identifier==''):
            lib.scigma_gui_atw_panel_define(self.objectID,create_string_buffer(bytes(defs.encode("ascii"))))
        else:
            lib.scigma_gui_atw_panel_define_entry(self.objectID,
                                                  create_string_buffer(bytes(identifier.encode("ascii"))),
                                                  create_string_buffer(bytes(defs.encode("ascii"))))

    def set_callback(self,function):
        self.py_callback=function

