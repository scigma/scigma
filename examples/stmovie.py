import math

def grid():
    for i in range(41):
        options.set("color",[1-0.05*abs(i-20),max(0,0.05*(i-20)),max(0,0.05*(20-i))])
        if i<=20:
            picking.set("p",0.025*i)
            picking.set("q",0.5+0.0125*i)
        else:
            picking.set("p",0.025*i)
            picking.set("q",0)
        iteration.plot(1000)
        
l=0
window=win.glWindow
console=win.console
options.set("threads","off")
for i in range(31):
    picking.set("lambda",l)
    window.stall()
    graphs.clear(win)
    grid()
    console.write("lambda = ")
    console.write_data(str(l)+"\n")
    win.process_messages()
    gui.sleep(0.01)
    window.flush()
    l+=0.1
options.set("threads","on")
