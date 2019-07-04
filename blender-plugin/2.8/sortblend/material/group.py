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
import random
import nodeitems_utils
from .. import base, renderer
from nodeitems_utils import NodeItemCustom, NodeItem

class SORTPatternNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == SORTShaderNodeTree.bl_idname and renderer.SORTRenderEngine.is_active(context)

@base.register_class
class SORTShaderNodeTree(bpy.types.NodeTree):
    bl_idname = 'SORTShaderNodeTree'
    bl_label = 'SORT Shader Editor'
    bl_icon = 'MATERIAL'
    node_categories = {}

    @classmethod
    def poll(cls, context):
        return renderer.SORTRenderEngine.is_active(context)

    # Return a node tree from the context to be used in the editor
    @classmethod
    def get_from_context(cls, context):
        ob = context.active_object
        if ob is not None and ob.active_material is not None:
            mat = ob.active_material
            return mat.sort_material , mat , mat
        return (None, None, None)

    @classmethod
    def register_node(cls,category):
        def registrar(nodecls):
            base.register_class(nodecls)
            d = cls.node_categories.setdefault(category, [])
            d.append(nodecls)
            return nodecls
        return registrar

    @classmethod
    def register(cls):
        bpy.types.Material.sort_material = bpy.props.PointerProperty(type=bpy.types.NodeTree, name='SORT Material Settings')

        # Register all nodes
        cats = []
        for c, l in sorted(cls.node_categories.items()):
            cid = 'SORT_' + c.replace(' ', '').upper()
            items = [nodeitems_utils.NodeItem(nc.__name__) for nc in l]
            cats.append(SORTPatternNodeCategory(cid, c, items=items))
        cats.append(SORTPatternNodeCategory('SORT_LAYOUT', 'Layout', items=[nodeitems_utils.NodeItem('NodeFrame'),nodeitems_utils.NodeItem('NodeReroute')]))
        cats.append(SORTPatternNodeCategory('SORT_Group', 'Group', items=sort_node_group_items))

        nodeitems_utils.register_node_categories('SHADER_NODES_SORT', cats)

    @classmethod
    def unregister(cls):
        nodeitems_utils.unregister_node_categories('SHADER_NODES_SORT')
        del bpy.types.Material.sort_material

@base.register_class
class SORTNodeGroupData(bpy.types.PropertyGroup):
    group_name_id : bpy.props.StringProperty( name='Group Tree Id', default='')
    @classmethod
    def register(cls):
        SORTShaderNodeTree.sort_data = bpy.props.PointerProperty(name="SORT Data", type=cls)
    @classmethod
    def unregister(cls):
        del SORTShaderNodeTree.sort_data

#------------------------------------------------------------------------------------#
#                                  Shader Group Nodes                                #
#------------------------------------------------------------------------------------#

@base.register_class
class SORTDummySocket(bpy.types.NodeSocket):
    bl_idname = "sort_dummy_socket"
    bl_label = "SPRT Dummy Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.6, 0.6, 0.5)

map_lookup = {'outputs': 'inputs', 'inputs': 'outputs'}
class SORTNodeSocketConnectorHelper:
    socket_map = {'outputs': 'to_socket', 'inputs': 'from_socket'}
    node_kind : bpy.props.StringProperty()

    def update(self):
        kind = self.node_kind
        if not kind:
            return

        tree = self.id_data
        if tree.bl_idname != SORTShaderNodeTree.bl_idname:
            return

        socket_list = getattr(self, kind)

        if len(socket_list) == 0:
            return

        if socket_list[-1].is_linked:
            socket = socket_list[-1]
            cls = update_cls(tree)
            if kind == "outputs":
                new_name, new_type = cls.input_template[-1]
            else:
                new_name, new_type = cls.output_template[-1]

            new_socket = replace_socket(socket, new_type, new_name=new_name)

            # update instances
            for instance in instances(tree):
                sockets = getattr(instance, map_lookup[kind])
                new_socket = sockets.new(new_type, new_name)

            socket_list.new('sort_dummy_socket', '')

class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'
    osl_shader = ''

    # some material nodes depends on some heavy resources, this is the place for it to tell
    def populateResources( self , resources ):
        pass
    # generate open shading lanugage source code
    def generate_osl_source(self):
        return self.osl_shader
    # this function helps serializing the material information
    def serialize_prop(self,fs):
        fs.serialize(0)
    # unique name to identify the node type, because some node can output mutitple shaders, need to output all if necessary
    def type_identifier(self):
        return self.bl_label
    # whether the node is a group node
    def isGroupNode(self):
        return False
    # whether the node is a group input
    def isGroupInputNode(self):
        return False
    # whether the node is a group output
    def isGroupOutputNode(self):
        return False
    # get shader parameter name
    def getShaderInputParameterName(self,param):
        return param
    def getShaderOutputParameterName(self,param):
        return param

