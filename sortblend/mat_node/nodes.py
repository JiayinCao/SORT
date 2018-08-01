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
import xml.etree.cElementTree as ET
import nodeitems_utils
from . import sockets
from nodeitems_utils import NodeItem

# fix pbrt path , / will be recognized as escape letter, which will easily crash the system in PBRT
def fixPbrtPath(path):
    return path.replace( '\\' , '/' )

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

    # registered socket list
    socket_list = []

    def register_prop(self):
        # register all sockets
        for socket in self.socket_list:
            self.inputs.new( socket['class'].__name__ , socket['name'] )
            if 'default' in socket:
                self.inputs[socket['name']].default_value = socket['default']
        self.outputs.new( self.output_type , 'Result' )

    def export_prop(self, xml_node):
        pass

    def draw_props(self, context, layout, indented_label):
        pass

    def export_pbrt(self, file):
        pass
        
    def draw_buttons(self, context, layout):
        pass
    
    def draw_props(self, context, layout, indented_label):
        pass

    #draw property in node
    def draw_button(self, layout, label, prop):
        row = layout.row()
        split = row.split(0.3)
        split.label(label)
        prop_row = split.row()
        prop_row.prop(self,prop,text="")

    # draw non-socket property in panel
    def draw_prop(self, layout, label, prop, indented_label):
        split = layout.split(0.3)
        row = split.row()
        indented_label(row)
        row.label(label)
        prop_row = split.row()
        prop_row.prop(self,prop,text="")


# sort material node root
class SORTShadingNode_BXDF(SORTShadingNode):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'

    output_type = 'SORTNodeSocketBxdf'

    bxdf_socket_list = [ { 'class' : sockets.SORTNodeSocketNormal , 'name' : 'Normal' } ]

    pbrt_bxdf_type = ''

    def register_prop(self):
        super().register_prop()

        # register all sockets
        for socket in SORTShadingNode_BXDF.bxdf_socket_list:
            self.inputs.new( socket['class'].__name__ , socket['name'] )
            if 'default' in socket:
                self.inputs[socket['name']].default_value = socket['default']

    def export_pbrt(self, file):
        # disable pbrt export by default, not all materials are supported in pbrt
        if self.pbrt_bxdf_type is None:
            return

        # register all sockets
        file.write( "  \"string type\" \"" + self.pbrt_bxdf_type + "\"\n" )
        for socket in self.socket_list:
            if 'pbrt_name' in socket:
                file.write( "  \"" + self.inputs[socket['name']].export_pbrt_socket_type() + " " + socket['pbrt_name'] + "\" [%s]\n"%self.inputs[socket['name']].export_sort_socket_value())
        file.write( "\n" )

# output node
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = 'SORTNodeOutput'

    def init(self, context):
        input = self.inputs.new('sockets.SORTNodeSocketBxdf', 'Surface')


# microfacte node
class SORTNode_BXDF_MicrofacetReflection(SORTShadingNode_BXDF):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_BXDF_MicrofacetReflection'

    mfdist_item = [ ("Blinn", "Blinn", "", 1),
                    ("Beckmann" , "Beckmann" , "", 2),
                    ("GGX" , "GGX" , "" , 3) ]
    mfdist_prop = bpy.props.EnumProperty(name='NDF',items=mfdist_item)

    # fresnel parameters
    # dielectric-conductor parameters
    eta = bpy.props.FloatVectorProperty(name='Interior IOR', default=(0.37, 0.37, 0.37), min=0.10, max=10.0)
    k = bpy.props.FloatVectorProperty(name='Absorption Coefficient', default=(2.82, 2.82, 2.82), min=1.0, max=10.0)

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessU' , 'default' : 0.1 } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessV' , 'default' : 0.1 } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } ]

    def init(self, context):
        super().register_prop()

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "MicroFacetDistribution" , "mfdist_prop")
        self.draw_button(layout, "Interior IOR" , "eta")
        self.draw_button(layout, "Absorption Coefficient" , "k")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'MicroFacetDistribution' , 'mfdist_prop' , indented_label)
        self.draw_prop(layout, "Interior IOR" , "eta", indented_label)
        self.draw_prop(layout, "Absorption Coefficient" , "k", indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='MicroFacetDistribution' , type='string', value= self.mfdist_prop )
        ET.SubElement( xml_node , 'Property' , name='Interior IOR' , type='color', value= '%f %f %f'%(self.eta[0],self.eta[1],self.eta[2])  )
        ET.SubElement( xml_node , 'Property' , name='Absorption Coefficient' , type='color', value= '%f %f %f'%(self.k[0],self.k[1],self.k[2]) )


