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

# Base class for sort socket
class SORTSocket:
    ui_open = bpy.props.BoolProperty(name='UI Open', default=True)
    socket_color = (0.1, 0.1, 0.1, 0.75)
    sort_type = ''
    pbrt_type = ''
    # Customized color for the socket
    def draw_color(self, context, node):
        return self.socket_color
    # export type in SORT
    def export_sort_socket_type(self):
        return self.sort_type
    # export type in PBRT
    def export_pbrt_socket_type(self):
        return self.pbrt_type
    #draw socket property in node
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            layout.label(text)
        else:
            row = layout.row()
            split = row.split(0.4)
            split.label(text)
            if node.inputs[text].default_value is not None:
                split.prop(node.inputs[text],'default_value',text="")

# Socket for BXDF or Materials
class SORTNodeSocketBxdf(bpy.types.NodeSocketShader, SORTSocket):
    bl_idname = 'SORTNodeSocketBxdf'
    bl_label = 'SORT Shader Socket'
    socket_color = (0.2, 0.2, 1.0, 1.0)
    default_value = None

# Socket for Color
class SORTNodeSocketColor(bpy.types.NodeSocketColor, SORTSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'
    socket_color = (0.1, 1.0, 0.2, 1.0)
    sort_type = 'color'
    pbrt_type = 'rgb'
    default_value = bpy.props.FloatVectorProperty( name='Color' , default=(1.0, 1.0, 1.0) ,subtype='COLOR',soft_min = 0.0, soft_max = 1.0)
    def export_socket_value(self):
        return '%f %f %f'%(self.default_value[0],self.default_value[1],self.default_value[2])

# Socket for Float
class SORTNodeSocketFloat(bpy.types.NodeSocketFloat, SORTSocket):
    bl_idname = 'SORTNodeSocketFloat'
    bl_label = 'SORT Float Socket'
    socket_color = (0.1, 0.1, 0.3, 1.0)
    sort_type = 'float'
    pbrt_type = 'float'
    default_value = bpy.props.FloatProperty( name='Float' , default=0.0 , min=0.0, max=1.0 )
    def export_socket_value(self):
        return '%f'%(self.default_value)

# Socket for normal ( normal map )
class SORTNodeSocketNormal(bpy.types.NodeSocketVector, SORTSocket):
    bl_idname = 'SORTNodeSocketNormal'
    bl_label = 'SORT Normal Socket'
    socket_color = (0.1, 0.6, 0.3, 1.0)
    sort_type = 'vector'
    default_value = bpy.props.FloatVectorProperty( name='Normal' , default=(0.0,1.0,0.0) , min=-1.0, max=1.0 )
    def export_socket_value(self):
        return '%f %f %f'%(self.default_value[:])
    # normal socket doesn't show the vector because it is not supposed to be edited this way.
    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            layout.label(text)
        else:
            row = layout.row()
            split = row.split(0.4)
            split.label(text)