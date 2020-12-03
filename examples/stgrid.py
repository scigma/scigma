import math

def grid(win):
    for i in range(101):
#        win.glWindow.stall()
        options.set("color",[1-0.02*abs(i-50),max(0,0.02*(i-50)),max(0,0.02*(50-i))])
        if i<=50:
            picking.set("p",0.01*i)
            picking.set("q",0.5+0.0075*i)
        else:
            picking.set("p",0.01*i)
            picking.set("q",1-0.001*i)
        iteration.plot(1000)
        gui.pause(0.01)
#        win.glWindow.flush()

options.set("threads","off")
graphs.clear(win)
grid(win)
options.set("threads","on")