class SORTNode_BXDF_MicrofacetRefraction(SORTShadingNode_BXDF):
    bl_label = 'MicrofacetRefraction'
    bl_idname = 'SORTNode_BXDF_MicrofacetRefraction'

    mfdist_item = [ ("Blinn", "Blinn", "", 1),
                    ("Beckmann" , "Beckmann" , "", 2),
                    ("GGX" , "GGX" , "" , 3) ]
    mfdist_prop = bpy.props.EnumProperty(name='NDF',items=mfdist_item)

    # dielectric-dielectric parameters
    int_ior = bpy.props.FloatProperty(name='Interior IOR', default=1.1, min=1.0, max=10.0)
    ext_ior = bpy.props.FloatProperty(name='Exterior IOR', default=1.0, min=1.0, max=10.0)

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessU' , 'default' : 0.1 } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessV' , 'default' : 0.1 } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } ]

    def init(self, context):
        super().register_prop()

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "MicroFacetDistribution" , "mfdist_prop")
        self.draw_button(layout, "Interior IOR" , "int_ior")
        self.draw_button(layout, "Exterior IOR" , "ext_ior")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'MicroFacetDistribution' , 'mfdist_prop' , indented_label)
        self.draw_prop(layout, "Interior IOR" , "int_ior", indented_label)
        self.draw_prop(layout, "Exterior IOR" , "ext_ior", indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='MicroFacetDistribution' , type='string', value= self.mfdist_prop )
        ET.SubElement( xml_node , 'Property' , name='in_ior' , type='color', value= '%f'%(self.int_ior)  )
        ET.SubElement( xml_node , 'Property' , name='ext_ior' , type='color', value= '%f'%(self.ext_ior) )


class SORTNode_BXDF_AshikhmanShirley(SORTShadingNode_BXDF):
    bl_label = 'AshikhmanShirley'
    bl_idname = 'SORTNode_BXDF_AshikhmanShirley'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'Specular' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessU' , 'default' : 0.1 } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'RoughnessV' , 'default' : 0.1 } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

    def init(self, context):
        super().register_prop()


class SORTNode_BXDF_Lambert(SORTShadingNode_BXDF):
    bl_label = 'Lambert'
    bl_idname = 'SORTNode_BXDF_Lambert'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

    def init(self, context):
        super().register_prop()


class SORTNode_BXDF_LambertTransmission(SORTShadingNode_BXDF):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_BXDF_LambertTransmission'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

    def init(self, context):
        super().register_prop()


class SORTNode_BXDF_OrenNayar(SORTShadingNode_BXDF):
    bl_label = 'OrenNayar'
    bl_idname = 'SORTNode_BXDF_OrenNayar'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name'  : 'Roughness' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } ]

    def init(self, context):
        super().register_prop()


class SORTNode_BXDF_MERL(SORTShadingNode_BXDF):
    bl_label = 'MERL'
    bl_idname = 'SORTNode_BXDF_MERL'

    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        super().register_prop()

    def draw_buttons(self, context, layout):
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        abs_file_path = bpy.path.abspath( self.file_name_prop )
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='string', value= abs_file_path )

    def export_pbrt(self, file):
        file.write( "  \"string type\" \"matte\"\n" ) # Merl is not supported in pbrt 3.x
        file.write( "  \"rgb Kd\" [1.0 1.0 1.0]\n" )
        file.write( "\n" )


class SORTNode_BXDF_Fourier(SORTShadingNode_BXDF):
    bl_label = 'Fourier BXDF'
    bl_idname = 'SORTNode_BXDF_Fourier'

    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        super().register_prop()

    def draw_buttons(self, context, layout):
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        abs_file_path = bpy.path.abspath( self.file_name_prop )
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='string', value= abs_file_path )

    def export_pbrt(self, file):
        abs_file_path = bpy.path.abspath( self.file_name_prop )
        file.write( "  \"string type\" \"fourier\"\n" )
        file.write( "  \"string bsdffile\" \"%s\"\n" % fixPbrtPath(abs_file_path) )
        file.write( "\n" )


# operator nodes
class SORTNodeAdd(SORTShadingNode):
    bl_label = 'Add'
    bl_idname = 'SORTNodeAdd'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeOneMinus(SORTShadingNode):
    bl_label = 'One Minus'
    bl_idname = 'SORTNodeOneMinus'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeMultiply(SORTShadingNode):
    bl_label = 'Multiply'
    bl_idname = 'SORTNodeMultiply'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeBlend(SORTShadingNode):
    bl_label = 'Blend'
    bl_idname = 'SORTNodeBlend'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Factor1' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ,
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Factor2' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeLerp(SORTShadingNode):
    bl_label = 'Lerp'
    bl_idname = 'SORTNodeLerp'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ,
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Factor' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeLinearToGamma(SORTShadingNode):
    bl_label = 'LinearToGamma'
    bl_idname = 'SORTNodeLinearToGamma'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeGammaToLinear(SORTShadingNode):
    bl_label = 'GammaToLinear'
    bl_idname = 'SORTNodeGammaToLinear'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeDecodeNormal(SORTShadingNode):
    bl_label = 'DecodeNormal'
    bl_idname = 'SORTNodeDecodeNormal'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

    def init(self, context):
        super().register_prop()

