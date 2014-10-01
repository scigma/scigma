class Float(object):
    """ Wrapper for a float (not double) value

    this class is used to distinguish between different
    data types on the C++ side of the program. Float 
    instances are passed to C++ as float, normal Python
    floats are passed as double

    """
    def __init__(self,value):
        self.value=value

    def __str__(self):
        return str(self.value)

    def __repr__(self):
        return str(self.value)

def dict_entry(path, dictionary):
    """ retrieves an entry from a dict 

    identifier is a string of the form 'dir1.dir2.[...].dirN.entry',
    the return value is dictionary[dir1][dir2][...][dirN][entry], or None,
    if no such entry exists (can also be used to retrieve sub-dictionaries)

    """
    if path=='':
        return dictionary
    path=path.split('.')
    parent = dictionary
    for child in path:
        try:
            if not child in parent:
                return None
        except TypeError:
            return None
        parent = parent[child]
    return parent

def dict_full_paths(identifier,parent,results,dir=''):
    """ retrieves full paths for dict entries

    identifier is a string of the form 'dir1.dir2.[...].dirN.entry'
    parent is a dictionary
    dir is a prefix that is added to the returned path(s)
    results is a list of strings of the form 'dir.dir1.dir2.[...].dirN.entry'
    that contains all entries of parent with the hierarchy
    parent[arbitrary1][...][arbitraryM][dir1][dir2][...][dirN][entry] with
    the LOWEST possible depth (i.e. only the entries with the smallest value
    of M are returned)
    Example: fullpath('x.min',params,results,dir='root.params')
    called on 
    params={'axes':{'x':{min:-1,max:1},'y':{min:-1,max:1}},
            'limits':{'x':{min:-1000,max:1000},'t':{min:0,max:1000}},
            'variables':{'double':{'x':{min:-10,max:10}}}}
    returns ['root.params.axes.x.min','root.params.limits.x.min'],
    but does not include 'root.params.variables.double.x.min'
    """
    parts=identifier.partition('.')
    current=[]
    deeper=[]
    for key in parent:
        subdir=dir+'.'+key
        if key == parts[0]:
            if parts[1] is '':
                if not isinstance(parent[key],dict):
                    current.append(subdir.strip('.'))
            else:
                dict_full_paths(parts[2],parent[key],current,subdir)
        else:
            if isinstance(parent[key],dict):
                dict_full_paths(identifier,parent[key],deeper,subdir)
    # if there are results on the current level, ignore deeper dictionaries
    if len(current)!=0:
        results.extend(current)
    else:
        results.extend(deeper)
