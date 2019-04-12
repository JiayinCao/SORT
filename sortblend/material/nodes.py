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
            #for item in l :
            #    cls.nodetypes[c].append((item.__name__,item.bl_label,item.output_type))
        nodeitems_utils.register_node_categories('SORTSHADERNODES', cats)

    @classmethod
    def unregister(cls):
        nodeitems_utils.unregister_node_categories('SORTSHADERNODES')
        del bpy.types.Material.sort_material

class SORTShadingNode(bpy.types.Node):
    bl_label = 'ShadingNode'
    bl_idname = 'SORTShadingNode'
    bl_icon = 'MATERIAL'
    osl_shader = ''

    # register all properties in constructor
    def init(self, context):
        pass
    # output nothing by default
    def populateResources( self , resources ):
        pass
    # generate open shading lanugage source code
    def generate_osl_source(self):
        return self.osl_shader
    # empty implementation
    def serialize_prop(self,fs):
        fs.serialize(0)
        pass
    # to be deleted
    def draw_props(self, context, layout, indented_label):
        pass

@SORTPatternGraph.register_node('Output')
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'SORT_output'
    bl_idname = 'SORTNodeOutput'
    osl_shader = '''
        shader SORT_Shader( closure color Surface = color(0) ){
            Ci = Surface;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Shader' )

#------------------------------------------------------------------------------------#
#                                   BXDF Nodes                                       #
#------------------------------------------------------------------------------------#
@SORTPatternGraph.register_node('Materials')
class SORTNode_BXDF_Lambert(SORTShadingNode):
    bl_label = 'Lambert'
    bl_idname = 'SORTNode_BXDF_Lambert'
    osl_shader = '''
        shader Lambert( color Diffuse = @ ,
                        normal Normal = @ ,
                        output closure color Result = color(0) ){
            Result = lambert( Diffuse , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_BXDF_LambertTransmission(SORTShadingNode):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_BXDF_LambertTransmission'
    osl_shader = '''
        shader LambertTransmission( color Diffuse = @ ,
                                    normal Normal = @ ,
                                    output closure color Result = color(0) ){
            Result = lambertTransmission( Diffuse , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Color'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_BXDF_OrenNayar(SORTShadingNode):
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
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Color'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Mirror(SORTShadingNode):
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
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Color'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Plastic(SORTShadingNode):
    bl_label = 'Plastic'
    bl_idname = 'SORTNode_Material_Plastic'
    osl_shader = '''
        shader Plastic( color Diffuse = @ ,
                        color Specular = @ ,
                        float Roughness = @ ,
                        normal Normal = @ ,
                        output closure color Result = color(0) ){
            if( Diffuse[0] != 0 || Diffuse[1] != 0 || Diffuse[2] != 0 )
                Result += lambert( Diffuse , N );
            if( Specular[0] != 0 || Specular[1] != 0 || Specular[2] != 0 )
                Result += microfacetReflectionDieletric( "GGX", 1.0, 1.5, Roughness, Roughness, Specular , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Roughness'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Glass(SORTShadingNode):
    bl_label = 'Glass'
    bl_idname = 'SORTNode_Material_Glass'
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
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Reflectance' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Transmittance' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['Reflectance'].export_osl_value() )
        fs.serialize( self.inputs['Transmittance'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_UE4Principle(SORTShadingNode):
    bl_label = 'UE4 Principle'
    bl_idname = 'SORTNode_Material_UE4Principle'
    osl_shader = '''
        shader Principle( float RoughnessU = @ ,
                          float RoughnessV = @ ,
                          float Metallic = @ ,
                          float Specular = @ ,
                          color BaseColor = @ ,
                          normal Normal = @ ,
                          output closure color Result = color(0) ){
            // UE4 PBS model, this may very likely not very updated.
            Result = lambert( BaseColor , Normal ) * ( 1 - Metallic ) * 0.92 + microfacetReflection( "GGX", color( 0.37 ), color( 2.82 ), RoughnessU, RoughnessV, BaseColor , Normal ) * ( Metallic * 0.92 + 0.08 * Specular );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Metallic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Metallic'].default_value = 1.0
        self.inputs['RoughnessU'].default_value = 0.2
        self.inputs['RoughnessV'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 6 )
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['Metallic'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_DisneyBRDF(SORTShadingNode):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'
    osl_shader = '''
        shader Disney( float SubSurface = @ ,
                       float Metallic = @ ,
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
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Metallic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Anisotropic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Subsurface' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'SpecularTint' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Sheen' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'SheenTint' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Clearcoat' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'ClearcoatGloss' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Metallic'].default_value = 1.0
        self.inputs['Roughness'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 12 )
        fs.serialize( self.inputs['Subsurface'].export_osl_value() )
        fs.serialize( self.inputs['Metallic'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['SpecularTint'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Anisotropic'].export_osl_value() )
        fs.serialize( self.inputs['Sheen'].export_osl_value() )
        fs.serialize( self.inputs['SheenTint'].export_osl_value() )
        fs.serialize( self.inputs['Clearcoat'].export_osl_value() )
        fs.serialize( self.inputs['ClearcoatGloss'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Hair(SORTShadingNode):
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
                     float LongtitudinalRoughness = @ ,
                     float AzimuthalRoughness = @ ,
                     float IndexofRefraction = @ ,
                     output closure color Result = color(0) ){
            float inv = 1.0 / ( 5.969 - 0.215 * AzimuthalRoughness + 2.532 * pow(AzimuthalRoughness,2.0) - 10.73 * pow(AzimuthalRoughness,3.0) +
                        5.574 * pow(AzimuthalRoughness,4.0) + 0.245 * pow(AzimuthalRoughness, 5.0) );
            color sigma = color( helper(HairColor[0],inv) , helper(HairColor[1],inv) , helper(HairColor[2],inv) );
            Result = hair( sigma , LongtitudinalRoughness , AzimuthalRoughness , IndexofRefraction );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'HairColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Longtitudinal Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Azimuthal Roughness' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Index of Refraction' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Longtitudinal Roughness'].default_value = 0.2
        self.inputs['Azimuthal Roughness'].default_value = 0.2
        self.inputs['Index of Refraction'].default_value = 1.55
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        fs.serialize( self.inputs['HairColor'].export_osl_value() )
        fs.serialize( self.inputs['Longtitudinal Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Azimuthal Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Index of Refraction'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_Blend(SORTShadingNode):
    bl_label = 'Blend'
    bl_idname = 'SORTNode_Material_Blend'
    osl_shader = '''
        shader MaterialBlend(  closure color Surface0 = @ ,
                               closure color Surface1 = @ ,
                               float Factor = @ ,
                               output closure color Result = color(0) ){
            Result = Surface0 * ( 1.0 - Factor ) + Surface1 * Factor;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface0' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Surface0'].export_osl_value() )
        fs.serialize( self.inputs['Surface1'].export_osl_value() )
        fs.serialize( self.inputs['Factor'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_Material_DoubleSided(SORTShadingNode):
    bl_label = 'Double-Sided'
    bl_idname = 'SORTNode_Material_DoubleSided'
    osl_shader = '''
        shader MaterialBlend(  closure color FrontSurface = @ ,
                               closure color BackSurface = @ ,
                               output closure color Result = color(0) ){
            Result = doubleSided( FrontSurface , BackSurface );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Front Surface' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Back Surface' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Front Surface'].export_osl_value() )
        fs.serialize( self.inputs['Back Surface'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_BXDF_Coat(SORTShadingNode):
    bl_label = 'Coat'
    bl_idname = 'SORTNode_BXDF_Coat'
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        float helper( float x , float inv ){
            float y = log(x) * inv;
            return y * y;
        }
        shader Coat( lfloat    IndexofRefraction = @ ,
                     float     Roughness = @ ,
                     color     ColorTint = @ ,
                     closure color Surface = @ ,
                     normal Normal = @ ,
                     output closure color Result = color(0) ){
            float inv = 1.0 / ( 5.969 - 0.215 * Roughness + 2.532 * pow(Roughness,2.0) - 10.73 * pow(Roughness,3.0) + 5.574 * pow(Roughness,4.0) + 0.245 * pow(Roughness, 5.0) );
            color sigma = color( helper(ColorTint[0],inv) , helper(ColorTint[1],inv) , helper(ColorTint[2],inv) );
            Result = coat( Surface , Roughness , IndexofRefraction , sigma , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Index of Refration' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketColor' , 'ColorTint' )
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Index of Refration'].default_value = 1.55
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['Index of Refration'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['ColorTint'].export_osl_value() )
        fs.serialize( self.inputs['Surface'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTPatternGraph.register_node('Materials')
class SORTNode_BXDF_MicrofacetReflection(SORTShadingNode):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_BXDF_MicrofacetReflection'
    osl_shader = '''
        shader MicrofacetRelection(  string MicroFacetDistribution = @ ,
                                     vector InteriorIOR = @ ,
                                     vector AbsorptionCoefficient = @ ,
                                     float  RoughnessU = @ ,
                                     float  RoughnessV = @ ,
                                     color  BaseColor = @ ,
                                     normal Normal = @ ,
                                     output closure color Result = color(0) ){
            Result = microfacetReflection( MicroFacetDistribution , InteriorIOR , AbsorptionCoefficient , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    distribution = bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior = bpy.props.FloatVectorProperty( name='Interior IOR' , default=(1.55,1.55,1.55) , min=1.0, max=10.0 )
    absopt_co = bpy.props.FloatVectorProperty( name='Absoprtion Coefficient' , default=(2.8,2.8,2.8) , min=0.0, max=10.0 )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'distribution', text='Distribution')
        layout.prop(self, 'interior_ior', text='Interior IOR')
        layout.prop(self, 'absopt_co', text='Absorption Coefficient')
    def serialize_prop(self, fs):
        fs.serialize( 7 )
        fs.serialize( '\"%s\"'%(self.distribution))
        fs.serialize( 'vector( %f,%f,%f )'%(self.interior_ior[:]))
        fs.serialize( 'vector( %f,%f,%f )'%(self.absopt_co[:]))
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

#------------------------------------------------------------------------------------#
#                                   Image Nodes                                      #
#------------------------------------------------------------------------------------#
@SORTPatternGraph.register_node('Image')
class SORTNodeCheckerBoard(SORTShadingNode):
    bl_label = 'CheckerBoard'
    bl_idname = 'SORTNodeCheckerBoard'
    osl_shader = '''
        shader CheckerBoard( color Color1 = @ ,
                             color Color2 = @ ,
                             vector UVCoordinate = @ ,
                             output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            float fu = UVCoordinate[0] - floor( UVCoordinate[0] );
            float fv = UVCoordinate[1] - floor( UVCoordinate[1] );
            if( ( fu > 0.5 && fv > 0.5 ) || ( fu < 0.5 && fv < 0.5 ) )
                Result = Color1;
            else
                Result = Color2;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Color1'].export_osl_value() )
        fs.serialize( self.inputs['Color2'].export_osl_value() )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )

@SORTPatternGraph.register_node('Image')
class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'
    osl_shader = '''
        shader Grid( color Color1 = @ [[ color default_value = color(0.2) ]],
                     color Color2 = @ ,
                     float Treshold = @ [[ float default_value = 0.1 ]] ,
                     uv UVCoordinate = @ ,
                     output color Result = color( 0.0 , 0.0 , 0.0 ) ){
            float fu = UVCoordinate[0] - floor( UVCoordinate[0] ) - 0.5;
            float fv = UVCoordinate[1] - floor( UVCoordinate[1] ) - 0.5;
            float half_threshold = ( 1.0 - Treshold ) * 0.5;
            if( fu <= half_threshold && fu >= -half_threshold && fv <= half_threshold && fv >= -half_threshold )
                Result = Color1;
            else
                Result = Color2;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Treshold' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.inputs['Treshold'].default_value = 0.1
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        fs.serialize( self.inputs['Color1'].export_osl_value() )
        fs.serialize( self.inputs['Color2'].export_osl_value() )
        fs.serialize( self.inputs['Treshold'].export_osl_value() )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )

#------------------------------------------------------------------------------------#
#                                 Convertor Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTPatternGraph.register_node('Convertor')
class SORTNodeRemappingUV(SORTShadingNode):
    bl_label = 'RemappingUV'
    bl_idname = 'SORTNodeRemappingUV'
    output_type = 'SORTNodeSocketFloat'
    osl_shader = '''
        shader Extract( uv     UVCoordinate = @,
                        lfloat  TilingU = @ [[ lfloat default_value = 1.0 ]] ,
                        lfloat  TilingV = @ [[ lfloat default_value = 1.0 ]] ,
                        lfloat  OffsetU = @ ,
                        lfloat  OffsetV = @ ,
                        output uv Result = uv( 0.0 ) ){
            Result = uv( UVCoordinate[0] * TilingV + OffsetU , UVCoordinate[1] * TilingU + OffsetV , 0.0 );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coorindate' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'TilingU' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'TilingV' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'OffsetU' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'OffsetV' )
        self.outputs.new( 'SORTNodeSocketUV' , 'Result' )
        self.inputs['TilingU'].default_value = 1.0
        self.inputs['TilingV'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['UV Coorindate'].export_osl_value() )
        fs.serialize( self.inputs['TilingU'].export_osl_value() )
        fs.serialize( self.inputs['TilingV'].export_osl_value() )
        fs.serialize( self.inputs['OffsetU'].export_osl_value() )
        fs.serialize( self.inputs['OffsetV'].export_osl_value() )

@SORTPatternGraph.register_node('Convertor')
class SORTNodeExtract(SORTShadingNode):
    bl_label = 'Extract'
    bl_idname = 'SORTNodeExtract'
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
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Intensity' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Color'].export_osl_value() )


@SORTPatternGraph.register_node('Constant')
class SORTNodeColor(SORTShadingNode):
    bl_idname = 'SORTNodeColor'
    bl_label = 'Color'
    osl_shader = '''
        shader Extract( color Color = @,
                        output color Result = color(0)){
            Result = Color;
        }
    '''
    color = bpy.props.FloatVectorProperty(name='Color', subtype='COLOR', min=0.0, max=1.0, size=3, default=(1.0, 1.0, 1.0))
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.template_color_picker(self, 'color', value_slider=True)
        layout.prop(self, 'color', text='')
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        print(self.color)
        fs.serialize( 'color( %f,%f,%f )'%(self.color[:]) )
