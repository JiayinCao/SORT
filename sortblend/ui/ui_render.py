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
        ("pt", "Path Tracing", "", 2),
        ("lt", "Light Tracing", "", 3),
        ("ir", "Instant Radiosity", "", 4),
        ("ao", "Ambient Occlusion", "", 5),
        ("direct", "Direct Lighting", "", 6),
        ("whitted", "Whitted", "", 7),
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

    # general integrator parameters
    bpy.types.Scene.inte_max_recur_depth = bpy.props.IntProperty(name='Maximum Recursive Depth', default=16, min=1)

    # ao integrator parameters
    bpy.types.Scene.ao_max_dist = bpy.props.FloatProperty(name='Maximum Distance', default=3.0, min=0.01)

    # instant radiosity parameters
    bpy.types.Scene.ir_light_path_set_num = bpy.props.IntProperty(name='Light Path Set Num', default=1, min=1)
    bpy.types.Scene.ir_light_path_num = bpy.props.IntProperty(name='Light Path Num', default=64, min=1)
    bpy.types.Scene.ir_min_dist = bpy.props.FloatProperty(name='Minimum Distance', default=1.0, min=0.0)

    # bidirectional path tracing parameters
    bpy.types.Scene.bdpt_mis = bpy.props.BoolProperty(name='Multiple Importance Sampling', default=True)

    def draw(self, context):
        self.layout.prop(context.scene,"integrator_type_prop")
        integrator_type = context.scene.integrator_type_prop
        if integrator_type != "whitted" and integrator_type != "direct" and integrator_type != "ao":
            self.layout.prop(context.scene,"inte_max_recur_depth")
        if integrator_type == "ao":
            self.layout.prop(context.scene,"ao_max_dist")
        if integrator_type == "bdpt":
            self.layout.prop(context.scene,"bdpt_mis")
        if integrator_type == "ir":
            self.layout.prop(context.scene,"ir_light_path_set_num")
            self.layout.prop(context.scene,"ir_light_path_num")
            self.layout.prop(context.scene, "ir_min_dist")

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