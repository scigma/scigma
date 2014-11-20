import math

def grid():
    for i in range(41):
        options.set("color",[1-0.05*abs(i-20),max(0,0.05*(i-20)),max(0,0.05*(20-i))])
        if i<=20:
            set("p",0.025*i)
            set("q",0.5+0.0125*i)
        else:
            set("p",0.025*i)
            set("q",0)
        plot(2000,noThread=True)
    wait(0)

l=0
window=instance.glWindow
console=instance.console
for i in range(31):
    set("lambda",l)
    window.stall()
    clear()
    grid()
    console.write("lambda = ")
    console.write_data(str(l)+"\n")
    window.draw_frame()
    window.flush()
    l+=0.1
