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

# SORT Node property base class
class SORTNodeProperty:
    pbrt_type = ''
    # export type in PBRT
    def export_pbrt_socket_type(self):
        return self.pbrt_type
    # export value
    def export_socket_value(self):
        return ''
    # whether the property is a socket
    @classmethod
    def is_socket(cls):
        return False

# Base class for sort socket
class SORTNodeSocket(SORTNodeProperty):
    ui_open = bpy.props.BoolProperty(name='UI Open', default=True)
    socket_color = (0.1, 0.1, 0.1, 0.75)
    need_bxdf_node = False
    
    # this is not an inherited function
    def draw_label(self, context, layout, node, text):
        def socket_node_input(socket,context):
            return next((l.from_node for l in socket.links if l.to_socket == socket), None)

        source_node = socket_node_input(self,context)
        has_error = False
        if source_node is not None and source_node.output_type != type(self).__name__:
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

    @classmethod
    def is_socket(cls):
        return True

# Socket for BXDF or Materials
class SORTNodeSocketBxdf(bpy.types.NodeSocketShader, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketBxdf'
    bl_label = 'SORT Shader Socket'
    socket_color = (0.2, 0.2, 1.0, 1.0)
    default_value = None
    need_bxdf_node = True

    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            layout.label(text)

# Socket for Color
class SORTNodeSocketColor(bpy.types.NodeSocketColor, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'
    socket_color = (0.1, 1.0, 0.2, 1.0)
    pbrt_type = 'rgb'
    default_value = bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) ,subtype='COLOR',soft_min = 0.0, soft_max = 1.0)
    def export_socket_value(self):
        return '%f %f %f'%(self.default_value[:])

# Socket for Float
class SORTNodeSocketFloat(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    pbrt_type = 'float'
    default_value = bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0, max=1.0 )
    def export_socket_value(self):
        return '%f'%(self.default_value)

# Socket for Float
class SORTNodeSocketLargeFloat(bpy.types.NodeSocketFloat, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketLargeFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    pbrt_type = 'float'
    default_value = bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0)
    def export_socket_value(self):
        return '%f'%(self.default_value)

# Socket for normal ( normal map )
class SORTNodeSocketNormal(bpy.types.NodeSocketVector, SORTNodeSocket):
    bl_idname = 'SORTNodeSocketNormal'
    bl_label = 'SORT Normal Socket'
    socket_color = (0.1, 0.6, 0.3, 1.0)
    default_value = bpy.props.FloatVectorProperty( name='Normal' , default=(0.0,1.0,0.0) , min=-1.0, max=1.0 )
    def export_socket_value(self):
        return '%f %f %f'%(self.default_value[:])
    # normal socket doesn't show the vector because it is not supposed to be edited this way.
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            self.draw_label(context,layout,node,text)
        else:
            row = layout.row()
            split = row.split(0.4)
            split.label(text)

# Property for Float
class SORTNodePropertyFloat(SORTNodeProperty):
    def export_socket_value(self,value):
        return '%f'%value
    @classmethod
    def setup( cls , prop ):
        cls.default_value = bpy.props.FloatProperty( name=prop['name'] , default=prop['default'] , min=prop['min'], max=prop['max'] )

# Property for Float Vector
class SORTNodePropertyFloatVector(SORTNodeProperty):
    def export_socket_value(self,value):
        return '%f %f %f'%(value[:])
    @classmethod
    def setup( cls , prop ):
        cls.default_value = bpy.props.FloatVectorProperty( name=prop['name'] , default=prop['default'] , min=prop['min'], max=prop['max'] )

# Property for enum
class SORTNodePropertyEnum(SORTNodeProperty):
    def export_socket_value(self,value):
        return value
    @classmethod
    def setup( cls , prop ):
        cls.default_value = bpy.props.EnumProperty( name=prop['name'] , items=prop['items'] , default = prop['default'])

# Property for path
class SORTNodePropertyPath(SORTNodeProperty):
    pbrt_type = 'property'
    def export_socket_value(self,value):
        return bpy.path.abspath( value )
    @classmethod
    def setup( cls , prop ):
        cls.default_value = bpy.props.StringProperty( name=prop['name'] , subtype='FILE_PATH' )