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
import nodeitems_utils
from nodeitems_utils import NodeItem
from . import sockets

# node tree for sort
class SORTPatternGraph(bpy.types.NodeTree):
    '''A node tree comprised of sort nodes'''
    bl_idname = 'SORTPatternGraph'
    bl_label = 'SORT Pattern Graph'
    bl_icon = 'MATERIAL'
    nodetypes = {}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == 'sortblend'

    # Return a node tree from the context to be used in the editor
    @classmethod
    def get_from_context(cls, context):
        ob = context.active_object
        if ob and ob.type not in {'LAMP', 'CAMERA'}:
            ma = ob.active_material
            if ma != None:
                nt_name = ma.sort_material.sortnodetree
                if nt_name != '':
                    return bpy.data.node_groups[nt_name], ma, ma
        return (None, None, None)

# sort material node root
class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'
    output_type = 'SORTNodeSocketColor'
    property_list = []

    # register all property and sockets
    def register_prop(self):
        # register all sockets
        for socket in self.property_list:
            if socket['class'].is_socket() is False:
                continue
            self.inputs.new( socket['class'].__name__ , socket['name'] )
            if 'default' in socket:
                self.inputs[socket['name']].default_value = socket['default']
        self.outputs.new( self.output_type , 'Result' )

    # this is not an overriden interface
    # draw all properties ( not socket ) in material panel
    def draw_props(self, context, layout, indented_label):
        for prop in self.property_list:
            if prop['class'].is_socket():
                continue
            split = layout.split(0.3)
            row = split.row()
            indented_label(row)
            row.label(prop['name'])
            prop_row = split.row()
            prop_row.prop(self,prop['name'],text="")
        pass

    # override the base interface
    # draw all properties ( not socket ) in material nodes
    def draw_buttons(self, context, layout):
        for prop in self.property_list:
            if prop['class'].is_socket():
                continue
            layout.prop(self,prop['name'])
        pass

    # export data to pbrt
    def export_pbrt(self, output_pbrt_type , output_pbrt_prop):
        pass
    
    # export data to sort
    def export_sort(self, output_sort_prop):
        for prop in self.property_list:
            if prop['class'].is_socket():
                continue
            attr_wrapper = getattr(self, prop['name'] + '_wrapper' )
            attr = getattr(self, prop['name'])
            t = attr_wrapper.export_sort_socket_type(attr_wrapper)
            v = attr_wrapper.export_socket_value(attr_wrapper, attr)
            output_sort_prop( prop['name'] , t , v )

    @classmethod
    def get_mro(cls, attribname):
        for k in reversed(cls.__mro__):
            vs = vars(k)
            if attribname in vs:
                yield vs[attribname]

    @classmethod
    def register(cls):
        for props in cls.get_mro('property_list'):
            if not props:
                continue
            for prop in props:
                var_name = prop['name']
                value = prop['class']
                if value.is_socket():
                    continue
                value.setup( prop )
                if 'register_properties' in dir(value):
                    for kp, vp in value.get_properties(var_name):
                        setattr(cls, kp, vp)
                else:
                    setattr(cls, var_name + '_wrapper', value )
                    setattr(cls, var_name, value.default_value )

    @classmethod
    def unregister(cls):
        for props in cls.get_mro('property_list'):
            if not props:
                continue
            for prop in props:
                k = prop['name']
                v = prop['class']
                if v.is_socket():
                    continue
                if 'register_properties' in dir(v):
                    for kp, vp in v.get_properties(k):
                        delattr(cls, kp)
                else:
                    delattr(cls, k)
    
    # register all properties in constructor
    def init(self, context):
        def do_mro(self, methname, *args, **kwargs):
            for meth in self.get_mro(methname):
                yield meth(self, *args, **kwargs)
        for _ in do_mro(self, 'register_prop'):
            pass

