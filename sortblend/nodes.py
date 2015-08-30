import bpy

# Custom socket type for connecting shaders
class SORTShaderSocket(bpy.types.NodeSocketShader):
    '''Renderman shader input/output'''
    bl_idname = 'SORTShaderSocket'
    bl_label = 'SORT Shader Socket'

    default_value = bpy.props.FloatProperty( name='Base' , default = 0.0 )

    def draw_value(self, context, layout, node):
        layout.label(self.name)

    def draw_color(self, context, node):
        return (0.1, 1.0, 0.2, 0.75)

    def draw(self, context, layout, node, text):
        layout.label(text)
        pass

class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'

class SORTOutputNode(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = 'SORTOutputNode'

    def init(self, context):
        input = self.inputs.new('SORTShaderSocket', 'Surface')

class SORTLambertNode(SORTShadingNode):
    bl_label = 'SORT_lambert'
    bl_idname = 'SORTLambertNode'

    def init(self, context):
        self.inputs.new('SORTShaderSocket', 'BaseColor')
        self.outputs.new('SORTShaderSocket', 'Result')

class SORTBxdfNode(SORTShadingNode):
    bl_label = 'sort_bxdf'
    bl_idname = 'SORTBxdfNode'

    ext_eta = bpy.props.FloatProperty(name='Ext Eta', default=1.0, min=1.0, max=10.0)

    def init(self, context):
        input = self.inputs.new('SORTShaderSocket', 'Bxdf')
        input = self.inputs.new('SORTShaderSocket', 'Ext Eta')
        input = self.outputs.new('SORTShaderSocket', 'Displacement')
        self.inputs['Bxdf'].name = 'ext_eta'

    def draw_buttons(self, context, layout):
        layout.prop(self, 'ext_eta')

    def draw_buttons_ext(self, context, layout):
        self.inputs['RendermanShaderSocket'].name = 'Alpha Roughness'
        return

    #when a connection is made or removed see if we're in IPR mode and issue updates
    def update(self):
        pass
        #from . import engine
        #if engine.ipr != None and engine.ipr.is_interactive_running:
        #    nt, mat, something_else = RendermanPatternGraph.get_from_context(bpy.context)
        #    engine.ipr.issue_shader_edits(nt = nt)
