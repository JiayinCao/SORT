#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
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
from . import base, renderer
import bpy.utils.previews
from .strid import SID

SORT_NODE_GROUP_PREFIX = 'SORTGroupName_'

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
        bpy.types.NodeTree.volume_step = bpy.props.FloatProperty( name='Step' , default=0.1 , min=0.0, max=100.0 )
        bpy.types.NodeTree.volume_step_cnt = bpy.props.IntProperty( name='Max Step Count' , default=1024 , min=0, max=8192 )

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

    # This function is only used to update group inputs/outouts when GroupInputNode/GroupOutputNode is deleted
    def update(self):
        # only handle the deleting of group input/output nodes for now
        if is_sort_node_group(self) is False:
            return

        # get the edited tree
        tree = get_node_groups_by_id( self.sort_data.group_name_id )
        if tree is None:
            return

        # get group input and output
        input_node = tree.nodes.get( 'Group Inputs' )
        output_node = tree.nodes.get( 'Group Outputs' )

        # update instances
        for instance in instances(tree):
            if input_node is None:
                sockets = getattr(instance, 'inputs')
                sockets.clear()

            if output_node is None:
                sockets = getattr(instance, 'outputs')
                sockets.clear()

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
#                                Misc Helper function                                #
#------------------------------------------------------------------------------------#

# get all instances of a speific group type
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

    yield nodeitems_utils.NodeItemCustom(draw=group_tools_draw)

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
        yield nodeitems_utils.NodeItem(ng.sort_data.group_name_id, ng.name)

# keep appending a larger number until there is one available, there is not an optimal solution for sure
# given the limted number of paramters in each shader node, it is fine to use it.
def getUniqueSocketName(socket_names, socket_name):
    i = 0
    new_socket_name = socket_name
    while True:
        found_duplication = False
        for name in socket_names:
            if name == new_socket_name:
                found_duplication = True

        if found_duplication:
            new_socket_name = socket_name + str( i )
            i += 1
        else:
            break

    # should not be able to reach here
    return new_socket_name

def socket_index(socket):
    node = socket.node
    sockets = node.outputs if socket.is_output else node.inputs
    for i, s in enumerate(sockets):
        if s == socket:
            return i

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
        new_socket.sort_label = new_socket.name

        for to_socket in to_sockets:
            ng.links.new(new_socket, to_socket)

    else:
        inputs = socket.node.inputs
        from_socket = socket.links[0].from_socket if socket.is_linked else None

        inputs.remove(socket)
        new_socket = inputs.new(new_type, socket_name)
        inputs.move(len(inputs) - 1, socket_pos)
        new_socket.sort_label = new_socket.name

        if from_socket:
            ng.links.new(from_socket, new_socket)

    return new_socket

def get_other_socket(socket):
    if not socket.is_linked:
        return None

    for link in socket.links:
        other = link.from_socket if not socket.is_output else link.to_socket

        # special handling for reroute node
        if other.node.bl_idname == 'NodeReroute':
            sockets = other.node.inputs if not socket.is_output else other.node.outputs
            ret = None
            for socket in sockets:
                ret = get_other_socket( socket )
            if ret is not None:
                return ret
        else:
            return other
    return None

def get_socket_data(socket):
    other = get_other_socket(socket)
    if socket.bl_idname == "sort_dummy_socket":
        socket = get_other_socket(socket)
    # this could happen when connecting socket from group input/output node or shader group input node to a reroute node connects nothing
    # in which case there will be no way to deduce the type and name of the newly created socket.
    if socket is None:
        return '' , ''
    return socket.name, socket.bl_idname

def generate_inputs(tree):
    in_socket = []
    input_node = tree.nodes.get("Group Inputs")
    existed_name = []
    if input_node:
        for idx, socket in enumerate(input_node.outputs):
            if socket.is_linked:
                socket_name, socket_bl_idname = get_socket_data(socket)
                if socket_name == '':
                    assert( socket_bl_idname == '' )
                    continue
                socket_name = getUniqueSocketName( existed_name , socket_name )
                in_socket.append([socket_name, socket_bl_idname])
                existed_name.append( socket_name )
    return in_socket

def generate_outputs(tree):
    out_socket = []
    output_node = tree.nodes.get("Group Outputs")
    existed_name = []
    if output_node:
        for socket in output_node.inputs:
            if socket.is_linked:
                socket_name, socket_bl_idname = get_socket_data(socket)
                if socket_name == '':
                    assert( socket_bl_idname == '' )
                    continue
                socket_name = getUniqueSocketName( existed_name , socket_name )
                out_socket.append((socket_name, socket_bl_idname))
                existed_name.append( socket_name )
    return out_socket

# this is a very important helper function, every time the node interface is changed
# it is necessary to call this function to make it registered so that data is consistant
def update_cls(tree):
    class C(SORTGroupNode):
        bl_idname = tree.sort_data.group_name_id
        bl_label = 'SORT Group'
        input_template = generate_inputs(tree)
        output_template = generate_outputs(tree)

        @classmethod
        def getGroupTree(cls):
            return get_node_groups_by_id(cls.bl_idname)

    # re-register the class
    old_cls_ref = getattr(bpy.types, C.bl_idname, None)
    if old_cls_ref:
        bpy.utils.unregister_class(old_cls_ref)
    bpy.utils.register_class(C)

    return C

# approximate a proper location for node group input and output
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

@bpy.app.handlers.persistent
def node_groups_load_post(dummy):
    node_groups = [ng for ng in bpy.data.node_groups if is_sort_node_group(ng)]
    for ng in node_groups:
        update_cls(ng)

#------------------------------------------------------------------------------------#
#                                  Shader Node Socket                                #
#------------------------------------------------------------------------------------#

# this function is to avoid setting duplicated names for group input/output or shader group input
def update_socket_name(self, context):
    # check for name duplication
    node = self.node
    tree = node.id_data

    def get_one_instance(tree):
        for instance in instances(tree):
            return instance
        return None

    if node.isGroupInputNode():
        instance = get_one_instance( tree )
        if instance:
            for input in instance.inputs:
                # reset the name if there is a duplication
                if input.name == self.sort_label:
                    # this condition is necessary to avoid infinite recursive calling
                    if self.sort_label != self.name:
                        self.sort_label = self.name
                    return
    elif node.isGroupOutputNode():
        instance = get_one_instance( tree )
        if instance:
            for input in instance.outputs:
                # reset the name if there is a duplication
                if input.name == self.sort_label:
                    # this condition is necessary to avoid infinite recursive calling
                    if self.sort_label != self.name:
                        self.sort_label = self.name
                    return
    elif node.isShaderGroupInputNode():
        for input in node.inputs:
            # reset the name if there is a duplication
            if input.name == self.sort_label:
                # this condition is necessary to avoid infinite recursive calling
                if self.sort_label != self.name:
                    self.sort_label = self.name
                return
    else:
        return

    idx = -1
    if self.is_output:
        for i , output in enumerate(node.outputs):
            if output == self:
                idx = i
                break
    else:
        for i , input in enumerate(node.inputs):
            if input == self:
                idx = i
                break

    if node.isGroupInputNode():
        if idx >= 0 and idx < len( node.outputs ):
            node.outputs[idx].name = self.sort_label

        # need to update all instances' inputs
        tree = node.id_data
        for instance in instances(tree):
            if idx >= 0 and idx < len( instance.inputs ):
                instance.inputs[idx].name = self.sort_label
        return

    if node.isGroupOutputNode():
        if idx >= 0 and idx < len( node.inputs ):
            node.inputs[idx].name = self.sort_label

        # need to update all instances' inputs
        tree = node.id_data
        for instance in instances(tree):
            if idx >= 0 and idx < len( instance.outputs ):
                instance.outputs[idx].name = self.sort_label
        return

    # this must be shader group inputs
    if idx >= 0 and idx < len( node.inputs ):
        node.inputs[idx].name = self.sort_label
    if idx >= 0 and idx < len( node.outputs ):
        node.outputs[idx].name = self.sort_label

def get_from_socket(socket):
    if not socket.is_linked:
        return None
    other = socket.links[0].from_socket
    if other.node.bl_idname == 'NodeReroute':
        return get_from_socket(other.node.inputs[0])
    else:
        return other

# Base class for sort socket
class SORTNodeSocket:
    socket_color = (0.1, 0.1, 0.1, 0.75)

    # this is a very hacky way to support name update in Blender because I have no idea how to get callback function from native str class
    sort_label : bpy.props.StringProperty( name = '' , default = 'default' , update = update_socket_name )

    def __int__(self):
        self.sort_label.default = self.name

    # this is not an inherited function
    def draw_label(self, context, layout, node, text):
        source_socket = get_from_socket(self)
        has_error = False
        if source_socket is not None and source_socket.get_socket_data_type() != self.get_socket_data_type():
            has_error = True
        if has_error:
            layout.label(text=text,icon='CANCEL')
        else:
            layout.label(text=text)

    # Customized color for the socket
    def draw_color(self, context, node):
        return self.socket_color

    #draw socket property in node
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            layout.prop( node.inputs[text] , 'default_value' , text = text)

    def get_socket_data_type(self):
        return 'None'

    def isDummySocket(self):
        return False

