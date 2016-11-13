import inspect
from ctypes import *
from .. import lib

C_FType=CFUNCTYPE(c_voidp,POINTER(c_double),POINTER(c_double))
C_F_pType=CFUNCTYPE(c_voidp,POINTER(c_double),POINTER(c_double),POINTER(c_double))
C_F_tType=CFUNCTYPE(c_voidp,c_double,POINTER(c_double),POINTER(c_double))
C_F_ptType=CFUNCTYPE(c_voidp,c_double,POINTER(c_double),POINTER(c_double),POINTER(c_double)) 

def get_names(defs,typestring):
    namestring=''
    if typestring in defs:
        names=defs[typestring]
        for i in range(len(names)):
            namestring=namestring+names[i]+'|'
    return create_string_buffer(bytes(namestring.strip('|').encode("ascii")))  

def get_function(defs,name,argspec):
    if len(argspec[0])==2:
        fType=C_FType
    elif len(argspec[0])==3 and argspec[0][0]=='t':
        fType=C_F_tType
    elif len(argspec[0])==3:
        fType=C_F_pType
    elif len(argspec[0])==4:
        fType=C_F_ptType
    else:
        raise Exception("cannot resolve function signature for 'f'")
    if name in defs:
        f = defs[name]
        spec=inspect.getargspec(f)
        if spec[0][0:-1]!=argspec[0][0:-1] or len(spec[0])!=len(argspec[0]):
            raise Exception("argument specification of '"+name+"' does not fit specification of 'f'")
        return fType(defs[name])
    else:
        return cast(None,fType)

def set_values(eqsys,defs,names,values):
    nameList = defs[names] if names in defs else None
    valueList = defs[values] if values in defs else None
    if not values and names:
        return
    if len(nameList)!=len(valueList):
        raise Exception(names + " and " + values +" must have equal length")
    for i in range(len(nameList)):
        eqsys.parse(nameList[i]+'='+str(valueList[i]))

