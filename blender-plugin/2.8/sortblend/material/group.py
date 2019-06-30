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
import random
import nodeitems_utils
from .. import base
from .matbase import SORTShaderNodeTree , SORTPatternNodeCategory
from nodeitems_utils import NodeItemCustom

SORT_NODE_GROUP_PREFIX = 'SORTGroupName_'

def group_make():
    tree = bpy.data.node_groups.new('SORT Node Group', 'SORTShaderNodeTree')
    nodes = tree.nodes

    node_input = nodes.new('sort_shader_node_group_input')
    node_input.location = (-300, 0)
    node_input.selected = False

    node_output = nodes.new('sort_shader_node_group_output')
    node_output.location = (300, 0)
    node_output.selected = False

    return tree

def keys_sort(link):
    return (socket_index(link.to_socket), link.from_node.location.y)

def get_links(tree):
    input_links = sorted([l for l in tree.links if (not l.from_node.select) and (l.to_node.select)], key=keys_sort)
    output_links = sorted([l for l in tree.links if (l.from_node.select) and (not l.to_node.select)], key=keys_sort)
    return dict(input=input_links, output=output_links)

def socket_index(socket):
    node = socket.node
    sockets = node.outputs if socket.is_output else node.inputs
    for i, s in enumerate(sockets):
        if s == socket:
            return i

def link_tree(tree, links):
    nodes = tree.nodes
    input_node = nodes.get("Group Inputs")
    output_node = nodes.get("Group Outputs")
    relink_in = []
    relink_out = []
    inputs_remap = {}

    for index, l in enumerate(links['input']):
        i = socket_index(l.to_socket)
        socket = nodes[l.to_node.name].inputs[i]
        if l.from_socket in inputs_remap:
            out_index = inputs_remap[l.from_socket]
            from_socket = input_node.outputs[out_index]
            tree.links.new(from_socket, socket)
        else:
            inputs_remap[l.from_socket] = len(input_node.outputs) - 1
            tree.links.new(input_node.outputs[-1], socket)

        relink_in.append((l.from_socket, inputs_remap[l.from_socket]))

    for index, l in enumerate(links['output']):
        continue
        i = socket_index(l.from_socket)
        socket = nodes[l.from_node.name].outputs[i]
        tree.links.new(socket, output_node.inputs[-1])

        relink_out.append((index, l.to_node.name, socket_index(l.to_socket)))

    return relink_in, relink_out

def get_io_node_locations(nodes):
    offset = 220
    xs = [node.location[0] for node in nodes]
    ys = [node.location[1] for node in nodes]
    min_x = min(xs)
    max_x = max(xs)
    min_y = min(ys)
    max_y = max(ys)
    y = (min_y + max_y) * 0.5
    return (min_x - offset, y), (max_x + offset, y)

def get_socket_data(socket):
    other = get_other_socket(socket)
    if socket.bl_idname == "rpr_dummy_socket":
        socket = get_other_socket(socket)

    socket_bl_idname = socket.bl_idname
    socket_name = socket.name
    return socket_name, socket_bl_idname

def generate_inputs(tree):
    in_socket = []
    input_node = tree.nodes.get("Group Inputs")
    if input_node:
        for idx, socket in enumerate(input_node.outputs):
            if socket.is_linked:
                socket_name, socket_bl_idname = get_socket_data(socket)
                data = [socket_name, socket_bl_idname]
                in_socket.append(data)
    return in_socket


def generate_outputs(tree):
    out_socket = []
    output_node = tree.nodes.get("Group Outputs")
    if output_node:
        for socket in output_node.inputs:
            if socket.is_linked:
                socket_name, socket_bl_idname = get_socket_data(socket)
                out_socket.append((socket_name, socket_bl_idname))
    return out_socket

def update_cls(tree):
    cls_name = tree.sort_data.group_name_id

    class C(SORTGroupNode):
        bl_idname = cls_name
        bl_label = 'SORT Group'
        input_template = generate_inputs(tree)
        output_template = generate_outputs(tree)

    C.__name__ = cls_name

    print( C.bl_idname )
    old_cls_ref = getattr(bpy.types, cls_name, None)
    if old_cls_ref:
        bpy.utils.unregister_class(old_cls_ref)
    bpy.utils.register_class(C)

#    nodeitems_utils.register_node_categories('SHADER_NODES_SORT', [SORTPatternNodeCategory('SORT_Group','Group',nodeitems_utils.NodeItem(C.__name__))])

    return C

def link_tree_instance(node, relinks):
    tree = node.id_data
    input_relink, output_relink = relinks
    for socket, index in input_relink:
        tree.links.new(socket, node.inputs[index])
    for index, name, socket_index in output_relink:
        tree.links.new(node.outputs[index], tree.nodes[name].inputs[socket_index])

def get_node_groups_by_id(name):
    if not name.startswith(RPR_NODE_GROUP_PREFIX):
        return None
    trees = [ng for ng in bpy.data.node_groups if is_node_group_id(ng, name)]
    return trees[0] if len(trees) else None

