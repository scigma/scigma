from ctypes import *
from .. import lib
from .wave import Wave
from .iwave import IWave
from .bwave import BWave

class Mesh(object):
    """ Wrapper for Mesh objects """

    lib.scigma_dat_create_mesh.argtypes=[c_int,c_int, POINTER(c_double)]
    lib.scigma_dat_mesh_n_layers.restype=c_int
    lib.scigma_dat_mesh_n_points.restype=c_int
    
    def __init__(self, nDim, initial):
        if nDim==0:
            self.objectID = lib.scigma_dat_extend_mesh(initial.objectID)
        else:
            C_DoubleArrayType=c_double*len(initial)
            cValuesArray=C_DoubleArrayType(*initial)
            cValues=cast(cValuesArray,POINTER(c_double))
            nInitial=len(initial)//nDim
            self.objectID = lib.scigma_dat_create_mesh(nDim,nInitial,cValues)

    def destroy(self):
        lib.scigma_dat_destroy_mesh(self.objectID)

    def extend(self):
        return Mesh(0,self)

    def points_and_normals(self):
        return Wave(objectID = lib.scigma_dat_mesh_points_and_normals(self.objectID))

    def triangle_indices(self):
        return IWave(objectID = lib.scigma_dat_mesh_triangle_indices(self.objectID))

    def iso_indices(self):
        return IWave(objectID = lib.scigma_dat_mesh_iso_indices(self.objectID))

    def iso_end_points(self):
        return BWave(objectID = lib.scigma_dat_mesh_iso_end_points(self.objectID))
    
    def __str__(self):
        return 'Mesh('+str(lib.scigma_dat_mesh_n_points(self.objectID)) + 'p, '+str(lib.scigma_dat_mesh_n_layers(self.objectID))+'l)'

    def __repr__(self):
        return self.__str__()

