import bpy
import bl_ui
from .. import common

class SORTMaterialPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {'sortblend'}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class MaterialSlotPanel(SORTMaterialPanel, bpy.types.Panel):
    bl_label = common.material_slot_panel_bl_name

    def draw(self, context):
        layout = self.layout

        mat = context.material
        ob = context.object
        slot = context.material_slot
        space = context.space_data

        if ob:
            row = layout.row()

            row.template_list("MATERIAL_UL_matslots", "", ob, "material_slots", ob, "active_material_index", rows=4)

            col = row.column(align=True)

            col.operator("object.material_slot_add", icon='ZOOMIN', text="")
            col.operator("object.material_slot_remove", icon='ZOOMOUT', text="")

            if ob.mode == 'EDIT':
                row = layout.row(align=True)
                row.operator("object.material_slot_assign", text="Assign")
                row.operator("object.material_slot_select", text="Select")
                row.operator("object.material_slot_deselect", text="Deselect")

        split = layout.split(percentage=0.75)

        if ob:
            split.template_ID(ob, "active_material", new="material.new")
            row = split.row()

            if slot:
                row.prop(slot, "link", text="")
            else:
                row.label()
        elif mat:
            split.template_ID(space, "pin_id")
            split.separator()

#        node_tree_selector_draw(layout, mat, 'mitsuba_material_output_node')
#        if not panel_node_draw(layout, mat, 'mitsuba_material_output_node', 'Surface'):
#            row = self.layout.row(align=True)

def register():
    #bpy.utils.register_class(MaterialSlotPanel)
    print('')

def unregister():
    #bpy.utils.unregister_class(MaterialSlotPanel)
    print('')