@base.register_class
class SORT_Node_Group_Make_Operator(bpy.types.Operator):
    bl_label = "Make Group"
    bl_idname = "sort.node_group_make"

    def execute(self, context):
        tree = context.space_data.edit_tree
        for node in tree.nodes:
            if node.bl_idname == 'rpr_shader_node_group_input' or node.bl_idname == 'rpr_shader_node_group_output':
                node.select = False

        nodes = [node for node in tree.nodes if node.select]
        if not nodes:
            self.report({"WARNING"}, "No nodes selected")
            return {'CANCELLED'}

        bpy.ops.node.clipboard_copy()
        all_links = get_links(tree)
        group = group_make()

        # generate unique name
        cls_name = SORT_NODE_GROUP_PREFIX + str(id(group) ^ random.randint(0, 4294967296))
        group.sort_data.group_name_id = cls_name
        print( group.sort_data.group_name_id )

        path = context.space_data.path
        path.append(group)

        bpy.ops.node.clipboard_paste()

        # calculate position for input & output nodes
        input_location, output_loc = get_io_node_locations(nodes)

        input_node = group.nodes.get("Group Inputs")
        input_node.location = input_location
        output_node = group.nodes.get("Group Outputs")
        output_node.location = output_loc

        relinks = link_tree(group, all_links)

        # create class & register
        cls_ref = update_cls(group)
        parent_node = tree.nodes.new(cls_ref.bl_idname)
        parent_node.select = False
        parent_node.location = get_average_location(nodes)

        for node in nodes:
            tree.nodes.remove(node)

        link_tree_instance(parent_node, relinks)
        path.pop()
        path.append(group, node=parent_node)
        bpy.ops.node.view_all()
        return { 'FINISHED' }

@base.register_class
class SORT_Node_Group_Ungroup_Operator(bpy.types.Operator):
    bl_label = "Ungroup"
    bl_idname = "sort.node_group_ungroup"

    def execute(self, context):
        print( 'group ungroup' )
        return { 'FINISHED' }

def is_sort_node_group(ng):
    return hasattr(ng, 'sort_data') and ng.sort_data.group_name_id != ''

def is_node_group_id(ng, name):
    return is_sort_node_group(ng) and ng.sort_data.group_name_id == name

def get_node_groups_by_id(name):
    if not name.startswith(SORT_NODE_GROUP_PREFIX):
        return None
    trees = [ng for ng in bpy.data.node_groups if is_node_group_id(ng, name)]
    return trees[0] if len(trees) else None

def sort_node_group_items(context):
    if context is None:
        return
    space = context.space_data
    if not space:
        return

    tree = space.edit_tree
    if not tree:
        return

    def group_tools_draw(self, layout, context):
        layout.operator("sort.node_group_make")
        layout.operator("sort.node_group_ungroup")
        layout.separator()

    yield NodeItemCustom(draw=group_tools_draw)

    def contains_group(nodetree, group):
        if nodetree == group:
            return True

        for node in nodetree.nodes:
            found = get_node_groups_by_id(node.bl_idname)
            if found and contains_group(found, group):
                return True

        return False

    for ng in context.blend_data.node_groups:
        if not is_sort_node_group(ng) or contains_group(ng, tree):
            continue
        yield SORTPatternNodeCategory(ng.sort_data.group_name_id, ng.name)

#------------------------------------------------------------------------------------#
#                                  Shader Group Nodes                                #
#------------------------------------------------------------------------------------#
@base.register_class
class SORTShaderGroupInputsNode(bpy.types.Node):
    bl_idname = 'sort_shader_node_group_input'
    bl_label = 'Group Inputs'
    bl_icon = 'MATERIAL'
    bl_width_min = 100

    def init(self, context):
        self.use_custom_color = True
        self.color = (0.7, 0.72, 0.6)
        #self.outputs.new('rpr_dummy_socket', '')
        self.node_kind = 'outputs'


@base.register_class
class SORTShaderGroupOutputsNode(bpy.types.Node):
    bl_idname = 'sort_shader_node_group_output'
    bl_label = 'Group Outputs'
    bl_icon = 'MATERIAL'
    bl_width_min = 100

    def init(self, context):
        self.use_custom_color = True
        self.color = (0.7, 0.72, 0.6)
        #self.inputs.new('rpr_dummy_socket', '')
        self.node_kind = 'inputs'

class SORTGroupNode(bpy.types.Node):
    bl_icon = 'OUTLINER_OB_EMPTY'
    bl_width_min = 180

    @classmethod
    def poll(cls, context):
        return bpy.context.scene.render.engine == 'SORTPatternNodeCategory'

    def draw_buttons(self, context, layout):
        ng = get_node_groups_by_id(self.bl_idname)
        if ng:
            layout.prop(ng, 'name')
        layout.operator('sort.node_group_edit', text='Edit')

    def init(self, context):
        tree = get_node_groups_by_id(self.bl_idname)
        if not tree:
            return

        input_template = generate_inputs(tree)
        for socket_name, socket_bl_idname in input_template:
            s = self.inputs.new(socket_bl_idname, socket_name)

        output_template = generate_outputs(tree)
        for socket_name, socket_bl_idname in output_template:
            self.outputs.new(socket_bl_idname, socket_name)