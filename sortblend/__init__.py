import bpy
from . import renderer
from . import preference
from . import nodes
from . import material
from extensions_framework import Addon

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao",
    "version": (0, 0, 1),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

SORTAddon = Addon(bl_info)
addon_register, addon_unregister = SORTAddon.init_functions()

from .ui import ui_init

def register():
    addon_register()

    bpy.utils.register_module(__name__)

    ui_init.register()
    material.register()
    nodes.register()



def unregister():
    addon_unregister()

    bpy.utils.unregister_module(__name__)

    ui_init.unregister()
    material.unregister()
    nodes.unregister()

