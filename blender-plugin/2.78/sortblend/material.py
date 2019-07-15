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
import nodeitems_utils
import random
import bpy.utils.previews
from . import base

SORT_NODE_GROUP_PREFIX = 'SORTGroupName_'

class SORTPatternNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == SORTShaderNodeTree.bl_idname and context.scene.render.engine == 'SORT'

@base.register_class
class SORTShaderNodeTree(bpy.types.NodeTree):
    bl_idname = 'SORTShaderNodeTree'
    bl_label = 'SORT Shader Editor'
    bl_icon = 'MATERIAL'
    node_categories = {}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == 'SORT'

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
    group_name_id = bpy.props.StringProperty( name='Group Tree Id', default='')
    @classmethod
    def register(cls):
        SORTShaderNodeTree.sort_data = bpy.props.PointerProperty(name="SORT Data", type=cls)
    @classmethod
    def unregister(cls):
        del SORTShaderNodeTree.sort_data

#------------------------------------------------------------------------------------#
#                                  Shader Node Socket                                #
#------------------------------------------------------------------------------------#

# SORT Node property base class
class SORTNodeProperty:
    pbrt_type = ''
    # export type in PBRT
    def export_pbrt_socket_type(self):
        return self.pbrt_type

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
        
# Base class for sort socket
class SORTNodeSocket(SORTNodeProperty):
    ui_open = bpy.props.BoolProperty(name='UI Open', default=True)
    socket_color = (0.1, 0.1, 0.1, 0.75)
    need_bxdf_node = False

    # this is a very hacky way to support name update in Blender because I have no idea how to get callback function from native str class
    sort_label = bpy.props.StringProperty( name = '' , default = 'default' , update = update_socket_name )

    def __int__(self):
        self.sort_label.default = self.name

    # this is not an inherited function
    def draw_label(self, context, layout, node, text):
        def get_from_socket(socket):
            if not socket.is_linked:
                return None
            other = socket.links[0].from_socket
            if other.node.bl_idname == 'NodeReroute':
                return get_from_socket(other.node.inputs[0])
            else:
                return other

        source_socket = get_from_socket(self)
        has_error = False
        if source_socket is not None and source_socket.get_socket_data_type() != self.get_socket_data_type():
            has_error = True
        if has_error:
            layout.label(text,icon='CANCEL')
        else:
            layout.label(text)

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
class SORTNodeSocketBxdf(bpy.types.NodeSocketShader, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketBxdf'
    bl_label = 'SORT Shader Socket'
    socket_color = (0.2, 0.2, 1.0, 1.0)
    default_value = None
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            layout.label(text)
    def export_osl_value(self):
        return 'color(0)'
    def get_socket_data_type(self):
        return 'bxdf'

# Socket for Color
class SORTNodeSocketColor(bpy.types.NodeSocketColor, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'
    socket_color = (0.1, 1.0, 0.2, 1.0)
    pbrt_type = 'rgb'
    default_value = bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) ,subtype='COLOR',soft_min = 0.0, soft_max = 1.0)
    def export_osl_value(self):
        return 'color( %f, %f, %f )'%(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'

# Socket for Float
class SORTNodeSocketFloat(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    pbrt_type = 'float'
    default_value = bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0, max=1.0 )
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'

# Socket for Float Vector
class SORTNodeSocketFloatVector(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketFloatVector'
    bl_label = 'SORT Float Vector Socket'
    socket_color = (0.1, 0.6, 0.3, 1.0)
    pbrt_type = 'float'
    default_value = bpy.props.FloatVectorProperty( name='Float' , default=(0.0,0.0,0.0) , min=-float('inf'), max=float('inf') )
    def export_osl_value(self):
        return 'vector(%f,%f,%f)'%(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'

# Socket for Positive Float
class SORTNodeSocketLargeFloat(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketLargeFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    pbrt_type = 'float'
    default_value = bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0)
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'

# Socket for Any Float
class SORTNodeSocketAnyFloat(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketAnyFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    pbrt_type = 'float'
    default_value = bpy.props.FloatProperty( name='Float' , default=0.0 , min=-float('inf'), max=float('inf'))
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'

# Socket for normal ( normal map )
class SORTNodeSocketNormal(bpy.types.NodeSocketVector, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketNormal'
    bl_label = 'SORT Normal Socket'
    socket_color = (0.1, 0.4, 0.3, 1.0)
    default_value = bpy.props.FloatVectorProperty( name='Normal' , default=(0.0,1.0,0.0) , min=-1.0, max=1.0 )
    # normal socket doesn't show the vector because it is not supposed to be edited this way.
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            row = layout.row()
            split = row.split(0.4)
            split.label(text)
    def export_osl_value(self):
        return 'normal( %f , %f , %f )' %(self.default_value[:])
    def get_socket_data_type(self):
        return 'vector3'

# Socket for UV Mapping
class SORTNodeSocketUV(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketUV'
    bl_label = 'SORT UV Mapping'
    socket_color = (0.9, 0.2, 0.8, 1.0)
    pbrt_type = 'NA'
    default_value = bpy.props.FloatVectorProperty( name='Float' , default=(0.0,1.0,0.0) , min=0.0, max=1.0 )
    # uvmapping socket doesn't show the vector because it is not supposed to be edited this way.
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            row = layout.row()
            split = row.split(0.4)
            split.label(text)
    def export_osl_value(self):
        return 'vector( u , v , 0.0 )'
    def get_socket_data_type(self):
        return 'vector3'

# dummy socket used in shader group inputs/outputs
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

def get_socket_data(socket):
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

    other = get_other_socket(socket)
    if socket.bl_idname == "sort_dummy_socket":
        socket = get_other_socket(socket)
    return socket.name, socket.bl_idname

def generate_inputs(tree):
    in_socket = []
    input_node = tree.nodes.get("Group Inputs")
    existed_name = []
    if input_node:
        for idx, socket in enumerate(input_node.outputs):
            if socket.is_linked:
                socket_name, socket_bl_idname = get_socket_data(socket)
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

@SORTShaderNodeTree.register_node('Output')
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'Shader Output'
    bl_idname = 'SORTNodeOutput'
    osl_shader = '''
        shader SORT_Shader( closure color Surface = color(0) ){
            Ci = Surface;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface' )

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

        link = last_output.links[0]
        to_socket = link.to_socket

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
            input_links = sorted([l for l in tree.links if (not l.from_node.select) and (l.to_node.select)], key=keys_sort)
            output_links = sorted([l for l in tree.links if (l.from_node.select) and (not l.to_node.select)], key=keys_sort)
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
            if node.isGroupInputNode() or node.isGroupOutputNode() or node.isShaderGroupInputNode():
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
    bl_label = "Edit"
    bl_idname = "sort.node_group_edit"

    from_shortcut = bpy.props.BoolProperty()

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
#                                  Shader Group Nodes                                #
#------------------------------------------------------------------------------------#

map_lookup = {'outputs': 'inputs', 'inputs': 'outputs'}
class SORTNodeSocketConnectorHelper:
    node_kind = bpy.props.StringProperty()

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
        if ng:
            layout.prop(ng, 'name')
        layout.operator('sort.node_group_edit', text='Edit')

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
    osl_shader_diffuse = '''
        shader Lambert( color Diffuse = @ ,
                        normal Normal = @ ,
                        output closure color Result = color(0) ){
            Result = lambert( Diffuse , Normal );
        }
    '''
    osl_shader_orennayar = '''
        shader OrenNayar( float Roughness = @,
                          color Diffuse = @ ,
                          normal Normal = @ ,
                          output closure color Result = color(0) ){
            Result = orenNayar( Diffuse , Roughness , Normal );
        }
    '''
    def update_brdf(self,context):
        if self.brdf_type == 'OrenNayar':
            self.inputs['Roughness'].enabled = True
        else:
            self.inputs['Roughness'].enabled = False
    brdf_type = bpy.props.EnumProperty(name='Type', items=[('Lambert','Lambert','',1),('OrenNayar','OrenNayar','',2)], default='Lambert', update=update_brdf)
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
            fs.serialize( self.inputs['Roughness'].export_osl_value() )
            fs.serialize( self.inputs['Diffuse'].export_osl_value() )
            fs.serialize( self.inputs['Normal'].export_osl_value() )
        else:
            fs.serialize( 2 )
            fs.serialize( self.inputs['Diffuse'].export_osl_value() )
            fs.serialize( self.inputs['Normal'].export_osl_value() )
    def generate_osl_source(self):
        if self.brdf_type == 'Lambert':
            return self.osl_shader_diffuse
        return self.osl_shader_orennayar
    def type_identifier(self):
        return self.bl_label + self.brdf_type
    def export_pbrt(self,file):
        file.write( "\"string type\" \"matte\"\n" )
        file.write( "\"rgb Kd\" [%f %f %f]\n" %(self.inputs['Diffuse'].default_value[:]) )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_LambertTransmission(SORTShadingNode):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_Material_LambertTransmission'
    osl_shader = '''
        shader LambertTransmission( color Diffuse = @ ,
                                    normal Normal = @ ,
                                    output closure color Result = color(0) ){
            Result = lambertTransmission( Diffuse , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Color'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Mirror(SORTShadingNode):
    bl_label = 'Mirror'
    bl_idname = 'SORTNode_Material_Mirror'
    pbrt_bxdf_type = 'mirror'
    osl_shader = '''
        shader Mirror( color BaseColor = @ ,
                       normal Normal = @ ,
                       output closure color Result = color(0) ){
            Result = mirror( BaseColor , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Color'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Plastic(SORTShadingNode):
    bl_label = 'Plastic'
    bl_idname = 'SORTNode_Material_Plastic'
    osl_shader = '''
        shader Plastic( color Diffuse = @ ,
                        color Specular = @ ,
                        float Roughness = @ ,
                        normal Normal = @ ,
                        output closure color Result = color(0) ){
            if( Diffuse[0] != 0 || Diffuse[1] != 0 || Diffuse[2] != 0 )
                Result += lambert( Diffuse , N );
            if( Specular[0] != 0 || Specular[1] != 0 || Specular[2] != 0 )
                Result += microfacetReflectionDieletric( "GGX", 1.0, 1.5, Roughness, Roughness, Specular , Normal );
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
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Glass(SORTShadingNode):
    bl_label = 'Glass'
    bl_idname = 'SORTNode_Material_Glass'
    osl_shader = '''
        shader Glass( color Reflectance = @ ,
                      color Transmittance = @ ,
                      float RoughnessU = @ ,
                      float RoughnessV = @ ,
                      normal Normal = @ ,
                      output closure color Result = color(0) ){
            Result = dieletric( Reflectance , Transmittance , RoughnessU , RoughnessV , Normal );
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
        fs.serialize( self.inputs['Reflectance'].export_osl_value() )
        fs.serialize( self.inputs['Transmittance'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_UE4Principle(SORTShadingNode):
    bl_label = 'UE4 Principle'
    bl_idname = 'SORTNode_Material_UE4Principle'
    osl_shader = '''
        shader Principle( float RoughnessU = @ ,
                          float RoughnessV = @ ,
                          float Metallic = @ ,
                          float Specular = @ ,
                          color BaseColor = @ ,
                          normal Normal = @ ,
                          output closure color Result = color(0) ){
            // UE4 PBS model, this may very likely not very updated.
            Result = lambert( BaseColor , Normal ) * ( 1 - Metallic ) * 0.92 + microfacetReflection( "GGX", color( 0.37 ), color( 2.82 ), RoughnessU, RoughnessV, BaseColor , Normal ) * ( Metallic * 0.92 + 0.08 * Specular );
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
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['Metallic'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_DisneyBRDF(SORTShadingNode):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'
    osl_shader = '''
        shader Disney( float Metallic = @ ,
                       float Specular = @ ,
                       float SpecularTint = @ ,
                       float Roughness = @ ,
                       float Anisotropic = @ ,
                       float Sheen = @ ,
                       float SheenTint = @ ,
                       float Clearcoat = @ ,
                       float ClearcoatGlossiness = @ ,
                       float SpecularTransmittance = @ ,
                       float ScatterDistance = @ ,
                       float Flatness = @ ,
                       float DiffuseTransmittance = @ ,
                       int   IsThinSurface = @ ,
                       color BaseColor = @ ,
                       normal Normal = @ ,
                       output closure color Result = color(0) ){
            Result = disney( Metallic , Specular , SpecularTint , Roughness , Anisotropic , Sheen , SheenTint , Clearcoat , ClearcoatGlossiness , 
                             SpecularTransmittance , ScatterDistance , Flatness , DiffuseTransmittance , IsThinSurface , BaseColor , Normal );
        }
    '''
    bl_width_min = 200
    is_thin_surface = bpy.props.BoolProperty(name='Is Thin Surface', default=False)
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
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Scatter Distance')
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
        fs.serialize( 16 )
        fs.serialize( self.inputs['Metallic'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['Specular Tint'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Anisotropic'].export_osl_value() )
        fs.serialize( self.inputs['Sheen'].export_osl_value() )
        fs.serialize( self.inputs['Sheen Tint'].export_osl_value() )
        fs.serialize( self.inputs['Clearcoat'].export_osl_value() )
        fs.serialize( self.inputs['Clearcoat Glossiness'].export_osl_value() )
        fs.serialize( self.inputs['Specular Transmittance'].export_osl_value() )
        fs.serialize( self.inputs['Scatter Distance'].export_osl_value() )
        fs.serialize( self.inputs['Flatness'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse Transmittance'].export_osl_value() )
        fs.serialize( '1' if self.is_thin_surface else '0' )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'is_thin_surface', text='Is Thin Surface')
    def export_pbrt(self,file):
        file.write( "\"string type\" \"disney\"\n" )
        file.write( "\"rgb color\" [%f %f %f]\n"%(self.inputs['BaseColor'].default_value[:]))
        file.write( "\"float anisotropic\" %f\n"%(self.inputs['Anisotropic'].default_value))
        file.write( "\"float clearcoat\" %f\n"%(self.inputs['Clearcoat'].default_value))
        file.write( "\"float clearcoatgloss\" %f\n"%(self.inputs['Clearcoat Glossiness'].default_value))
        file.write( "\"float metallic\" %f\n"%(self.inputs['Metallic'].default_value))
        file.write( "\"float roughness\" %f\n"%(self.inputs['Roughness'].default_value))
        file.write( "\"float sheen\" %f\n"%(self.inputs['Sheen'].default_value))
        file.write( "\"float sheentint\" %f\n"%(self.inputs['Sheen Tint'].default_value))
        file.write( "\"float speculartint\" %f\n"%(self.inputs['Specular Tint'].default_value))
        file.write( "\"float spectrans\" %f\n"%(self.inputs['Specular Transmittance'].default_value))
        file.write( "\"rgb scatterdistance\" [%f %f %f]\n"%(self.inputs['Scatter Distance'].default_value,self.inputs['Scatter Distance'].default_value,self.inputs['Scatter Distance'].default_value))
        file.write( "\"bool thin\" %s\n"%('\"true\"' if self.is_thin_surface else '\"false\"'))
        file.write( "\"float flatness\" %f\n"%(self.inputs['Flatness'].default_value))
        file.write( "\"float difftrans\" %f\n"%(self.inputs['Diffuse Transmittance'].default_value))

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Hair(SORTShadingNode):
    bl_label = 'Hair'
    bl_idname = 'SORTNode_Material_Hair'
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        float helper( float x , float inv ){
            float y = log(x) * inv;
            return y * y;
        }
        shader Hair( color HairColor = @ ,
                     float LongtitudinalRoughness = @ ,
                     float AzimuthalRoughness = @ ,
                     float IndexofRefraction = @ ,
                     output closure color Result = color(0) ){
            float inv = 1.0 / ( 5.969 - 0.215 * AzimuthalRoughness + 2.532 * pow(AzimuthalRoughness,2.0) - 10.73 * pow(AzimuthalRoughness,3.0) +
                        5.574 * pow(AzimuthalRoughness,4.0) + 0.245 * pow(AzimuthalRoughness, 5.0) );
            color sigma = color( helper(HairColor[0],inv) , helper(HairColor[1],inv) , helper(HairColor[2],inv) );
            Result = hair( sigma , LongtitudinalRoughness , AzimuthalRoughness , IndexofRefraction );
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
        fs.serialize( self.inputs['HairColor'].export_osl_value() )
        fs.serialize( self.inputs['Longtitudinal Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Azimuthal Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Index of Refraction'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Coat(SORTShadingNode):
    bl_label = 'Coat'
    bl_idname = 'SORTNode_Material_Coat'
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        float helper( float x , float inv ){
            float y = log(x) * inv;
            return y * y;
        }
        shader Coat( float     IndexofRefraction = @ ,
                     float     Roughness = @ ,
                     color     ColorTint = @ ,
                     closure color Surface = @ ,
                     normal Normal = @ ,
                     output closure color Result = color(0) ){
            float inv = 1.0 / ( 5.969 - 0.215 * Roughness + 2.532 * pow(Roughness,2.0) - 10.73 * pow(Roughness,3.0) + 5.574 * pow(Roughness,4.0) + 0.245 * pow(Roughness, 5.0) );
            color sigma = color( helper(ColorTint[0],inv) , helper(ColorTint[1],inv) , helper(ColorTint[2],inv) );
            Result = coat( Surface , Roughness , IndexofRefraction , sigma , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Index of Refration' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketColor' , 'ColorTint' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Index of Refration'].default_value = 1.55
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['Index of Refration'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['ColorTint'].export_osl_value() )
        fs.serialize( self.inputs['Surface'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Measured(SORTShadingNode):
    bl_label = 'Measured BRDF'
    bl_idname = 'SORTNode_Material_Measured'
    osl_shader_merl = '''
        shader merlBRDF( string Filename = @,
                         normal Normal = @ ,
                         output closure color Result = color(0) ){
            Result = merlBRDF( @ , Normal );
        }
    '''
    osl_shader_fourier = '''
        shader FourierBRDF( string Filename = @,
                            normal Normal = @ ,
                            output closure color Result = color(0) ){
            Result = fourierBRDF( @ , Normal );
        }
    '''
    osl_shader = osl_shader_fourier
    brdf_type = bpy.props.EnumProperty(name='Type', items=[('FourierBRDF','FourierBRDF','',1),('MERL','MERL','',2)], default='FourierBRDF')
    file_path = bpy.props.StringProperty( name='FilePath' , subtype='FILE_PATH' )
    ResourceIndex = bpy.props.IntProperty( name='ResourceId' )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.ResourceIndex = 0
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)
        layout.prop(self, 'file_path', text='File Path')
    def generate_osl_source(self):
        if self.brdf_type == 'FourierBRDF':
            return self.osl_shader_fourier
        return self.osl_shader_merl
    def populateResources( self , resources ):
        found = False
        for resource in resources:
            if resource[1] == self.file_path:
                found = True
        if not found:
            self.ResourceIndex = len(resources)
            if self.brdf_type == 'FourierBRDF':
                resources.append( ( self.file_path , 'FourierBRDFMeasuredData' ) )
            else:
                resources.append( ( self.file_path , 'MerlBRDFMeasuredData' ) )
        pass
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( '\"%s\"'%self.file_path )
        fs.serialize( self.inputs['Normal'].export_osl_value() )
        fs.serialize( '%i'%self.ResourceIndex )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_MicrofacetReflection(SORTShadingNode):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_Material_MicrofacetReflection'
    bl_width_min = 256
    osl_shader = '''
        shader MicrofacetRelection(  string MicroFacetDistribution = @ ,
                                     vector InteriorIOR = @ ,
                                     vector AbsorptionCoefficient = @ ,
                                     float  RoughnessU = @ ,
                                     float  RoughnessV = @ ,
                                     color  BaseColor = @ ,
                                     normal Normal = @ ,
                                     output closure color Result = color(0) ){
            Result = microfacetReflection( MicroFacetDistribution , InteriorIOR , AbsorptionCoefficient , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    distribution = bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior = bpy.props.FloatVectorProperty( name='Interior IOR' , default=(0.37,0.37,0.37) , min=1.0, max=10.0 )
    absopt_co = bpy.props.FloatVectorProperty( name='Absoprtion Coefficient' , default=(2.82,2.82,2.82) , min=0.0, max=10.0 )
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
        fs.serialize( 7 )
        fs.serialize( '\"%s\"'%(self.distribution))
        fs.serialize( 'vector( %f,%f,%f )'%(self.interior_ior[:]))
        fs.serialize( 'vector( %f,%f,%f )'%(self.absopt_co[:]))
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_MicrofacetRefraction(SORTShadingNode):
    bl_label = 'MicrofacetRefraction'
    bl_idname = 'SORTNode_Material_MicrofacetRefraction'
    bl_width_min = 256
    osl_shader = '''
        shader MicrofacetRefraction( string MicroFacetDistribution = @ ,
                                     float  InteriorIOR = @ ,
                                     float  ExteriorIOR = @ ,
                                     float  RoughnessU = @ ,
                                     float  RoughnessV = @ ,
                                     color  BaseColor = @ ,
                                     normal Normal = @ ,
                                     output closure color Result = color(0) ){
            Result = microfacetRefraction( MicroFacetDistribution , InteriorIOR , ExteriorIOR , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    distribution = bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior = bpy.props.FloatProperty( name='Interior IOR' , default=1.1 , min=1.0, max=10.0 )
    exterior_ior = bpy.props.FloatProperty( name='Exterior IOR' , default=1.0 , min=0.0, max=10.0 )
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
        fs.serialize( 7 )
        fs.serialize( '\"%s\"'%(self.distribution))
        fs.serialize( '%f'%(self.interior_ior))
        fs.serialize( '%f'%(self.exterior_ior))
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_AshikhmanShirley(SORTShadingNode):
    bl_label = 'AshikhmanShirley'
    bl_idname = 'SORTNode_Material_AshikhmanShirley'
    osl_shader = '''
        shader AshikhmanShirley( float Specular = @ ,
                                 float RoughnessU = @ ,
                                 float RoughnessV = @ ,
                                 color Diffuse = @ ,
                                 normal Normal = @ ,
                                 output closure color Result = color(0) ){
            Result = ashikhmanShirley( Specular , RoughnessU , RoughnessV , Diffuse , Normal );
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
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_ModifiedPhong(SORTShadingNode):
    bl_label = 'Modified Phong'
    bl_idname = 'SORTNode_Material_ModifiedPhong'
    osl_shader = '''
        shader Phong( float SpecularPower = @ ,
                      float DiffuseRatio = @ ,
                      color Specular = @ ,
                      color Diffuse = @ ,
                      normal Normal = @ ,
                      output closure color Result = color(0) ){
            Result = phong( Diffuse * DiffuseRatio , ( 1.0 - DiffuseRatio ) * Specular , SpecularPower , Normal );
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
        fs.serialize( self.inputs['Specular Power'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse Ratio'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Cloth(SORTShadingNode):
    bl_label = 'Cloth'
    bl_idname = 'SORTNode_Material_Cloth'
    osl_shader_dbrdf = '''
        shader DistributionBRDF(  color BaseColor = @ ,
                                  float Roughness = @ ,
                                  float Specular = @ ,
                                  float SpecularTint = @ ,
                                  normal Normal = @ ,
                                  output closure color Result = color(0) ){
            Result = distributionBRDF( BaseColor , Roughness , Specular , SpecularTint , Normal );
        }
    '''
    osl_shader_dwa_fabric = '''
        shader DWA_Fabric( color BaseColor = @ ,
                           float Roughness = @ ,
                           normal Normal = @ ,
                           output closure color Result = color(0) ){
            Result = fabric( BaseColor , Roughness , Normal );
        }
    '''
    def update_brdf(self,context):
        if self.brdf_type == 'TheOrder_Fabric':
            self.inputs['Specular'].enabled = True
            self.inputs['SpecularTint'].enabled = True
        else:
            self.inputs['Specular'].enabled = False
            self.inputs['SpecularTint'].enabled = False
    brdf_type = bpy.props.EnumProperty(name='Type', items=[('TheOrder_Fabric','TheOrder_Fabric','',1),('DreamWorks_Fabric','DreamWorks_Fabric','',2)], default='TheOrder_Fabric', update=update_brdf)
    def generate_osl_source(self):
        if self.brdf_type == 'TheOrder_Fabric':
            return self.osl_shader_dbrdf
        return self.osl_shader_dwa_fabric
    def type_identifier(self):
        return self.bl_label + self.brdf_type
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
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        if self.brdf_type == 'TheOrder_Fabric':
            fs.serialize( self.inputs['Specular'].export_osl_value() )
            fs.serialize( self.inputs['SpecularTint'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Blend(SORTShadingNode):
    bl_label = 'Blend'
    bl_idname = 'SORTNode_Material_Blend'
    osl_shader = '''
        shader MaterialBlend(  closure color Surface0 = @ ,
                               closure color Surface1 = @ ,
                               float Factor = @ ,
                               output closure color Result = color(0) ){
            Result = Surface0 * ( 1.0 - Factor ) + Surface1 * Factor;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface0' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Surface0'].export_osl_value() )
        fs.serialize( self.inputs['Surface1'].export_osl_value() )
        fs.serialize( self.inputs['Factor'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_DoubleSided(SORTShadingNode):
    bl_label = 'Double-Sided'
    bl_idname = 'SORTNode_Material_DoubleSided'
    osl_shader = '''
        shader MaterialBlend(  closure color FrontSurface = @ ,
                               closure color BackSurface = @ ,
                               output closure color Result = color(0) ){
            Result = doubleSided( FrontSurface , BackSurface );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Front Surface' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Back Surface' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Front Surface'].export_osl_value() )
        fs.serialize( self.inputs['Back Surface'].export_osl_value() )

#------------------------------------------------------------------------------------#
#                                   Texture Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Textures')
class SORTNodeCheckerBoard(SORTShadingNode):
    bl_label = 'CheckerBoard'
    bl_idname = 'SORTNodeCheckerBoard'
    osl_shader = '''
        shader CheckerBoard( color Color1 = @ ,
                             color Color2 = @ ,
                             vector UVCoordinate = @ ,
                             output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                             output float Red = 0.0 ,
                             output float Green = 0.0 ,
                             output float Blue = 0.0 ){
            float fu = UVCoordinate[0] - floor( UVCoordinate[0] );
            float fv = UVCoordinate[1] - floor( UVCoordinate[1] );
            if( ( fu > 0.5 && fv > 0.5 ) || ( fu < 0.5 && fv < 0.5 ) )
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
    show_separate_channels = bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Color1'].export_osl_value() )
        fs.serialize( self.inputs['Color2'].export_osl_value() )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, "show_separate_channels")

@SORTShaderNodeTree.register_node('Textures')
class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'
    osl_shader = '''
        shader Grid( color Color1 = @ ,
                     color Color2 = @ ,
                     float Treshold = @ ,
                     vector UVCoordinate = @ ,
                     output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                     output float Red = 0.0 ,
                     output float Green = 0.0 ,
                     output float Blue = 0.0 ){
            float fu = UVCoordinate[0] - floor( UVCoordinate[0] ) - 0.5;
            float fv = UVCoordinate[1] - floor( UVCoordinate[1] ) - 0.5;
            float half_threshold = ( 1.0 - Treshold ) * 0.5;
            if( fu <= half_threshold && fu >= -half_threshold && fv <= half_threshold && fv >= -half_threshold )
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
    show_separate_channels = bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Treshold' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
        self.inputs['Treshold'].default_value = 0.1
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        fs.serialize( self.inputs['Color1'].export_osl_value() )
        fs.serialize( self.inputs['Color2'].export_osl_value() )
        fs.serialize( self.inputs['Treshold'].export_osl_value() )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, "show_separate_channels")

preview_collections = {}
@SORTShaderNodeTree.register_node('Textures')
class SORTNodeImage(SORTShadingNode):
    bl_label = 'Image'
    bl_idname = 'SORTNodeImage'
    bl_width_min = 200
    items = (('Linear', "Linear", "Linear"), ('sRGB', "sRGB", "sRGB"), ('Normal', 'Normal', 'Normal'))
    color_space_type = bpy.props.EnumProperty(name='Color Space', items=items, default='Linear')
    wrap_items = (('REPEAT', "Repeat", "Repeating Texture"),
             ('MIRRORED_REPEAT', "Mirror", "Texture mirrors outside of 0-1"),
             ('CLAMP_TO_EDGE', "Clamp to Edge", "Clamp to Edge.  Outside 0-1 the texture will smear."),
             ('CLAMP_ZERO', "Clamp to Black", "Clamp to Black outside 0-1"),
             ('CLAMP_ONE', "Clamp to White", "Clamp to White outside 0-1"),)
    wrap_type = bpy.props.EnumProperty(name='Wrap Type', items=wrap_items, default='REPEAT')
    image_preview = bpy.props.BoolProperty(name='Preview Image', default=True)
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels = bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    osl_shader_linear = '''
        shader ImageShaderLinear( string Filename = @ ,
                                  vector UVCoordinate = @ ,
                                  output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                                  output float Red = 0.0 ,
                                  output float Green = 0.0 ,
                                  output float Blue = 0.0 ){
            Result = texture( Filename , UVCoordinate[0] , UVCoordinate[1] );
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    osl_shader_gamma = '''
        shader ImageShaderGamma( string Filename = @ ,
                                 vector UVCoordinate = @ ,
                                 output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                                 output float Red = 0.0 ,
                                 output float Green = 0.0 ,
                                 output float Blue = 0.0 ){
            color gamma_color = texture( Filename , UVCoordinate[0] , UVCoordinate[1] );
            Result = pow( gamma_color , 2.2 );
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    osl_shader_normal = '''
        shader ImageShaderNormal( string Filename = @ ,
                                 vector UVCoordinate = @ ,
                                 output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                                 output float Red = 0.0 ,
                                 output float Green = 0.0 ,
                                 output float Blue = 0.0 ){
            color encoded_color = texture( Filename , UVCoordinate[0] , UVCoordinate[1] );
            Result = 2.0 * color( encoded_color[0] , encoded_color[2] , encoded_color[1] ) - 1.0;
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
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
        img = self.get_image()
        if img:
            new_image_name =img.name
            if item.image_name == new_image_name:
                return item.previews
            else:
                item.image_name = new_image_name
            item.clear()
            thumb = item.load(img.name, bpy.path.abspath(img.filepath), 'IMAGE')
            enum_items = [(img.filepath, img.name, '', thumb.icon_id, 0)]
        item.previews = enum_items
        return item.previews
    image = bpy.props.PointerProperty(type=bpy.types.Image)
    preview = bpy.props.EnumProperty(items=generate_preview)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
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
    def draw_label(self):
        img = self.get_image()
        if not img:
            return self.name
        return img.name
    def get_image(self):
        return self.image
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( '\"%s\"'%(bpy.path.abspath(self.image.filepath)) )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
    def generate_osl_source(self):
        if self.color_space_type == 'sRGB':
            return self.osl_shader_gamma
        elif self.color_space_type == 'Normal':
            return self.osl_shader_normal
        return self.osl_shader_linear
    def type_identifier(self):
        return self.bl_label + self.color_space_type

#------------------------------------------------------------------------------------#
#                                 Convertor Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeRemappingUV(SORTShadingNode):
    bl_label = 'RemappingUV'
    bl_idname = 'SORTNodeRemappingUV'
    output_type = 'SORTNodeSocketFloat'
    osl_shader = '''
        shader Extract( vector UVCoordinate = @,
                        float  TilingU = @ ,
                        float  TilingV = @ ,
                        float  OffsetU = @ ,
                        float  OffsetV = @ ,
                        output vector Result = vector( 0.0 ) ){
            Result = vector( UVCoordinate[0] * TilingV + OffsetU , UVCoordinate[1] * TilingU + OffsetV , 0.0 );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'TilingU' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'TilingV' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'OffsetU' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'OffsetV' )
        self.outputs.new( 'SORTNodeSocketUV' , 'Result' )
        self.inputs['TilingU'].default_value = 1.0
        self.inputs['TilingV'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
        fs.serialize( self.inputs['TilingU'].export_osl_value() )
        fs.serialize( self.inputs['TilingV'].export_osl_value() )
        fs.serialize( self.inputs['OffsetU'].export_osl_value() )
        fs.serialize( self.inputs['OffsetV'].export_osl_value() )

@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeExtract(SORTShadingNode):
    bl_label = 'Extract'
    bl_idname = 'SORTNodeExtract'
    osl_shader = '''
        shader Extract( color Color = @,
                        output float Red = 0.0 ,
                        output float Green = 0.0 ,
                        output float Blue = 0.0 ,
                        output float Intensity = 0.0 ){
            Red = Color[0];
            Green = Color[1];
            Blue = Color[2];
            Intensity = Color[0] * 0.212671 + Color[1] * 0.715160 + Color[2] * 0.072169;
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
        fs.serialize( self.inputs['Color'].export_osl_value() )

@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeComposite(SORTShadingNode):
    bl_label = 'Composite'
    bl_idname = 'SORTNodeComposite'
    osl_shader = '''
        shader Composite( float Red = @ ,
                          float Green = @ ,
                          float Blue = @ ,
                          output color Color = color( 0.0 , 0.0 , 0.0 ) ){
            Color = color( Red , Green , Blue );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Color' )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Red'].export_osl_value() )
        fs.serialize( self.inputs['Green'].export_osl_value() )
        fs.serialize( self.inputs['Blue'].export_osl_value() )

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
        shader ConstantFloat( float Value = @ ,
                              output float Result = 0.0 ){
            Result = Value;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketLargeFloat' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Value'].export_osl_value() )

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputFloatVector(SORTShadingNode):
    bl_label = 'Vector'
    bl_idname = 'SORTNodeInputFloatVector'
    bl_width_min = 256
    osl_shader = '''
        shader ConstantFloat( vector Value = @ ,
                              output vector Result = 0.0 ){
            Result = Value;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value' )
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Value'].export_osl_value() )

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputColor(SORTShadingNode):
    bl_label = 'Color'
    bl_idname = 'SORTNodeInputColor'
    osl_shader = '''
        shader Extract( color Color = @,
                        output color Result = color(0)){
            Result = Color;
        }
    '''
    color = bpy.props.FloatVectorProperty(name='Color', subtype='COLOR', min=0.0, max=1.0, size=3, default=(1.0, 1.0, 1.0))
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.template_color_picker(self, 'color', value_slider=True)
        layout.prop(self, 'color', text='')
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( 'color( %f,%f,%f )'%(self.color[:]) )

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
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Value'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value'].default_value = 1.0
    op_type = bpy.props.EnumProperty(name='Type',default='-',items=[
        ('-','Negation','',1), ('1.0-','One Minus','',2), ('sin','Sin','',3), ('cos','Cos','',4), ('tan','Tan','',5), ('asin','Asin','',6), ('acos','Acos','',7), ('atan','Atan','',8),
        ('exp','Exp','',9), ('exp2','Exp2','',10), ('log','Log','',11), ('log2','Log2','',12), ('log10','Log10','',13), ('sqrt','Sqrt','',14), ('inversesqrt','Inverse Sqrt','',15),
        ('fabs','Abs','', 16), ('sign','Sign','',17), ('floor','Floor','',18), ('ceil','Ceil','',19), ('round','Round','',20), ('trunc','Trunc','',21) ])
    data_type = bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
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
        return self.bl_label + self.data_type + self.op_type
        
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
    op_type = bpy.props.EnumProperty(name='Type',default='+',items=[('+','Add','',1),('-','Substract','',2),('*','Multiply','',3),('/','Divide','',4)])
    data_type = bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
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
        return self.bl_label + self.data_type + self.op_type

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpDotProduce(SORTShadingNode):
    bl_label = 'Dot Product'
    bl_idname = 'SORTNodeMathOpDotProduce'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( vector Value0 = @ ,
                             vector Value1 = @ ,
                             output vector Result = 0.0 ){
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
    data_type = bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
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
        return self.bl_label + self.data_type

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
    data_type = bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
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
        return self.bl_label + self.data_type