class SORTGroupNode(SORTShadingNode,bpy.types.PropertyGroup):
    bl_icon = 'OUTLINER_OB_EMPTY'
    bl_width_min = 180

    @classmethod
    def poll(cls, context):
        return bpy.context.scene.render.engine == 'SORT'

    def isGroupNode(self):
        return True

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

    def getOuputSocket( self , socket ):
        for socket_name, socket_bl_idname in self.output_template:
            if socket_bl_idname == socket.bl_idname:
                output_node = self.group_tree.nodes.get("Group Outputs")
                return output_node.inputs[socket_name]
        return None

    # get shader parameter name
    def getShaderInputParameterName(self,param):
        return 'i' + param.replace(' ', '')
    def getShaderOutputParameterName(self,param):
        return 'o' + param.replace(' ', '')

    # this is just a proxy node
    def generate_osl_source(self):
        socket_type_mapping = {'SORTNodeSocketBxdf': 'closure color', 
                               'SORTNodeSocketColor': 'color',
                               'SORTNodeSocketFloat': 'float',
                               'SORTNodeSocketFloatVector': 'vector',
                               'SORTNodeSocketLargeFloat': 'float',
                               'SORTNodeSocketAnyFloat': 'float',
                               'SORTNodeSocketNormal': 'normal',
                               'SORTNodeSocketUV': 'vector'}

        inputs = self.inputs

        osl_shader = 'shader PassThrough_GroupInput('
        for i in range( 0 , len(inputs) ):
            input = inputs[i]
            input_type = socket_type_mapping[input.bl_idname]
            osl_shader += input_type + ' ' + self.getShaderInputParameterName(input.name) + ' = @ \n'
        for i in range( 0 , len(inputs) ):
            output = inputs[i]
            output_type = socket_type_mapping[output.bl_idname]
            osl_shader += 'output ' + output_type + ' ' + self.getShaderOutputParameterName(output.name) + ' = @'
            if i < len(inputs) - 1 :
                osl_shader += ',\n'
            else:
                osl_shader += '){\n'

        for i in range( 0 , len(inputs) ):
            var_name = inputs[i].name
            osl_shader += self.getShaderOutputParameterName(var_name) + ' = ' + self.getShaderInputParameterName(var_name) + ';\n'
        osl_shader += '}'
        return osl_shader

@base.register_class
class SORTShaderGroupInputsNode(SORTNodeSocketConnectorHelper, SORTShadingNode):
    bl_idname = 'sort_shader_node_group_input'
    bl_label = 'Group Inputs'
    bl_icon = 'MATERIAL'
    bl_width_min = 100

    def init(self, context):
        self.use_custom_color = True
        self.color = (0.7, 0.72, 0.6)
        self.outputs.new('sort_dummy_socket', '')
        self.node_kind = 'outputs'

    # whether the node is a group input
    def isGroupInputNode(self):
        return True
    # get shader parameter name
    def getShaderInputParameterName(self,param):
        return 'i' + param.replace(' ', '')
    def getShaderOutputParameterName(self,param):
        return 'o' + param.replace(' ', '')

@base.register_class
class SORTShaderGroupOutputsNode(SORTNodeSocketConnectorHelper, SORTShadingNode):
    bl_idname = 'sort_shader_node_group_output'
    bl_label = 'Group Outputs'
    bl_icon = 'MATERIAL'
    bl_width_min = 100

    def init(self, context):
        self.use_custom_color = True
        self.color = (0.7, 0.72, 0.6)
        self.inputs.new('sort_dummy_socket', '')
        self.node_kind = 'inputs'
    # whether the node is a group output
    def isGroupOutputNode(self):
        return True

@bpy.app.handlers.persistent
def node_groups_load_post(dummy):
    node_groups = [ng for ng in bpy.data.node_groups if is_sort_node_group(ng)]
    for ng in node_groups:
        update_cls(ng)

SORT_NODE_GROUP_PREFIX = 'SORTGroupName_'

def instances(tree):
    res = []
    all_trees = [ng for ng in bpy.data.node_groups if is_sort_node_group(ng) and ng.nodes]

    for material in bpy.data.materials:
        t = material.sort_material
        if not t or not t.nodes:
            continue
        all_trees.append(t)

    for t in all_trees:
        for node in t.nodes:
            if is_node_group_id(tree, node.bl_idname):
                res.append(node)

    return res

def group_make():
    tree = bpy.data.node_groups.new('SORT Node Group', SORTShaderNodeTree.bl_idname)
    tree.use_fake_user = True
    nodes = tree.nodes

    node_input = nodes.new('sort_shader_node_group_input')
    node_input.location = (-300, 0)
    node_input.selected = False
    node_input.tree = tree

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

def get_other_socket(socket):
    if not socket.is_linked:
        return None
    if not socket.is_output:
        other = socket.links[0].from_socket
    else:
        other = socket.links[0].to_socket

    if other.node.bl_idname == 'NodeReroute':
        if not socket.is_output:
            return get_other_socket(other.node.inputs[0])
        else:
            return get_other_socket(other.node.outputs[0])
    else:
        return other

