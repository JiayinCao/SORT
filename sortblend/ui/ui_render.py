import bpy
import bl_ui
from extensions_framework.ui import property_group_renderer
from .. import renderer
from .. import preference
from .. import common

class OBJECT_OT_PropCopyButton(bpy.types.Operator):
    bl_idname = "prop.propcopy"
    bl_label = "Propcopy"

    def execute(self, context):
        sc0 = context.scene
        for sc in bpy.data.scenes:
            sc.scn_property = sc0.scn_property
        return{'FINISHED'}

class UIPanel(bpy.types.Panel):
    bl_label = "Integrator"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"
    bpy.types.Scene.integrator_prop = bpy.props.FloatProperty(name = "IntegratorProperty")

    display_property_groups = [
        (('scene',), 'sort_integrator')
    ]

    def draw(self, context):
        self.layout.operator("prop.propcopy", text='Update Scenes')
        self.layout.prop(context.scene,"integrator_prop")

def register():
    # register UI component
    bl_ui.properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.add(common.renderer_bl_name)

    bpy.utils.register_class(UIPanel)

def unregister():
    bpy.utils.unregister_class(UIPanel)