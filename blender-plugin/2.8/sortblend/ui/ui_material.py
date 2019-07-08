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
from .. import base
from ..material import nodes, group

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

class SORT_new_material_base(bpy.types.Operator):
    bl_label = "New"

    def execute(self, context):
        # currently picked object
        obj = bpy.context.object

        # add the new material
        material = bpy.data.materials.new( 'Material' )

        # initialize default sort shader nodes
        material.sort_material = bpy.data.node_groups.new( 'SORT_(' + material.name + ')' , type=nodes.SORTShaderNodeTree.bl_idname)

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
class SORT_new_material(SORT_new_material_base):
    """Add a new material"""
    bl_idname = "sort_material.new"

@base.register_class
class SORT_new_material_menu(SORT_new_material_base):
    """Add a new material"""
    bl_idname = "node.new_node_tree"

@base.register_class
class MATERIAL_PT_MaterialSlotPanel(SORTMaterialPanel, bpy.types.Panel):
    bl_label = 'Material Slot'

    def draw(self, context):
        layout = self.layout
        ob = context.object

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
        elif context.material:
            split.template_ID(context.space_data, "pin_id")
            split.separator()

@base.register_class
class MATERIAL_PT_MaterialParameterPanel(SORTMaterialPanel, bpy.types.Panel):
    bl_label = 'Material Parameters'

    def draw(self, context):
        tree = context.material.sort_material

        group_input_node = tree.nodes.get( "Shader Inputs" )
        if group_input_node is None:
            return
        
        for input in group_input_node.inputs:
            self.layout.prop( input , 'default_value' , text = input.name )

@base.register_class
class SORT_OT_node_socket_base(bpy.types.Operator):
    """Move socket"""
    bl_idname = "sort.node_socket_base"
    bl_label = "Move Socket"

    type : bpy.props.EnumProperty(
        items=(('up', '', ''),
               ('down', '', ''),
               ('remove', '', ''),
               ),
    )
    pos : bpy.props.IntProperty()
    node_name : bpy.props.StringProperty()

    def execute(self, context):
        node = context.space_data.edit_tree.nodes[self.node_name]
        tree = node.id_data
        kind = node.node_kind
        io = getattr(node, kind)
        socket = io[self.pos]

        if self.type == 'remove':
            io.remove(socket)
            if group.is_sort_node_group(tree):
                # update instances
                for instance in group.instances(tree):
                    sockets = getattr(instance, group.map_lookup[kind])
                    sockets.remove(sockets[self.pos])
            else:
                # update root shader inputs
                shader_input_node = tree.nodes.get( 'Shader Inputs' )
                sockets = getattr(shader_input_node, group.map_lookup[kind])
                sockets.remove(sockets[self.pos])
        else:
            step = -1 if self.type == 'up' else 1
            count = len(io) - 1

            def calc_new_position(pos, step, count):
                return max(0, min(pos + step, count - 1))

            new_pos = calc_new_position(self.pos, step, count)
            io.move(self.pos, new_pos)

            if group.is_sort_node_group(tree):
                # update instances
                for instance in group.instances(tree):
                    sockets = getattr(instance, group.map_lookup[kind])
                    new_pos = calc_new_position(self.pos, step, len(sockets))
                    sockets.move(self.pos, new_pos)
            else:
                shader_input_node = tree.nodes.get( 'Shader Inputs' )
                sockets = getattr(shader_input_node, group.map_lookup[kind])
                new_pos = calc_new_position(self.pos, step, len(sockets))
                sockets.move(self.pos, new_pos)

        group.update_cls(tree)
        return {"FINISHED"}

@base.register_class
class SORT_OT_node_socket_move(SORT_OT_node_socket_base):
    """Move socket"""
    bl_idname = "sort.node_socket_move"
    bl_label = "Remove Socket"

    type : bpy.props.EnumProperty(
        items=(('up', '', ''),
               ('down', '', ''),
               ('remove', '', ''),
               ),
    )
    pos : bpy.props.IntProperty()
    node_name : bpy.props.StringProperty()

