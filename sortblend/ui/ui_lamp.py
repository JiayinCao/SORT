import bpy
import bl_ui
from .. import common

class SORTLampPanel(bl_ui.properties_data_lamp.DataButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    COMPAT_ENGINES = {common.default_bl_name}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class LampPanel(SORTLampPanel, bpy.types.Panel):
    bl_label = 'Lamp Property'
    
    # sampler count
    bpy.types.Scene.sampler_count_prop = bpy.props.IntProperty(name='Count',default=1, min=1)
    
    def draw(self, context):
        lamp = context.lamp
        if lamp is not None:
            layout = self.layout
            
            # lamp type
            layout.prop(lamp, "type", expand=True)

            layout.prop(lamp, "color")
            layout.prop(lamp, "energy")
