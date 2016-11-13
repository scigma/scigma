from ctypes import *
from .. import lib
from . import Float, Enum

class Blob(object):
    """ Wrapper for Blob objects """

    lib.scigma_common_blob_set_bool.argtypes=[c_int,c_char_p,c_int]
    lib.scigma_common_blob_set_int.argtypes=[c_int,c_char_p,c_int]
    lib.scigma_common_blob_set_float.argtypes=[c_int,c_char_p,c_float]
    lib.scigma_common_blob_set_double.argtypes=[c_int,c_char_p,c_double]
    lib.scigma_common_blob_set_color.argtypes=[c_int,c_char_p,POINTER(c_float)]
    lib.scigma_common_blob_set_direction.argtypes=[c_int,c_char_p,POINTER(c_float)]
    lib.scigma_common_blob_set_string.argtypes=[c_int,c_char_p,c_char_p]

    def __init__(self, data=None):
        self.objectID=lib.scigma_common_create_blob()
        if not data:
            return
        else:
            self.set_from_dict(data,"")
        
    def destroy(self):
        lib.scigma_common_destroy_blob(self.objectID)

    def set_from_dict(self,data,identifier):
        for key in data:
            idf = key if identifier == "" else identifier+"."+key
            if isinstance(data[key], dict):
                self.set_from_dict(data[key],idf);
            else:
                self.set(idf, data[key]);
    
    def set(self, identifier, value):
        if isinstance(value,bool):
            if value:
                b=1
            else:
                b=0
            lib.scigma_common_blob_set_bool(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),b)
        elif isinstance(value,int):
            lib.scigma_common_blob_set_int(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),value)
        elif isinstance(value,Float):
            lib.scigma_common_blob_set_float(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),
                                                  c_float(value.value))
        elif isinstance(value,float):
            lib.scigma_common_blob_set_double(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),c_double(value))
        elif isinstance(value,str):
            lib.scigma_common_blob_set_string(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),
                                                   create_string_buffer(bytes(value.encode("ascii"))))
        elif isinstance(value,Enum):
            lib.scigma_common_blob_set_int(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),value.definition[value.label])
        elif isinstance(value,list):
            if len(value)==3:
                d=C_Direction(value[0],value[1],value[2])
                lib.scigma_common_blob_set_direction(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),
                                                          cast(d,POINTER(c_float)))
            elif len(value)==4:
                c=C_Color(value[0],value[1],value[2],value[3])
                lib.scigma_common_blob_set_color(self.objectID,create_string_buffer(bytes(identifier.encode("ascii"))),
                                                      cast(c,POINTER(c_float)))
