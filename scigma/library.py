from sys import platform
from inspect import getfile
from ctypes import CDLL,c_float,c_bool
import os
from . import default
""" Load shared library:"""
if platform=='linux2':
    sep='/'
    libfile="libscigma.so"
elif platform=='darwin':
    sep='/'
    libfile="libscigma.dylib"
elif platform=='win32':
    sep='\\'
    libfile="libscigma.dll"
else:
    quit()

libpath=getfile(default).rpartition(sep)[0]
path=os.getcwd()
try:
    os.chdir(libpath)
    lib=CDLL(libfile)
    os.chdir(path)
except:
    os.chdir(path)
    lib=CDLL(libpath+sep+libfile)

largeFontsFlag=False
c_bool.in_dll(lib,"LARGE_FONTS_FLAG").value=largeFontsFlag
