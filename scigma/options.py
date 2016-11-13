from . import gui
from . import common
from . import windowlist
from . import color

def get(identifier, win=None):
    win = windowlist.fetch(win)
    path=common.dict_single_path(identifier,win.options,"option")

    parts=path.partition('.')
    panel=win.optionPanels[parts[0]]
    identifier=parts[2]
    return panel.get(identifier)

def get_string(identifier,win=None):
    result=get(identifier,win)
    if isinstance(result,list):
        return ' '.join([str(x) for x in result])
    else:
        return str(result)

def set(identifier, value, win=None):
    win = windowlist.fetch(win)
    path=common.dict_single_path(identifier,win.options,"option")
    
    parts=path.partition('.')
    panel=win.optionPanels[parts[0]]
    identifier=parts[2]
    parts=identifier.rpartition('.')
    d=common.dict_entry(parts[0],panel.data)
    key=parts[2]
    oldval=d[key]
    if isinstance(value,int):
        if isinstance(oldval,list):
            if len(oldval)==4:
                value=color.values[color.names[value]]
    if isinstance(value,str):
        if isinstance(oldval,bool):
            if value.lower()=='true':
                value=True
            elif value.lower()=='false':
                value=False
            else:
                raise Exception(key+" must be 'true' or 'false'")
        elif isinstance(oldval, int):
            try:
                value=int(value)
            except ValueError:
                raise Exception(key+" must be an integer")
        elif isinstance(oldval,common.Float):
            try:
                value=common.Float(float(value))
            except ValueError:
                raise Exception(key+" must be a float")
        elif isinstance(oldval,float):
            try:
                value=float(value)
            except ValueError:
                raise Exception(key+ " must be a float")
        elif isinstance(oldval,list):
            value=value.split()
            if len(oldval)==4:
                if len(value)==1:
                    try:
                        colstring=value[0]
                        value=color.from_string(colstring)
                    except Exception:
                        raise Exception(key +": color "+value[0]+" not recognized")
                elif len(value)==3:
                    try:
                        value=[float(value[0]),float(value[1]),float(value[2]),1.0]
                    except ValueError:
                        raise Exception("could not read rgb values for "+key)
                elif len(value)==4:
                    try:
                        value=[float(value[0]),float(value[1]),float(value[2]),float(value[3])]
                    except ValueError:
                        raise Exception("could not read rgba values for "+key)
                else:
                    raise Exception(key+ ": expected color name, number or rgb(a) bbvalues")
            else:
                if len(value)==3:
                    try:
                        value=[float(value[0]),float(value[1]),float(value[2])]
                    except ValueError:
                        raise Exception("could not read xyz values for "+key)
                    else:
                        raise Exception(key+": expected xyz values")
        elif isinstance(oldval,common.Enum):
            newval=common.Enum(oldval.definition)
            for label in oldval.definition:
                if value.lower()==label.lower():
                    newval.label=label
            if not newval.label:
                raise Exception(value+ " is not an allowed enum value for "+key)
            else:
                value=newval
        elif isinstance(oldval,gui.Button) or isinstance(oldval, gui.Separator):
            return
        
    panel.set(identifier,value)
