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
import nodeitems_utils
from . import osl_parser
from . import properties
from .. import base

class SORTPatternNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'SORTPatternGraph'

class SORTMaterial(bpy.types.PropertyGroup):
    sortnodetree = bpy.props.StringProperty(name="Nodetree",default='')
    use_sort_nodes = bpy.props.BoolProperty(name="Nodetree",default=False)

# node tree for sort
@base.register_class
class SORTPatternGraph(bpy.types.NodeTree):
    bl_idname = 'SORTPatternGraph'
    bl_label = 'SORT Pattern Graph'
    bl_icon = 'MATERIAL'
    nodetypes = {}
    node_categories = {}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine == 'SORT_RENDERER'

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

    @classmethod
    def register_node(cls, category):
        def registrar(nodecls):
            base.register_class(nodecls)
            d = cls.node_categories.setdefault(category, [])
            d.append(nodecls)
            return nodecls
        return registrar

    @classmethod
    def register(cls):
        bpy.types.Material.sort_material = bpy.props.PointerProperty(type=SORTMaterial, name="SORT Material Settings")
        cats = []
        for c, l in sorted(cls.node_categories.items()):
            cid = 'SORT_' + c.replace(' ', '').upper()
            items = [nodeitems_utils.NodeItem(nc.__name__) for nc in l]
            cats.append(SORTPatternNodeCategory(cid, c, items=items))

            cls.nodetypes[c] = []
            for item in l :
                cls.nodetypes[c].append((item.__name__,item.bl_label,item.output_type))
        nodeitems_utils.register_node_categories('SORTSHADERNODES', cats)

    @classmethod
    def unregister(cls):
        nodeitems_utils.unregister_node_categories('SORTSHADERNODES')
        del bpy.types.Material.sort_material

# sort material node root
class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'
    output_type = 'SORTNodeSocketColor'
    property_list = []
    osl_shader = ''

    # register all property and sockets
    def register_prop(self,disable_output = False):
        inputs_list , outputs_list = osl_parser.parse_osl_params( self.osl_shader )
        for input_param in inputs_list:
            if input_param[1]:  # make sure it is a socket
                param_name = input_param[2][0]
                self.inputs.new( input_param[0].__name__ , param_name )
                for meta_data in input_param[3]:
                    if meta_data[2][0] == 'default_value' and self.inputs[param_name].no_default_value() is False:
                        self.inputs[param_name].default_value = meta_data[2][1]
        for output in outputs_list:
            self.outputs.new( output[0].__name__ , output[2][0] )

    # this is not an overriden interface
    # draw all properties ( not socket ) in material panel
    def draw_props(self, context, layout, indented_label):
        inputs_list , dummy_outputs = osl_parser.parse_osl_params( self.osl_shader )
        for input_list in inputs_list:
            # make sure it is not a socket first
            if input_list[1]:
                continue

            var_name = input_list[2][0]
            value = input_list[0]

            split = layout.split(0.3)
            row = split.row()
            indented_label(row)
            row.label(var_name)
            prop_row = split.row()
            prop_row.prop(self,var_name,text="")

    # override the base interface
    # draw all properties ( not socket ) in material nodes
    def draw_buttons(self, context, layout):
        inputs_list , dummy_outputs = osl_parser.parse_osl_params( self.osl_shader )
        for input_list in inputs_list:
            # make sure it is not a socket first
            if input_list[1]:
                continue
            var_name = input_list[2][0]
            layout.prop(self,var_name)

    # export data to pbrt
    def export_pbrt(self, output_pbrt_type , output_pbrt_prop):
        pass

    def serialize_prop(self, fs):
        inputs_list , dummy_outputs = osl_parser.parse_osl_params( self.osl_shader )
        fs.serialize( len( inputs_list ) )
        for param in inputs_list:
            pram_name = param[2][0]
            # check if it is a socket or not
            if param[1]:
                value = self.inputs[pram_name].export_osl_value()
                fs.serialize( value )
            else:
                attr_wrapper = getattr(self, pram_name + '_wrapper' )
                attr = getattr(self, pram_name)
                value = attr_wrapper.export_osl_value(attr_wrapper,attr)
                fs.serialize( value )

    # register all properties in the class
    @classmethod
    def register(cls):
        inputs_list , dummy_outputs = osl_parser.parse_osl_params( cls.osl_shader )

        for input_list in inputs_list:
            # make sure it is not a socket first
            if input_list[1]:
                continue

            var_name = input_list[2][0]
            value = input_list[0]
            meta_data = input_list[3]

            prop = {}
            prop['name'] = var_name
            for md in meta_data:
                prop[md[2][0]] = md[2][1]

            # populate items in a drop box
            prop['items'] = []
            if  value.__name__ == 'SORTNodePropertyEnum':
                items_string = prop['sort_items']
                items = items_string.split(';')
                k = 1
                for item in items:
                    prop['items'].append( ( item , item , "" , k ) )
                    k += 1

            if 'default_value' not in prop:
                prop['default_value'] = 0.0
            if 'min' not in prop:
                prop['min'] = 0.0
            if 'max' not in prop:
                prop['max'] = 0.0

            value.setup( prop )
            setattr(cls, var_name + '_wrapper', value )
            setattr(cls, var_name, value.default_value )

    # unregister all properties in the class
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

    @classmethod
    def get_mro(cls, attribname):
        for k in reversed(cls.__mro__):
            vs = vars(k)
            if attribname in vs:
                yield vs[attribname]
    # register all properties in constructor
    def init(self, context):
        def do_mro(self, methname, *args, **kwargs):
            for meth in self.get_mro(methname):
                yield meth(self, *args, **kwargs)
        for _ in do_mro(self, 'register_prop'):
            pass

    # output nothing by default
    def populateResources( self , resources ):
        pass

    # generate open shading lanugage source code
    def generate_osl_source(self):
        return self.osl_shader

