import re
from ctypes import *
from .. import lib
from .constants import *
from .. import objects

lib.scigma_gui_graph_set_indexed_colors.argtype=[POINTER(c_float),c_int]
lib.scigma_gui_graph_rebuild_shaders.argtypes=[c_int,c_char_p,c_char_p]
lib.scigma_gui_graph_rebuild_shaders.restype=c_char_p

C_ColorListType=None

def set_indexed_colors(colors):
    nColors=len(colors)
    values=[item for sublist in colors for item in sublist]
    nValues=len(values)
    C_ColorListType=c_float*nValues
    c_colorList=C_ColorListType(*values)
    lib.scigma_gui_graph_set_indexed_colors(cast(c_colorList,POINTER(c_float)),nColors)

def zero_fill_expressions(expressions,instance):
    axes=instance.options['View']['axes'].label
    for i in range(N_COORDINATES):
        if VIEW_TYPE[axes]&COORDINATE_FLAG[COORDINATE_NAME[i]]:
            if expressions[i]=='':
                expressions[i]='0.0'
        else:
            if i==C_INDEX:
                expressions[i]=''
            else:
                expressions[i]='0.0'


GLSL_BUILT_IN_FUNCTIONS=["sin","cos","tan","asin","acos","atan","abs",
                           "mod","sign","step","pow","exp","log","sqrt"]

def is_variable(x):
    if x[0]>'0'and x[0]<='9':
        return False
    if x in GLSL_BUILT_IN_FUNCTIONS:
        return False
    return True

def independent_variables(expressions,variables):
    skip = (expressions[C_INDEX]=='')
    variables[:]=[]
    for j in range(N_COORDINATES):
        if j==C_INDEX and skip:
            continue
        exprlist = re.sub('[^0-9_.a-zA-Z]+', ' ', expressions[j]).split()   
        for expr in exprlist:
            try:
                x=float(expr)
            except:
                if is_variable(expr):
                    variables.append(expr)
    variables[:]=sorted(list(set(variables)))

def rebuild_shaders(instance):
    expressions=[]
    for i in range(N_COORDINATES):
        expressions.append(instance.options['View'][COORDINATE_NAME[i]])
    zero_fill_expressions(expressions,instance)
    independentVariables=[]
    independent_variables(expressions,independentVariables)
    arg1=''
    for x in expressions:
        arg1+='|'+x
    arg1=bytes(arg1.strip('|').encode("ascii"))
    
    arg2=''
    for x in independentVariables:
        arg2+='|'+x
    arg2=bytes(arg2.strip('|').encode("ascii"))
    
    error=lib.scigma_gui_graph_rebuild_shaders(instance.glWindow.objectID,
                                               create_string_buffer(arg1),
                                               create_string_buffer(arg2))
    if error:
        raise Exception(str(error.decode()))
    instance.plotVariables=independentVariables
    for key in instance.objects:
        for obj in instance.objects[key]:
            if obj['__visible__']:
                objects.set_view(obj,independentVariables)
    for obj in instance.cursorList:
        if obj['__visible__']:
            objects.set_view(obj,independentVariables)
    instance.glWindow.request_redraw()
