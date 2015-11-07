import bpy
import xml.etree.cElementTree as ET
from . import common
from . import utility
import nodeitems_utils
from nodeitems_utils import NodeCategory, NodeItem

# node tree for sort
class SORTPatternGraph(bpy.types.NodeTree):
    '''A node tree comprised of sort nodes'''
    bl_idname = 'SORTPatternGraph'
    bl_label = 'SORT Pattern Graph'
    bl_icon = 'MATERIAL'
    nodetypes = {}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == common.default_bl_name

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

class SORTSocket:
    ui_open = bpy.props.BoolProperty(name='UI Open', default=True)

    # Optional function for drawing the socket input value
    def draw_value(self, context, layout, node):
        layout.prop(node, self.name)

    # green node for color
    def draw_color(self, context, node):
        return (0.1, 0.1, 0.1, 0.75)

    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            layout.label(text)
        else:
            row = layout.row()
            split = row.split(common.label_percentage)
            left_row = split.row()
            left_row.label(text)
            if node.inputs[text].IsEmptySocket() is False:
                split.prop(node.inputs[text], 'default_value')

    def IsEmptySocket(self):
        return False

# socket for SORT node
class SORTShaderSocket(bpy.types.NodeSocketShader, SORTSocket):
    '''Renderman shader input/output'''
    bl_idname = 'SORTShaderSocket'
    bl_label = 'SORT Shader Socket'
    default_value = None

    # green node for color
    def draw_color(self, context, node):
        return (1.0, 1.0, 0.2, 1.0)

    def IsEmptySocket(self):
        return True

class SORTNodeSocketFloat(bpy.types.NodeSocketFloat, SORTSocket):
    bl_idname = 'SORTNodeSocketFloat'
    bl_label = 'SORT Float Socket'

    default_value = bpy.props.FloatProperty( name='' , default=0.0 )

    # green node for color
    def draw_color(self, context, node):
        return (0.1, 0.1, 0.3, 1.0)

    def output_default_value_to_str(self):
        return '%f'%(self.default_value)

    def output_type_str(self):
        return 'float'

class SORTNodeSocketFloat2(bpy.types.NodeSocketFloat, SORTSocket):
    bl_idname = 'SORTNodeSocketFloat2'
    bl_label = 'SORT Float2 Socket'

    default_value = bpy.props.FloatVectorProperty( name='' , default=(1.0, 1.0) ,subtype='NONE' ,size=2 )

    # green node for color
    def draw_color(self, context, node):
        return (0.1, 0.65 , 0.3, 1.0)

    def output_default_value_to_str(self):
        return '%f %f'%(self.default_value[0], self.default_value[1])

    def output_type_str(self):
        return 'float2'