# Base class for SORT BXDF Node
class SORTShadingNode_BXDF(SORTShadingNode):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'
    output_type = 'SORTNodeSocketBxdf'
    pbrt_bxdf_type = ''

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

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Material Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Principle(SORTShadingNode_BXDF):
    bl_label = 'Principle'
    bl_idname = 'SORTNode_Material_Principle'

    osl_shader = '''
        shader Principle( float RoughnessU = @ ,
                          float RoughnessV = @ ,
                          float Metallic = @ [[ float default_value = 1.0 ]] ,
                          float Specular = @ ,
                          color BaseColor = @ ,
                          normal Normal = @ ,
                          output closure color Result = color(0) ){
            // UE4 PBS model, this may very likely not very updated.
            Result = lambert( BaseColor , Normal ) * ( 1 - Metallic ) * 0.92 + microfacetReflection( "GGX", color( 0.37 ), color( 2.82 ), RoughnessU, RoughnessV, BaseColor , Normal ) * ( Metallic * 0.92 + 0.08 * Specular );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_DisneyBRDF(SORTShadingNode_BXDF):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'
    pbrt_bxdf_type = 'disney'

    osl_shader = '''
        shader Disney( float SubSurface = @ ,
                       float Metallic = @ [[ float default_value = 1.0 ]] ,
                       float Specular = @ ,
                       float SpecularTint = @ ,
                       float Roughness = @ ,
                       float Anisotropic = @ ,
                       float Sheen = @ ,
                       float SheenTint = @ ,
                       float Clearcoat = @ ,
                       float ClearcoatGloss = @ ,
                       color BaseColor = @ ,
                       normal Normal = @ ,
                       output closure color Result = color(0) ){
            Result = disney( SubSurface , Metallic , Specular , SpecularTint , Roughness , Anisotropic , Sheen , SheenTint , Clearcoat , ClearcoatGloss , BaseColor , Normal );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Glass(SORTShadingNode_BXDF):
    bl_label = 'Glass'
    bl_idname = 'SORTNode_Material_Glass'
    pbrt_bxdf_type = 'glass'

    osl_shader = '''
        shader Glass( color Reflectance = @ ,
                      color Transmittance = @ ,
                      float RoughnessU = @ ,
                      float RoughnessV = @ ,
                      normal Normal = @ ,
                      output closure color Result = color(0) ){
            Result = dieletric( Reflectance , Transmittance , RoughnessU , RoughnessV , Normal );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Plastic(SORTShadingNode_BXDF):
    bl_label = 'Plastic'
    bl_idname = 'SORTNode_Material_Plastic'
    pbrt_bxdf_type = 'plastic'

    osl_shader = '''
        shader Plastic( color Diffuse = @ ,
                        color Specular = @ ,
                        float Roughness = @ [[ float default_value = 0.2 ]] ,
                        normal Normal = @ ,
                        output closure color Result = color(0) ){
            if( Diffuse[0] != 0 || Diffuse[1] != 0 || Diffuse[2] != 0 )
                Result += lambert( Diffuse , N );
            if( Specular[0] != 0 || Specular[1] != 0 || Specular[2] != 0 )
                Result += microfacetReflectionDieletric( "GGX", 1.0, 1.5, Roughness, Roughness, Specular , Normal );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Matte(SORTShadingNode_BXDF):
    bl_label = 'Matte'
    bl_idname = 'SORTNode_Material_Matte'
    pbrt_bxdf_type = 'matte'

    osl_shader = '''
        shader Matte( color BaseColor = @ ,
                      float Roughness = @ , 
                      normal Normal = @ ,
                      output closure color Result = color(0)){
            if( Roughness == 0.0 )
                Result = lambert( BaseColor , N );
            else
                Result = orenNayar( BaseColor , Roughness , Normal );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Mirror(SORTShadingNode_BXDF):
    bl_label = 'Mirror'
    bl_idname = 'SORTNode_Material_Mirror'
    pbrt_bxdf_type = 'mirror'

    osl_shader = '''
        shader Mirror( color BaseColor = @ , 
                       normal Normal = @ ,
                       output closure color Result = color(0) ){
            Result = microfacetReflection( "GGX" , 1.0 , 1.0 , 0.0 , 0.0 , BaseColor , Normal );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Hair(SORTShadingNode_BXDF):
    bl_label = 'Hair'
    bl_idname = 'SORTNode_Material_Hair'

    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        float helper( float x , float inv ){
            float y = log(x) * inv;
            return y * y;
        }
        shader Hair( color HairColor = @ ,
                     float LongtitudinalRoughness = @ [[ float default_value = 0.2 ]] ,
                     float AzimuthalRoughness = @ [[ float default_value = 0.2 ]] ,
                     lfloat IndexofRefraction = @ [[ lfloat default_value = 1.55 ]] , 
                     output closure color Result = color(0) ){
            float inv = 1.0 / ( 5.969 - 0.215 * AzimuthalRoughness + 2.532 * pow(AzimuthalRoughness,2.0) - 10.73 * pow(AzimuthalRoughness,3.0) + 
                        5.574 * pow(AzimuthalRoughness,4.0) + 0.245 * pow(AzimuthalRoughness, 5.0) );
            color sigma = color( helper(HairColor[0],inv) , helper(HairColor[1],inv) , helper(HairColor[2],inv) );
            Result = hair( sigma , LongtitudinalRoughness , AzimuthalRoughness , IndexofRefraction );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Blend(SORTShadingNode_BXDF):
    bl_label = 'Blend'
    bl_idname = 'SORTNode_Material_Blend'

    osl_shader = '''
        shader MaterialBlend(  closure color Bxdf0 = @ ,
                               closure color Bxdf1 = @ ,
                               float Factor = @ ,
                               output closure color Result = color(0) ){
            Result = Bxdf0 * ( 1.0 - Factor ) + Bxdf1 * Factor;
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_DoubleSided(SORTShadingNode_BXDF):
    bl_label = 'Double-Sided'
    bl_idname = 'SORTNode_Material_DoubleSided'

    osl_shader = '''
        shader MaterialBlend(  closure color Bxdf0 = @ ,
                               closure color Bxdf1 = @ ,
                               output closure color Result = color(0) ){
            Result = doubleSided( Bxdf0 , Bxdf1 );
        }
    '''

@SORTPatternGraph.register_node('Materials')
class SORTNode_BXDF_Coat(SORTShadingNode_BXDF):
    bl_label = 'Coat'
    bl_idname = 'SORTNode_BXDF_Coat'
                      
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        float helper( float x , float inv ){
            float y = log(x) * inv;
            return y * y;
        }
        shader Coat( lfloat     IOR = @ [[ lfloat default_value = 1.5 ]] ,
                     float     Roughness = @ ,
                     color     ColorTint = @ ,
                     closure color Surface = @ ,
                     normal Normal = @ ,
                     output closure color Result = color(0) ){
            float inv = 1.0 / ( 5.969 - 0.215 * Roughness + 2.532 * pow(Roughness,2.0) - 10.73 * pow(Roughness,3.0) + 5.574 * pow(Roughness,4.0) + 0.245 * pow(Roughness, 5.0) );
            color sigma = color( helper(ColorTint[0],inv) , helper(ColorTint[1],inv) , helper(ColorTint[2],inv) );
            Result = coat( Surface , Roughness , IOR , sigma , Normal );
        }
    '''

#------------------------------------------------------------------------------------------------------------------------------------
#                                               BXDF Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_MicrofacetReflection(SORTShadingNode_BXDF):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_BXDF_MicrofacetReflection'

    osl_shader = '''
        shader MicrofacetRelection(  string MicroFacetDistribution = @ [[ string sort_items = "Blinn;GGX;Beckmann" , string default_value = "GGX" ]] ,
                                     vector Interior_IOR = @ [[ vector default_value = vector(0.37) ]] ,
                                     vector Absorption_Coefficient = @ [[ vector default_value = vector(2.82) ]] ,
                                     float  RoughnessU = @ [[ float default_value = 0.1 ]] ,
                                     float  RoughnessV = @ [[ float default_value = 0.1 ]] ,
                                     color  BaseColor = @ ,
                                     normal Normal = @ ,
                                     output closure color Result = color(0) ){
            Result = microfacetReflection( MicroFacetDistribution , Interior_IOR , Absorption_Coefficient , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_MicrofacetRefraction(SORTShadingNode_BXDF):
    bl_label = 'MicrofacetRefraction'
    bl_idname = 'SORTNode_BXDF_MicrofacetRefraction'

    osl_shader = '''
        shader MicrofacetRefraction( string MicroFacetDistribution = @ [[ string sort_items = "Blinn;GGX;Beckmann" , string default_value = "GGX" ]] ,
                                     lfloat  Interior_IOR = @ [[ float default_value = 1.1 ]] ,
                                     lfloat  Exterior_IOR = @ [[ float default_value = 1.0 ]] ,
                                     float  RoughnessU = @ [[ float default_value = 0.1 ]] ,
                                     float  RoughnessV = @ [[ float default_value = 0.1 ]] ,
                                     color  BaseColor = @ , 
                                     normal Normal = @ ,
                                     output closure color Result = color(0) ){
            Result = microfacetRefraction( MicroFacetDistribution , Interior_IOR , Exterior_IOR , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_AshikhmanShirley(SORTShadingNode_BXDF):
    bl_label = 'AshikhmanShirley'
    bl_idname = 'SORTNode_BXDF_AshikhmanShirley'

    osl_shader = '''
        shader AshikhmanShirley( float Specular = @ ,
                                 float RoughnessU = @ [[ float default_value = 0.1 ]] ,
                                 float RoughnessV = @ [[ float default_value = 0.1 ]] ,
                                 color Diffuse = @ ,
                                 normal Normal = @ ,
                                 output closure color Result = color(0) ){
            Result = ashikhmanShirley( Specular , RoughnessU , RoughnessV , Diffuse , Normal );
        }
    '''

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_Phong(SORTShadingNode_BXDF):
    bl_label = 'Phong'
    bl_idname = 'SORTNode_BXDF_Phong'

    osl_shader = '''
        shader Phong( lfloat SpecularPower = @ [[ lfloat default_value = 32.0 ]] ,
                      float DiffuseRatio = @  [[ float default_value = 0.2 ]] ,
                      color Specular = @ ,
                      color Diffuse = @ ,
                      normal Normal = @ ,
                      output closure color Result = color(0) ){
            Result = phong( Diffuse * DiffuseRatio , ( 1.0 - DiffuseRatio ) * Specular , SpecularPower , Normal );
        }
    '''

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_Lambert(SORTShadingNode_BXDF):
    bl_label = 'Lambert'
    bl_idname = 'SORTNode_BXDF_Lambert'

    osl_shader = '''
        shader Lambert( color Diffuse = @ [[ color default_value = color( 1.0 ) ]] ,
                        normal Normal = @ [[ normal default_value = normal( 0.0 , 1.0 , 0.0 ) , float min = 0.0 , string pbrt_name = \"test_for_now\" ]] ,
                        output closure color Result = color(0) ){
            Result = lambert( Diffuse , Normal );
        }
    ''' 

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_LambertTransmission(SORTShadingNode_BXDF):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_BXDF_LambertTransmission'

    osl_shader = '''
        shader LambertTransmission( color Diffuse = @ ,
                                    normal Normal = @ ,
                                    output closure color Result = color(0) ){
            Result = lambertTransmission( Diffuse , Normal );
        }
    '''

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_OrenNayar(SORTShadingNode_BXDF):
    bl_label = 'OrenNayar'
    bl_idname = 'SORTNode_BXDF_OrenNayar'

    osl_shader = '''
        shader OrenNayar( float roughness = @,
                          color Diffuse = @ ,
                          normal Normal = @ ,
                          output closure color Result = color(0) ){
            Result = orenNayar( Diffuse , roughness , Normal );
        }
    '''

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_MERL(SORTShadingNode_BXDF):
    bl_label = 'MERL'
    bl_idname = 'SORTNode_BXDF_MERL'

    osl_shader = '''
        shader merlBRDF( path Filename = @,
                         normal Normal = @ ,
                         output closure color Result = color(0) ){
            Result = merlBRDF( %s , Normal );
        }
    '''

    def generate_osl_source(self):
        return self.osl_shader%(self.ResourceIndex)

    def populateResources( self , resources ):
        found = False
        for resource in resources:
            if resource[1] == self.Filename:
                found = True
        if not found:
            self.ResourceIndex = len(resources)
            resources.append( ( self.Filename , 'MerlBRDFMeasuredData' ) )
        pass

@SORTPatternGraph.register_node('BXDFs')
class SORTNode_BXDF_Fourier(SORTShadingNode_BXDF):
    bl_label = 'Fourier BXDF'
    bl_idname = 'SORTNode_BXDF_Fourier'
    osl_shader = '''
        shader FourierBRDF( path Filename = @,
                            normal Normal = @ ,
                            output closure color Result = color(0) ){
            Result = fourierBRDF( %s , Normal );
        }
    '''

    def generate_osl_source(self):
        return self.osl_shader%(self.ResourceIndex)

    def populateResources( self , resources ):
        found = False
        for resource in resources:
            if resource[1] == self.Filename:
                found = True
        if not found:
            self.ResourceIndex = len(resources)
            resources.append( ( self.Filename , 'FourierBRDFMeasuredData' ) )
        pass

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Operator Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
@SORTPatternGraph.register_node('Operator')
class SORTNodeAdd(SORTShadingNode):
    bl_label = 'Add'
    bl_idname = 'SORTNodeAdd'
    osl_shader = '''
        shader Add( color Color1 = @ ,
                    color Color2 = @ ,
                    output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = Color1 + Color2;
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeOneMinus(SORTShadingNode):
    bl_label = 'One Minus'
    bl_idname = 'SORTNodeOneMinus'
    osl_shader = '''
        shader OneMinus( color Color = @ ,
                         output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = color( 1.0 ) - Color;
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeMultiply(SORTShadingNode):
    bl_label = 'Multiply'
    bl_idname = 'SORTNodeMultiply'
    osl_shader = '''
        shader Multiply( color Color1 = @ ,
                         color Color2 = @ ,
                         output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = Color1 * Color2;
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeBlend(SORTShadingNode):
    bl_label = 'Blend'
    bl_idname = 'SORTNodeBlend'
    osl_shader = '''
        shader Blend( color Color1 = @ ,
                      float Factor1 = @ ,
                      color Color2 = @ ,
                      float Factor2 = @ ,
                      output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = Color1 * Factor1 + Color2 * Factor2;
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeLerp(SORTShadingNode):
    bl_label = 'Lerp'
    bl_idname = 'SORTNodeLerp'
    osl_shader = '''
        shader Lerp( color Color1 = @ ,
                     color Color2 = @ ,
                     float Factor = @ ,
                     output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = Color1 * ( 1.0 - Factor ) + Color2 * Factor;
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeLinearToGamma(SORTShadingNode):
    bl_label = 'LinearToGamma'
    bl_idname = 'SORTNodeLinearToGamma'
    osl_shader = '''
        shader LinearToGamma( color Color = @ ,
                              output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = pow( Color , 1.0/2.2 );
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeGammaToLinear(SORTShadingNode):
    bl_label = 'GammaToLinear'
    bl_idname = 'SORTNodeGammaToLinear'
    osl_shader = '''
        shader GammaToLinear( color Color = @ ,
                              output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = pow( Color , 2.2 );
        }
    '''

@SORTPatternGraph.register_node('Operator')
class SORTNodeDecodeNormal(SORTShadingNode):
    bl_label = 'DecodeNormal'
    bl_idname = 'SORTNodeDecodeNormal'
    output_type = 'SORTNodeSocketNormal'
    osl_shader = '''
        shader DecodeNormal( color Color = @ ,
                             output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = 2.0 * color( Color[0] , Color[2] , Color[1] ) - 1.0;
        }
    '''

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Texture Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
@SORTPatternGraph.register_node('Image')
class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'
    osl_shader = '''
        shader Grid( color Color1 = @ [[ color default_value = color(0.2) ]],
                     color Color2 = @ ,
                     float Treshold = @ [[ float default_value = 0.1 ]] ,
                     uv UVMapping = @ ,
                     output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            float fu = UVMapping[0] - floor( UVMapping[0] ) - 0.5;
            float fv = UVMapping[1] - floor( UVMapping[1] ) - 0.5;
            float half_threshold = ( 1.0 - Treshold ) * 0.5;
            if( fu <= half_threshold && fu >= -half_threshold && fv <= half_threshold && fv >= -half_threshold )
                Result = Color1;
            else
                Result = Color2;
        }
    '''
    
@SORTPatternGraph.register_node('Image')
class SORTNodeCheckerBoard(SORTShadingNode):
    bl_label = 'CheckerBoard'
    bl_idname = 'SORTNodeCheckerBoard'
    osl_shader = '''
        shader CheckerBoard( color Color1 = @ [[ color default_value = color(0.2) ]],
                             color Color2 = @ ,
                             uv UVMapping = @ ,
                             output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            float fu = UVMapping[0] - floor( UVMapping[0] );
            float fv = UVMapping[1] - floor( UVMapping[1] );
            if( ( fu > 0.5 && fv > 0.5 ) || ( fu < 0.5 && fv < 0.5 ) )
                Result = Color1;
            else
                Result = Color2;
        }
    '''

@SORTPatternGraph.register_node('Image')
class SORTNodeImage(SORTShadingNode):
    bl_label = 'Image'
    bl_idname = 'SORTNodeImage'
    osl_shader = '''
        shader ImageTexture( path Filename = @ ,
                             uv UVMapping = @ ,
                             output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = texture( Filename , UVMapping[0] , UVMapping[1] );
        }
    '''

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Input Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
@SORTPatternGraph.register_node('Input')
class SORTNodeConstant(SORTShadingNode):
    bl_label = 'Constant Color'
    bl_idname = 'SORTNodeConstant'
    osl_shader = '''
        shader ConstantColor( color Color = @ ,
                              output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = Color;
        }
    '''

@SORTPatternGraph.register_node('Input')
class SORTNodeConstantFloat(SORTShadingNode):
    bl_label = 'Constant Float'
    bl_idname = 'SORTNodeConstantFloat'
    osl_shader = '''
        shader ConstantFloat( float Value = @ ,
                              output float Result = 0.0 ){
            Result = Value;
        }
    '''

@SORTPatternGraph.register_node('Input')
class SORTNodeInput(SORTShadingNode):
    bl_label = 'Input'
    bl_idname = 'SORTNodeInput'
    osl_shader = '''
        shader DataInput( output uv     UVCoordinate = uv( 0.0 ) ){
            UVCoordinate = uv( u , v , 0.0 );
        }
    '''

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Convertor Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
@SORTPatternGraph.register_node('Convertor')
class SORTNodeComposite(SORTShadingNode):
    bl_label = 'Composite'
    bl_idname = 'SORTNodeComposite'
    osl_shader = '''
        shader Composite( float Red = @ ,
                          float Green = @ ,
                          float Blue = @ ,
                          output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            Result = color( Red , Green , Blue );
        }
    '''

@SORTPatternGraph.register_node('Convertor')
class SORTNodeExtract(SORTShadingNode):
    bl_label = 'Extract'
    bl_idname = 'SORTNodeExtract'
    output_type = 'SORTNodeSocketFloat'
    osl_shader = '''
        shader Extract( color Color = @,
                        output float Red = 0.0 ,
                        output float Green = 0.0 ,
                        output float Blue = 0.0 ,
                        output float Intensity = 0.0 ){
            Red = Color[0];
            Green = Color[1];
            Blue = Color[2];
            Intensity = Color[0] * 0.212671 + Color[1] * 0.715160 + Color[2] * 0.072169;
        }
    '''

@SORTPatternGraph.register_node('Convertor')
class SORTNodeRemappingUV(SORTShadingNode):
    bl_label = 'RemappingUV'
    bl_idname = 'SORTNodeRemappingUV'
    output_type = 'SORTNodeSocketFloat'
    osl_shader = '''
        shader Extract( uv     Coord = @,
                        lfloat  TilingU = @ [[ lfloat default_value = 1.0 ]] ,
                        lfloat  TilingV = @ [[ lfloat default_value = 1.0 ]] ,
                        lfloat  OffsetU = @ ,
                        lfloat  OffsetV = @ ,
                        output uv Result = uv( 0.0 ) ){
            Result = uv( Coord[0] * TilingV + OffsetU , Coord[1] * TilingU + OffsetV , 0.0 );
        }
    '''

#------------------------------------------------------------------------------------------------------------------------------------
#                                               Output Nodes for SORT
#------------------------------------------------------------------------------------------------------------------------------------
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = 'SORTNodeOutput'
    
    osl_shader = '''
        shader SORT_Shader( closure color Surface = @ ){
            Ci = Surface;
        }
    '''