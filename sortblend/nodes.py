import bpy
import xml.etree.cElementTree as ET
from . import common
import nodeitems_utils
from nodeitems_utils import NodeCategory, NodeItem

# node tree for sort
class SORTPatternGraph(bpy.types.NodeTree):
    '''A node tree comprised of renderman nodes'''
    bl_idname = 'SORTPatternGraph'
    bl_label = 'SORT Pattern Graph'
    bl_icon = 'TEXTURE_SHADED'
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

    def draw_color(self, context, node):
        return (0.1, 1.0, 0.2, 0.75)

    def draw(self, context, layout, node, text):
        if self.is_linked or self.is_output:
            layout.label(text)
        else:
            row = layout.row()
            split = row.split(common.label_percentage)
            left_row = split.row()
            left_row.label(text)
            split.prop(node.inputs[text], 'default_value')

# socket for SORT node
class SORTShaderSocket(bpy.types.NodeSocketShader, SORTSocket):
    '''Renderman shader input/output'''
    bl_idname = 'SORTShaderSocket'
    bl_label = 'SORT Shader Socket'
    default_value = None

class SORTNodeSocketColor(bpy.types.NodeSocketColor, SORTSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'

    default_value = bpy.props.FloatVectorProperty( name='' , default=(1.0, 1.0, 1.0) ,subtype='COLOR' )

    def output_default_value_to_str(self):
        return '%f %f %f'%(self.default_value[0],self.default_value[1],self.default_value[2])

class SORTNodeSocketString(bpy.types.NodeSocketString, SORTSocket):
    bl_idname = 'SORTNodeSocketString'
    bl_label = 'SORT String Socket'

    default_value = bpy.props.StringProperty( name='' , default='' ,subtype='FILE_PATH' )

    def output_default_value_to_str(self):
        return self.default_value

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
class SORTOutputNode(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = 'SORTOutputNode'

    def init(self, context):
        input = self.inputs.new('SORTShaderSocket', 'Surface')

# lambert node
class SORTLambertNode(SORTShadingNode):
    bl_label = 'SORT_lambert'
    bl_idname = 'SORTLambertNode'

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.outputs.new('SORTShaderSocket', 'Result')

# microfacte node
class SORTMicrofacetNode(SORTShadingNode):
    bl_label = 'SORT_microfacet'
    bl_idname = 'SORTMicrofacetNode'

    fresnel_item = [ ("FresnelNo", "No Fresnel", "", 1),
                     ("FresnelConductor" , "FresnelConductor" , "", 2),
                     ("FresnelDielectric" , "FresnelDielectric" , "", 3)
                   ]
    fresnel_prop = bpy.props.EnumProperty(name='',items=fresnel_item)

    mfdist_item = [ ("MicroFacetDistribution", "Blinn", "", 1),
                     ("MicroFacetDistribution" , "Anisotropic" , "", 2)
                   ]
    mfdist_prop = bpy.props.EnumProperty(name='',items=mfdist_item)

    def init(self, context):
        self.inputs.new('SORTNodeSocketColor', 'BaseColor')
        self.outputs.new('SORTShaderSocket', 'Result')

    def draw_buttons(self, context, layout):
        self.draw_button(layout, "Fresnel" , "fresnel_prop")
        self.draw_button(layout, "MicroFacetDistribution" , "mfdist_prop")

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Fresnel' , 'fresnel_prop' , indented_label)
        self.draw_prop(layout, 'MicroFacetDistribution' , 'mfdist_prop' , indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='Fresnel' , type='value', value= self.fresnel_prop )
        ET.SubElement( xml_node , 'Property' , name='MicroFacetDistribution' , type='value', value= self.mfdist_prop )

# merl node
class SORTMerlNode(SORTShadingNode):
    bl_label = 'SORT_merl'
    bl_idname = 'SORTMerlNode'

    file_name_prop = bpy.props.StringProperty(name="", default="", subtype='FILE_PATH' )

    def init(self, context):
        self.outputs.new('SORTShaderSocket', 'Result')

    def draw_buttons(self, context, layout):
        row = layout.row()
        row.label("Filename")
        row.prop(self,'file_name_prop')

    def draw_props(self, context, layout, indented_label):
        self.draw_prop(layout, 'Filename' , 'file_name_prop' , indented_label)

    def export_prop(self, xml_node):
        ET.SubElement( xml_node , 'Property' , name='Filename' , type='value', value= self.file_name_prop )

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
        SORTPatternNodeCategory("SORT_output_nodes", "SORT outputs",items = []),
        SORTPatternNodeCategory("SORT_bxdf", "SORT Bxdfs",items= [NodeItem("SORTLambertNode"),NodeItem("SORTMerlNode"),NodeItem("SORTMicrofacetNode")] ),
    ]
    nodeitems_utils.register_node_categories("SORTSHADERNODES",node_categories)

    # register node types
    SORTPatternGraph.nodetypes[SORTLambertNode] = 'SORTLambertNode'