@base.register_class
class SORT_OT_node_socket_remove(SORT_OT_node_socket_base):
    """Remove socket"""
    bl_idname = "sort.node_socket_remove"
    bl_label = "Remove Socket"

    type : bpy.props.EnumProperty(
        items=(('up', '', ''),
               ('down', '', ''),
               ('remove', '', ''),
               ),
    )
    pos : bpy.props.IntProperty()
    node_name : bpy.props.StringProperty()

@base.register_class
class SORT_OT_node_socket_restore_input_node(bpy.types.Operator):
    """Move socket"""
    bl_idname = "sort.node_socket_restore_group_input"
    bl_label = "Restore Group Input"

    def execute(self, context):
        tree = context.space_data.edit_tree

        nodes = tree.nodes
        if group.is_sort_node_group(tree):
            node_input = nodes.new('sort_shader_node_group_input')    
        else:
            node_input = nodes.new('SORTNodeExposedInputs')

        node_input.location = (-300, 0)
        node_input.selected = False
        node_input.tree = tree

        return {"FINISHED"}

@base.register_class
class SORT_OT_node_socket_restore_output_node(bpy.types.Operator):
    """Move socket"""
    bl_idname = "sort.node_socket_restore_group_output"
    bl_label = "Restore Group Output"

    def execute(self, context):
        tree = context.space_data.edit_tree
        nodes = tree.nodes

        node_input = nodes.new('sort_shader_node_group_output')
        node_input.location = (300, 0)
        node_input.selected = False
        node_input.tree = tree

        return {"FINISHED"}

@base.register_class
class MATERIAL_PT_SORTInOutGroupEditor(SORTMaterialPanel, bpy.types.Panel):
    bl_label = "SORT In/Out Group Editor"
    bl_space_type = 'NODE_EDITOR'
    bl_region_type = 'UI'

    @classmethod
    def poll(cls, context):
        tree = context.space_data.edit_tree
        if not tree:
            return False
        return tree.bl_idname == nodes.SORTShaderNodeTree.bl_idname

    def draw(self, context):
        def set_attrs(cls, **kwargs):
            for name, value in kwargs.items():
                setattr(cls, name, value)

        tree = context.space_data.edit_tree
        is_group_node = group.is_sort_node_group(tree)

        layout = self.layout
        row = layout.row()
        col1 = layout.row().box().column()
        if is_group_node:
            layout.separator()
            col2 = layout.row().box().column()

        def draw_socket(col, socket, index):
            if socket.bl_idname == 'sort_dummy_socket':
                return
            params = dict(node_name=socket.node.name, pos=index)

            row = col.row(align=True)
            row.template_node_socket(color=(0.35, 0.5, 0.8, 1.0))

            row.prop( socket , 'name' , text = '' )
            op = row.operator('sort.node_socket_move', icon='TRIA_UP', text='')
            set_attrs(op, type='up', **params)
            op = row.operator('sort.node_socket_move', icon='TRIA_DOWN', text='')
            set_attrs(op, type='down', **params)
            op = row.operator('sort.node_socket_remove', icon='X', text='')
            set_attrs(op, type='remove', **params)

        input_node = tree.nodes.get("Group Inputs")
        output_node = tree.nodes.get("Group Outputs")

        if group.is_sort_node_group( tree ) is False:
            input_node = tree.nodes.get( 'Shader Inputs' )
            output_node = None

        display_label = 'Group Inputs' if is_group_node else 'Shader Inputs'
        col1.label( text = display_label )
        if input_node is not None:
            for i, socket in enumerate(input_node.outputs):
                draw_socket(col1, socket, i)
        else:
            row = col1.row(align=True)
            display_text = 'Restore Group Input Node' if is_group_node else 'Add Shader Inputs'
            row.operator('sort.node_socket_restore_group_input', text=display_text)

        # root shader group doesn't have output
        if is_group_node:
            col2.label(text='Group Outputs:')
            if output_node is not None:
                for i, socket in enumerate(output_node.inputs):
                    draw_socket(col2, socket, i)
            else:
                row = col2.row(align=True)
                row.operator('sort.node_socket_restore_group_output', text='Restore Group Output Node')