class SORTNodeSocketColor(bpy.types.NodeSocketColor, SORTSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'

    default_value = bpy.props.FloatVectorProperty( name='' , default=(1.0, 1.0, 1.0) ,subtype='COLOR' )

    # green node for color
    def draw_color(self, context, node):
        return (0.1, 1.0, 0.2, 1.0)

    def output_default_value_to_str(self):
        return '%f %f %f'%(self.default_value[0],self.default_value[1],self.default_value[2])

    def output_type_str(self):
        return 'color'

# sort material node root
class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'

    def export_prop(self, xml_node):
        pass

    def draw_props(self, context, layout, indented_label):
        pass

    def draw_button(self, layout, label, prop):
        row = layout.row()
        row.label(label)
        row.prop(self,prop)

    def draw_prop(self, layout, label, prop, indented_label):
        row = layout.row()
        split = row.split(common.label_percentage)
        left_row = split.row()
        indented_label(left_row)
        left_row.label(label)
        split.prop(self, prop)

# output node
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = common.sort_node_output_bl_name

    def init(self, context):
        input = self.inputs.new('SORTNodeSocketColor', 'Surface')

# lambert node
class SORTNodeLambert(SORTShadingNode):
    bl_label = 'SORT_lambert'
    bl_idname = 'SORTNodeLambert'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.outputs.new('SORTNodeSocketColor', 'Result')

# microfacte node
class SORTNodeMicrofacetReflection(SORTShadingNode):
    bl_label = 'SORT_microfacet_reflection'
    bl_idname = 'SORTNodeMicrofacetReflection'

    mfdist_item = [ ("Blinn", "Blinn", "", 1),
                    ("Beckmann" , "Beckmann" , "", 2),
                    ("GGX" , "GGX" , "" , 3)
                   ]
    mfdist_prop = bpy.props.EnumProperty(name='',items=mfdist_item)

    mfvis_item = [ ("Implicit", "Implicit", "", 1),
                   ("Neumann" , "Neumann" , "", 2),
                   ("Kelemen" , "Kelemen" , "" , 3),
                   ("Schlick" , "Schlick" , "" , 4),
                   ("Smith" , "Smith" , "" , 5),
                   ("SmithJointApprox" , "SmithJointApprox" , "" , 6 ),
                   ("CookTorrance" , "CookTorrance" , "" , 7)
                   ]
    mfvis_prop = bpy.props.EnumProperty(name='',items=mfvis_item, default="CookTorrance")

    # fresnel parameters
    # dielectric-conductor parameters
    eta = bpy.props.FloatVectorProperty(name='', default=(0.37, 0.37, 0.37), min=0.10, max=10.0)
    k = bpy.props.FloatVectorProperty(name='', default=(2.82, 2.82, 2.82), min=1.0, max=10.0)

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.inputs.new('SORTNodeSocketFloat', 'Roughness')
        self.outputs.new('SORTNodeSocketColor', 'Result')

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "MicroFacetDistribution" , "mfdist_prop")
        self.draw_button(layout, "VisibilityTerm" , "mfvis_prop")
        self.draw_button(layout, "In IOR" , "eta" )
        self.draw_button(layout, "Absorption Coefficient" , "k")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'MicroFacetDistribution' , 'mfdist_prop' , indented_label)
        self.draw_prop(layout, 'VisibilityTerm' , 'mfvis_prop' , indented_label)
        self.draw_prop(layout, "In IOR" , "eta" , indented_label)
        self.draw_prop(layout, "Absorption Coefficient" , "k", indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='MicroFacetDistribution' , type='string', value= self.mfdist_prop )
        ET.SubElement( xml_node , 'Property' , name='Visibility' , type='string', value= self.mfvis_prop )
        ET.SubElement( xml_node , 'Property' , name='eta' , type='color', value= '%f %f %f'%(self.eta[0],self.eta[1],self.eta[2])  )
        ET.SubElement( xml_node , 'Property' , name='k' , type='color', value= '%f %f %f'%(self.k[0],self.k[1],self.k[2]) )

class SORTNodeMicrofacetRefraction(SORTShadingNode):
    bl_label = 'SORT_microfacet_refraction'
    bl_idname = 'SORTNodeMicrofacetRefraction'

    mfdist_item = [ ("Blinn", "Blinn", "", 1),
                    ("Beckmann" , "Beckmann" , "", 2),
                    ("GGX" , "GGX" , "" , 3)
                   ]
    mfdist_prop = bpy.props.EnumProperty(name='',items=mfdist_item)

    mfvis_item = [ ("Implicit", "Implicit", "", 1),
                   ("Neumann" , "Neumann" , "", 2),
                   ("Kelemen" , "Kelemen" , "" , 3),
                   ("Schlick" , "Schlick" , "" , 4),
                   ("Smith" , "Smith" , "" , 5),
                   ("SmithJointApprox" , "SmithJointApprox" , "" , 6 ),
                   ("CookTorrance" , "CookTorrance" , "" , 7)
                   ]
    mfvis_prop = bpy.props.EnumProperty(name='',items=mfvis_item,default="CookTorrance")

    # dielectric-dielectric parameters
    int_ior = bpy.props.FloatProperty(name='', default=1.0, min=1.0, max=10.0)
    ext_ior = bpy.props.FloatProperty(name='', default=1.1, min=1.0, max=10.0)

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.inputs.new('SORTNodeSocketFloat', 'Roughness')
        self.outputs.new('SORTNodeSocketColor', 'Result')

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "MicroFacetDistribution" , "mfdist_prop")
        self.draw_button(layout, "VisibilityTerm" , "mfvis_prop")
        self.draw_button(layout, "In IOR" , "int_ior")
        self.draw_button(layout, "Ext IOR" , "ext_ior")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'MicroFacetDistribution' , 'mfdist_prop' , indented_label)
        self.draw_prop(layout, 'VisibilityTerm' , 'mfvis_prop' , indented_label)
        self.draw_prop(layout, "In IOR" , "int_ior", indented_label)
        self.draw_prop(layout, "Ext IOR" , "ext_ior", indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='MicroFacetDistribution' , type='string', value= self.mfdist_prop )
        ET.SubElement( xml_node , 'Property' , name='Visibility' , type='string', value= self.mfvis_prop )
        ET.SubElement( xml_node , 'Property' , name='in_ior' , type='color', value= '%f'%(self.int_ior)  )
        ET.SubElement( xml_node , 'Property' , name='ext_ior' , type='color', value= '%f'%(self.ext_ior) )

