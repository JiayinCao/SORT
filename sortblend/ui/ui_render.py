import bpy
import bl_ui
from .. import common

class IntegratorPanel(bpy.types.Panel):
    bl_label = common.integrator_panel_bl_name
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"

    # Integrator type
    integrator_types = [
        ("bdpt", "Bidirectional Path Tracing", "", 1),
        ("pathtracing", "Path Tracing", "", 2),
        ("direct", "Direct Lighting", "", 3),
        ("whitted", "Whitted", "", 4),
        ]
    bpy.types.Scene.integrator_type_prop = bpy.props.EnumProperty(items=integrator_types, name='Type')

    def draw(self, context):
        self.layout.prop(context.scene,"integrator_type_prop")

class MultiThreadPanel(bpy.types.Panel):
    bl_label = common.thread_panel_bl_name
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"

    bpy.types.Scene.thread_num_prop = bpy.props.IntProperty(name='Thread Num', default=8, min=1, max=16)

    def draw(self, context):
        self.layout.prop(context.scene,"thread_num_prop")

class SamplerPanel(bpy.types.Panel):
    bl_label = common.sampler_panel_bl_name
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"

    # sampler type
    sampler_types = [
        ("stratified", "Stratified", "", 3),
        ("random", "Random", "", 2),
        ("regular", "Uniform", "", 1),
        ]
    bpy.types.Scene.sampler_type_prop = bpy.props.EnumProperty(items=sampler_types, name='Type')

    # sampler count
    bpy.types.Scene.sampler_count_prop = bpy.props.IntProperty(name='Count',default=1, min=1)

    def draw(self, context):
        self.layout.prop(context.scene,"sampler_type_prop")
        self.layout.prop(context.scene,"sampler_count_prop")

def register():
    # register UI component
    bl_ui.properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.add(common.renderer_bl_name)

    # register integrator panel
    bpy.utils.register_class(IntegratorPanel)
    bpy.utils.register_class(SamplerPanel)
    bpy.utils.register_class(MultiThreadPanel)

def unregister():
    # unregister integrator panel
    bpy.utils.unregister_class(IntegratorPanel)
    bpy.utils.unregister_class(SamplerPanel)
    bpy.utils.unregister_class(MultiThreadPanel)