class Enum(object):
    def __init__(self,definition={},label=None):
        self.definition=definition
        self.label=label   
    def __str__(self):
        return self.label
    def __repr__(self):
        return self.label

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


def stdisnode(entry):
    return isinstance(entry,dict)

def stdisleaf(entry):
    return not isinstance(entry,dict)
    
def dict_enter(path, dictionary, element):
    if not path:
        raise Exception("dict_enter: path cannot be empty")
    if dict_entry(path,dictionary):
        raise Exception("dict_enter: "+path+" is already in use, choose different name")

    path=path.split('.')
    parent = dictionary
    for child in path:
        if child not in parent:
            parent[child]={}
        d = parent   
        parent = parent[child]
    key = child
    d[key]=element
    
def dict_entry(path, dictionary, isnode=stdisnode):
    """ retrieves an entry from a dict 

    path is a string of the form 'dir1.dir2.[...].dirN.entry',
    the return value is dictionary[dir1][dir2][...][dirN][entry], or None,
    if no such entry exists (can also be used to retrieve sub-dictionaries)

    """
    if path=='':
        return dictionary
    path=path.split('.')
    parent = dictionary
    stop=False
    for child in path:
        if stop:
            return None
        try:
            if not child in parent:
                return None
        except TypeError:
            return None
        parent = parent[child]
        if not isnode(parent):
            stop=True
    return parent


def dict_full_paths(identifier,parent,results,isnode=stdisnode,dir=''):
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
    (can also be used to retrieve subdirectories)
    """
    parts=identifier.partition('.')
    current=[]
    deeper=[]

    for key in parent:
        subdir=dir+'.'+key
        if key == parts[0]:
            if parts[1] == '':
                current.append(subdir.strip('.'))
            elif isnode(parent[key]):
                dict_full_paths(parts[2],parent[key],current,isnode,subdir)
        elif isnode(parent[key]):
            dict_full_paths(identifier,parent[key],deeper,isnode,subdir)
    # if there are results on the current level, ignore deeper dictionaries
    if len(current)!=0:
        results.extend(current)
    else:
        deeper=sorted(deeper,key=lambda item:item.count('.'))
        deeper=[item for item in deeper if item.count('.')==deeper[0].count('.')]
        results.extend(deeper)

def dict_single_path(identifier,parent,typeid='path', isnode=stdisnode):
    paths=[]

    dict_full_paths(identifier,parent,paths,isnode)
    if len(paths) == 0:
        raise Exception(identifier+": "+ typeid+" not found")
    elif len(paths) != 1:
        error = "ambiguous "+typeid+"; use qualified name, e.g. " + paths[0] + " or " + paths[1]
        raise Exception(error)
    return paths[0]
        
def dict_single_entry(identifier,parent,typeid='entry',isnode=stdisnode):
    return dict_entry(dict_single_path(identifier,parent,typeid,isnode),parent)
        
def dict_leaves(parent,results,isnode=stdisnode, isleaf=stdisleaf):
    if isleaf(parent):
        results.append(parent)
        return
    if isnode(parent):
        for key in parent:
            dict_leaves(parent[key],results,isnode,isleaf)
            

            