# Socket for BXDF or Materials
@base.register_class
class SORTNodeSocketBxdf(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketBxdf'
    bl_label = 'SORT Shader Socket'
    socket_color = (0.2, 0.2, 1.0, 1.0)
    default_value = None
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            layout.label(text=text)
    def export_osl_value(self):
        return 'color(0)'
    def get_socket_data_type(self):
        return 'bxdf'
    def serialize(self,fs):
        pass

# Socket for volume
@base.register_class
class SORTNodeSocketVolume(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketVolume'
    bl_label = 'SORT Volume Socket'
    socket_color = (1.0, 0.5, 0.1, 1.0)
    default_value = None
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            layout.label(text=text)
    def export_osl_value(self):
        return 'color(0)'
    def get_socket_data_type(self):
        return 'volume'

# Socket for Color
@base.register_class
class SORTNodeSocketColor(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'
    socket_color = (0.1, 1.0, 0.2, 1.0)
    default_value : bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) ,subtype='COLOR',soft_min = 0.0, soft_max = 1.0)

    def export_osl_value(self):
        return 'color( %f, %f, %f )'%(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(3)
        fs.serialize(self.default_value[:])

# Socket for Float
@base.register_class
class SORTNodeSocketFloat(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    default_value : bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0, max=1.0 )
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(1)
        fs.serialize(self.default_value)

# Socket for Float Vector
@base.register_class
class SORTNodeSocketFloatVector(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketFloatVector'
    bl_label = 'SORT Float Vector Socket'
    socket_color = (0.1, 0.6, 0.3, 1.0)
    default_value : bpy.props.FloatVectorProperty( name='Float' , default=(0.0,0.0,0.0) , min=-float('inf'), max=float('inf') )
    def export_osl_value(self):
        return 'vector(%f,%f,%f)'%(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(3)
        fs.serialize(self.default_value[:])

@base.register_class
class SORTNodeSocketPositiveFloatVector(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketPositiveFloatVector'
    bl_label = 'SORT Float Vector Socket'
    socket_color = (0.1, 0.6, 0.3, 1.0)
    default_value : bpy.props.FloatVectorProperty( name='Float' , default=(0.0,0.0,0.0) , min=float(0.0), max=float('inf') )
    def export_osl_value(self):
        return 'vector(%f,%f,%f)'%(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(3)
        fs.serialize(self.default_value[:])

# Socket for Positive Float
@base.register_class
class SORTNodeSocketLargeFloat(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketLargeFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    default_value : bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0)
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(1)
        fs.serialize(self.default_value)

# Socket for Any Float
@base.register_class
class SORTNodeSocketAnyFloat(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketAnyFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    default_value : bpy.props.FloatProperty( name='Float' , default=0.0 , min=-float('inf'), max=float('inf'))
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(1)
        fs.serialize(self.default_value)

# Socket for normal ( normal map )
@base.register_class
class SORTNodeSocketNormal(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketNormal'
    bl_label = 'SORT Normal Socket'
    socket_color = (0.1, 0.4, 0.3, 1.0)
    default_value : bpy.props.FloatVectorProperty( name='Normal' , default=(0.0,1.0,0.0) , min=-1.0, max=1.0 )
    # normal socket doesn't show the vector because it is not supposed to be edited this way.
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            row = layout.row()
            split = row.split(factor=0.4)
            split.label(text=text)
    def export_osl_value(self):
        return 'normal( %f , %f , %f )' %(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(3)
        fs.serialize(self.default_value[:])

# Socket for UV Mapping
@base.register_class
class SORTNodeSocketUV(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketUV'
    bl_label = 'SORT UV Mapping'
    socket_color = (0.9, 0.2, 0.8, 1.0)
    default_value : bpy.props.FloatVectorProperty( name='Float' , default=(0.0,1.0,0.0) , min=0.0, max=1.0 )
    # uvmapping socket doesn't show the vector because it is not supposed to be edited this way.
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            row = layout.row()
            split = row.split(factor=0.4)
            split.label(text=text)
    def export_osl_value(self):
        return 'vector( u , v , 0.0 )'
    def get_socket_data_type(self):
        return 'vector3'
    def serialize(self,fs):
        fs.serialize(self.name.replace(" " , ""))
        fs.serialize(4) # this is just a magic number to indicate system value type, will refactor later
        fs.serialize('uvw')

@base.register_class
class SORTDummySocket(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = "sort_dummy_socket"
    bl_label = "SPRT Dummy Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.6, 0.6, 0.5)

    def isDummySocket(self):
        return True

#------------------------------------------------------------------------------------#
#                         Root base class for all SORT Node                          #
#------------------------------------------------------------------------------------#

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
        return self.bl_idname
    # whether the node is a group node
    def isGroupNode(self):
        return False
    # whether the node is a group input
    def isGroupInputNode(self):
        return False
    # whether the node is a group output
    def isGroupOutputNode(self):
        return False
    # whether the node is a shader group input
    def isShaderGroupInputNode(self):
        return False
    # get shader parameter name
    def getShaderInputParameterName(self,param):
        return param.replace( ' ' , '' )
    def getShaderOutputParameterName(self,param):
        return param.replace( ' ' , '' )
    # get unique name
    def getUniqueName(self):
        return self.name + str( self.as_pointer() )
    # whether the node is a transparent node
    def isTransparentNode(self):
        return False
    # whether the node is a sss node
    def isSSSNode(self):
        return False
    # whether the shader node needs to export its shader source
    def needSerializingShader(self):
        return True
    # serialize shader resource data
    def serialize_shader_resource(self, fs):
        fs.serialize(0)

#------------------------------------------------------------------------------------#
#                                Group Node Operators                                #
#------------------------------------------------------------------------------------#

@base.register_class
class SORT_Node_Group_Make_Operator(bpy.types.Operator):
    bl_label = "Make Group"
    bl_idname = "sort.node_group_make"

    def execute(self, context):
        # get all connected links
        def get_links(tree):
            def keys_sort(link):
                return (socket_index(link.to_socket), link.from_node.location.y)
            def is_input_connection_valid( link ):
                if ( (not link.from_node.select) and (link.to_node.select) ) is False:
                    return False
                if link.to_node.bl_idname != 'NodeReroute':
                    return True
                return get_other_socket( link.from_socket ) is not None

            def is_output_connection_valid( link ):
                if ( (link.from_node.select) and (not link.to_node.select) ) is False:
                    return False
                if link.from_node.bl_idname != 'NodeReroute':
                    return True
                return get_other_socket( link.to_socket ) is not None

            input_links = sorted([l for l in tree.links if is_input_connection_valid(l) ], key=keys_sort)
            output_links = sorted([l for l in tree.links if is_output_connection_valid(l) ], key=keys_sort)
            return dict(input=input_links, output=output_links)

        # get average location of all nodes picked
        def get_average_location(nodes):
            x, y = 0, 0
            for node in nodes:
                x += node.location[0]
                y += node.location[1]
            d = 1.0 / len(nodes)
            return x * d, y * d

        # link nodes inside the group
        def link_tree(tree, links):
            nodes = tree.nodes
            input_node = nodes.get("Group Inputs")
            output_node = nodes.get("Group Outputs")
            relink_in = []
            relink_out = []

            inputs_remap = {}
            for index, l in enumerate(links['input']):
                i = socket_index(l.to_socket)
                to_socket = nodes[l.to_node.name].inputs[i]
                if l.from_socket in inputs_remap:
                    out_index = inputs_remap[l.from_socket]
                    from_socket = input_node.outputs[out_index]
                    tree.links.new(from_socket, to_socket)
                else:
                    inputs_remap[l.from_socket] = len(input_node.outputs) - 1
                    tree.links.new(input_node.outputs[-1], to_socket)
                relink_in.append((l.from_socket, inputs_remap[l.from_socket]))

            outputs_map = {}
            for index, l in enumerate(links['output']):
                i = socket_index(l.from_socket)
                from_socket = nodes[l.from_node.name].outputs[i]
                if from_socket in outputs_map:
                    index = outputs_map[from_socket]
                    tree.links.new(from_socket, output_node.inputs[index])
                else:
                    outputs_map[from_socket] = len(output_node.inputs) - 1
                    tree.links.new(from_socket, output_node.inputs[-1])
                relink_out.append((outputs_map[from_socket], l.to_node.name, socket_index(l.to_socket)))

            return relink_in, relink_out

        # make a group
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

        # link all connections for parent node
        def link_tree_instance(node, relinks):
            tree = node.id_data
            input_relink, output_relink = relinks
            for socket, index in input_relink:
                tree.links.new(socket, node.inputs[index])
            for index, name, socket_index in output_relink:
                tree.links.new(node.outputs[index], tree.nodes[name].inputs[socket_index])

        tree = context.space_data.edit_tree
        for node in tree.nodes:
            if node.bl_idname == 'NodeReroute' or node.bl_idname == 'NodeFrame':
                continue
            if node.isGroupInputNode() or node.isGroupOutputNode() or node.isShaderGroupInputNode() or node.bl_idname == SORTNodeOutput.bl_idname:
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
        # get the picked node by ID
        def get_selected_node_by_idname(tree, name):
            for node in tree.nodes:
                if not node.select:
                    continue
                if node.bl_idname == name:
                    return node
            return None

        # current group node
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
    '''Edit the group'''
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

#------------------------------------------------------------------------------------#
#                              Shader Input/Output Nodes                             #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Shader Input Output')
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'Shader Output'
    bl_idname = 'SORTNodeOutput'
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface' )
        self.inputs.new( 'SORTNodeSocketVolume' , 'Volume' )

    # whether the shader node needs to export its shader source
    def needSerializingShader(self):
        return False
    # no unique name for output node
    def getUniqueName(self):
        return 'ShaderOutput_'
    # get the surface shader if connected
    def getSurfaceShader(self):
        from_socket = get_from_socket( self.inputs[0] )
        return ( None , False ) if from_socket is None else ( from_socket.node , True )
    # whether there is connection in volume shader
    def getVolumeShader(self):
        # this is a hacky line to make old assets compatible with the new shader output node layout
        if len(self.inputs) is 1:
            return ( None , False )
        from_socket = get_from_socket( self.inputs[1] )
        return ( None , False ) if from_socket is None else ( from_socket.node , True )

@base.register_class
class SORTNodeExposedInputs(SORTShadingNode):
    bl_label = 'Shader Inputs'
    bl_idname = 'SORTNodeExposedInputs'
    node_kind = 'outputs'

    def init(self, context):
        self.outputs.new( 'sort_dummy_socket' , 'Input' )

    # whether the node is a shader group input
    def isShaderGroupInputNode(self):
        return True

    # need to expose this node every time it has an instance
    def type_identifier(self):
        return self.bl_idname + str( self.as_pointer() )

    def update(self):
        last_output = self.outputs[-1]

        # if last socket is not connected, nothing to do here.
        if len(last_output.links) == 0:
            return

        to_socket = get_other_socket(last_output)
        if to_socket is None:
            return

        socket_names = []
        for output in self.outputs:
            socket_names.append( output.name )

        new_socket_name = getUniqueSocketName( socket_names , to_socket.name )
        replace_socket(last_output, to_socket.bl_idname, new_name=new_socket_name)

        # it also needs an input since it is a real node with shader
        input_socket = self.inputs.new( to_socket.bl_idname , new_socket_name )
        input_socket.enabled = False

        self.inputs[to_socket.name].sort_label = new_socket_name
        self.outputs[to_socket.name].sort_label = new_socket_name

        # create another dummy socket at last
        self.outputs.new('sort_dummy_socket', '')

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
            osl_shader += input_type + ' ' + self.getShaderInputParameterName(input.name) + ' = @, \n'
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

    # this function helps serializing the material information
    def serialize_prop(self,fs):
        inputs = self.inputs
        fs.serialize(len(inputs)*2)
        for input in inputs:
            fs.serialize( input.export_osl_value() )
        # this time it is for output default values, this is useless, but needed by OSL compiler
        for input in inputs:
            fs.serialize( input.export_osl_value() )

#------------------------------------------------------------------------------------#
#                                  Shader Group Nodes                                #
#------------------------------------------------------------------------------------#
map_lookup = {'outputs': 'inputs', 'inputs': 'outputs'}
class SORTNodeSocketConnectorHelper:
    node_kind : bpy.props.StringProperty()

    def update(self):
        def get_socket_names(sockets):
            socket_names = []
            for socket in sockets:
                socket_names.append( socket.name )
            return socket_names

        def get_one_instance(tree):
            for instance in instances(tree):
                return instance
            return None

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
            if get_other_socket( socket_list[-1] ) is None:
                return

            socket = socket_list[-1]
            cls = update_cls(tree)
            socket_names = []
            if kind == "outputs":
                new_name, new_type = cls.input_template[-1]
                instance = get_one_instance(tree)
                if instance is not None:
                    socket_names = get_socket_names( instance.inputs )
            else:
                new_name, new_type = cls.output_template[-1]
                instance = get_one_instance(tree)
                if instance is not None:
                    socket_names = get_socket_names( instance.outputs )

            # make sure the name is unique
            new_name = getUniqueSocketName( socket_names , new_name )

            new_socket = replace_socket(socket, new_type, new_name=new_name)

            # update instances
            for instance in instances(tree):
                sockets = getattr(instance, map_lookup[kind])
                new_socket = sockets.new(new_type, new_name)

            socket_list.new('sort_dummy_socket', '')

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
        assert( ng is not None )

        row = layout.row()
        row.prop(ng, 'name', text='')
        row.operator('sort.node_group_edit', text='', icon= 'GROUP')

    def init(self, context):
        tree = get_node_groups_by_id(self.bl_idname)
        if not tree:
            return

        input_template = generate_inputs(tree)
        for socket_name , socket_bl_idname in input_template:
            input_socket = self.inputs.new(socket_bl_idname, socket_name)
            input_socket.sort_label = input_socket.name

        output_template = generate_outputs(tree)
        for socket_name , socket_bl_idname in output_template:
            output_socket = self.outputs.new(socket_bl_idname, socket_name)
            output_socket.sort_label = output_socket.name

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
            osl_shader += input_type + ' ' + self.getShaderInputParameterName(input.name) + ' = @, \n'
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

    # this function helps serializing the material information
    def serialize_prop(self,fs):
        inputs = self.inputs
        fs.serialize(len(inputs)*2)
        for input in inputs:
            fs.serialize( input.export_osl_value() )
        # this time it is for output default values, this is useless, but needed by OSL compiler
        for input in inputs:
            fs.serialize( input.export_osl_value() )

    # get unique name, group node doesn't need to have instance even if it has, but the shaders are exactly the same
    def getUniqueName(self):
        return self.bl_idname + str( self.as_pointer() )

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

        last_input_id = -1
        osl_shader = 'shader PassThrough_GroupInput('
        for i in range( 0 , len(inputs) ):
            input = inputs[i]
            if input.isDummySocket():
                continue
            input_type = socket_type_mapping[input.bl_idname]
            osl_shader += input_type + ' ' + self.getShaderInputParameterName(input.name) + ' = ' + input.export_osl_value() + ', \n'

            last_input_id = i
        for i in range( 0 , len(inputs) ):
            output = inputs[i]
            if output.isDummySocket():
                continue
            output_type = socket_type_mapping[output.bl_idname]
            osl_shader += 'output ' + output_type + ' ' + self.getShaderOutputParameterName(output.name) + ' = ' + output.export_osl_value() + ', \n'
            if i != last_input_id:
                osl_shader += ',\n'
            else:
                osl_shader += '){\n'

        for i in range( 0 , len(inputs) ):
            if inputs[i].isDummySocket():
                continue
            var_name = inputs[i].name
            osl_shader += self.getShaderOutputParameterName(var_name) + ' = ' + self.getShaderInputParameterName(var_name) + ';\n'
        osl_shader += '}'
        return osl_shader

#------------------------------------------------------------------------------------#
#                                   BXDF Nodes                                       #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Diffuse(SORTShadingNode):
    bl_label = 'Diffuse'
    bl_idname = 'SORTNode_Material_Diffuse'
    tsl_shader_diffuse = '''
        shader bxdf_lambert(color Diffuse, vector Normal, out closure Result){
            Result = make_closure<lambert>( Diffuse , Normal );
        }
    '''
    tsl_shader_orennayar = '''
        shader bxdf_orennayar( float Roughness,
                               color Diffuse,
                               vector Normal,
                               out closure Result ){
            Result = make_closure<oren_nayar>( Diffuse , Roughness , Normal );
        }
    '''
    def update_brdf(self,context):
        if self.brdf_type == 'OrenNayar':
            self.inputs['Roughness'].enabled = True
        else:
            self.inputs['Roughness'].enabled = False
    brdf_type : bpy.props.EnumProperty(name='Type', items=[('Lambert','Lambert','',1),('OrenNayar','OrenNayar','',2)], default='Lambert', update=update_brdf)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Roughness'].enabled = False
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)
    def serialize_prop(self, fs):
        if self.brdf_type == 'OrenNayar':
            fs.serialize( 3 )
            self.inputs['Roughness'].serialize(fs)
            self.inputs['Diffuse'].serialize(fs)
            self.inputs['Normal'].serialize(fs)
        else:
            fs.serialize( 2 )
            self.inputs['Diffuse'].serialize(fs)
            self.inputs['Normal'].serialize(fs)
    def generate_osl_source(self):
        if self.brdf_type == 'Lambert':
            return self.tsl_shader_diffuse
        return self.tsl_shader_orennayar
    def type_identifier(self):
        return self.bl_idname + self.brdf_type

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_LambertTransmission(SORTShadingNode):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_Material_LambertTransmission'
    osl_shader = '''
        shader bxdf_lamberttransmission( color BaseColor ,
                                    vector Normal ,
                                    out closure Result ){
            Result = make_closure<lambert_transmission>(BaseColor, Normal);
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Mirror(SORTShadingNode):
    bl_label = 'Mirror'
    bl_idname = 'SORTNode_Material_Mirror'
    osl_shader = '''
        shader Mirror( color BaseColor ,
                       vector Normal ,
                       out closure Result ){
            Result = make_closure<mirror>( BaseColor , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Plastic(SORTShadingNode):
    bl_label = 'Plastic'
    bl_idname = 'SORTNode_Material_Plastic'
    osl_shader = '''
        shader bxdf_plastic( color Diffuse ,
                             color Specular ,
                             float Roughness ,
                             vector Normal ,
                             out closure Result ){
            // Ideally, if closure supports += operator, this shader could have been much simplified.
            bool has_diffuse = Diffuse.r != 0.0f || Diffuse.g != 0.0f || Diffuse.b != 0.0f;
            bool has_specular = Specular.r != 0.0f || Specular.b != 0.0f || Specular.b != 0.0f;

            if( has_diffuse && has_specular )
                Result = make_closure<lambert>( Diffuse , Normal ) + make_closure<microfacet_dielectric>( 1.0f, 1.5f, Roughness, Roughness, Specular , Normal );
            else if( has_diffuse )
                Result = make_closure<lambert>( Diffuse , Normal );
            else if( has_specular )
                Result = make_closure<microfacet_dielectric>( 1.0f, 1.5f, Roughness, Roughness, Specular , Normal );
            else
                Result = make_closure<empty_closure>(0.0f);
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Roughness'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        self.inputs['Diffuse'].serialize(fs)
        self.inputs['Specular'].serialize(fs)
        self.inputs['Roughness'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Glass(SORTShadingNode):
    bl_label = 'Glass'
    bl_idname = 'SORTNode_Material_Glass'
    osl_shader = '''
        shader bxdf_glass( color Reflectance ,
                      color Transmittance ,
                      float RoughnessU ,
                      float RoughnessV ,
                      vector Normal ,
                      out closure Result ){
            Result = make_closure<dieletric>( Reflectance , Transmittance , RoughnessU , RoughnessV , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Reflectance' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Transmittance' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        self.inputs['Reflectance'].serialize(fs)
        self.inputs['Transmittance'].serialize(fs)
        self.inputs['RoughnessU'].serialize(fs)
        self.inputs['RoughnessV'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_UE4Principle(SORTShadingNode):
    bl_label = 'UE4 Principle'
    bl_idname = 'SORTNode_Material_UE4Principle'
    osl_shader = '''
        shader Principle( float RoughnessU ,
                          float RoughnessV ,
                          float Metallic ,
                          float Specular ,
                          color BaseColor ,
                          vector Normal ,
                          out closure Result ){
            // UE4 PBS model, this is obviously very wrong since I have no time digging into UE4 for now.
            color ior, absorb;
            ior.r = ior.g = ior.b = 0.37f;
            absorb.r = absorb.g = absorb.b = 2.82f;
            Result = make_closure<lambert>( BaseColor , Normal ) * ( 1.0f - Metallic ) * 0.92f + 
                     make_closure<microfacet_reflection_ggx>( ior, absorb, RoughnessU, RoughnessV, BaseColor , Normal ) * ( Metallic * 0.92 + 0.08 * Specular );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Metallic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Metallic'].default_value = 1.0
        self.inputs['RoughnessU'].default_value = 0.2
        self.inputs['RoughnessV'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 6 )
        self.inputs['RoughnessU'].serialize(fs)
        self.inputs['RoughnessV'].serialize(fs)
        self.inputs['Metallic'].serialize(fs)
        self.inputs['Specular'].serialize(fs)
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_DisneyBRDF(SORTShadingNode):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'
    tsl_shader_thin_surface = '''
        shader bxdf_disney( float Metallic ,
                       float Specular ,
                       float SpecularTint ,
                       float Roughness ,
                       float Anisotropic ,
                       float Sheen ,
                       float SheenTint ,
                       float Clearcoat ,
                       float ClearcoatGlossiness ,
                       float SpecularTransmittance ,
                       vector ScatterDistance ,
                       float Flatness ,
                       float DiffuseTransmittance ,
                       color BaseColor ,
                       vector Normal ,
                       out closure Result ){
            Result = make_closure<disney>( Metallic , Specular , SpecularTint , Roughness , Anisotropic , Sheen , SheenTint , Clearcoat , ClearcoatGlossiness ,
                             SpecularTransmittance , ScatterDistance , Flatness , DiffuseTransmittance , true , BaseColor , Normal );
        }
    '''
    tsl_shader_non_thin_surface = '''
        shader bxdf_disney( float Metallic ,
                       float Specular ,
                       float SpecularTint ,
                       float Roughness ,
                       float Anisotropic ,
                       float Sheen ,
                       float SheenTint ,
                       float Clearcoat ,
                       float ClearcoatGlossiness ,
                       float SpecularTransmittance ,
                       vector ScatterDistance ,
                       float Flatness ,
                       float DiffuseTransmittance ,
                       color BaseColor ,
                       vector Normal ,
                       out closure Result ){
            Result = make_closure<disney>( Metallic , Specular , SpecularTint , Roughness , Anisotropic , Sheen , SheenTint , Clearcoat , ClearcoatGlossiness ,
                             SpecularTransmittance , ScatterDistance , Flatness , DiffuseTransmittance , false , BaseColor , Normal );
        }
    '''

    bl_width_min = 200
    is_thin_surface : bpy.props.BoolProperty(name='Is Thin Surface', default=False)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Metallic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Anisotropic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular Tint' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Sheen' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Sheen Tint' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Clearcoat' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Clearcoat Glossiness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular Transmittance')
        self.inputs.new( 'SORTNodeSocketPositiveFloatVector' , 'Scatter Distance')
        self.inputs.new( 'SORTNodeSocketFloat' , 'Flatness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Diffuse Transmittance' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Metallic'].default_value = 1.0
        self.inputs['Roughness'].default_value = 0.2
        self.inputs['Specular'].default_value = 1.0
        self.inputs['Clearcoat Glossiness'].default_value = 1.0
        self.inputs['Sheen'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 15 )
        self.inputs['Metallic'].serialize(fs)
        self.inputs['Specular'].serialize(fs)
        self.inputs['Specular Tint'].serialize(fs)
        self.inputs['Roughness'].serialize(fs)
        self.inputs['Anisotropic'].serialize(fs)
        self.inputs['Sheen'].serialize(fs)
        self.inputs['Sheen Tint'].serialize(fs)
        self.inputs['Clearcoat'].serialize(fs)
        self.inputs['Clearcoat Glossiness'].serialize(fs)
        self.inputs['Specular Transmittance'].serialize(fs)
        self.inputs['Scatter Distance'].serialize(fs)
        self.inputs['Flatness'].serialize(fs)
        self.inputs['Diffuse Transmittance'].serialize(fs)
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Normal'].serialize(fs)
    def draw_buttons(self, context, layout):
        layout.prop(self, 'is_thin_surface', text='Is Thin Surface')
    def isSSSNode(self):
        metallic = self.inputs['Metallic'].default_value
        if metallic == 1:
            return False
        sd = self.inputs['Scatter Distance'].default_value
        return sd[0] > 0 or sd[1] > 0 or sd[2] > 0
    def generate_osl_source(self):
        if self.is_thin_surface:
            return self.tsl_shader_thin_surface
        return self.tsl_shader_non_thin_surface

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Hair(SORTShadingNode):
    bl_label = 'Hair'
    bl_idname = 'SORTNode_Material_Hair'
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        // functions defined by c library
        float powf( float base , float exp );
        float logf( float x );

        float helper( float x , float inv ){
            float y = logf(x) * inv;
            return y * y;
        }
        shader Hair( color HairColor ,
                     float LongtitudinalRoughness ,
                     float AzimuthalRoughness ,
                     float IndexofRefraction ,
                     out closure Result ){
            float inv = 1.0 / ( 5.969 - 0.215 * AzimuthalRoughness + 2.532 * powf(AzimuthalRoughness,2.0) - 10.73 * powf(AzimuthalRoughness,3.0) +
                        5.574 * powf(AzimuthalRoughness,4.0) + 0.245 * powf(AzimuthalRoughness, 5.0) );
            color sigma;
            sigma.r = helper(HairColor[0],inv);
            sigma.g = helper(HairColor[1],inv);
            sigma.b = helper(HairColor[2],inv);
            Result = make_closure<hair>( sigma , LongtitudinalRoughness , AzimuthalRoughness , IndexofRefraction );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'HairColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Longtitudinal Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Azimuthal Roughness' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Index of Refraction' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Longtitudinal Roughness'].default_value = 0.2
        self.inputs['Azimuthal Roughness'].default_value = 0.2
        self.inputs['Index of Refraction'].default_value = 1.55
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        self.inputs['HairColor'].serialize(fs)
        self.inputs['Longtitudinal Roughness'].serialize(fs)
        self.inputs['Azimuthal Roughness'].serialize(fs)
        self.inputs['Index of Refraction'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Coat(SORTShadingNode):
    bl_label = 'Coat'
    bl_idname = 'SORTNode_Material_Coat'
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        // functions defined by c library
        float powf( float base , float exp );
        float logf( float x );

        float helper( float x , float inv ){
            float y = logf(x) * inv;
            return y * y;
        }
        shader bxdf_coat(   float     IndexofRefraction ,
                            float     Roughness ,
                            color     ColorTint ,
                            closure   Surface ,
                            vector    Normal ,
                            out closure Result ){
            float inv = 1.0 / ( 5.969 - 0.215 * Roughness + 2.532 * powf(Roughness,2.0) - 10.73 * powf(Roughness,3.0) + 5.574 * powf(Roughness,4.0) + 0.245 * powf(Roughness, 5.0) );
            color sigma;
            sigma.r = helper(ColorTint.r,inv);
            sigma.g = helper(ColorTint.g,inv);
            sigma.b = helper(ColorTint.b,inv);
            Result = make_closure<coat>( Surface , Roughness , IndexofRefraction , sigma , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Index of Refraction' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketColor' , 'ColorTint' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Index of Refraction'].default_value = 1.55
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        self.inputs['Index of Refraction'].serialize(fs)
        self.inputs['Roughness'].serialize(fs)
        self.inputs['ColorTint'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Measured(SORTShadingNode):
    bl_label = 'Measured BRDF'
    bl_idname = 'SORTNode_Material_Measured'
    tsl_shader_merl = '''
        shader_resource measured_data;
        shader bxdf_merl( vector Normal ,
                          out closure Result ){
            Result = make_closure<merl>( measured_data , Normal );
        }
    '''
    tsl_shader_fourier = '''
        shader_resource measured_data;
        shader bxdf_fourier( vector Normal ,
                             out closure Result ){
            Result = make_closure<fourier>( measured_data , Normal );
        }
    '''
    brdf_type : bpy.props.EnumProperty(name='Type', items=[('FourierBRDF','FourierBRDF','',1),('MERL','MERL','',2)], default='FourierBRDF')
    file_path : bpy.props.StringProperty( name='FilePath' , subtype='FILE_PATH' )
    ResourceIndex : bpy.props.IntProperty( name='ResourceId' )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.ResourceIndex = 0
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)
        layout.prop(self, 'file_path', text='File Path')
    def generate_osl_source(self):
        return self.tsl_shader_fourier if self.brdf_type == 'FourierBRDF' else self.tsl_shader_merl
    def populateResources( self , resources ):
        found = False
        for resource in resources:
            if resource[1] == self.file_path:
                found = True
        if not found:
            self.ResourceIndex = len(resources)
            if self.brdf_type == 'FourierBRDF':
                resources.append( ( self.file_path , SID('FourierBRDFMeasuredData') ) )
            else:
                resources.append( ( self.file_path , SID('MerlBRDFMeasuredData') ) )
        pass
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        self.inputs['Normal'].serialize(fs)
    def serialize_shader_resource(self, fs):
        fs.serialize(1)
        fs.serialize('measured_data')
        fs.serialize(self.file_path)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_MicrofacetReflection(SORTShadingNode):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_Material_MicrofacetReflection'
    bl_width_min = 256
    tsl_shader_ggx = '''
        shader bxdf_microfacet_reflection_ggx(  vector InteriorIOR ,
                                                vector AbsorptionCoefficient ,
                                                float  RoughnessU ,
                                                float  RoughnessV ,
                                                color  BaseColor ,
                                                vector Normal ,
                                                out closure Result ){
            Result = make_closure<microfacet_reflection_ggx>( InteriorIOR , AbsorptionCoefficient , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    tsl_shader_blinn = '''
        shader bxdf_microfacet_reflection_ggx(  vector InteriorIOR ,
                                                vector AbsorptionCoefficient ,
                                                float  RoughnessU ,
                                                float  RoughnessV ,
                                                color  BaseColor ,
                                                vector Normal ,
                                                out closure Result ){
            Result = make_closure<microfacet_reflection_blinn>( InteriorIOR , AbsorptionCoefficient , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    tsl_shader_beckmann = '''
        shader bxdf_microfacet_reflection_ggx(  vector InteriorIOR ,
                                                vector AbsorptionCoefficient ,
                                                float  RoughnessU ,
                                                float  RoughnessV ,
                                                color  BaseColor ,
                                                vector Normal ,
                                                out closure Result ){
            Result = make_closure<microfacet_reflection_beckmann>( InteriorIOR , AbsorptionCoefficient , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    distribution : bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior : bpy.props.FloatVectorProperty( name='Interior IOR' , default=(0.37,0.37,0.37) , min=1.0, max=10.0 )
    absopt_co : bpy.props.FloatVectorProperty( name='Absoprtion Coefficient' , default=(2.82,2.82,2.82) , min=0.0, max=10.0 )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'distribution', text='Distribution', expand=True)
        layout.prop(self, 'interior_ior', text='Interior IOR')
        layout.prop(self, 'absopt_co', text='Absorption Coefficient')
    def serialize_prop(self, fs):
        fs.serialize( 6 )
        fs.serialize('InteriorIOR')
        fs.serialize(3)
        fs.serialize(self.interior_ior[:])
        fs.serialize('AbsorptionCoefficient')
        fs.serialize(3)
        fs.serialize(self.absopt_co[:])
        self.inputs['RoughnessU'].serialize(fs)
        self.inputs['RoughnessV'].serialize(fs)
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Normal'].serialize(fs)
    def generate_osl_source(self):
        if self.distribution == 'GGX':
            return self.tsl_shader_ggx
        if self.distribution == 'Blinn':
            return self.tsl_shader_blinn
        return self.tsl_shader_beckmann

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_MicrofacetRefraction(SORTShadingNode):
    bl_label = 'MicrofacetRefraction'
    bl_idname = 'SORTNode_Material_MicrofacetRefraction'
    bl_width_min = 256
    tsl_shader_ggx = '''
        shader bxdf_microfacetRefraction_ggx(   float  InteriorIOR ,
                                                float  ExteriorIOR ,
                                                float  RoughnessU ,
                                                float  RoughnessV ,
                                                color  BaseColor ,
                                                vector Normal ,
                                                out closure Result ){
            Result = make_closure<microfacet_refraction_ggx>( InteriorIOR , ExteriorIOR , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    tsl_shader_blinn = '''
        shader bxdf_microfacetRefraction_blinn( float  InteriorIOR ,
                                                float  ExteriorIOR ,
                                                float  RoughnessU ,
                                                float  RoughnessV ,
                                                color  BaseColor ,
                                                vector Normal ,
                                                out closure Result ){
            Result = make_closure<microfacet_refraction_blinn>( InteriorIOR , ExteriorIOR , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    tsl_shader_beckmann = '''
        shader bxdf_microfacetRefraction_beckmann(  float  InteriorIOR ,
                                                    float  ExteriorIOR ,
                                                    float  RoughnessU ,
                                                    float  RoughnessV ,
                                                    color  BaseColor ,
                                                    vector Normal ,
                                                    out closure Result ){
            Result = make_closure<microfacet_refraction_beckmann>( InteriorIOR , ExteriorIOR , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    distribution : bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior : bpy.props.FloatProperty( name='Interior IOR' , default=1.1 , min=1.0, max=10.0 )
    exterior_ior : bpy.props.FloatProperty( name='Exterior IOR' , default=1.0 , min=0.0, max=10.0 )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['RoughnessU'].default_value = 0.2
        self.inputs['RoughnessV'].default_value = 0.2
    def draw_buttons(self, context, layout):
        layout.prop(self, 'distribution', text='Distribution', expand=True)
        layout.prop(self, 'interior_ior', text='Interior IOR')
        layout.prop(self, 'exterior_ior', text='Exterior IOR')
    def serialize_prop(self, fs):
        fs.serialize(6)
        fs.serialize('InteriorIOR')
        fs.serialize(1)
        fs.serialize(self.interior_ior)
        fs.serialize('ExteriorIOR')
        fs.serialize(1)
        fs.serialize(self.exterior_ior)

        self.inputs['RoughnessU'].serialize(fs)
        self.inputs['RoughnessV'].serialize(fs)
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

    def generate_osl_source(self):
        if self.distribution == 'GGX':
            return self.tsl_shader_ggx
        if self.distribution == 'Blinn':
            return self.tsl_shader_blinn
        return self.tsl_shader_beckmann

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_AshikhmanShirley(SORTShadingNode):
    bl_label = 'AshikhmanShirley'
    bl_idname = 'SORTNode_Material_AshikhmanShirley'
    osl_shader = '''
        shader bxdf_ashikhmanshirley( float Specular ,
                                      float RoughnessU ,
                                      float RoughnessV ,
                                      color Diffuse ,
                                      vector Normal ,
                                      out closure Result ){
            Result = make_closure<ashikhman_shirley>(Specular, RoughnessU, RoughnessV, Diffuse, Normal);
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['RoughnessU'].default_value = 0.2
        self.inputs['RoughnessV'].default_value = 0.2
        self.inputs['Specular'].default_value = 0.5
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        self.inputs['Specular'].serialize(fs)
        self.inputs['RoughnessU'].serialize(fs)
        self.inputs['RoughnessV'].serialize(fs)
        self.inputs['Diffuse'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_ModifiedPhong(SORTShadingNode):
    bl_label = 'Modified Phong'
    bl_idname = 'SORTNode_Material_ModifiedPhong'
    osl_shader = '''
        shader bxdf_modified_phong( float SpecularPower ,
                                    float DiffuseRatio ,
                                    color Specular ,
                                    color Diffuse ,
                                    vector Normal ,
                                    out closure Result ){
            // Ideally, TSL should support float * float3, however this is a workaround for now since it is now supported.
            // I need to get this supported eventually
            color resolved_diffuse;
            resolved_diffuse.r = Diffuse.r * DiffuseRatio;
            resolved_diffuse.g = Diffuse.g * DiffuseRatio;
            resolved_diffuse.b = Diffuse.b * DiffuseRatio;
            color resolved_specular;
            resolved_specular.r = ( 1.0f - DiffuseRatio ) * Specular.r;
            resolved_specular.g = ( 1.0f - DiffuseRatio ) * Specular.g;
            resolved_specular.b = ( 1.0f - DiffuseRatio ) * Specular.b;
            Result = make_closure<phong>( resolved_diffuse , resolved_specular , SpecularPower , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Specular Power' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Diffuse Ratio' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Specular Power'].default_value = 32.0
        self.inputs['Diffuse Ratio'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        self.inputs['Specular Power'].serialize(fs)
        self.inputs['Diffuse Ratio'].serialize(fs)
        self.inputs['Diffuse'].serialize(fs)
        self.inputs['Specular'].serialize(fs)
        self.inputs['Normal'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Cloth(SORTShadingNode):
    bl_label = 'Cloth'
    bl_idname = 'SORTNode_Material_Cloth'
    tsl_shader_dbrdf = '''
        shader DistributionBRDF(  color BaseColor ,
                                  float Roughness ,
                                  float Specular ,
                                  float SpecularTint ,
                                  vector Normal ,
                                  out closure Result ){
            Result = make_closure<distribution_brdf>( BaseColor , Roughness , Specular , SpecularTint , Normal );
        }
    '''
    tsl_shader_dwa_fabric = '''
        shader DWA_Fabric( color BaseColor ,
                           float Roughness ,
                           vector Normal ,
                           out closure Result ){
            Result = make_closure<fabric>( BaseColor , Roughness , Normal );
        }
    '''
    def update_brdf(self,context):
        if self.brdf_type == 'TheOrder_Fabric':
            self.inputs['Specular'].enabled = True
            self.inputs['SpecularTint'].enabled = True
        else:
            self.inputs['Specular'].enabled = False
            self.inputs['SpecularTint'].enabled = False
    brdf_type : bpy.props.EnumProperty(name='Type', items=[('TheOrder_Fabric','TheOrder_Fabric','',1),('DreamWorks_Fabric','DreamWorks_Fabric','',2)], default='TheOrder_Fabric', update=update_brdf)
    def generate_osl_source(self):
        if self.brdf_type == 'TheOrder_Fabric':
            return self.tsl_shader_dbrdf
        return self.tsl_shader_dwa_fabric
    def type_identifier(self):
        return self.bl_idname + self.brdf_type
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'SpecularTint' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        if self.brdf_type == 'TheOrder_Fabric':
            fs.serialize( 5 )
        else:
            fs.serialize( 3 )
        self.inputs['BaseColor'].serialize(fs)
        self.inputs['Roughness'].serialize(fs)
        if self.brdf_type == 'TheOrder_Fabric':
            self.inputs['Specular'].serialize(fs)
            self.inputs['SpecularTint'].serialize(fs)
        self.inputs['Normal'].serialize(fs)
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_SSS(SORTShadingNode):
    bl_label = 'Subsurface Scattering'
    bl_idname = 'SORTNode_Material_SSS'
    bl_width_min = 300
    osl_shader = '''
        shader MaterialSSS(  color BaseColor ,
                             color ScatterDistance ,
                             vector Normal ,
                             out closure Result ){
            Result = make_closure<subsurface_scattering>( BaseColor , ScatterDistance , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Base Color' )
        self.inputs.new( 'SORTNodeSocketPositiveFloatVector' , 'Scatter Distance')
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )

        self.inputs['Scatter Distance'].default_value = ( 1.0 , 0.2 , 0.1 )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        self.inputs['Base Color'].serialize(fs)
        self.inputs['Scatter Distance'].serialize(fs)
        self.inputs['Normal'].serialize(fs)
    def isSSSNode(self):
        return True

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Transparent(SORTShadingNode):
    bl_label = 'Transparent'
    bl_idname = 'SORTNode_Material_Transparent'
    osl_shader = '''
        shader bxdf_transparent(  color Attenuation ,
                                  out closure Result ){
            Result = make_closure<transparent>( Attenuation );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Attenuation' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        self.inputs['Attenuation'].serialize(fs)
    def isTransparentNode(self):
        return True

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Add(SORTShadingNode):
    bl_label = 'Add'
    bl_idname = 'SORTNode_Material_Add'
    osl_shader = '''
        shader bxdf_material_add(   closure Surface0 ,
                                    closure Surface1 ,
                                    out closure Result ){
            Result = Surface0 + Surface1;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface0' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface1' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 0 )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Blend(SORTShadingNode):
    bl_label = 'Blend'
    bl_idname = 'SORTNode_Material_Blend'
    osl_shader = '''
        shader bxdf_material_blend(  closure Surface0 ,
                                     closure Surface1 ,
                                     float Factor ,
                                     out closure Result ){
            Result = Surface0 * ( 1.0 - Factor ) + Surface1 * Factor;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface0' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        self.inputs['Factor'].serialize(fs)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_DoubleSided(SORTShadingNode):
    bl_label = 'Double-Sided'
    bl_idname = 'SORTNode_Material_DoubleSided'
    osl_shader = '''
        shader MaterialBlend(  closure FrontSurface ,
                               closure BackSurface ,
                               out closure Result ){
            Result = make_closure<double_sided>( FrontSurface , BackSurface );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Front Surface' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Back Surface' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 0 )

#------------------------------------------------------------------------------------#
#                                   Texture Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Textures')
class SORTNodeCheckerBoard(SORTShadingNode):
    bl_label = 'CheckerBoard'
    bl_idname = 'SORTNodeCheckerBoard'
    osl_shader = '''
        float floorf(float x);

        shader CheckerBoard( color Color1 ,
                             color Color2 ,
                             vector UVCoordinate ,
                             float  UVTiling ,
                             out color Result ,
                             out float Red ,
                             out float Green ,
                             out float Blue ){
            // this will be revived once TSL supports proper multiplication later.
            // vector scaledUV = UVCoordinate * UVTiling;
            vector scaledUV;
            scaledUV.x = UVCoordinate.x * UVTiling;
            scaledUV.y = UVCoordinate.y * UVTiling;
            scaledUV.z = UVCoordinate.z * UVTiling;
            float fu = scaledUV.x - floorf( scaledUV.x );
            float fv = scaledUV.y - floorf( scaledUV.y );
            if( ( fu > 0.5f && fv > 0.5f ) || ( fu < 0.5f && fv < 0.5f ) )
                Result = Color1;
            else
                Result = Color2;
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels : bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'UV Tiling' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
        self.inputs['UV Tiling'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        self.inputs['Color1'].serialize(fs)
        self.inputs['Color2'].serialize(fs)
        self.inputs['UV Coordinate'].serialize(fs)
        self.inputs['UV Tiling'].serialize(fs)
    def draw_buttons(self, context, layout):
        layout.prop(self, "show_separate_channels")

@SORTShaderNodeTree.register_node('Textures')
class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'
    osl_shader = '''
        float floorf(float x);

        shader Grid( color Color1 ,
                     color Color2 ,
                     float Threshold ,
                     vector UVCoordinate ,
                     float  UVTiling ,
                     out color Result ,
                     out float Red ,
                     out float Green ,
                     out float Blue ){
            // this will be revived once TSL supports proper multiplication later.
            // vector scaledUV = UVCoordinate * UVTiling;
            vector scaledUV;
            scaledUV.x = UVCoordinate.x * UVTiling;
            scaledUV.y = UVCoordinate.y * UVTiling;
            scaledUV.z = UVCoordinate.z * UVTiling;

            float fu = scaledUV.x - floorf( scaledUV.x ) - 0.5f;
            float fv = scaledUV.y - floorf( scaledUV.y ) - 0.5f;
            float half_threshold = ( 1.0 - Threshold ) * 0.5f;
            if( fu <= half_threshold && fu >= -half_threshold && fv <= half_threshold && fv >= -half_threshold )
                Result = Color1;
            else
                Result = Color2;
            Red = Result.r;
            Green = Result.g;
            Blue = Result.b;
        }
    '''
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels : bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Threshold' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'UV Tiling' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
        self.inputs['Threshold'].default_value = 0.1
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
        self.inputs['UV Tiling'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        self.inputs['Color1'].serialize(fs)
        self.inputs['Color2'].serialize(fs)
        self.inputs['Threshold'].serialize(fs)
        self.inputs['UV Coordinate'].serialize(fs)
        self.inputs['UV Tiling'].serialize(fs)
    def draw_buttons(self, context, layout):
        layout.prop(self, "show_separate_channels")

preview_collections = {}
@SORTShaderNodeTree.register_node('Textures')
class SORTNodeImage(SORTShadingNode):
    bl_label = 'Image'
    bl_idname = 'SORTNodeImage'
    bl_width_min = 200
    items = (('Linear', "Linear", "Linear"), ('sRGB', "sRGB", "sRGB"), ('Normal', 'Normal', 'Normal'))
    color_space_type : bpy.props.EnumProperty(name='Color Space', items=items, default='Linear')
    wrap_items = (('REPEAT', "Repeat", "Repeating Texture"),
             ('MIRRORED_REPEAT', "Mirror", "Texture mirrors outside of 0-1"),
             ('CLAMP_TO_EDGE', "Clamp to Edge", "Clamp to Edge.  Outside 0-1 the texture will smear."),
             ('CLAMP_ZERO', "Clamp to Black", "Clamp to Black outside 0-1"),
             ('CLAMP_ONE', "Clamp to White", "Clamp to White outside 0-1"),)
    wrap_type : bpy.props.EnumProperty(name='Wrap Type', items=wrap_items, default='REPEAT')
    image_preview : bpy.props.BoolProperty(name='Preview Image', default=True)
    tsl_shader_linear = '''
        texture2d g_texture;
        shader ImageShaderLinear( vector UVCoordinate ,
                                  float  UVTiling ,
                                  out color Result ,
                                  out float Alpha ,
                                  out float Red ,
                                  out float Green ,
                                  out float Blue ){
            // this will be revived once TSL supports proper multiplication later.
            // vector scaledUV = UVCoordinate * UVTiling;
            vector scaledUV;
            scaledUV.x = UVCoordinate.x * UVTiling;
            scaledUV.y = UVCoordinate.y * UVTiling;
            scaledUV.z = UVCoordinate.z * UVTiling;

            Result = texture2d_sample<g_texture>( scaledUV[0] , scaledUV[1] );
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    tsl_shader_gamma = '''
        float powf( float base , float exp );

        texture2d g_texture;
        shader ImageShaderGamma( vector UVCoordinate ,
                                 float  UVTiling ,
                                 out color Result ,
                                 out float Alpha ,
                                 out float Red ,
                                 out float Green ,
                                 out float Blue ){
            // this will be revived once TSL supports proper multiplication later.
            // vector scaledUV = UVCoordinate * UVTiling;
            vector scaledUV;
            scaledUV.x = UVCoordinate.x * UVTiling;
            scaledUV.y = UVCoordinate.y * UVTiling;
            scaledUV.z = UVCoordinate.z * UVTiling;

            Result = texture2d_sample<g_texture>( scaledUV[0] , scaledUV[1] );
            Result = powf( gamma_color , 2.2 );
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    tsl_shader_normal = '''
        texture2d g_texture;
        shader ImageShaderNormal(vector UVCoordinate ,
                                 float  UVTiling ,
                                 out color Result ,
                                 out float Alpha ,
                                 out float Red ,
                                 out float Green ,
                                 out float Blue ){
            // this will be revived once TSL supports proper multiplication later.
            // vector scaledUV = UVCoordinate * UVTiling;
            vector scaledUV;
            scaledUV.x = UVCoordinate.x * UVTiling;
            scaledUV.y = UVCoordinate.y * UVTiling;
            scaledUV.z = UVCoordinate.z * UVTiling;
            
            vector encoded_normal = texture2d_sample<g_texture>( scaledUV.x , scaledUV.y );
            Result.x = 2.0f * encoded_normal.x - 1.0f;
            Result.y = 2.0f * encoded_normal.z - 1.0f;
            Result.z = 2.0f * encoded_normal.y - 1.0f;

            Red = Result.x;
            Green = Result.y;
            Blue = Result.z;
        }
    '''
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels : bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def generate_preview(self, context):
        name = self.name + '_' + self.id_data.name
        if name not in preview_collections:
            item = bpy.utils.previews.new()
            item.previews = ()
            item.image_name = ''
            preview_collections[name] = item
        item = preview_collections[name]
        wm = context.window_manager
        enum_items = []
        img = self.image
        if img:
            new_image_name = img.name
            if item.image_name == new_image_name:
                return item.previews
            else:
                item.image_name = new_image_name
            item.clear()
            thumb = item.load(img.name, bpy.path.abspath(img.filepath), 'IMAGE')

            # somehow, it doesn't show the preview without this line
            thumb.image_size[0]

            enum_items = [(img.filepath, img.name, '', thumb.icon_id, 0)]
        item.previews = enum_items
        return item.previews
    image : bpy.props.PointerProperty(type=bpy.types.Image)
    preview : bpy.props.EnumProperty(items=generate_preview)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'UV Tiling' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Alpha' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.inputs['UV Tiling'].default_value = 1.0
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    def draw_buttons(self, context, layout):
        layout.template_ID(self, "image", open="image.open")
        layout.prop(self, 'image_preview' )
        if self.image_preview:
            layout.template_icon_view(self, 'preview', show_labels=True)
        layout.prop(self, 'show_separate_channels' )
        layout.prop(self, 'color_space_type', expand=True)
        layout.prop(self, 'wrap_type')
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        self.inputs['UV Coordinate'].serialize(fs)
        self.inputs['UV Tiling'].serialize(fs)
    def generate_osl_source(self):
        if self.color_space_type == 'sRGB':
            return self.tsl_shader_gamma
        elif self.color_space_type == 'Normal':
            return self.tsl_shader_normal
        return self.tsl_shader_linear
    def type_identifier(self):
        return self.bl_idname + self.color_space_type
    def populateResources( self , resources ):
        found = False
        for resource in resources:
            if resource[1] == self.image.filepath:
                found = True
        if not found:
            self.ResourceIndex = len(resources)
            resources.append( ( self.image.filepath , SID('Texture2D') ) )
        pass
    # serialize shader resource data
    def serialize_shader_resource(self, fs):
        fs.serialize(1)
        fs.serialize('g_texture')
        fs.serialize(self.image.filepath)

#------------------------------------------------------------------------------------#
#                                 Convertor Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeRemappingUV(SORTShadingNode):
    bl_label = 'RemappingUV'
    bl_idname = 'SORTNodeRemappingUV'
    output_type = 'SORTNodeSocketFloat'
    osl_shader = '''
        shader RemappingUV( vector UVCoordinate,
                            float  TilingU ,
                            float  TilingV ,
                            float  OffsetU ,
                            float  OffsetV ,
                            out vector Result ){
            Result.x = UVCoordinate.x * TilingU + OffsetU;
            Result.y = UVCoordinate.y * TilingV + OffsetV;
            Result.z = 0.0f;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'TilingU' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'TilingV' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'OffsetU' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'OffsetV' )
        self.outputs.new( 'SORTNodeSocketUV' , 'Result' )
        self.inputs['TilingU'].default_value = 1.0
        self.inputs['TilingV'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        self.inputs['UV Coordinate'].serialize(fs)
        self.inputs['TilingU'].serialize(fs)
        self.inputs['TilingV'].serialize(fs)
        self.inputs['OffsetU'].serialize(fs)
        self.inputs['OffsetV'].serialize(fs)

@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeExtract(SORTShadingNode):
    bl_label = 'Extract'
    bl_idname = 'SORTNodeExtract'
    osl_shader = '''
        shader Extract( color Color,
                        out float Red ,
                        out float Green ,
                        out float Blue ,
                        out float Intensity ){
            Red = Color.r;
            Green = Color.g;
            Blue = Color.b;
            Intensity = Color.r * 0.212671f + Color.g * 0.715160f + Color.b * 0.072169f;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Intensity' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        self.inputs['Color'].serialize(fs)

@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeComposite(SORTShadingNode):
    bl_label = 'Composite'
    bl_idname = 'SORTNodeComposite'
    osl_shader = '''
        shader Composite( float Red ,
                          float Green ,
                          float Blue ,
                          out color Color ){
            Color.r = Red;
            Color.g = Green;
            Color.b = Blue;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Color' )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        self.inputs['Red'].serialize(fs)
        self.inputs['Green'].serialize(fs)
        self.inputs['Blue'].serialize(fs)

#------------------------------------------------------------------------------------#
#                                 Input Nodes                                        #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputIntersection(SORTShadingNode):
    bl_label = 'Intersection'
    bl_idname = 'SORTNodeInputIntersection'
    bl_width_min = 160
    osl_shader = '''
        shader InputShader( output vector WorldPosition = P ,
                            output vector WorldViewDirection = I ,
                            output vector WorldShadingNormal = N ,
                            output vector WorldGeometryNormal = Ng ,
                            output vector UVCoordinate = vector( u , v , 0.0 ) ){
        }
    '''
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'World Position' )
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'World View Direction' )
        self.outputs.new( 'SORTNodeSocketNormal' , 'World Shading Normal' )
        self.outputs.new( 'SORTNodeSocketNormal' , 'World Geometry Normal' )
        self.outputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputFloat(SORTShadingNode):
    bl_label = 'Float'
    bl_idname = 'SORTNodeInputFloat'
    osl_shader = '''
        shader constant_float( float Value,
                              out float Result ){
            Result = Value;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketLargeFloat' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        self.inputs['Value'].serialize(fs)

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputFloatVector(SORTShadingNode):
    bl_label = 'Vector'
    bl_idname = 'SORTNodeInputFloatVector'
    bl_width_min = 256
    osl_shader = '''
        shader ConstantFloatVector( vector Value ,
                                    out vector Result ){
            Result = Value;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value' )
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        self.inputs['Value'].serialize(fs)

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputColor(SORTShadingNode):
    bl_label = 'Color'
    bl_idname = 'SORTNodeInputColor'
    osl_shader = '''
        shader constant_color( color Color,
                               out color Result ){
            Result = Color;
        }
    '''
    color : bpy.props.FloatVectorProperty(name='Color', subtype='COLOR', min=0.0, max=1.0, size=3, default=(1.0, 1.0, 1.0))
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.template_color_picker(self, 'color', value_slider=True)
        layout.prop(self, 'color', text='')
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize('Color')
        fs.serialize(3)
        fs.serialize(self.color[:])

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputFresnel(SORTShadingNode):
    bl_label = 'Fresnel'
    bl_idname = 'SORTNodeInputFresnel'
    osl_shader = '''
        shader SchlickFresnel( float F0 = @,
                               output float Result = 0.0 ){
            float cos_theta = dot( N , I );
            Result = F0 + pow( 1.0 - cos_theta , 5.0 ) * ( 1.0 - F0 );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'F0' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Result' )
        self.inputs['F0'].default_value = 0.08

    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['F0'].export_osl_value() )

#------------------------------------------------------------------------------------#
#                                 Math Op Nodes                                      #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpUnary(SORTShadingNode):
    bl_label = 'Unary Operator'
    bl_idname = 'SORTNodeMathOpUnary'
    bl_width_min = 240
    osl_shader = '''
        shader MathUnaryOp( %s Value = @ ,
                            output %s Result = 0.0 ){
            Result = %s(Value);
        }
    '''
    def change_type(self,context):
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Value' )
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type == 'SORTNodeSocketFloatVector':
            self.inputs['Value'].default_value = ( 0.0 , 0.0 , 0.0 )
        elif self.data_type == 'SORTNodeSocketColor':
            self.inputs['Value'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value'].default_value = 1.0
    op_type : bpy.props.EnumProperty(name='Type',default='-',items=[
        ('-','Negation','',1), ('1.0-','One Minus','',2), ('sin','Sin','',3), ('cos','Cos','',4), ('tan','Tan','',5), ('asin','Asin','',6), ('acos','Acos','',7), ('atan','Atan','',8),
        ('exp','Exp','',9), ('exp2','Exp2','',10), ('log','Log','',11), ('log2','Log2','',12), ('log10','Log10','',13), ('sqrt','Sqrt','',14), ('inversesqrt','Inverse Sqrt','',15),
        ('fabs','Abs','', 16), ('sign','Sign','',17), ('floor','Floor','',18), ('ceil','Ceil','',19), ('round','Round','',20), ('trunc','Trunc','',21) ])
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'op_type', text='Type')
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Value'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , self.op_type )
    def type_identifier(self):
        return self.bl_idname + self.data_type + self.op_type

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpBinary(SORTShadingNode):
    bl_label = 'Binary Operator'
    bl_idname = 'SORTNodeMathOpBinary'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( %s Value0 = @ ,
                             %s Value1 = @ ,
                             output %s Result = 0.0 ){
            Result = Value0 %s Value1;
        }
    '''
    def change_type(self,context):
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Value0' )
        self.inputs.new( self.data_type , 'Value1' )
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Value0'].default_value = ( 1.0 , 1.0 , 1.0 )
            self.inputs['Value1'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value0'].default_value = 1.0
            self.inputs['Value1'].default_value = 1.0
    op_type : bpy.props.EnumProperty(name='Type',default='+',items=[('+','Add','',1),('-','Substract','',2),('*','Multiply','',3),('/','Divide','',4)])
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value0' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value1' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'op_type', text='Type')
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Value0'].export_osl_value() )
        fs.serialize( self.inputs['Value1'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , dtype , self.op_type )
    def type_identifier(self):
        return self.bl_idname + self.data_type + self.op_type

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpDotProduce(SORTShadingNode):
    bl_label = 'Dot Product'
    bl_idname = 'SORTNodeMathOpDotProduce'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( vector Value0 = @ ,
                             vector Value1 = @ ,
                             output float Result = 0.0 ){
            Result = dot( Value0 , Value1 );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value0' )
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value1' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Value0'].export_osl_value() )
        fs.serialize( self.inputs['Value1'].export_osl_value() )

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpLerp(SORTShadingNode):
    bl_label = 'Lerp'
    bl_idname = 'SORTNodeMathOpLerp'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( %s Value0 = @ ,
                             %s Value1 = @ ,
                             float Factor = @ ,
                             output %s Result = 0.0 ){
            Result = Value0 * ( 1.0 - Factor ) + Value1 * Factor;
        }
    '''
    def change_type(self,context):
        old_factor = self.inputs['Factor'].default_value
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Value0' )
        self.inputs.new( self.data_type , 'Value1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.inputs['Factor'].default_value = old_factor
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Value0'].default_value = ( 1.0 , 1.0 , 1.0 )
            self.inputs['Value1'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value0'].default_value = 1.0
            self.inputs['Value1'].default_value = 1.0
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value0' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Value0'].export_osl_value() )
        fs.serialize( self.inputs['Value1'].export_osl_value() )
        fs.serialize( self.inputs['Factor'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , dtype )
    def type_identifier(self):
        return self.bl_idname + self.data_type

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpClamp(SORTShadingNode):
    bl_label = 'Clamp'
    bl_idname = 'SORTNodeMathOpClamp'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( %s MinValue = @ ,
                             %s MaxValue = @ ,
                             %s Value = @ ,
                             output %s Result = 0.0 ){
            Result = min( MaxValue , max( MinValue , Value ) );
        }
    '''
    def change_type(self,context):
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Min Value' )
        self.inputs.new( self.data_type , 'Max Value' )
        self.inputs.new( self.data_type , 'Value' )
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Min Value'].default_value = ( 0.0 , 0.0 , 0.0 )
            self.inputs['Max Value'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Min Value'].default_value = 0.0
            self.inputs['Max Value'].default_value = 1.0
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Min Value' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Max Value' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
        self.inputs['Max Value'].default_value = 1.0
    def draw_buttons(self, context, layout):
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Min Value'].export_osl_value() )
        fs.serialize( self.inputs['Max Value'].export_osl_value() )
        fs.serialize( self.inputs['Value'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , dtype , dtype )
    def type_identifier(self):
        return self.bl_idname + self.data_type

#------------------------------------------------------------------------------------#
#                                    Volume Node                                     #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Volume')
class SORTNodeAbsorption(SORTShadingNode):
    bl_label = 'Absorption Medium'
    bl_idname = 'SORTNodeAbsorption'
    absorption_color : bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) , subtype='COLOR', soft_min = 0.0, soft_max = 1.0)
    absorption_coeffcient : bpy.props.FloatProperty( name='Absorption Density' , default=1.0 , min=0.0, max=float('inf') )
    osl_shader = '''
        shader AbsoprtionMedium( color base_color, 
                                 float absorption,
                                 out closure Result ){
            Result = make_closure<volume_absorption>(base_color, absorption);
        }
    '''
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketVolume' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( 'base_color' )
        fs.serialize( 3 )
        fs.serialize( self.absorption_color[:] )
        fs.serialize( 'absorption' )
        fs.serialize( 1 )
        fs.serialize( self.absorption_coeffcient )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'absorption_color')
        layout.prop(self, 'absorption_coeffcient')

@SORTShaderNodeTree.register_node('Volume')
class SORTNodeHomogeneous(SORTShadingNode):
    bl_label = 'Homogeneous Medium'
    bl_idname = 'SORTNodeHomogeneous'
    absorption_color : bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) , subtype='COLOR', soft_min = 0.0, soft_max = 1.0)
    absorption_coeffcient : bpy.props.FloatProperty( name='Absorption' , default=0.5 , min=0.0, max=float('inf') )
    scattering_coeffcient : bpy.props.FloatProperty( name='Scattering' , default=0.5 , min=0.0, max=float('inf') )
    emission_coefficient  : bpy.props.FloatProperty( name='Emission' , default=0.5 , min=0.0, max=float('inf') )
    anisotropy_coeffcient : bpy.props.FloatProperty( name='Anisotropy' , default=0.0 , min=-1.0, max=1.0 )
    osl_shader = '''
        shader HomogenenousMedium( color absorption_color, 
                                   float emission_coefficient,
                                   float absorption_coeffcient,
                                   float scattering_coeffcient,
                                   float anisotropy_coeffcient,
                                   out closure Result ){
            Result = make_closure<medium_homogeneous>( absorption_color , emission_coefficient , absorption_coeffcient , scattering_coeffcient , anisotropy_coeffcient );
        }
    '''
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketVolume' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize('absorption_color')
        fs.serialize(3)
        fs.serialize(self.absorption_color[:])
        fs.serialize('emission_coefficient')
        fs.serialize(1)
        fs.serialize(self.emission_coefficient)
        fs.serialize('absorption_coeffcient')
        fs.serialize(1)
        fs.serialize(self.absorption_coeffcient)
        fs.serialize('scattering_coeffcient')
        fs.serialize(1)
        fs.serialize(self.scattering_coeffcient)
        fs.serialize('anisotropy_coeffcient')
        fs.serialize(1)
        fs.serialize(self.anisotropy_coeffcient)

    def draw_buttons(self, context, layout):
        layout.prop(self, 'absorption_color')
        layout.prop(self, 'emission_coefficient' )
        layout.prop(self, 'absorption_coeffcient')
        layout.prop(self, 'scattering_coeffcient')
        layout.prop(self, 'anisotropy_coeffcient')

@SORTShaderNodeTree.register_node('Volume')
class SORTNodeHeterogeneous(SORTShadingNode):
    bl_label = 'Heterogeneous Medium'
    bl_idname = 'SORTNodeHeterogeneous'
    osl_shader = '''
        shader HeterogeneousMedium( color Color ,
                                    float Emission ,
                                    float Absorption ,
                                    float Scattering ,
                                    float Anisotropy ,
                                    out closure Result ){
            Result = make_closure<medium_heterogeneous>(Color, Emission, Absorption, Scattering, Anisotropy );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Emission' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Absorption' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Scattering' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Anisotropy' )
        self.outputs.new( 'SORTNodeSocketVolume' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        self.inputs['Color'].serialize(fs)
        self.inputs['Emission'].serialize(fs)
        self.inputs['Absorption'].serialize(fs)
        self.inputs['Scattering'].serialize(fs)
        self.inputs['Anisotropy'].serialize(fs)

#------------------------------------------------------------------------------------#
#                                 Volume Input Node                                  #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Volume Input')
class SORTNodeVolumeColor(SORTShadingNode):
    bl_label = 'Volume Color'
    bl_idname = 'SORTNodeVolumeColor'
    osl_shader = '''
        shader VolumeDensity( output vector Result = 0.0 ){
            // this is a very special 3d texture that is treated different in the renderer
            Result = texture3d( "volume_color" , P );
        }
    '''
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 0 )

@SORTShaderNodeTree.register_node('Volume Input')
class SORTNodeVolumeDensity(SORTShadingNode):
    bl_label = 'Volume Density'
    bl_idname = 'SORTNodeVolumeDensity'
    osl_shader = '''
        shader VolumeDensity( output float Result = 0.0 ){
            // this is a very special 3d texture that is treated different in the renderer
            Result = texture3d( "volume_density" , P );
        }
    '''
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketLargeFloat' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 0 )