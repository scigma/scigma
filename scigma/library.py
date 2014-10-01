from sys import platform
from inspect import getfile
from ctypes import CDLL,c_float,c_bool
import os
from . import default
""" Load shared library:"""
if platform=='linux2':
    libpath=getfile(default).rpartition('/')[0]
    libfile="libscigma.so"
elif platform=='darwin':
    libpath=getfile(default).rpartition('/')[0]
    libfile="libscigma.dylib"
elif platform=='win32':
    libpath=getfile(default).rpartition('\\')[0]
    libfile="libscigma.dll"
else:
    quit()

path=os.getcwd()
os.chdir(libpath)
print(libpath)
print(libfile)
lib=CDLL(libfile)
os.chdir(path)

largeFontsFlag=False
c_bool.in_dll(lib,"LARGE_FONTS_FLAG").value=largeFontsFlag
