from . import ui_render
from . import ui_camera

def register():
    ui_render.register()
    ui_camera.register()

def unregister():
    ui_render.unregister()
    #ui_camera.unregister()