# texture nodes
class SORTNodeConstant(SORTShadingNode):
    bl_label = 'Constant'
    bl_idname = 'SORTNodeConstant'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' , 'default' : ( 0.2 , 0.2 , 0.2 ) } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeCheckbox(SORTShadingNode):
    bl_label = 'CheckBox'
    bl_idname = 'SORTNodeCheckbox'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color1' , 'default' : ( 0.2 , 0.2 , 0.2 ) } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Color2' } ]

    def init(self, context):
        super().register_prop()

class SORTNodeImage(SORTShadingNode):
    bl_label = 'Image'
    bl_idname = 'SORTNodeImage'

    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        super().register_prop()

    def draw_buttons(self, context, layout):
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        abs_file_path = bpy.path.abspath( self.file_name_prop )
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='string', value= abs_file_path )

class SORTPatternNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'SORTPatternGraph'

def socket_node_input(nt, socket):
    return next((l.from_node for l in nt.links if l.to_socket == socket), None)

def find_node_input(node, name):
    for input in node.inputs:
        if input.name == name:
            return input
    return None

# find the output node
def find_node(material, nodetype):
    if material and material.sort_material and material.sort_material.sortnodetree:
        ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
        for node in ntree.nodes:
            if getattr(node, "bl_idname", None) == nodetype:
                return node
    return None

#bass class for operator to add a node
class SORT_Add_Node:
    '''
    For generating cycles-style ui menus to add new nodes,
    connected to a given input socket.
    '''
    def get_type_items(self, context):
        items = []
        for nodetype in SORTPatternGraph.nodetypes.values():
            items.append((nodetype, nodetype,nodetype))
        items.append(('REMOVE', 'Remove',
                        'Remove the node connected to this socket'))
        items.append(('DISCONNECT', 'Disconnect',
                        'Disconnect the node connected to this socket'))
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
    '''
    For generating cycles-style ui menus to add new bxdfs,
    connected to a given input socket.
    '''
    bl_idname = 'node.add_surface'
    bl_label = 'Add Bxdf Node'
    bl_description = 'Connect a Bxdf to this socket'
    input_type = bpy.props.StringProperty(default='Result')

class SORTNode_Material_Principle(SORTShadingNode_BXDF):
    bl_label = 'Principle'
    bl_idname = 'SORTNode_Material_Principle'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'RoughnessU' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'RoughnessV' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Metallic' , 'default' : 1.0 } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Specular' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } ]

    def init(self, context):
        super().register_prop()

    def export_pbrt(self, file):
        reflectance = self.inputs[4].default_value
        metallic = self.inputs[2].default_value
        roughness = self.inputs[0].default_value
        file.write( "  \"string type\" \"disney\"\n" )
        file.write( "  \"rgb color\" [%f %f %f]\n"%(reflectance[:]))
        file.write( "  \"float metallic\" [%f]\n"%metallic)
        file.write( "  \"float roughness\" [%s]\n" %roughness )    # no anisotropic in pbrt disney model yet
        return

class SORTNode_Material_DisneyBRDF(SORTShadingNode_BXDF):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'

    pbrt_bxdf_type = 'disney'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'SubSurface' } , 
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

    def init(self, context):
        super().register_prop()

class SORTNode_Material_Glass(SORTShadingNode_BXDF):
    bl_label = 'Glass'
    bl_idname = 'SORTNode_Material_Glass'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Reflectance' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Transmittance' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Roughness' } ] 

    def init(self, context):
        super().register_prop()

    def export_pbrt(self, file):
        reflectance = self.inputs[0].default_value
        transmittance = self.inputs[1].default_value
        roughness = self.inputs[2].default_value
        file.write( "  \"string type\" \"glass\"\n" )
        file.write( "  \"rgb Kr\" [%f %f %f]\n"%(reflectance[:]))
        file.write( "  \"rgb Kt\" [%f %f %f]\n"%(transmittance[:]))
        file.write( "  \"float uroughness\" [%s]\n" %roughness )
        file.write( "  \"float vroughness\" [%s]\n" %roughness )
        return

