import os
from ctypes import *
from .. import lib

C_CallbackType=CFUNCTYPE(None)

lib.scigma_gui_application_set_loop_callback.argtypes=[C_CallbackType]
lib.scigma_gui_application_set_idle_callback.argtypes=[C_CallbackType]
lib.scigma_gui_application_loop.argtypes=[c_double]

py_loop_callbacks=[]
py_idle_callbacks=[]

def loop_callback():
    for f in py_loop_callbacks:
        f()

def idle_callback():
    for f in py_idle_callbacks:
        f()

c_loop_callback=C_CallbackType(loop_callback)
c_idle_callback=C_CallbackType(idle_callback)

def loop(seconds):
    lib.scigma_gui_application_loop(seconds)

def break_loop():
    lib.scigma_gui_application_break_loop()

def add_loop_callback(func):
    if py_loop_callbacks==[]:
        lib.scigma_gui_application_set_loop_callback(c_loop_callback)
    if not func in py_loop_callbacks:
        py_loop_callbacks.append(func)

def remove_loop_callback(func):
    if func in py_loop_callbacks:
        py_loop_callbacks.remove(func)
    if py_loop_callbacks==[]:
        lib.scigma_gui_application_set_loop_callback.argtypes=[c_void_p]
        lib.scigma_gui_application_set_loop_callback(None)
        lib.scigma_gui_application_set_loop_callback.argtypes=[C_CallbackType]

def add_idle_callback(func):
    if py_idle_callbacks==[]:
        lib.scigma_gui_application_set_idle_callback(c_idle_callback)
    if not func in py_idle_callbacks:
        py_idle_callbacks.append(func)

def remove_idle_callback(func):
    if func in py_idle_callbacks:
        py_idle_callbacks.remove(func)
    if py_idle_callbacks==[]:
        lib.scigma_gui_application_set_loop_callback.argtypes=[c_void_p]
        lib.scigma_gui_application_set_loop_callback(None)
        lib.scigma_gui_application_set_loop_callback.argtypes=[C_CallbackType]

