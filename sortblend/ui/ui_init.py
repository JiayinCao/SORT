from . import ui_render
from . import ui_camera
from . import ui_lamp

def register():
    ui_render.register()
    ui_camera.register()
    ui_lamp.register()

def unregister():
    ui_render.unregister()
    #ui_camera.unregister()