class SORTNode_Material_Plastic(SORTShadingNode_BXDF):
    bl_label = 'Plastic'
    bl_idname = 'SORTNode_Material_Plastic'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Diffuse' } , 
                    { 'class' : sockets.SORTNodeSocketColor , 'name' : 'Specular' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Roughness' , 'default' : 0.2 } ] 

    def init(self, context):
        super().register_prop()

    def export_pbrt(self, file):
        basecolor = self.inputs[0].default_value
        specular = self.inputs[1].default_value
        roughness = self.inputs[2].default_value
        file.write( "  \"string type\" \"plastic\"\n" )
        file.write( "  \"rgb Kd\" [%f %f %f]\n"%(basecolor[:]))
        file.write( "  \"rgb Ks\" [%f %f %f]\n"%(specular[:]))
        file.write( "  \"float roughness\" [%s]\n" %roughness )
        return

class SORTNode_Material_Matte(SORTShadingNode_BXDF):
    bl_label = 'Matte'
    bl_idname = 'SORTNode_Material_Matte'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } , 
                    { 'class' : sockets.SORTNodeSocketFloat , 'name' : 'Roughness' } ] 

    def init(self, context):
        super().register_prop()

    def export_pbrt(self, file):
        basecolor = self.inputs[0].default_value
        roughness = self.inputs[1].default_value
        file.write( "  \"string type\" \"matte\"\n" )
        file.write( "  \"rgb Kd\" [%f %f %f]\n"%(basecolor[:]))
        file.write( "  \"float sigma\" [%s]\n" %roughness )
        return

class SORTNode_Material_Mirror(SORTShadingNode_BXDF):
    bl_label = 'Mirror'
    bl_idname = 'SORTNode_Material_Mirror'

    # node sockets
    socket_list = [ { 'class' : sockets.SORTNodeSocketColor , 'name' : 'BaseColor' } ] 

    def init(self, context):
        self.register_prop()

    def export_pbrt(self, file):
        basecolor = self.inputs[0].default_value
        file.write( "  \"string type\" \"mirror\"\n" )
        file.write( "  \"rgb Kd\" [%f %f %f]\n"%(basecolor[:]))
        return

class SORTNode_Material_Measured(SORTShadingNode_BXDF):
    bl_label = 'Measured'
    bl_idname = 'SORTNode_Material_Measured'

    type_item = [ ("Fourier", "Fourier", "", 1), ("MERL" , "MERL" , "", 2) ]
    type_prop = bpy.props.EnumProperty(name='Type',items=type_item,default="Fourier")
    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        super().register_prop()

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "Type" , "type_prop")
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Type' , 'type_prop' , intended_label)
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        abs_file_path = bpy.path.abspath( self.file_name_prop )
        ET.SubElement( xml_node , 'Property' , name='Type' , type='string', value = self.type_prop )
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='string', value= abs_file_path )

    def export_pbrt(self, file):
        abs_file_path = bpy.path.abspath( self.file_name_prop )
        if self.type_prop == 'Fourier':
            file.write( "  \"string type\" \"fourier\"\n" )
            file.write( "  \"string bsdffile\" \"%s\"\n" % fixPbrtPath(abs_file_path) )
        else:
            file.write( "  \"string type\" \"matte\"\n" ) # Merl is not supported in pbrt 3.x
            file.write( "  \"rgb Kd\" [1.0 1.0 1.0]\n" )
        file.write( "\n" )

# layered bxdf node
class SORTNode_Material_Layered(SORTShadingNode_BXDF):
    bl_label = 'Layered Material'
    bl_idname = 'SORTNode_Material_Layered'

    bxdf_count = bpy.props.IntProperty( name = 'Bxdf Count' , default = 2 , min = 1 , max = 8 )

    def init(self, context):
        super().register_prop()
        for x in range(0,8):
            self.inputs.new( 'SORTNodeSocketBxdf' , 'Bxdf'+str(x) )
            self.inputs.new( 'SORTNodeSocketColor' , 'Weight'+str(x) )

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "Bxdf Count" , "bxdf_count")

        for x in range( 0 , 8 ):
            if x < self.bxdf_count:
                if self.inputs.get('Bxdf'+str(x)) is None:
                    self.inputs.new( 'SORTNodeSocketBxdf' , 'Bxdf'+str(x) )
                    self.inputs.new( 'SORTNodeSocketColor' , 'Weight'+str(x) )
            else:
                if self.inputs.get('Bxdf'+str(x)) is not None:
                    self.inputs.remove( self.inputs['Bxdf' + str(x)] )
                    self.inputs.remove( self.inputs['Weight' + str(x)] )

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Bxdf Count' , 'bxdf_count' , indented_label)

def register():
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
    nodeitems_utils.unregister_node_categories("SORTSHADERNODES")