class EquationSystem(object):
    """ Wrapper for the EquationSystem object.
    """    
    lib.scigma_num_create_equation_system_II.argtypes=[c_char_p,C_FType,C_FType,c_char_p,C_FType]
    lib.scigma_num_create_equation_system_III.argtypes=[c_char_p,c_char_p,C_F_pType,C_F_pType,C_F_pType,c_char_p,C_F_pType]
    lib.scigma_num_create_equation_system_IV.argtypes=[c_char_p,C_F_tType,C_F_tType,c_char_p,C_F_tType]
    lib.scigma_num_create_equation_system_V.argtypes=[c_char_p,c_char_p,C_F_ptType,C_F_ptType,C_F_ptType,c_char_p,C_F_ptType]
    lib.scigma_num_equation_system_time_stamp.restype=c_ulonglong
    lib.scigma_num_equation_system_parse.argtypes=[c_int,c_char_p]
    lib.scigma_num_equation_system_parse.restype=c_char_p
    lib.scigma_num_equation_system_set.argtypes=[c_int,c_char_p,c_double]
    lib.scigma_num_equation_system_set.restype=c_char_p
    lib.scigma_num_equation_system_get.argtypes=[c_int,c_char_p]
    lib.scigma_num_equation_system_get.restype=c_char_p
    lib.scigma_num_equation_system_time.restype=c_double
    lib.scigma_num_equation_system_variables.restype=c_char_p
    lib.scigma_num_equation_system_variable_names.restype=c_char_p
    lib.scigma_num_equation_system_variable_values.restype=POINTER(c_double)
    lib.scigma_num_equation_system_parameters.restype=c_char_p
    lib.scigma_num_equation_system_parameter_names.restype=c_char_p
    lib.scigma_num_equation_system_parameter_values.restype=POINTER(c_double)
    lib.scigma_num_equation_system_functions.restype=c_char_p
    lib.scigma_num_equation_system_function_names.restype=c_char_p
    lib.scigma_num_equation_system_function_values.restype=POINTER(c_double)
    lib.scigma_num_equation_system_constants.restype=c_char_p
    lib.scigma_num_equation_system_constant_names.restype=c_char_p
    lib.scigma_num_equation_system_constant_values.restype=POINTER(c_double)
    lib.scigma_num_equation_system_variable_definitions.restype=c_char_p
    lib.scigma_num_equation_system_function_definitions.restype=c_char_p
    lib.scigma_num_equation_system_constant_definitions.restype=c_char_p
    
    INTERNAL=0
    LIBRARY=1
    PYTHON=2
    SOURCE={'internal':INTERNAL,'library':LIBRARY,'python':PYTHON}
    
    def __init_from_script__(self,script):
        try:
            self.defs={}
            execfile(script,self.defs)
            varstring=get_names(self.defs,'v_names')
            parstring=get_names(self.defs,'p_names')
            funcstring=get_names(self.defs,'f_names')
            f=self.defs['f']
            argspec=inspect.getargspec(f)
            self.f=get_function(self.defs,'f',argspec)
            self.dfdx=get_function(self.defs,'dfdx',argspec) 
            self.dfdp=get_function(self.defs,'dfdp',argspec) 
            self.func=get_function(self.defs,'func',argspec)
            if len(argspec[0])==2:
                self.objectID=lib.scigma_num_create_equation_system_II(varstring,self.f,self.dfdx,funcstring,self.func)
            elif len(argspec[0])==3 and argspec[0][0]=='t':
                self.objectID=lib.scigma_num_create_equation_system_IV(varstring,self.f,self.dfdx,funcstring,self.func)
            elif len(argspec[0])==3:
                self.objectID=lib.scigma_num_create_equation_system_III(varstring,parstring,self.f,self.dfdx,self.dfdp,funcstring,self.func)
            elif len(argspec[0])==4:
                self.objectID=lib.scigma_num_create_equation_system_V(varstring,parstring,self.f,self.dfdx,self.dfdp,funcstring,self.func)
            set_values(self,self.defs,'v_names','v_values')
            set_values(self,self.defs,'p_names','p_values')
            if 't' in self.defs:
                lib.scigma_num_equation_system_set(self.objectID,create_string_buffer(bytes('t'.encode("ascii"))),self.defs['t'])
            else:
                lib.scigma_num_equation_system_set(self.objectID,create_string_buffer(bytes('t'.encode("ascii"))),0.0)
        except IOError:
            raise Exception(script+": file not found")
    
    def __init__(self, script=None, library=None):
        variables=None
        parameters=None
        functions=None
        if script:
            self.__init_from_script__(script)
        elif library:
            pass
        else:
            self.objectID = lib.scigma_num_create_equation_system()
    
    def destroy(self):
        lib.scigma_num_destroy_equation_system(self.objectID)
    
    def stall(self):
        lib.scigma_num_equation_system_stall(self.objectID)
    
    def flush(self):
        lib.scigma_num_equation_system_flush(self.objectID)
    
    def timestamp(self):
        return lib.scigma_num_equation_system_time_stamp(self.objectID)
    
    def parse(self,line):
        line=bytes(line.encode("ascii"))
        return str(lib.scigma_num_equation_system_parse(self.objectID, create_string_buffer(line)).decode())
    
    def time(self):
        return lib.scigma_num_equation_system_time(self.objectID)
    
    def n_vars(self):
        return lib.scigma_num_equation_system_n_variables(self.objectID)
    
    def n_pars(self):
        return lib.scigma_num_equation_system_n_parameters(self.objectID)
    
    def n_funcs(self):
        return lib.scigma_num_equation_system_n_functions(self.objectID)
    
    def n_consts(self):
        return lib.scigma_num_equation_system_n_constants(self.objectID)
    
    def vars(self):
        result=lib.scigma_num_equation_system_variables(self.objectID)
        return self.string_to_list(result)
    
    def pars(self):
        result=lib.scigma_num_equation_system_parameters(self.objectID)
        return self.string_to_list(result)
    
    def funcs(self):
        result=lib.scigma_num_equation_system_functions(self.objectID)
        return self.string_to_list(result)
    
    def consts(self):
        result=lib.scigma_num_equation_system_constants(self.objectID)
        return self.string_to_list(result)
    
    def var_names(self):
        result=lib.scigma_num_equation_system_variable_names(self.objectID)
        return self.string_to_list(result)
    
    def par_names(self):
        result=lib.scigma_num_equation_system_parameter_names(self.objectID)
        return self.string_to_list(result)
    
    def func_names(self):
        result=lib.scigma_num_equation_system_function_names(self.objectID)
        return self.string_to_list(result)
    
    def const_names(self):
        result=lib.scigma_num_equation_system_constant_names(self.objectID)
        return self.string_to_list(result)
    
    def var_vals(self):
        result=lib.scigma_num_equation_system_variable_values(self.objectID)
        return self.array_to_list(result,self.n_vars())
    
    def par_vals(self):
        result=lib.scigma_num_equation_system_parameter_values(self.objectID)
        return self.array_to_list(result,self.n_pars())
    
    def func_vals(self):
        result=lib.scigma_num_equation_system_function_values(self.objectID)
        return self.array_to_list(result,self.n_funcs())
    
    def const_vals(self):
        result=lib.scigma_num_equation_system_constant_values(self.objectID)
        return self.array_to_list(result,self.n_consts())
    
    def var_defs(self):
        result=lib.scigma_num_equation_system_variable_definitions(self.objectID)
        return self.string_to_list(result)
    
    def func_defs(self):
        result=lib.scigma_num_equation_system_function_definitions(self.objectID)
        return self.string_to_list(result)
    
    def const_defs(self):
        result=lib.scigma_num_equation_system_constant_definitions(self.objectID)
        return self.string_to_list(result)
    
    def is_autonomous(self):
        return lib.scigma_num_equation_system_is_autonomous(self.objectID)
    
    def string_to_list(self,result):
        result=str(result.decode()).split('|')
        try:
            result.remove('')
        except ValueError:
            pass
        return result
    
    def array_to_list(self,result,length):
        retval=[]
        for i in range(length):
            retval.append(result[i])
        return retval
