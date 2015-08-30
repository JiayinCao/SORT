import bpy
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
            layout.label(text)
            layout.prop(node.inputs[text], 'default_value')

# Custom socket type for connecting shaders
class SORTShaderSocket(bpy.types.NodeSocketShader, SORTSocket):
    '''Renderman shader input/output'''
    bl_idname = 'SORTShaderSocket'
    bl_label = 'SORT Shader Socket'
    default_value = None

    def draw_value(self, context, layout, node):
        layout.label(self.name)

    def draw_color(self, context, node):
        return (0.1, 1.0, 0.2, 0.75)

    def draw(self, context, layout, node, text):
        layout.label(text)
        pass

class SORTNodeSocketColor(bpy.types.NodeSocketColor, SORTSocket):
    bl_idname = 'SORTNodeSocketColor'
    bl_label = 'SORT Color Socket'

    default_value = bpy.props.FloatVectorProperty( name='' , default=(1.0, 1.0, 1.0) ,subtype='COLOR' )

    def draw_color(self, context, node):
        return (1.0, 1.0, .5, 0.75)

# sort material node root
class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'

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
        self.inputs.new('SORTNodeSocketColor', 'Other')
        self.outputs.new('SORTShaderSocket', 'Result')

# our own base class with an appropriate poll function,
# so the categories only show up in our own tree type
class SORTPatternNodeCategory(NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'SORTPatternGraph'

def register():
    # all categories in a list
    node_categories = [
        # identifier, label, items list
        SORTPatternNodeCategory("SORT_output_nodes", "SORT outputs",items = []),
        SORTPatternNodeCategory("SORT_bxdf", "SORT Bxdfs",items= [NodeItem("SORTLambertNode")] ),
    ]
    nodeitems_utils.register_node_categories("SORTSHADERNODES",node_categories)