# merl node
class SORTNodeMerl(SORTShadingNode):
    bl_label = 'SORT_merl'
    bl_idname = 'SORTNodeMerl'

    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        self.outputs.new('SORTNodeSocketColor', 'Result')

    def draw_buttons(self, context, layout):
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='string', value= self.file_name_prop )

# oren nayar node
class SORTNodeOrenNayar(SORTShadingNode):
    bl_label = 'SORT_orennayar'
    bl_idname = 'SORTNodeOrenNayar'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.inputs.new('SORTNodeSocketFloat', 'Roughness')
        self.outputs.new('SORTNodeSocketColor', 'Result')

# reflection node
class SORTNodeReflection(SORTShadingNode):
    bl_label = 'SORT_reflection'
    bl_idname = 'SORTNodeReflection'

    eta = bpy.props.FloatVectorProperty(name='', default=(0.37, 0.37, 0.37), min=0.10, max=10.0)
    k = bpy.props.FloatVectorProperty(name='', default=(2.82, 2.82, 2.82), min=1.0, max=10.0)

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.outputs.new('SORTNodeSocketColor', 'Result')

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "In IOR" , "eta" )
        self.draw_button(layout, "Absorption Coefficient" , "k")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, "In IOR" , "eta" , indented_label)
        self.draw_prop(layout, "Absorption Coefficient" , "k", indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='eta' , type='color', value= '%f %f %f'%(self.eta[0],self.eta[1],self.eta[2])  )
        ET.SubElement( xml_node , 'Property' , name='k' , type='color', value= '%f %f %f'%(self.k[0],self.k[1],self.k[2]) )

# reflection node
class SORTNodeRefraction(SORTShadingNode):
    bl_label = 'SORT_refraction'
    bl_idname = 'SORTNodeRefraction'

    int_ior = bpy.props.FloatProperty(name='', default=1.0, min=1.0, max=10.0)
    ext_ior = bpy.props.FloatProperty(name='', default=1.1, min=1.0, max=10.0)

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.outputs.new('SORTNodeSocketColor', 'Result')

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "In IOR" , "int_ior")
        self.draw_button(layout, "Ext IOR" , "ext_ior")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, "In IOR" , "int_ior", indented_label)
        self.draw_prop(layout, "Ext IOR" , "ext_ior", indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='in_ior' , type='color', value= '%f'%(self.int_ior)  )
        ET.SubElement( xml_node , 'Property' , name='ext_ior' , type='color', value= '%f'%(self.ext_ior) )

# operator nodes
class SORTNodeAdd(SORTShadingNode):
    bl_label = 'SORT_add'
    bl_idname = 'SORTNodeAdd'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'Color1')
        self.inputs.new('SORTNodeSocketColor', 'Color2')
        self.outputs.new('SORTNodeSocketColor', 'Result')

class SORTNodeMultiply(SORTShadingNode):
    bl_label = 'SORT_multiply'
    bl_idname = 'SORTNodeMultiply'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'Color1')
        self.inputs.new('SORTNodeSocketColor', 'Color2')
        self.outputs.new('SORTNodeSocketColor', 'Result')

class SORTNodeBlend(SORTShadingNode):
    bl_label = 'SORT_blend'
    bl_idname = 'SORTNodeBlend'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'Color1')
        self.inputs.new('SORTNodeSocketFloat', 'Factor1')
        self.inputs.new('SORTNodeSocketColor', 'Color2')
        self.inputs.new('SORTNodeSocketFloat', 'Factor2')
        self.outputs.new('SORTNodeSocketColor', 'Result')

class SORTNodeLerp(SORTShadingNode):
    bl_label = 'SORT_lerp'
    bl_idname = 'SORTNodeLerp'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'Color1')
        self.inputs.new('SORTNodeSocketColor', 'Color2')
        self.inputs.new('SORTNodeSocketFloat', 'Factor')
        self.outputs.new('SORTNodeSocketColor', 'Result')

