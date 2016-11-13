windows=[]

def fetch(win=None):
    if win in windows:
        return win
    return windows[0]