# Base class for SORT BXDF Node
class SORTShadingNode_BXDF(SORTShadingNode):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'
    output_type = 'SORTNodeSocketBxdf'
    bxdf_property_list = [ { 'class' : sockets.SORTNodeSocketNormal , 'name' : 'Normal' } ]
    pbrt_bxdf_type = ''

    def register_prop(self):
        # register all sockets in BXDF
        for prop in SORTShadingNode_BXDF.bxdf_property_list:
            self.inputs.new( prop['class'].__name__ , prop['name'] )
            if 'default' in prop:
                self.inputs[prop['name']].default_value = prop['default']

    def export_pbrt(self, output_pbrt_type , output_pbrt_prop):
        # disable pbrt export by default, not all materials are supported in pbrt
        if self.pbrt_bxdf_type is '':
            # fall back to a default grey matte material
            output_pbrt_type( 'matte' )
            return
        # output pbrt properties
        output_pbrt_type( self.pbrt_bxdf_type )
        for prop in self.property_list:
            if 'pbrt_name' in prop:
                n = prop['pbrt_name']
                t = self.inputs[prop['name']].export_pbrt_socket_type()
                v = self.inputs[prop['name']].export_socket_value()
                output_pbrt_prop( n , t , v )

class SORTPatternNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'SORTPatternGraph'

class SORT_Add_Node:
    def get_type_items(self, context):
        items = []
        for nodetype in SORTPatternGraph.nodetypes.values():
            items.append((nodetype, nodetype,nodetype))
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
            nt.nodes.remove(input_node)
            return {'FINISHED'}

        if new_type == 'DISCONNECT':
            link = next((l for l in nt.links if l.to_socket == socket), None)
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

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Material Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
class SORTNode_Material_Principle(SORTShadingNode_BXDF):
    bl_label = 'Principle'
    bl_idname = 'SORTNode_Material_Principle'
    property_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'RoughnessU' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'RoughnessV' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Metallic' , 'default' : 1.0 } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Specular' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } ]

class SORTNode_Material_DisneyBRDF(SORTShadingNode_BXDF):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'
    pbrt_bxdf_type = 'disney'
    property_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'SubSurface' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Metallic' , 'default' : 1.0 , 'pbrt_name' : 'metallic' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Speulcar' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'SpecularTint' , 'pbrt_name' : 'speculartint' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Roughness' , 'pbrt_name' : 'roughness' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Anisotropic' , 'pbrt_name' : 'anisotropic' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Sheen' , 'pbrt_name' : 'sheen' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'SheenTint' , 'pbrt_name' : 'sheentint' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Clearcoat' , 'pbrt_name' : 'clearcoat' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'ClearcoatGloss' , 'pbrt_name' : 'clearcoatgloass' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' , 'pbrt_name' : 'color' } ]

class SORTNode_Material_Glass(SORTShadingNode_BXDF):
    bl_label = 'Glass'
    bl_idname = 'SORTNode_Material_Glass'
    pbrt_bxdf_type = 'glass'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Reflectance' , 'pbrt_name' : 'Kr' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Transmittance' , 'pbrt_name' : 'Kt' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'RoughnessU' , 'pbrt_name' : 'uroughness' } ,
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'RoughnessV' , 'pbrt_name' : 'vroughness' } ] 

class SORTNode_Material_Plastic(SORTShadingNode_BXDF):
    bl_label = 'Plastic'
    bl_idname = 'SORTNode_Material_Plastic'
    pbrt_bxdf_type = 'plastic'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' , 'pbrt_name' : 'Kd' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Specular' , 'pbrt_name' : 'Ks' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Roughness' , 'default' : 0.2 , 'pbrt_name' : 'roughness' } ] 

class SORTNode_Material_Matte(SORTShadingNode_BXDF):
    bl_label = 'Matte'
    bl_idname = 'SORTNode_Material_Matte'
    pbrt_bxdf_type = 'matte'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' , 'pbrt_name' : 'Kd' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Roughness' , 'pbrt_name' : 'sigma' } ] 

class SORTNode_Material_Mirror(SORTShadingNode_BXDF):
    bl_label = 'Mirror'
    bl_idname = 'SORTNode_Material_Mirror'
    pbrt_bxdf_type = 'mirror'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' , 'pbrt_name' : 'Kd' } ] 

