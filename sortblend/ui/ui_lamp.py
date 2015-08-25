import bpy
import bl_ui
from .. import common

def register():
    # register UI component
    bl_ui.properties_data_lamp.DATA_PT_lamp.COMPAT_ENGINES.add(common.renderer_bl_name)