from . import gui
from . import graphs
from . import view
from . import tkfile

def generic_popup(labels,commands,separators,win):
    x=gui.tkroot.winfo_pointerx()
    y=gui.tkroot.winfo_pointery()

    menu=gui.tk.Menu(gui.tkroot, tearoff=0)
    for i in range(len(commands)):
        if i in separators:
            menu.add_separator()
        menu.add_command(label=labels[i],command=commands[i])

    menu.tk_popup(x,y)

def window(win):
    labels=["fit",
            "clear",
            "reset",
            "load...", 
            "save history...",
            "save session..."]
    commands=[lambda:view.fit(None, win),
              lambda:graphs.clear(win),
              lambda:win.commands['reset'](win),
              lambda:win.commands['load'](win=win),
              lambda:win.commands['history'](win=win),
              lambda:win.commands['session'](win=win)]

    separators = [1,3,4,6]

    generic_popup(labels, commands, separators, win)
    
def graph(g,extra_labels,extra_commands,extra_separators,win):
    labels1=["fit","hide","replay","delete"];
    labels2=["save","save as..."];
    commands1=[lambda:view.fit(g['identifier'],win),
               lambda:graphs.hide(g['identifier'],win),
               lambda:graphs.replay(g['identifier'],win),
               lambda:graphs.delete(g['identifier'],win)]
    commands2=[lambda:graphs.save(g,'',win),
               lambda:graphs.save(g,tkfile.asksaveasfilename(initialfile=g['identifier']+".dat"),win)]

    labels = labels1+extra_labels+labels2
    commands = commands1+extra_commands+commands2
    separators = [1,3]+[i+4 for i in extra_separators]+[len(commands)-2]

    generic_popup(labels,commands,separators,win)

def plug(win):
    if not win.register_plugin('popups', lambda:unplug(win), {}):
        return

    if gui.tk:
        mouse=gui.Mouse(lambda: window(win))
        win.glWindow.connect(mouse)
        setattr(win,'mouse',mouse)

def unplug(win):
    if not win.unregister_plugin('popups'):
        return

    if gui.tk:
        win.glWindow.disconnect(win.mouse)
        win.mouse.destroy()
        delattr(win,'mouse')