class SORTNode_Material_Measured(SORTShadingNode_BXDF):
    bl_label = 'Measured'
    bl_idname = 'SORTNode_Material_Measured'
    property_list = [ { 'class' : sockets.SORTNodePropertyEnum , 'name' : 'Type' , 'items' : [ ("Fourier", "Fourier", "", 1) , ("MERL" , "MERL" , "", 2) ] , 'default' : 'Fourier' } ,
                      { 'class' : sockets.SORTNodePropertyPath , 'name' : 'Filename' } ]
    def export_pbrt(self, output_pbrt_type , output_pbrt_prop):
        abs_file_path = bpy.path.abspath( self.Filename )
        if self.Type == 'Fourier':
            output_pbrt_type( 'fourier' )
            output_pbrt_prop( n , 'string' , abs_file_path.replace( '\\' , '/' ) )
        else:
            output_pbrt_type( 'matte' )

class SORTNode_Material_Layered(SORTShadingNode_BXDF):
    bl_label = 'Layered Material'
    bl_idname = 'SORTNode_Material_Layered'
    bxdf_count = bpy.props.IntProperty( name = 'Bxdf Count' , default = 2 , min = 1 , max = 8 )

    def init(self, context):
        SORTShadingNode.register_prop(self)
        for x in range(0,8):
            self.inputs.new( 'SORTNodeSocketBxdf' , 'Bxdf'+str(x) )
            self.inputs.new( 'SORTNodeSocketColor' , 'Weight'+str(x) )

    def draw_buttons(self, context, layout):
        row = layout.row()
        split = row.split(0.4)
        split.label("BXDF Count")
        prop_row = split.row()
        prop_row.prop(self,'bxdf_count',text="")

        for x in range( 0 , 8 ):
            if x < self.bxdf_count:
                if self.inputs.get('Bxdf'+str(x)) is None:
                    self.inputs.new( 'SORTNodeSocketBxdf' , 'Bxdf'+str(x) )
                    self.inputs.new( 'SORTNodeSocketColor' , 'Weight'+str(x) )
            else:
                if self.inputs.get('Bxdf'+str(x)) is not None:
                    self.inputs.remove( self.inputs['Bxdf' + str(x)] )
                    self.inputs.remove( self.inputs['Weight' + str(x)] )

#------------------------------------------------------------------------------------------------------------------------------------
#                                               BXDF Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
# microfacte node
class SORTNode_BXDF_MicrofacetReflection(SORTShadingNode_BXDF):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_BXDF_MicrofacetReflection'
    property_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessU' , 'default' : 0.1 } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessV' , 'default' : 0.1 } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } ,
                      { 'class' : sockets.SORTNodePropertyEnum , 'name' : 'MicroFacetDistribution' , 'default' : 'GGX' , 'items' : [ ("Blinn", "Blinn", "", 1), ("Beckmann" , "Beckmann" , "", 2), ("GGX" , "GGX" , "" , 3) ] } , 
                      { 'class' : sockets.SORTNodePropertyFloatVector , 'name' : 'Interior_IOR' , 'default' : (0.37, 0.37, 0.37) , 'min' : 0.1 , 'max' : 10.0 } , 
                      { 'class' : sockets.SORTNodePropertyFloatVector , 'name' : 'Absorption_Coefficient' , 'default' : (2.82, 2.82, 2.82) , 'min' : 0.1 , 'max' : 10.0 } ]

class SORTNode_BXDF_MicrofacetRefraction(SORTShadingNode_BXDF):
    bl_label = 'MicrofacetRefraction'
    bl_idname = 'SORTNode_BXDF_MicrofacetRefraction'
    property_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessU' , 'default' : 0.1 } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessV' , 'default' : 0.1 } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' },
                      { 'class' : sockets.SORTNodePropertyEnum , 'name' : 'MicroFacetDistribution' , 'default' : 'GGX' , 'items' : [ ("Blinn", "Blinn", "", 1), ("Beckmann" , "Beckmann" , "", 2), ("GGX" , "GGX" , "" , 3) ] } , 
                      { 'class' : sockets.SORTNodePropertyFloat , 'name' : 'Interior_IOR' , 'default' : 1.1 , 'min' : 1.0 , 'max' : 10.0 } , 
                      { 'class' : sockets.SORTNodePropertyFloat , 'name' : 'Exterior_IOR' , 'default' : 1.0 , 'min' : 1.0 , 'max' : 10.0 } ]

