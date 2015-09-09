import bpy
from .. import common
from .. import exporter

class SORTRenderPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"
    COMPAT_ENGINES = {common.default_bl_name}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class IntegratorPanel(SORTRenderPanel,bpy.types.Panel):
    bl_label = common.integrator_panel_bl_name

    # Integrator type
    integrator_types = [
        ("bdpt", "Bidirectional Path Tracing", "", 1),
        ("pathtracing", "Path Tracing", "", 2),
        ("direct", "Direct Lighting", "", 3),
        ("whitted", "Whitted", "", 4),
        ]
    bpy.types.Scene.integrator_type_prop = bpy.props.EnumProperty(items=integrator_types, name='Integrator')

    # Accelerator type
    accelerator_types = [
        ("kd_tree", "SAH KDTree", "", 1),
        ("bvh", "Bounding Volume Hierarchy", "", 2),
        ("uniform_grid", "Uniform Grid", "", 3),
        ("bruteforce", "No Accelerator", "", 4),
        ]
    bpy.types.Scene.accelerator_type_prop = bpy.props.EnumProperty(items=accelerator_types, name='Accelerator')

    def draw(self, context):
        self.layout.prop(context.scene,"integrator_type_prop")
        self.layout.prop(context.scene,"accelerator_type_prop")

class MultiThreadPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = common.thread_panel_bl_name

    bpy.types.Scene.thread_num_prop = bpy.props.IntProperty(name='Thread Num', default=8, min=1, max=16)

    def draw(self, context):
        self.layout.prop(context.scene,"thread_num_prop")

class SamplerPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = common.sampler_panel_bl_name

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

# export debug scene
class SORT_export_debug_scene(bpy.types.Operator):
    bl_idname = "sort.export_debug_scene"
    bl_label = "Export Debug Scene"

    def execute(self, context):
        # export the scene to debug folder
        exporter.export_blender(context.scene,True)
        return {'FINISHED'}

class DebugPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = common.debug_panel_bl_name

    bpy.types.Scene.debug_prop = bpy.props.BoolProperty(name='Debug', default=False)

    def draw(self, context):
        self.layout.prop(context.scene,"debug_prop")
        self.layout.operator("sort.export_debug_scene")