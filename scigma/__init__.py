import os
from .library import lib
from .library import largeFontsFlag
from .log import Log
from .instance import Instance
from . import options
from .commands import *
from . import default 

# do not know why this is necessary,
# but on my machine cwd gets altered
# during some ctypes call
cwd = os.getcwd()
default.instance=Instance()
os.chdir(cwd)

#initial compilation of shaders
expr('x','x',default.instance);

