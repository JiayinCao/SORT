import bpy
from . import renderer
from . import preference
from .ui import ui_init

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao",
    "version": (0, 0, 1),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

def register():
    ui_init.register()

    bpy.utils.register_module(__name__)

def unregister():
    ui_init.unregister()

    bpy.utils.unregister_module(__name__)
