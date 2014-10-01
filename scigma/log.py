from ctypes import *
from . import lib

class Log(object):
    """ Wrapper for Log objects """

    lib.scigma_log_pop.restype=c_char_p

    def __init__(self):
        self.objectID=lib.scigma_create_log()

    def destroy(self):
        lib.scigma_destroy_log(self.objectID)

    def pop(self):
        retval=lib.scigma_log_pop(self.objectID)
        if not retval:
            return ""
        else:
            return str(retval.decode())

