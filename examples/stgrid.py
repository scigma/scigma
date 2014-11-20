import math

def grid(window):
    for i in range(101):
        options.set("color",[1-0.02*abs(i-50),max(0,0.02*(i-50)),max(0,0.02*(50-i))])
        if i<=50:
            set("p",0.01*i)
            set("q",0.5+0.0075*i)
        else:
            set("p",0.01*i)
            set("q",1-0.001*i)
        plot(1000,noThread=True)
        wait(0)
        window.request_redraw()

grid(instance.glWindow)

