from . import ui_render
from . import ui_camera
from . import ui_lamp
from . import ui_material

def register():
    ui_render.register()
    ui_camera.register()
    ui_lamp.register()
    ui_material.register()

def unregister():
    ui_render.unregister()
    ui_lamp.unregister()
    ui_camera.unregister()
    ui_material.unregister()