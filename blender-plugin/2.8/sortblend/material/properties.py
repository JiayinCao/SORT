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

# duplicated definition, need to resolve this later
def instances(tree):
    def is_sort_node_group(ng):
        return hasattr(ng, 'sort_data') and ng.sort_data.group_name_id != ''

    def is_node_group_id(ng, name):
        return is_sort_node_group(ng) and ng.sort_data.group_name_id == name

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

def update_socket_name(self, context):
    # check for name duplication
    node = self.node
    for input in node.inputs:
        # reset the name if there is a duplication
        if input.name == self.sort_label:
            # this condition is necessary to avoid infinite recursive calling
            if self.sort_label != self.name:
                self.sort_label = self.name
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
class SORTNodeSocket:
    socket_color = (0.1, 0.1, 0.1, 0.75)

    # this is a very hacky way to support name update in Blender because I have no idea how to get callback function from native str class
    sort_label : bpy.props.StringProperty( name = '' , default = 'default' , update = update_socket_name )

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
    def __int__(self):
        self.sort_label.default = self.name

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

@base.register_class
class SORTDummySocket(bpy.types.NodeSocket, SORTNodeSocket):
    bl_idname = "sort_dummy_socket"
    bl_label = "SPRT Dummy Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.6, 0.6, 0.6, 0.5)