def get_socket_data(socket):
    other = get_other_socket(socket)
    if socket.bl_idname == "sort_dummy_socket":
        socket = get_other_socket(socket)

    socket_bl_idname = socket.bl_idname
    socket_name = socket.name
    return socket_name, socket_bl_idname

def replace_socket(socket, new_type, new_name=None, new_pos=None):
    socket_name = new_name or socket.name
    socket_pos = new_pos or socket_index(socket)
    ng = socket.id_data

    if socket.is_output:
        outputs = socket.node.outputs
        to_sockets = [l.to_socket for l in socket.links]

        outputs.remove(socket)
        new_socket = outputs.new(new_type, socket_name)
        outputs.move(len(outputs) - 1, socket_pos)

        for to_socket in to_sockets:
            ng.links.new(new_socket, to_socket)

    else:
        inputs = socket.node.inputs
        from_socket = socket.links[0].from_socket if socket.is_linked else None

        inputs.remove(socket)
        new_socket = inputs.new(new_type, socket_name)
        inputs.move(len(inputs) - 1, socket_pos)

        if from_socket:
            ng.links.new(from_socket, new_socket)

    return new_socket

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
        group_tree = tree

    C.__name__ = cls_name

    old_cls_ref = getattr(bpy.types, cls_name, None)
    if old_cls_ref:
        bpy.utils.unregister_class(old_cls_ref)
    bpy.utils.register_class(C)

    return C

def link_tree_instance(node, relinks):
    tree = node.id_data
    input_relink, output_relink = relinks
    for socket, index in input_relink:
        tree.links.new(socket, node.inputs[index])
    for index, name, socket_index in output_relink:
        tree.links.new(node.outputs[index], tree.nodes[name].inputs[socket_index])

def get_average_location(nodes):
    x, y = 0, 0
    for node in nodes:
        x += node.location[0]
        y += node.location[1]
    d = 1.0 / len(nodes)
    return x * d, y * d

@base.register_class
class SORT_Node_Group_Make_Operator(bpy.types.Operator):
    bl_label = "Make Group"
    bl_idname = "sort.node_group_make"

    def execute(self, context):
        tree = context.space_data.edit_tree
        for node in tree.nodes:
            if node.bl_idname == 'sort_shader_node_group_input' or node.bl_idname == 'sort_shader_node_group_output':
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

def get_selected_node_by_idname(tree, name):
    for node in tree.nodes:
        if not node.select:
            continue
        if node.bl_idname == name:
            return node
    return None

@base.register_class
class SORT_Node_Group_Ungroup_Operator(bpy.types.Operator):
    bl_label = "Ungroup"
    bl_idname = "sort.node_group_ungroup"

    @classmethod
    def poll(cls, context):
        if context.scene.render.engine != 'SORT':
            return False
        group_node = context.active_node
        if not group_node:
            return False
        return get_node_groups_by_id(group_node.bl_idname) != None

    def execute(self, context):
        group_node = context.active_node

        # copy data
        bpy.ops.node.select_all(action='DESELECT')
        tree = get_node_groups_by_id(group_node.bl_idname)
        if not tree:
            return {'CANCELLED'}
        path = context.space_data.path
        path.append(tree)
        bpy.ops.node.select_all(action='SELECT')
        bpy.ops.node.clipboard_copy()
        path.pop()
        bpy.ops.node.clipboard_paste()

        current_tree = context.space_data.edit_tree
        input_node = get_selected_node_by_idname(current_tree, 'sort_shader_node_group_input')
        output_node = get_selected_node_by_idname(current_tree, 'sort_shader_node_group_output')
        if not input_node or not output_node:
            return {'CANCELLED'}

        bpy.ops.node.select_all(action='DESELECT')

        # relink input sockets
        for socket, in_socket in zip(group_node.inputs, input_node.outputs):
            if in_socket.is_linked and socket.is_linked:
                from_socket = socket.links[0].from_socket
                for link in in_socket.links:
                    current_tree.links.new(from_socket, link.to_socket)

        # relink output sockets
        for out_socket, socket in zip(output_node.inputs, group_node.outputs):
            if out_socket.is_linked and socket.is_linked:
                from_socket = out_socket.links[0].from_socket
                for link in socket.links:
                    current_tree.links.new(from_socket, link.to_socket)

        for node in (group_node, input_node, output_node):
            current_tree.nodes.remove(node)

        return {"FINISHED"}

@base.register_class
class SORT_Node_Group_Edit_Operator(bpy.types.Operator):
    bl_label = "Edit"
    bl_idname = "sort.node_group_edit"

    from_shortcut : bpy.props.BoolProperty()

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == 'SORT'

    def execute(self, context):
        ng = bpy.data.node_groups
        node = context.active_node if self.from_shortcut else context.node
        parent_tree = node.id_data
        group_tree = get_node_groups_by_id(node.bl_idname)

        path = context.space_data.path
        space_data = context.space_data
        if len(path) == 1:
            path.start(parent_tree)
            path.append(group_tree, node=node)
        else:
            path.append(group_tree, node=node)

        return {"FINISHED"}

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
        yield NodeItem(ng.sort_data.group_name_id, ng.name)