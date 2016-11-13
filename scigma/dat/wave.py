import sys
if sys.version_info.major == 2:
    version=2
elif sys.version_info.major == 3:
    version=3

from ctypes import *
from .. import lib

class Wave(object):
    """ Wrapper for Wave objects """

    lib.scigma_dat_create_wave.argtypes=[c_int]
    lib.scigma_dat_wave_push_back.argtypes=[c_int,POINTER(c_double),c_int]
    lib.scigma_dat_wave_data.restype=POINTER(c_double)

    def __init__(self, capacity = 0x1000, objectID=-1):
        if objectID != -1:
            self.objectID=objectID
        else:
            self.objectID = lib.scigma_dat_create_wave(int(capacity))

    def destroy(self):
        lib.scigma_dat_destroy_wave(self.objectID)

    def __str__(self):
        size=str(self.size())
        return 'Wave('+size+')'

    def __repr__(self):
        return self.__str__()

    def push_back(self, values):
        C_DoubleArrayType=c_double*len(values)
        cValuesArray=C_DoubleArrayType(*values)
        cValues=cast(cValuesArray,POINTER(c_double))
        nValues=len(values)
        self.lock()
        lib.scigma_dat_wave_push_back(self.objectID, cValues,nValues)
        self.unlock()
        
    def pop_back(self, nValues):
        self.lock()
        lib.scigma_dat_wave_pop_back(self.objectID,nValues)
        self.unlock()
                
    def lock(self):
        lib.scigma_dat_wave_lock(self.objectID)

    def unlock(self):
        lib.scigma_dat_wave_unlock(self.objectID)

    def size(self):
        self.lock()
        size= lib.scigma_dat_wave_size(self.objectID)
        self.unlock()
        return size
        
    def data(self):
        return lib.scigma_dat_wave_data(self.objectID)

    def __getitem__(self, key):
        length=self.size()
        if isinstance(key, slice):
            self.lock()
            data=self.data()
            if(version<3):
                result = [data[i] for i in xrange(*key.indices(length))]
            else:
                result = [data[i] for i in range(*key.indices(length))]
            self.unlock()
        elif isinstance( key, int ) :
            if key < 0 : #Handle negative indices
                key += length
            if key >= length or key <0:
                raise IndexError("The index (%d) is out of range."%key)
            self.lock()
            result=self.data()[key]
            self.unlock()
        else:
            raise TypeError("Invalid argument type.")
        return result;
