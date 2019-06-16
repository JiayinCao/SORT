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

# SORT Node property base class
class SORTNodeProperty:
    pbrt_type = ''
    # export type in PBRT
    def export_pbrt_socket_type(self):
        return self.pbrt_type

# Base class for sort socket
class SORTNodeSocket(SORTNodeProperty):
    ui_open = bpy.props.BoolProperty(name='UI Open', default=True)
    socket_color = (0.1, 0.1, 0.1, 0.75)
    need_bxdf_node = False

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
    default_value : bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) ,subtype='COLOR',soft_min = 0.0, soft_max = 1.0)
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
    default_value : bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0, max=1.0 )
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
    default_value : bpy.props.FloatVectorProperty( name='Float' , default=(0.0,0.0,0.0) , min=-float('inf'), max=float('inf') )
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
    default_value : bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0)
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
    default_value : bpy.props.FloatProperty( name='Float' , default=0.0 , min=-float('inf'), max=float('inf'))
    def export_osl_value(self):
        return '%f'%(self.default_value)
    def get_socket_data_type(self):
        return 'float'

# Socket for normal ( normal map )
class SORTNodeSocketNormal(bpy.types.NodeSocketVector, SORTNodeSocket):
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
    default_value : bpy.props.FloatVectorProperty( name='Float' , default=(0.0,1.0,0.0) , min=0.0, max=1.0 )
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