class SORTNode_BXDF_AshikhmanShirley(SORTShadingNode_BXDF):
    bl_label = 'AshikhmanShirley'
    bl_idname = 'SORTNode_BXDF_AshikhmanShirley'
    property_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'Specular' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessU' , 'default' : 0.1 } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessV' , 'default' : 0.1 } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

class SORTNode_BXDF_Lambert(SORTShadingNode_BXDF):
    bl_label = 'Lambert'
    bl_idname = 'SORTNode_BXDF_Lambert'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

class SORTNode_BXDF_LambertTransmission(SORTShadingNode_BXDF):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_BXDF_LambertTransmission'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

class SORTNode_BXDF_OrenNayar(SORTShadingNode_BXDF):
    bl_label = 'OrenNayar'
    bl_idname = 'SORTNode_BXDF_OrenNayar'
    property_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'Roughness' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

class SORTNode_BXDF_MERL(SORTShadingNode_BXDF):
    bl_label = 'MERL'
    bl_idname = 'SORTNode_BXDF_MERL'
    property_list = [ { 'class' : sockets.SORTNodePropertyPath , 'name' : 'Filename' } ]

class SORTNode_BXDF_Fourier(SORTShadingNode_BXDF):
    bl_label = 'Fourier BXDF'
    bl_idname = 'SORTNode_BXDF_Fourier'
    property_list = [ { 'class' : sockets.SORTNodePropertyPath , 'name' : 'Filename' } ]

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Operator Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
class SORTNodeAdd(SORTShadingNode):
    bl_label = 'Add'
    bl_idname = 'SORTNodeAdd'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

class SORTNodeOneMinus(SORTShadingNode):
    bl_label = 'One Minus'
    bl_idname = 'SORTNodeOneMinus'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

class SORTNodeMultiply(SORTShadingNode):
    bl_label = 'Multiply'
    bl_idname = 'SORTNodeMultiply'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

class SORTNodeBlend(SORTShadingNode):
    bl_label = 'Blend'
    bl_idname = 'SORTNodeBlend'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Factor1' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ,
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Factor2' } ]

