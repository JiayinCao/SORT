import bpy
from . import renderer
from . import preference

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
    # Register other components
    preference.register()
    renderer.register()

def unregister():
    # Unregister preference
    renderer.unregister()
    preference.unregister()
