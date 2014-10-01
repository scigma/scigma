from ctypes import *
from .. import lib

class Wave(object):
    """ Wrapper for Wave objects """

    lib.scigma_dat_create_wave.argtypes=[c_int,POINTER(c_double),c_int,c_int]
    lib.scigma_dat_wave_data.restype=POINTER(c_double)

    def __init__(self, values=None, columns = None, lines = 0x1000):
        if not values:
            cValues=None
            nValues=0
            if not columns:
                columns=0
        else:
            C_DoubleArrayType=c_double*len(values)
            cValuesArray=C_DoubleArrayType(*values)
            cValues=cast(cValuesArray,POINTER(c_double))
            nValues=len(values)
            if not columns:
                columns=nValues
        self.objectID = lib.scigma_dat_create_wave(columns,cValues,nValues,lines)

    def destroy(self):
        lib.scigma_dat_destroy_wave(self.objectID)

    def __str__(self):
        return 'Wave('+str(self.columns())+'x'+str(self.rows())+')'

    def __repr__(self):
        return self.__str__()

    def columns(self):
        return lib.scigma_dat_wave_columns(self.objectID)

    def rows(self):
        return lib.scigma_dat_wave_rows(self.objectID)

    def lock(self):
        lib.scigma_dat_wave_lock(self.objectID)

    def unlock(self):
        lib.scigma_dat_wave_unlock(self.objectID)

    def data(self):
        return lib.scigma_dat_wave_data(self.objectID)

