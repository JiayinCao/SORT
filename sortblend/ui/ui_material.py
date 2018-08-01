#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
# 
#    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
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
from .. import common

class SORTMaterialPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {common.default_bl_name}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

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
            split.template_ID(ob, "active_material", new="material.new")
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

        nt = bpy.data.node_groups.new(group_name, type='SORTPatternGraph')
        nt.use_fake_user = True

        mat.sort_material.sortnodetree = nt.name
        output = nt.nodes.new(common.sort_node_output_bl_name)
        default = nt.nodes.new('SORTNode_Material_Principle')
        default.location = output.location
        default.location[0] -= 300
        nt.links.new(default.outputs[0], output.inputs[0])
        return {'FINISHED'}

def draw_node_properties_recursive(layout, context, nt, node, level=0):

    def indented_label(layout):
        for i in range(level):
            layout.label('',icon='BLANK1')

    layout.context_pointer_set("nodetree", nt)
    layout.context_pointer_set("node", node)

    # draw socket property in panel
    def draw_props(node, layout):
        # node properties
        node.draw_props(context,layout,indented_label)

        # inputs
        for socket in node.inputs:
            layout.context_pointer_set("socket", socket)

            if socket.is_linked:
                def socket_node_input(nt, socket):
                    return next((l.from_node for l in nt.links if l.to_socket == socket), None)
                input_node = socket_node_input(nt, socket)
                ui_open = socket.ui_open
                icon = 'DISCLOSURE_TRI_DOWN' if ui_open else 'DISCLOSURE_TRI_RIGHT'
                split = layout.split(common.label_percentage)
                row = split.row()
                indented_label(row)
                row.prop(socket, "ui_open", icon=icon, text='', icon_only=True, emboss=False)
                row.label(socket.name+":")
                split.operator_menu_enum("node.add_surface" , "node_type", text=input_node.bl_idname , icon= 'DOT')
                if socket.ui_open:
                    draw_node_properties_recursive(layout, context, nt, input_node, level=level+1)
            else:
                split = layout.split(common.label_percentage)
                row = split.row()
                indented_label(row)
                row.label(socket.name)
                prop_panel = split.row( align=True )
                if socket.default_value is not None:
                    prop_panel.prop(socket,'default_value',text="")
                prop_panel.operator_menu_enum("node.add_surface" , "node_type", text='',icon='DOT')

    draw_props(node, layout)
    layout.separator()

def panel_node_draw(layout, context, id_data, input_name):
    # find current material
    target = None
    for group in bpy.data.node_groups:
        if group.name == id_data.sort_material.sortnodetree:
            target = group

    if target is None:
        layout.operator("sort.use_shading_nodes", icon='NODETREE')
        return False

    ntree = bpy.data.node_groups[id_data.sort_material.sortnodetree]

    # find the output node
    def find_node(material, nodetype):
        if material and material.sort_material and material.sort_material.sortnodetree:
            ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
            for node in ntree.nodes:
                if getattr(node, "bl_idname", None) == nodetype:
                    return node
        return None

    output_node = find_node(id_data, common.sort_node_output_bl_name)

    if output_node is None:
        layout.operator("sort.use_shading_nodes", icon='NODETREE')
        return False

    socket = output_node.inputs[input_name]

    layout.context_pointer_set("nodetree", ntree)
    layout.context_pointer_set("node", output_node)
    layout.context_pointer_set("socket", socket)

    if output_node is not None:
        draw_node_properties_recursive(layout, context, ntree, output_node)

from .. import material

class SORTMaterialInstance(SORTMaterialPanel, bpy.types.Panel):
    bl_label = "Surface"

    @classmethod
    def poll(cls, context):
        return context.material and SORTMaterialPanel.poll(context)

    def draw(self, context):
        panel_node_draw(self.layout, context, context.material, 'Surface')