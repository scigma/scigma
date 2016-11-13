names=['red','green','blue','yellow',
        'pink','lime','azure','orange',
        'brown','forest','navy','teal',
        'rose','aqua','sky','beige',
        'black','darkGray','gray',
        'lightGray','white','cyan','magenta']

values={'red':[0.8125,0.0,0.0,1.0],
        'green':[0.0,0.6875,0.0,1.0],
        'blue':[0.0,0.0,0.8125,1.0],
        'yellow':[1.0,0.8125,0.0,1.0],
        'pink':[1.0,0.0,0.5,1.0],
        'lime':[0.625,1.0,0.0,1.0],
        'azure':[0.0,0.625,1.0,1.0],
        'orange':[1.0,0.4375,0.0,1.0],
        'brown':[0.4375,0.0,0.0,1.0],
        'forest':[0.0,0.4375,0.0,1.0],
        'navy':[0.0,0.0,0.4375,1.0],
        'teal':[0.0,0.5,0.5,1.0],
        'rose':[1.0,0.5625,0.5625,1.0],
        'aqua':[0.375,0.875,0.625,1.0],
        'sky':[0.5625,0.6875,1.0,1.0],
        'beige':[1.0,0.875,0.5625,1.0],
        'black':[0.0,0.0,0.0,1.0],
        'darkGray':[0.25,0.25,0.25,1.0],
        'gray':[0.5,0.5,0.5,1.0],
        'lightGray':[0.75,0.75,0.75,1.0],
        'white':[1.0,1.0,1.0,1.0],
        'cyan':[0.0,1.0,1.0,1.0],
        'magenta':[1.0,0.0,1.0,1.0]}

def from_string(colstring):
    if colstring.lower() in values:
        return values[colstring]
    else:
        try:
            index=int(colstring)
        except ValueError:
            raise Exception
        if index<len(names):
            return values[names[index]]
        else:
            raise Exception




