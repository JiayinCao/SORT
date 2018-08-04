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
from ..material import nodes
from .. import base
from bl_ui import properties_data_camera

class SORTMaterialPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {'SORT_RENDERER'}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

class SORT_Add_Node:
    def get_type_items(self, context):
        items = []
        for category , types in nodes.SORTPatternGraph.nodetypes.items():
            items.append(('', category, ''))
            for type in types:
                items.append(( type , type , type))
        items.append(('', 'Link', ''))
        items.append(('REMOVE', 'Remove', 'Remove the node connected to this socket'))
        items.append(('DISCONNECT', 'Disconnect', 'Disconnect the node connected to this socket'))
        return items

    node_type = bpy.props.EnumProperty(name="Node Type",
        description='Node type to add to this socket',
        items=get_type_items)

    def execute(self, context):
        new_type = self.properties.node_type
        if new_type == 'DEFAULT':
            return {'CANCELLED'}

        nt = context.nodetree
        node = context.node
        socket = context.socket
        def socket_node_input(nt, socket):
            return next((l.from_node for l in nt.links if l.to_socket == socket), None)
        input_node = socket_node_input(nt, socket)

        if new_type == 'REMOVE':
            if input_node is not None:
                nt.nodes.remove(input_node)
            return {'FINISHED'}

        if new_type == 'DISCONNECT':
            link = next((l for l in nt.links if l.to_socket == socket), None)
            if link is not None:
                nt.links.remove(link)
            return {'FINISHED'}

        # add a new node to existing socket
        if input_node is None:
            newnode = nt.nodes.new(new_type)
            newnode.location = node.location
            newnode.location[0] -= 300
            newnode.selected = False
            if self.input_type == 'Pattern':
                link_node(nt, newnode, socket)
            else:
                nt.links.new(newnode.outputs[self.input_type], socket)

        # replace input node with a new one
        else:
            newnode = nt.nodes.new(new_type)
            input = socket
            old_node = input.links[0].from_node
            nt.links.new(newnode.outputs[self.input_type], socket)
            newnode.location = old_node.location

            nt.nodes.remove(old_node)
        return {'FINISHED'}

class NODE_OT_add_surface(bpy.types.Operator, SORT_Add_Node):
    bl_idname = 'node.add_surface'
    bl_label = 'Add Bxdf Node'
    bl_description = 'Connect a Bxdf to this socket'
    input_type = bpy.props.StringProperty(default='Result')

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
        output = nt.nodes.new('SORTNodeOutput')
        default = nt.nodes.new('SORTNode_Material_Principle')
        default.location = output.location
        default.location[0] -= 300
        nt.links.new(default.outputs[0], output.inputs[0])
        return {'FINISHED'}


class SORTMaterialInstance(SORTMaterialPanel, bpy.types.Panel):
    bl_label = "Surface"

    @classmethod
    def poll(cls, context):
        return context.material and SORTMaterialPanel.poll(context)

    def draw(self, context):
        # find current material
        target = None
        for group in bpy.data.node_groups:
            if group.name == context.material.sort_material.sortnodetree:
                target = group

        if target is None:
            self.layout.operator("sort.use_shading_nodes", icon='NODETREE')
            return

        ntree = bpy.data.node_groups[context.material.sort_material.sortnodetree]

        # find the output node, duplicated code, to be cleaned
        def find_output_node(material):
            if material and material.sort_material and material.sort_material.sortnodetree:
                ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
                for node in ntree.nodes:
                    if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
                        return node
            return None

        output_node = find_output_node(context.material)
        if output_node is None:
            self.layout.operator("sort.use_shading_nodes", icon='NODETREE')
            return

        socket = output_node.inputs['Surface']

        self.layout.context_pointer_set("nodetree", ntree)
        self.layout.context_pointer_set("node", output_node)
        self.layout.context_pointer_set("socket", socket)

        if output_node is not None:
            self.draw_node_properties_recursive(self.layout, context, ntree, output_node)

    def draw_node_properties_recursive(self,layout, context, nt, node, level=0):
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
                    split = layout.split(0.3)
                    row = split.row()
                    indented_label(row)
                    row.prop(socket, "ui_open", icon=icon, text='', icon_only=True, emboss=False)
                    row.label(socket.name+":")
                    split.operator_menu_enum("node.add_surface" , "node_type", text=input_node.bl_idname , icon= 'DOT')
                    if socket.ui_open:
                        self.draw_node_properties_recursive(layout, context, nt, input_node, level=level+1)
                else:
                    split = layout.split(0.3)
                    row = split.row()
                    indented_label(row)
                    row.label(socket.name)
                    prop_panel = split.row( align=True )
                    if socket.default_value is not None:
                        prop_panel.prop(socket,'default_value',text="")
                    prop_panel.operator_menu_enum("node.add_surface" , "node_type", text='',icon='DOT')

        draw_props(node, layout)
        layout.separator()