from .. import lib

class Navigator(object):
    """ Wrapper for Navigator objects.
    
    This class exposes the functionality of the Navigator class.
    Navigator instances are EventSinks (see 'events.h') for mouse
    and keyboard events that enable controlling the ViewingVolume 
    of a Window. Therefore, they must be connected to a GLWindow
    in order to work properly.

    """

    def __init__(self,view):
        self.objectID = lib.scigma_gui_create_navigator(view)

    def destroy(self):
        lib.scigma_gui_destroy_navigator(self.objectID)
