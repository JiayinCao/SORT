#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
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
from ..material import nodes
from .. import base
from bl_ui import properties_data_camera

class SORTMaterialPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {'SORT'}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

class SORTMaterialPreview(SORTMaterialPanel, bpy.types.Panel):
    bl_label = "Preview"
    bl_context = "material"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        return context.material and SORTMaterialPanel.poll(context)

    def draw(self, context):
        self.layout.template_preview(context.material)

class MaterialSlotPanel(SORTMaterialPanel, bpy.types.Panel):
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
            col.operator("object.material_slot_add", icon='ZOOMIN', text="")
            col.operator("object.material_slot_remove", icon='ZOOMOUT', text="")
            if ob.mode == 'EDIT':
                row = layout.row(align=True)
                row.operator("object.material_slot_assign", text="Assign")
                row.operator("object.material_slot_select", text="Select")
                row.operator("object.material_slot_deselect", text="Deselect")
        split = layout.split(percentage=0.75)
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

class SORT_use_shading_nodes(bpy.types.Operator):
    """Enable nodes on a material, world or lamp"""
    bl_idname = "sort.use_shading_nodes"
    bl_label = "Use Nodes"
    idtype = bpy.props.StringProperty(name="ID Type", default="material")

    @classmethod
    def poll(cls, context):
        return (getattr(context, "material", False) or getattr(context, "world", False) or
                getattr(context, "lamp", False))

    def execute(self, context):
        mat = context.material
        idtype = self.properties.idtype
        context_data = {'material':context.material, 'lamp':context.lamp }
        idblock = context_data[idtype]

        group_name = 'SORTGroup_' + idblock.name

        nt = bpy.data.node_groups.new(group_name, type='SORTShaderNodeTree')
        nt.use_fake_user = True

        mat.sort_material = nt
        output = nt.nodes.new('SORTNodeOutput')
        default = nt.nodes.new('SORTNode_Material_Diffuse')
        default.location = output.location
        default.location[0] -= 300
        nt.links.new(default.outputs[0], output.inputs[0])
        return {'FINISHED'}

class SORT_new_material_base(bpy.types.Operator):
    bl_label = "New"

    def execute(self, context):
        # currently picked object
        obj = bpy.context.object

        # add the new material
        mat = bpy.data.materials.new( 'Material' )

        # initialize default sort shader nodes
        mat.sort_material = bpy.data.node_groups.new( 'SORT_(' + mat.name + ')' , type=nodes.SORTShaderNodeTree.bl_idname)

        output = mat.sort_material.nodes.new('SORTNodeOutput')
        default = mat.sort_material.nodes.new('SORTNode_Material_Diffuse')
        output.location[0] += 200
        output.location[1] += 200
        default.location[1] += 200
        mat.sort_material.links.new(default.outputs[0], output.inputs[0])

        # add a new material slot or assign the newly added material in the picked empty slot
        materials = obj.data.materials
        cur_mat_id = obj.active_material_index
        if cur_mat_id >= 0 and cur_mat_id < len(materials) and materials[cur_mat_id] is None:
            materials[cur_mat_id] = mat
        else:
            materials.append(mat)

        return { 'FINISHED' }

class SORT_new_material(SORT_new_material_base):
    """Add a new material"""
    bl_idname = "sort_material.new"

class SORT_new_material_menu(SORT_new_material_base):
    """Add a new material"""
    bl_idname = "node.new_node_tree"