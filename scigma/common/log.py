from ctypes import *
from .. import lib

class Log(object):
    """ Wrapper for Log objects """
    
    lib.scigma_common_log_pop.restype=c_char_p

    SUCCESS=0
    FAIL=1
    DATA=2
    WARNING=3
    ERROR=4
    DEFAULT=5
    
    def __init__(self):
        self.objectID=lib.scigma_common_create_log()
        
    def destroy(self):
        lib.scigma_common_destroy_log(self.objectID)
        
    def pop(self):
        retval=lib.scigma_common_log_pop(self.objectID)
        retval=str(retval.decode())
        return int(retval[0]), str(retval[1:])

                                                                        
