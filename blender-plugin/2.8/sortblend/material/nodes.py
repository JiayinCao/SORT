#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
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
import bpy.utils.previews
from . import group
from .. import base
from .group import SORTPatternNodeCategory, SORTShaderNodeTree, SORTShadingNode

#------------------------------------------------------------------------------------#
#                              Shader Input/Output Nodes                             #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Shader Input Output')
class SORTNodeOutput(SORTShadingNode):
    bl_label = 'Shader Output'
    bl_idname = 'SORTNodeOutput'
    osl_shader = '''
        shader SORT_Shader( closure color Surface = color(0) ){
            Ci = Surface;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketBxdf' , 'Surface' )

@SORTShaderNodeTree.register_node('Shader Input Output')
class SORTNodeExposedInputs(SORTShadingNode):
    bl_label = 'Shader Inputs'
    bl_idname = 'SORTNodeExposedInputs'
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Input' )

#------------------------------------------------------------------------------------#
#                                   BXDF Nodes                                       #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Diffuse(SORTShadingNode):
    bl_label = 'Diffuse'
    bl_idname = 'SORTNode_Material_Diffuse'
    osl_shader_diffuse = '''
        shader Lambert( color Diffuse = @ ,
                        normal Normal = @ ,
                        output closure color Result = color(0) ){
            Result = lambert( Diffuse , Normal );
        }
    '''
    osl_shader_orennayar = '''
        shader OrenNayar( float Roughness = @,
                          color Diffuse = @ ,
                          normal Normal = @ ,
                          output closure color Result = color(0) ){
            Result = orenNayar( Diffuse , Roughness , Normal );
        }
    '''
    def update_brdf(self,context):
        if self.brdf_type == 'OrenNayar':
            self.inputs['Roughness'].enabled = True
        else:
            self.inputs['Roughness'].enabled = False
    brdf_type : bpy.props.EnumProperty(name='Type', items=[('Lambert','Lambert','',1),('OrenNayar','OrenNayar','',2)], default='Lambert', update=update_brdf)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Roughness'].enabled = False
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)
    def serialize_prop(self, fs):
        if self.brdf_type == 'OrenNayar':
            fs.serialize( 3 )
            fs.serialize( self.inputs['Roughness'].export_osl_value() )
            fs.serialize( self.inputs['Diffuse'].export_osl_value() )
            fs.serialize( self.inputs['Normal'].export_osl_value() )
        else:
            fs.serialize( 2 )
            fs.serialize( self.inputs['Diffuse'].export_osl_value() )
            fs.serialize( self.inputs['Normal'].export_osl_value() )
    def generate_osl_source(self):
        if self.brdf_type == 'Lambert':
            return self.osl_shader_diffuse
        return self.osl_shader_orennayar
    def type_identifier(self):
        return self.bl_label + self.brdf_type

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_LambertTransmission(SORTShadingNode):
    bl_label = 'Lambert Transmission'
    bl_idname = 'SORTNode_Material_LambertTransmission'
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

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Mirror(SORTShadingNode):
    bl_label = 'Mirror'
    bl_idname = 'SORTNode_Material_Mirror'
    osl_shader = '''
        shader Mirror( color BaseColor = @ ,
                       normal Normal = @ ,
                       output closure color Result = color(0) ){
            Result = mirror( BaseColor , Normal );
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

@SORTShaderNodeTree.register_node('Materials')
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

@SORTShaderNodeTree.register_node('Materials')
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

@SORTShaderNodeTree.register_node('Materials')
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

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_DisneyBRDF(SORTShadingNode):
    bl_label = 'Disney BRDF'
    bl_idname = 'SORTNode_Material_DisneyBRDF'
    osl_shader = '''
        shader Disney( float Metallic = @ ,
                       float Specular = @ ,
                       float SpecularTint = @ ,
                       float Roughness = @ ,
                       float Anisotropic = @ ,
                       float Sheen = @ ,
                       float SheenTint = @ ,
                       float Clearcoat = @ ,
                       float ClearcoatGlossiness = @ ,
                       float SpecularTransmittance = @ ,
                       float ScatterDistance = @ ,
                       float Flatness = @ ,
                       float DiffuseTransmittance = @ ,
                       int   IsThinSurface = @ ,
                       color BaseColor = @ ,
                       normal Normal = @ ,
                       output closure color Result = color(0) ){
            Result = disney( Metallic , Specular , SpecularTint , Roughness , Anisotropic , Sheen , SheenTint , Clearcoat , ClearcoatGlossiness , 
                             SpecularTransmittance , ScatterDistance , Flatness , DiffuseTransmittance , IsThinSurface , BaseColor , Normal );
        }
    '''
    bl_width_min = 200
    is_thin_surface : bpy.props.BoolProperty(name='Is Thin Surface', default=False)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Metallic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Anisotropic' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular Tint' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Sheen' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Sheen Tint' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Clearcoat' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Clearcoat Glossiness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular Transmittance')
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Scatter Distance')
        self.inputs.new( 'SORTNodeSocketFloat' , 'Flatness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Diffuse Transmittance' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Metallic'].default_value = 1.0
        self.inputs['Roughness'].default_value = 0.2
        self.inputs['Specular'].default_value = 1.0
        self.inputs['Clearcoat Glossiness'].default_value = 1.0
        self.inputs['Sheen'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 16 )
        fs.serialize( self.inputs['Metallic'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['Specular Tint'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        fs.serialize( self.inputs['Anisotropic'].export_osl_value() )
        fs.serialize( self.inputs['Sheen'].export_osl_value() )
        fs.serialize( self.inputs['Sheen Tint'].export_osl_value() )
        fs.serialize( self.inputs['Clearcoat'].export_osl_value() )
        fs.serialize( self.inputs['Clearcoat Glossiness'].export_osl_value() )
        fs.serialize( self.inputs['Specular Transmittance'].export_osl_value() )
        fs.serialize( self.inputs['Scatter Distance'].export_osl_value() )
        fs.serialize( self.inputs['Flatness'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse Transmittance'].export_osl_value() )
        fs.serialize( '1' if self.is_thin_surface else '0' )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'is_thin_surface', text='Is Thin Surface')

@SORTShaderNodeTree.register_node('Materials')
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

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Coat(SORTShadingNode):
    bl_label = 'Coat'
    bl_idname = 'SORTNode_Material_Coat'
    # A Practical and Controllable Hair and Fur Model for Production Path Tracing
    # https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/147/asset/siggraph2015Fur.pdf
    osl_shader = '''
        float helper( float x , float inv ){
            float y = log(x) * inv;
            return y * y;
        }
        shader Coat( float     IndexofRefraction = @ ,
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

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Measured(SORTShadingNode):
    bl_label = 'Measured BRDF'
    bl_idname = 'SORTNode_Material_Measured'
    osl_shader_merl = '''
        shader merlBRDF( string Filename = @,
                         normal Normal = @ ,
                         output closure color Result = color(0) ){
            Result = merlBRDF( @ , Normal );
        }
    '''
    osl_shader_fourier = '''
        shader FourierBRDF( string Filename = @,
                            normal Normal = @ ,
                            output closure color Result = color(0) ){
            Result = fourierBRDF( @ , Normal );
        }
    '''
    osl_shader = osl_shader_fourier
    brdf_type : bpy.props.EnumProperty(name='Type', items=[('FourierBRDF','FourierBRDF','',1),('MERL','MERL','',2)], default='FourierBRDF')
    file_path : bpy.props.StringProperty( name='FilePath' , subtype='FILE_PATH' )
    ResourceIndex : bpy.props.IntProperty( name='ResourceId' )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.ResourceIndex = 0
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)
        layout.prop(self, 'file_path', text='File Path')
    def generate_osl_source(self):
        if self.brdf_type == 'FourierBRDF':
            return self.osl_shader_fourier
        return self.osl_shader_merl
    def populateResources( self , resources ):
        found = False
        for resource in resources:
            if resource[1] == self.file_path:
                found = True
        if not found:
            self.ResourceIndex = len(resources)
            if self.brdf_type == 'FourierBRDF':
                resources.append( ( self.file_path , 'FourierBRDFMeasuredData' ) )
            else:
                resources.append( ( self.file_path , 'MerlBRDFMeasuredData' ) )
        pass
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( '\"%s\"'%self.file_path )
        fs.serialize( self.inputs['Normal'].export_osl_value() )
        fs.serialize( '%i'%self.ResourceIndex )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_MicrofacetReflection(SORTShadingNode):
    bl_label = 'MicrofacetRelection'
    bl_idname = 'SORTNode_Material_MicrofacetReflection'
    bl_width_min = 256
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
    distribution : bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior : bpy.props.FloatVectorProperty( name='Interior IOR' , default=(0.37,0.37,0.37) , min=1.0, max=10.0 )
    absopt_co : bpy.props.FloatVectorProperty( name='Absoprtion Coefficient' , default=(2.82,2.82,2.82) , min=0.0, max=10.0 )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'distribution', text='Distribution', expand=True)
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

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_MicrofacetRefraction(SORTShadingNode):
    bl_label = 'MicrofacetRefraction'
    bl_idname = 'SORTNode_Material_MicrofacetRefraction'
    bl_width_min = 256
    osl_shader = '''
        shader MicrofacetRefraction( string MicroFacetDistribution = @ ,
                                     float  InteriorIOR = @ ,
                                     float  ExteriorIOR = @ ,
                                     float  RoughnessU = @ ,
                                     float  RoughnessV = @ ,
                                     color  BaseColor = @ ,
                                     normal Normal = @ ,
                                     output closure color Result = color(0) ){
            Result = microfacetRefraction( MicroFacetDistribution , InteriorIOR , ExteriorIOR , RoughnessU , RoughnessV , BaseColor , Normal );
        }
    '''
    distribution : bpy.props.EnumProperty(name='MicroFacetDistribution',default='GGX',items=[('GGX','GGX','',1),('Blinn','Blinn','',2),('Beckmann','Beckmann','',3)])
    interior_ior : bpy.props.FloatProperty( name='Interior IOR' , default=1.1 , min=1.0, max=10.0 )
    exterior_ior : bpy.props.FloatProperty( name='Exterior IOR' , default=1.0 , min=0.0, max=10.0 )
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['RoughnessU'].default_value = 0.2
        self.inputs['RoughnessV'].default_value = 0.2
    def draw_buttons(self, context, layout):
        layout.prop(self, 'distribution', text='Distribution', expand=True)
        layout.prop(self, 'interior_ior', text='Interior IOR')
        layout.prop(self, 'exterior_ior', text='Exterior IOR')
    def serialize_prop(self, fs):
        fs.serialize( 7 )
        fs.serialize( '\"%s\"'%(self.distribution))
        fs.serialize( '%f'%(self.interior_ior))
        fs.serialize( '%f'%(self.exterior_ior))
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_AshikhmanShirley(SORTShadingNode):
    bl_label = 'AshikhmanShirley'
    bl_idname = 'SORTNode_Material_AshikhmanShirley'
    osl_shader = '''
        shader AshikhmanShirley( float Specular = @ ,
                                 float RoughnessU = @ ,
                                 float RoughnessV = @ ,
                                 color Diffuse = @ ,
                                 normal Normal = @ ,
                                 output closure color Result = color(0) ){
            Result = ashikhmanShirley( Specular , RoughnessU , RoughnessV , Diffuse , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessU' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'RoughnessV' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['RoughnessU'].default_value = 0.2
        self.inputs['RoughnessV'].default_value = 0.2
        self.inputs['Specular'].default_value = 0.5
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessU'].export_osl_value() )
        fs.serialize( self.inputs['RoughnessV'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_ModifiedPhong(SORTShadingNode):
    bl_label = 'Modified Phong'
    bl_idname = 'SORTNode_Material_ModifiedPhong'
    osl_shader = '''
        shader Phong( float SpecularPower = @ ,
                      float DiffuseRatio = @ ,
                      color Specular = @ ,
                      color Diffuse = @ ,
                      normal Normal = @ ,
                      output closure color Result = color(0) ){
            Result = phong( Diffuse * DiffuseRatio , ( 1.0 - DiffuseRatio ) * Specular , SpecularPower , Normal );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Specular Power' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Diffuse Ratio' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Diffuse' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
        self.inputs['Specular Power'].default_value = 32.0
        self.inputs['Diffuse Ratio'].default_value = 0.2
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['Specular Power'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse Ratio'].export_osl_value() )
        fs.serialize( self.inputs['Diffuse'].export_osl_value() )
        fs.serialize( self.inputs['Specular'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )

@SORTShaderNodeTree.register_node('Materials')
class SORTNode_Material_Cloth(SORTShadingNode):
    bl_label = 'Cloth'
    bl_idname = 'SORTNode_Material_Cloth'
    osl_shader_dbrdf = '''
        shader DistributionBRDF(  color BaseColor = @ ,
                                  float Roughness = @ ,
                                  float Specular = @ ,
                                  float SpecularTint = @ ,
                                  normal Normal = @ ,
                                  output closure color Result = color(0) ){
            Result = distributionBRDF( BaseColor , Roughness , Specular , SpecularTint , Normal );
        }
    '''
    osl_shader_dwa_fabric = '''
        shader DWA_Fabric( color BaseColor = @ ,
                           float Roughness = @ ,
                           normal Normal = @ ,
                           output closure color Result = color(0) ){
            Result = fabric( BaseColor , Roughness , Normal );
        }
    '''
    def update_brdf(self,context):
        if self.brdf_type == 'TheOrder_Fabric':
            self.inputs['Specular'].enabled = True
            self.inputs['SpecularTint'].enabled = True
        else:
            self.inputs['Specular'].enabled = False
            self.inputs['SpecularTint'].enabled = False
    brdf_type : bpy.props.EnumProperty(name='Type', items=[('TheOrder_Fabric','TheOrder_Fabric','',1),('DreamWorks_Fabric','DreamWorks_Fabric','',2)], default='TheOrder_Fabric', update=update_brdf)
    def generate_osl_source(self):
        if self.brdf_type == 'TheOrder_Fabric':
            return self.osl_shader_dbrdf
        return self.osl_shader_dwa_fabric
    def type_identifier(self):
        return self.bl_label + self.brdf_type
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'BaseColor' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Roughness' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Specular' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'SpecularTint' )
        self.inputs.new( 'SORTNodeSocketNormal' , 'Normal' )
        self.outputs.new( 'SORTNodeSocketBxdf' , 'Result' )
    def serialize_prop(self, fs):
        if self.brdf_type == 'TheOrder_Fabric':
            fs.serialize( 5 )
        else:
            fs.serialize( 3 )
        fs.serialize( self.inputs['BaseColor'].export_osl_value() )
        fs.serialize( self.inputs['Roughness'].export_osl_value() )
        if self.brdf_type == 'TheOrder_Fabric':
            fs.serialize( self.inputs['Specular'].export_osl_value() )
            fs.serialize( self.inputs['SpecularTint'].export_osl_value() )
        fs.serialize( self.inputs['Normal'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'brdf_type', text='BRDF Type', expand=True)

@SORTShaderNodeTree.register_node('Materials')
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

@SORTShaderNodeTree.register_node('Materials')
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

#------------------------------------------------------------------------------------#
#                                   Texture Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Textures')
class SORTNodeCheckerBoard(SORTShadingNode):
    bl_label = 'CheckerBoard'
    bl_idname = 'SORTNodeCheckerBoard'
    osl_shader = '''
        shader CheckerBoard( color Color1 = @ ,
                             color Color2 = @ ,
                             vector UVCoordinate = @ ,
                             output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                             output float Red = 0.0 ,
                             output float Green = 0.0 ,
                             output float Blue = 0.0 ){
            float fu = UVCoordinate[0] - floor( UVCoordinate[0] );
            float fv = UVCoordinate[1] - floor( UVCoordinate[1] );
            if( ( fu > 0.5 && fv > 0.5 ) || ( fu < 0.5 && fv < 0.5 ) )
                Result = Color1;
            else
                Result = Color2;
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels : bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Color1'].export_osl_value() )
        fs.serialize( self.inputs['Color2'].export_osl_value() )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, "show_separate_channels")

@SORTShaderNodeTree.register_node('Textures')
class SORTNodeGrid(SORTShadingNode):
    bl_label = 'Grid'
    bl_idname = 'SORTNodeGrid'
    osl_shader = '''
        shader Grid( color Color1 = @ ,
                     color Color2 = @ ,
                     float Treshold = @ ,
                     vector UVCoordinate = @ ,
                     output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                     output float Red = 0.0 ,
                     output float Green = 0.0 ,
                     output float Blue = 0.0 ){
            float fu = UVCoordinate[0] - floor( UVCoordinate[0] ) - 0.5;
            float fv = UVCoordinate[1] - floor( UVCoordinate[1] ) - 0.5;
            float half_threshold = ( 1.0 - Treshold ) * 0.5;
            if( fu <= half_threshold && fu >= -half_threshold && fv <= half_threshold && fv >= -half_threshold )
                Result = Color1;
            else
                Result = Color2;
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels : bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketColor' , 'Color1' )
        self.inputs.new( 'SORTNodeSocketColor' , 'Color2' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Treshold' )
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
        self.inputs['Treshold'].default_value = 0.1
        self.inputs['Color1'].default_value = ( 0.2 , 0.2 , 0.2 )
    def serialize_prop(self, fs):
        fs.serialize( 4 )
        fs.serialize( self.inputs['Color1'].export_osl_value() )
        fs.serialize( self.inputs['Color2'].export_osl_value() )
        fs.serialize( self.inputs['Treshold'].export_osl_value() )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
    def draw_buttons(self, context, layout):
        layout.prop(self, "show_separate_channels")

preview_collections = {}
@SORTShaderNodeTree.register_node('Textures')
class SORTNodeImage(SORTShadingNode):
    bl_label = 'Image'
    bl_idname = 'SORTNodeImage'
    bl_width_min = 200
    items = (('Linear', "Linear", "Linear"), ('sRGB', "sRGB", "sRGB"), ('Normal', 'Normal', 'Normal'))
    color_space_type : bpy.props.EnumProperty(name='Color Space', items=items, default='Linear')
    wrap_items = (('REPEAT', "Repeat", "Repeating Texture"),
             ('MIRRORED_REPEAT', "Mirror", "Texture mirrors outside of 0-1"),
             ('CLAMP_TO_EDGE', "Clamp to Edge", "Clamp to Edge.  Outside 0-1 the texture will smear."),
             ('CLAMP_ZERO', "Clamp to Black", "Clamp to Black outside 0-1"),
             ('CLAMP_ONE', "Clamp to White", "Clamp to White outside 0-1"),)
    wrap_type : bpy.props.EnumProperty(name='Wrap Type', items=wrap_items, default='REPEAT')
    image_preview : bpy.props.BoolProperty(name='Preview Image', default=True)
    osl_shader_linear = '''
        shader ImageShaderLinear( string Filename = @ ,
                                  vector UVCoordinate = @ ,
                                  output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                                  output float Red = 0.0 ,
                                  output float Green = 0.0 ,
                                  output float Blue = 0.0 ){
            Result = texture( Filename , UVCoordinate[0] , UVCoordinate[1] );
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    osl_shader_gamma = '''
        shader ImageShaderGamma( string Filename = @ ,
                                 vector UVCoordinate = @ ,
                                 output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                                 output float Red = 0.0 ,
                                 output float Green = 0.0 ,
                                 output float Blue = 0.0 ){
            color gamma_color = texture( Filename , UVCoordinate[0] , UVCoordinate[1] );
            Result = pow( gamma_color , 2.2 );
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    osl_shader_normal = '''
        shader ImageShaderNormal( string Filename = @ ,
                                 vector UVCoordinate = @ ,
                                 output color Result = color( 0.0 , 0.0 , 0.0 ) ,
                                 output float Red = 0.0 ,
                                 output float Green = 0.0 ,
                                 output float Blue = 0.0 ){
            color encoded_color = texture( Filename , UVCoordinate[0] , UVCoordinate[1] );
            Result = 2.0 * color( encoded_color[0] , encoded_color[2] , encoded_color[1] ) - 1.0;
            Red = Result[0];
            Green = Result[1];
            Blue = Result[2];
        }
    '''
    def toggle_result_channel(self,context):
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    show_separate_channels : bpy.props.BoolProperty(name='All Channels', default=False, update=toggle_result_channel)
    def generate_preview(self, context):
        name = self.name + '_' + self.id_data.name
        if name not in preview_collections:
            item = bpy.utils.previews.new()
            item.previews = ()
            item.image_name = ''
            preview_collections[name] = item
        item = preview_collections[name]
        wm = context.window_manager
        enum_items = []
        img = self.image
        if img:
            new_image_name =img.name
            if item.image_name == new_image_name:
                return item.previews
            else:
                item.image_name = new_image_name
            item.clear()
            thumb = item.load(img.name, bpy.path.abspath(img.filepath), 'IMAGE')
            enum_items = [(img.filepath, img.name, '', thumb.icon_id, 0)]
        item.previews = enum_items
        return item.previews
    image : bpy.props.PointerProperty(type=bpy.types.Image)
    preview : bpy.props.EnumProperty(items=generate_preview)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.outputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs['Red'].enabled = self.show_separate_channels
        self.outputs['Blue'].enabled = self.show_separate_channels
        self.outputs['Green'].enabled = self.show_separate_channels
    def draw_buttons(self, context, layout):
        layout.template_ID(self, "image", open="image.open")
        layout.prop(self, 'image_preview' )
        if self.image_preview:
            layout.template_icon_view(self, 'preview', show_labels=True)
        layout.prop(self, 'show_separate_channels' )
        layout.prop(self, 'color_space_type', expand=True)
        layout.prop(self, 'wrap_type')
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( '\"%s\"'%(bpy.path.abspath(self.image.filepath)) )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
    def generate_osl_source(self):
        if self.color_space_type == 'sRGB':
            return self.osl_shader_gamma
        elif self.color_space_type == 'Normal':
            return self.osl_shader_normal
        return self.osl_shader_linear
    def type_identifier(self):
        return self.bl_label + self.color_space_type

#------------------------------------------------------------------------------------#
#                                 Convertor Nodes                                    #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeRemappingUV(SORTShadingNode):
    bl_label = 'RemappingUV'
    bl_idname = 'SORTNodeRemappingUV'
    output_type = 'SORTNodeSocketFloat'
    osl_shader = '''
        shader Extract( vector UVCoordinate = @,
                        float  TilingU = @ ,
                        float  TilingV = @ ,
                        float  OffsetU = @ ,
                        float  OffsetV = @ ,
                        output vector Result = vector( 0.0 ) ){
            Result = vector( UVCoordinate[0] * TilingV + OffsetU , UVCoordinate[1] * TilingU + OffsetV , 0.0 );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'TilingU' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'TilingV' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'OffsetU' )
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'OffsetV' )
        self.outputs.new( 'SORTNodeSocketUV' , 'Result' )
        self.inputs['TilingU'].default_value = 1.0
        self.inputs['TilingV'].default_value = 1.0
    def serialize_prop(self, fs):
        fs.serialize( 5 )
        fs.serialize( self.inputs['UV Coordinate'].export_osl_value() )
        fs.serialize( self.inputs['TilingU'].export_osl_value() )
        fs.serialize( self.inputs['TilingV'].export_osl_value() )
        fs.serialize( self.inputs['OffsetU'].export_osl_value() )
        fs.serialize( self.inputs['OffsetV'].export_osl_value() )

@SORTShaderNodeTree.register_node('Convertor')
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

@SORTShaderNodeTree.register_node('Convertor')
class SORTNodeComposite(SORTShadingNode):
    bl_label = 'Composite'
    bl_idname = 'SORTNodeComposite'
    osl_shader = '''
        shader Composite( float Red = @ ,
                          float Green = @ ,
                          float Blue = @ ,
                          output color Color = color( 0.0 , 0.0 , 0.0 ) ){
            Color = color( Red , Green , Blue );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloat' , 'Red' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Green' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Blue' )
        self.outputs.new( 'SORTNodeSocketColor' , 'Color' )
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Red'].export_osl_value() )
        fs.serialize( self.inputs['Green'].export_osl_value() )
        fs.serialize( self.inputs['Blue'].export_osl_value() )

#------------------------------------------------------------------------------------#
#                                 Input Nodes                                        #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputIntersection(SORTShadingNode):
    bl_label = 'Intersection'
    bl_idname = 'SORTNodeInputIntersection'
    bl_width_min = 160
    osl_shader = '''
        shader InputShader( output vector WorldPosition = P ,
                            output vector WorldViewDirection = I ,
                            output vector WorldShadingNormal = N ,
                            output vector WorldGeometryNormal = Ng ,
                            output vector UVCoordinate = vector( u , v , 0.0 ) ){
        }
    '''
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'World Position' )
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'World View Direction' )
        self.outputs.new( 'SORTNodeSocketNormal' , 'World Shading Normal' )
        self.outputs.new( 'SORTNodeSocketNormal' , 'World Geometry Normal' )
        self.outputs.new( 'SORTNodeSocketUV' , 'UV Coordinate' )

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputFloat(SORTShadingNode):
    bl_label = 'Float'
    bl_idname = 'SORTNodeInputFloat'
    osl_shader = '''
        shader ConstantFloat( float Value = @ ,
                              output float Result = 0.0 ){
            Result = Value;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketLargeFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketLargeFloat' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Value'].export_osl_value() )

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputFloatVector(SORTShadingNode):
    bl_label = 'Vector'
    bl_idname = 'SORTNodeInputFloatVector'
    bl_width_min = 256
    osl_shader = '''
        shader ConstantFloat( vector Value = @ ,
                              output vector Result = 0.0 ){
            Result = Value;
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value' )
        self.outputs.new( 'SORTNodeSocketFloatVector' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Value'].export_osl_value() )

@SORTShaderNodeTree.register_node('Input')
class SORTNodeInputColor(SORTShadingNode):
    bl_label = 'Color'
    bl_idname = 'SORTNodeInputColor'
    osl_shader = '''
        shader Extract( color Color = @,
                        output color Result = color(0)){
            Result = Color;
        }
    '''
    color : bpy.props.FloatVectorProperty(name='Color', subtype='COLOR', min=0.0, max=1.0, size=3, default=(1.0, 1.0, 1.0))
    def init(self, context):
        self.outputs.new( 'SORTNodeSocketColor' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.template_color_picker(self, 'color', value_slider=True)
        layout.prop(self, 'color', text='')
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( 'color( %f,%f,%f )'%(self.color[:]) )

#------------------------------------------------------------------------------------#
#                                 Math Op Nodes                                      #
#------------------------------------------------------------------------------------#
@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpUnary(SORTShadingNode):
    bl_label = 'Unary Operator'
    bl_idname = 'SORTNodeMathOpUnary'
    bl_width_min = 240
    osl_shader = '''
        shader MathUnaryOp( %s Value = @ ,
                            output %s Result = 0.0 ){
            Result = %s(Value);
        }
    '''
    def change_type(self,context):
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Value' )
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type == 'SORTNodeSocketFloatVector':
            self.inputs['Value'].default_value = ( 0.0 , 0.0 , 0.0 )
        elif self.data_type == 'SORTNodeSocketColor':
            self.inputs['Value'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value'].default_value = 1.0
    op_type : bpy.props.EnumProperty(name='Type',default='-',items=[
        ('-','Negation','',1), ('1.0-','One Minus','',2), ('sin','Sin','',3), ('cos','Cos','',4), ('tan','Tan','',5), ('asin','Asin','',6), ('acos','Acos','',7), ('atan','Atan','',8),
        ('exp','Exp','',9), ('exp2','Exp2','',10), ('log','Log','',11), ('log2','Log2','',12), ('log10','Log10','',13), ('sqrt','Sqrt','',14), ('inversesqrt','Inverse Sqrt','',15),
        ('fabs','Abs','', 16), ('sign','Sign','',17), ('floor','Floor','',18), ('ceil','Ceil','',19), ('round','Round','',20), ('trunc','Trunc','',21) ])
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'op_type', text='Type')
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 1 )
        fs.serialize( self.inputs['Value'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , self.op_type )
    def type_identifier(self):
        return self.bl_label + self.data_type + self.op_type
        
@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpBinary(SORTShadingNode):
    bl_label = 'Binary Operator'
    bl_idname = 'SORTNodeMathOpBinary'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( %s Value0 = @ ,
                             %s Value1 = @ ,
                             output %s Result = 0.0 ){
            Result = Value0 %s Value1;
        }
    '''
    def change_type(self,context):
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Value0' )
        self.inputs.new( self.data_type , 'Value1' )
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Value0'].default_value = ( 1.0 , 1.0 , 1.0 )
            self.inputs['Value1'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value0'].default_value = 1.0
            self.inputs['Value1'].default_value = 1.0
    op_type : bpy.props.EnumProperty(name='Type',default='+',items=[('+','Add','',1),('-','Substract','',2),('*','Multiply','',3),('/','Divide','',4)])
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value0' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value1' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'op_type', text='Type')
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Value0'].export_osl_value() )
        fs.serialize( self.inputs['Value1'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , dtype , self.op_type )
    def type_identifier(self):
        return self.bl_label + self.data_type + self.op_type

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpDotProduce(SORTShadingNode):
    bl_label = 'Dot Product'
    bl_idname = 'SORTNodeMathOpDotProduce'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( vector Value0 = @ ,
                             vector Value1 = @ ,
                             output vector Result = 0.0 ){
            Result = dot( Value0 , Value1 );
        }
    '''
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value0' )
        self.inputs.new( 'SORTNodeSocketFloatVector' , 'Value1' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def serialize_prop(self, fs):
        fs.serialize( 2 )
        fs.serialize( self.inputs['Value0'].export_osl_value() )
        fs.serialize( self.inputs['Value1'].export_osl_value() )
    
@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpLerp(SORTShadingNode):
    bl_label = 'Lerp'
    bl_idname = 'SORTNodeMathOpLerp'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( %s Value0 = @ ,
                             %s Value1 = @ ,
                             float Factor = @ ,
                             output %s Result = 0.0 ){
            Result = Value0 * ( 1.0 - Factor ) + Value1 * Factor;
        }
    '''
    def change_type(self,context):
        old_factor = self.inputs['Factor'].default_value
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Value0' )
        self.inputs.new( self.data_type , 'Value1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.inputs['Factor'].default_value = old_factor
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Value0'].default_value = ( 1.0 , 1.0 , 1.0 )
            self.inputs['Value1'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Value0'].default_value = 1.0
            self.inputs['Value1'].default_value = 1.0
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value0' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value1' )
        self.inputs.new( 'SORTNodeSocketFloat' , 'Factor' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
    def draw_buttons(self, context, layout):
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Value0'].export_osl_value() )
        fs.serialize( self.inputs['Value1'].export_osl_value() )
        fs.serialize( self.inputs['Factor'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , dtype )
    def type_identifier(self):
        return self.bl_label + self.data_type

@SORTShaderNodeTree.register_node('Math Ops')
class SORTNodeMathOpClamp(SORTShadingNode):
    bl_label = 'Clamp'
    bl_idname = 'SORTNodeMathOpClamp'
    bl_width_min = 240
    osl_shader = '''
        shader MathBinaryOp( %s MinValue = @ ,
                             %s MaxValue = @ ,
                             %s Value = @ ,
                             output %s Result = 0.0 ){
            Result = min( MaxValue , max( MinValue , Value ) );
        }
    '''
    def change_type(self,context):
        self.inputs.clear()
        self.outputs.clear()
        self.inputs.new( self.data_type , 'Min Value' )
        self.inputs.new( self.data_type , 'Max Value' )
        self.inputs.new( self.data_type , 'Value' )
        self.outputs.new( self.data_type , 'Result' )
        if self.data_type != 'SORTNodeSocketAnyFloat':
            self.inputs['Min Value'].default_value = ( 0.0 , 0.0 , 0.0 )
            self.inputs['Max Value'].default_value = ( 1.0 , 1.0 , 1.0 )
        else:
            self.inputs['Min Value'].default_value = 0.0
            self.inputs['Max Value'].default_value = 1.0
    data_type : bpy.props.EnumProperty(name='Type',default='SORTNodeSocketAnyFloat',items=[('SORTNodeSocketAnyFloat','Float','',1),('SORTNodeSocketColor','Color','',2),('SORTNodeSocketFloatVector','Vector','',3)],update=change_type)
    def init(self, context):
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Min Value' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Max Value' )
        self.inputs.new( 'SORTNodeSocketAnyFloat' , 'Value' )
        self.outputs.new( 'SORTNodeSocketAnyFloat' , 'Result' )
        self.inputs['Max Value'].default_value = 1.0
    def draw_buttons(self, context, layout):
        layout.prop(self, 'data_type', text='Type', expand=True)
    def serialize_prop(self, fs):
        fs.serialize( 3 )
        fs.serialize( self.inputs['Min Value'].export_osl_value() )
        fs.serialize( self.inputs['Max Value'].export_osl_value() )
        fs.serialize( self.inputs['Value'].export_osl_value() )
    def generate_osl_source(self):
        dtype = 'float'
        if self.data_type == 'SORTNodeSocketColor':
            dtype = 'color'
        elif self.data_type == 'SORTNodeSocketFloatVector':
            dtype = 'vector'
        return self.osl_shader % ( dtype , dtype , dtype , dtype )
    def type_identifier(self):
        return self.bl_label + self.data_type