class SORTNodeLerp(SORTShadingNode):
    bl_label = 'Lerp'
    bl_idname = 'SORTNodeLerp'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ,
                      { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Factor' } ]

class SORTNodeLinearToGamma(SORTShadingNode):
    bl_label = 'LinearToGamma'
    bl_idname = 'SORTNodeLinearToGamma'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

class SORTNodeGammaToLinear(SORTShadingNode):
    bl_label = 'GammaToLinear'
    bl_idname = 'SORTNodeGammaToLinear'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

class SORTNodeDecodeNormal(SORTShadingNode):
    bl_label = 'DecodeNormal'
    bl_idname = 'SORTNodeDecodeNormal'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Texture Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' , 'default' : ( 0.2 , 0.2 , 0.2 ) } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

class SORTNodeCheckbox(SORTShadingNode):
    bl_label = 'CheckBox'
    bl_idname = 'SORTNodeCheckbox'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' , 'default' : ( 0.2 , 0.2 , 0.2 ) } , 
                      { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

class SORTNodeImage(SORTShadingNode):
    bl_label = 'Image'
    bl_idname = 'SORTNodeImage'
    property_list = [ { 'class' : sockets.SORTNodePropertyPath , 'name' : 'Filename' } ]

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Constant Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
class SORTNodeConstant(SORTShadingNode):
    bl_label = 'Constant'
    bl_idname = 'SORTNodeConstant'
    property_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Output Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = 'SORTNodeOutput'
    property_list = [ { 'class' : sockets.SORTNodeSocketBxdf , 'name' : 'Surface' } ]
#------------------------------------------------------------------------------------------------------------------------------------

class SORTMaterial(bpy.types.PropertyGroup):
    sortnodetree = bpy.props.StringProperty(name="Nodetree",default='')
    use_sort_nodes = bpy.props.BoolProperty(name="Nodetree",default=False)

def register():
    bpy.types.Material.sort_material = bpy.props.PointerProperty(type=SORTMaterial, name="SORT Material Settings")

    # all categories in a list
    node_categories = [
        # identifier, label, items list
        SORTPatternNodeCategory("Material", "Materials",items = [NodeItem("SORTNode_Material_DisneyBRDF"),NodeItem("SORTNode_Material_Principle"),NodeItem("SORTNode_Material_Glass"),NodeItem("SORTNode_Material_Mirror"),NodeItem("SORTNode_Material_Plastic"),NodeItem("SORTNode_Material_Matte"),NodeItem("SORTNode_Material_Measured"),NodeItem("SORTNode_Material_Layered")] ),
        SORTPatternNodeCategory("Operator", "Operator",items= [NodeItem("SORTNodeAdd"),NodeItem("SORTNodeOneMinus"),NodeItem("SORTNodeMultiply"),NodeItem("SORTNodeBlend"),NodeItem("SORTNodeLerp"),NodeItem("SORTNodeLinearToGamma"),NodeItem("SORTNodeGammaToLinear"),NodeItem("SORTNodeDecodeNormal")] ),
        SORTPatternNodeCategory("Texture", "Texture",items= [NodeItem("SORTNodeGrid"),NodeItem("SORTNodeCheckbox"),NodeItem("SORTNodeImage")] ),
        SORTPatternNodeCategory("Constant", "Constant",items= [NodeItem("SORTNodeConstant")] ),
        SORTPatternNodeCategory("Input", "Input",items=[],),
        SORTPatternNodeCategory("BXDF", "BXDF",items = [NodeItem("SORTNode_BXDF_AshikhmanShirley"),NodeItem("SORTNode_BXDF_Lambert"),NodeItem("SORTNode_BXDF_LambertTransmission"),NodeItem("SORTNode_BXDF_OrenNayar"),NodeItem("SORTNode_BXDF_MicrofacetReflection"),NodeItem("SORTNode_BXDF_MicrofacetRefraction"),NodeItem("SORTNode_BXDF_Fourier"),NodeItem("SORTNode_BXDF_MERL")]),
    ]
    nodeitems_utils.register_node_categories("SORTSHADERNODES",node_categories)

    # bxdf nodes
    SORTPatternGraph.nodetypes[SORTNode_BXDF_AshikhmanShirley] = 'SORTNode_BXDF_AshikhmanShirley'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_Lambert] = 'SORTNode_BXDF_Lambert'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_LambertTransmission] = 'SORTNode_BXDF_LambertTransmission'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_OrenNayar] = 'SORTNode_BXDF_OrenNayar'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_Fourier] = 'SORTNode_BXDF_Fourier'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_MERL] = 'SORTNode_BXDF_MERL'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_MicrofacetReflection] = 'SORTNode_BXDF_MicrofacetReflection'
    SORTPatternGraph.nodetypes[SORTNode_BXDF_MicrofacetRefraction] = 'SORTNode_BXDF_MicrofacetRefraction'

    # material nodes
    SORTPatternGraph.nodetypes[SORTNode_Material_Principle] = 'SORTNode_Material_Principle'
    SORTPatternGraph.nodetypes[SORTNode_Material_Glass] = 'SORTNode_Material_Glass'
    SORTPatternGraph.nodetypes[SORTNode_Material_Plastic] = 'SORTNode_Material_Plastic'
    SORTPatternGraph.nodetypes[SORTNode_Material_Matte] = 'SORTNode_Material_Matte'
    SORTPatternGraph.nodetypes[SORTNode_Material_Measured] = 'SORTNode_Material_Measured'
    SORTPatternGraph.nodetypes[SORTNode_Material_Layered] = 'SORTNode_Material_Layered'
    SORTPatternGraph.nodetypes[SORTNode_Material_DisneyBRDF] = 'SORTNode_Material_DisneyBRDF'
    SORTPatternGraph.nodetypes[SORTNode_Material_Mirror] = 'SORTNode_Material_Mirror'

def unregister():
    del bpy.types.Material.sort_material
    nodeitems_utils.unregister_node_categories("SORTSHADERNODES")

