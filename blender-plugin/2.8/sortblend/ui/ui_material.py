#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
#
#    SORT is a free software written for educational purpose. Anyone can distribute
#    or modify it under the the terms of the GNU General Public License Version 3 as
#    published by the Free Software Foundation. However, there is NO warranty that
#    all components are functional in a perfect manner. Without even the implied
#    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along with
#    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.

import bpy
from .. import base
from ..material import nodes

# Whether there is output node in the material node tree
def has_sort_output_node(ntree):
    if ntree is None:
        return False
    for node in ntree.nodes:
        if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
            return True
    return False

class SORTMaterialPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {'SORT'}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

@base.register_class
class SORT_new_material(bpy.types.Operator):
    """Add a new material"""
    bl_idname = "sort_material.new"
    bl_label = "Add a new material"

    def execute(self, context):
        # currently picked object
        obj = bpy.context.object

        # add the new material
        material = bpy.data.materials.new( 'Material' )

        # initialize default sort shader nodes
        material.sort_material = bpy.data.node_groups.new(material.name, type='SORTPatternGraph')
        material.sort_material.use_fake_user = True
        output = material.sort_material.nodes.new('SORTNodeOutput')
        default = material.sort_material.nodes.new('SORTNode_Material_Diffuse')
        output.location[0] += 200
        output.location[1] += 200
        default.location[1] += 200
        material.sort_material.links.new(default.outputs[0], output.inputs[0])

        # add a new material slot or assign the newly added material in the picked empty slot
        materials = obj.data.materials
        cur_mat_id = obj.active_material_index
        if cur_mat_id >= 0 and cur_mat_id < len(materials) and materials[cur_mat_id] is None:
            materials[cur_mat_id] = material
        else:
            materials.append(material)

        return { 'FINISHED' }

@base.register_class
class MATERIAL_PT_MaterialSlotPanel(SORTMaterialPanel, bpy.types.Panel):
    bl_label = 'Material Slot'

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
            col.operator("object.material_slot_add", icon='ADD', text="")
            col.operator("object.material_slot_remove", icon='REMOVE', text="")
            if ob.mode == 'EDIT':
                row = layout.row(align=True)
                row.operator("object.material_slot_assign", text="Assign")
                row.operator("object.material_slot_select", text="Select")
                row.operator("object.material_slot_deselect", text="Deselect")
        split = layout.split(factor=0.75)
        if ob:
            split.template_ID(ob, "active_material", new="sort_material.new")
            row = split.row()
            if slot:
                row.prop(slot, "link", text="")
            else:
                row.label()
        elif mat:
            split.template_ID(space, "pin_id")
            split.separator()