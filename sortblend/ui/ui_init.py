import bpy
from . import ui_render
from . import ui_camera
from . import ui_lamp
from . import ui_material

def get_panels():
    types = bpy.types
    panels = [
        "DATA_PT_lens",
        "DATA_PT_camera",
        "RENDER_PT_dimensions",
        ]

    return [getattr(types, p) for p in panels if hasattr(types, p)]

def register():
    for panel in get_panels():
        panel.COMPAT_ENGINES.add('sortblend')
        
def unregister():
    for panel in get_panels():
        panel.COMPAT_ENGINES.add('sortblend')