# input nodoes
class SORTNodePosition(SORTShadingNode):
    bl_label = 'SORT_position'
    bl_idname = 'SORTNodePosition'

    def init(self, context):
        self.outputs.new('SORTShaderSocket', 'Result')

class SORTNodeNormal(SORTShadingNode):
    bl_label = 'SORT_normal'
    bl_idname = 'SORTNodeNormal'

    def init(self, context):
        self.outputs.new('SORTShaderSocket', 'Result')

class SORTNodeUV(SORTShadingNode):
    bl_label = 'SORT_uv'
    bl_idname = 'SORTNodeUV'

    def init(self, context):
        self.outputs.new('SORTNodeSocketFloat2', 'Result')

# texture nodes
class SORTNodeConstant(SORTShadingNode):
    bl_label = 'SORT_constant'
    bl_idname = 'SORTNodeConstant'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'Color')
        self.outputs.new('SORTNodeSocketColor', 'Result')

class SORTNodeGrid(SORTShadingNode):
    bl_label = 'SORT_grid'
    bl_idname = 'SORTNodeGrid'

    def init(self, context):
        #socket = self.inputs.new('SORTNodeSocketFloat2','UV')
        self.inputs.new('SORTNodeSocketColor', 'Color1')
        self.inputs.new('SORTNodeSocketColor', 'Color2')
        self.outputs.new('SORTNodeSocketColor', 'Result')

class SORTNodeCheckbox(SORTShadingNode):
    bl_label = 'SORT_checkbox'
    bl_idname = 'SORTNodeCheckbox'

    def init(self, context):
        #socket = self.inputs.new('SORTNodeSocketFloat2','UV')
        self.inputs.new('SORTNodeSocketColor', 'Color1')
        self.inputs.new('SORTNodeSocketColor', 'Color2')
        self.outputs.new('SORTNodeSocketColor', 'Result')

class SORTNodeImage(SORTShadingNode):
    bl_label = 'SORT_image'
    bl_idname = 'SORTNodeImage'

    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        #socket = self.inputs.new('SORTNodeSocketFloat2','UV')
        self.outputs.new('SORTNodeSocketColor', 'Result')

    def draw_buttons(self, context, layout):
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='string', value= self.file_name_prop )

# our own base class with an appropriate poll function,
# so the categories only show up in our own tree type
class SORTPatternNodeCategory(NodeCategory):
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

def register():
    # all categories in a list
    node_categories = [
        # identifier, label, items list
        SORTPatternNodeCategory("SORT_bxdf", "SORT Bxdfs",items= [NodeItem("SORTNodeLambert"),NodeItem("SORTNodeMerl"),NodeItem("SORTNodeMicrofacetReflection"),NodeItem("SORTNodeMicrofacetRefraction"),NodeItem("SORTNodeOrenNayar"),NodeItem("SORTNodeReflection"),NodeItem("SORTNodeRefraction")] ),
        SORTPatternNodeCategory("SORT_operator", "SORT Operator",items= [NodeItem("SORTNodeAdd"),NodeItem("SORTNodeMultiply"),NodeItem("SORTNodeBlend"),NodeItem("SORTNodeLerp")] ),
        SORTPatternNodeCategory("SORT_texture", "SORT Texture",items= [NodeItem("SORTNodeGrid"),NodeItem("SORTNodeCheckbox"),NodeItem("SORTNodeImage")] ),
        SORTPatternNodeCategory("SORT_constant", "SORT Constant",items= [NodeItem("SORTNodeConstant")] ),
        SORTPatternNodeCategory("SORT_input", "SORT Input",items=[],),
    ]
    nodeitems_utils.register_node_categories("SORTSHADERNODES",node_categories)

    # register node types
    SORTPatternGraph.nodetypes[SORTNodeLambert] = 'SORTNodeLambert'
    SORTPatternGraph.nodetypes[SORTNodeMerl] = 'SORTNodeMerl'
    SORTPatternGraph.nodetypes[SORTNodeMicrofacetReflection] = 'SORTNodeMicrofacetReflection'
    SORTPatternGraph.nodetypes[SORTNodeMicrofacetRefraction] = 'SORTNodeMicrofacetRefraction'
    SORTPatternGraph.nodetypes[SORTNodeOrenNayar] = 'SORTNodeOrenNayar'
    SORTPatternGraph.nodetypes[SORTNodeReflection] = 'SORTNodeReflection'
    SORTPatternGraph.nodetypes[SORTNodeRefraction] = 'SORTNodeRefraction'