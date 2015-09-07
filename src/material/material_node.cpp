/*
   FileName:      material_node.cpp

   Created Time:  2015-09-04 20:22:00

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "material_node.h"
#include "bsdf/lambert.h"
#include "bsdf/merl.h"
#include "bsdf/orennayar.h"
#include "bsdf/microfacet.h"
#include "bsdf/reflection.h"
#include "bsdf/refraction.h"
#include "bsdf/microfacetdistribution.h"
#include "bsdf/fresnel.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "thirdparty/tinyxml/tinyxml.h"
#include "geometry/intersection.h"

IMPLEMENT_CREATOR( LambertNode );
IMPLEMENT_CREATOR( MerlNode );
IMPLEMENT_CREATOR( OrenNayarNode );
IMPLEMENT_CREATOR( MicrofacetNode );
IMPLEMENT_CREATOR( ReflectionNode );
IMPLEMENT_CREATOR( RefractionNode );
IMPLEMENT_CREATOR( AddNode );
IMPLEMENT_CREATOR( LerpNode );
IMPLEMENT_CREATOR( BlendNode );
IMPLEMENT_CREATOR( GridTexNode );
IMPLEMENT_CREATOR( CheckBoxTexNode );
IMPLEMENT_CREATOR( ImageTexNode );

// get node property
MaterialNodeProperty* MaterialNode::getProperty( const string& name )
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.find( name );
	if( it != m_props.end() )
		return it->second;
	return 0;
}

// get node property value
MaterialPropertyValue MaterialNodePropertyColor::GetPropertyValue( Bsdf* bsdf )
{
	if( node )
		return node->GetNodeValue( bsdf );
	return MaterialPropertyValue(value);
}

// get node property value
MaterialPropertyValue MaterialNodePropertyFloat2::GetPropertyValue( Bsdf* bsdf )
{
	if( node )
		return node->GetNodeValue( bsdf );
	return Spectrum( x , y , 0.0f );
}

// get node property value
MaterialPropertyValue MaterialNodePropertyFloat::GetPropertyValue( Bsdf* bsdf )
{
	if( node )
		return node->GetNodeValue( bsdf );
	return value;
}

// set node property
void MaterialNodePropertyColor::SetNodeProperty( const string& prop )
{
	value = SpectrumFromStr( prop );
}

// set node property
void MaterialNodePropertyFloat::SetNodeProperty( const string& prop )
{
	value = (float)atof( prop.c_str() );
}

// set node property
void MaterialNodePropertyFloat2::SetNodeProperty( const string& prop )
{
	string str = prop;
	string r = NextToken( str , ' ' );
	string g = NextToken( str , ' ' );

	x = (float)atof( r.c_str() );
	y = (float)atof( g.c_str() );
}

// set node property
void MaterialNodePropertyString::SetNodeProperty( const string& prop )
{
	str = prop;
}

// parse property or socket
void MaterialNode::ParseProperty( TiXmlElement* element , MaterialNode* node )
{
	TiXmlElement* prop = element->FirstChildElement( "Property" );
	while(prop)
	{
		string prop_name = prop->Attribute( "name" );
		string prop_type = prop->Attribute( "type" );

		MaterialNodeProperty* node_prop = node->getProperty( prop_name );
		if( node_prop == 0 )
		{
			// output error log
			LOG_WARNING<<"Node property "<<prop_name<<" is ignored."<<ENDL;

			// get next property
			prop = element->NextSiblingElement( "Property" );

			// proceed to the next property
			continue;
		}

		// add socket input
		if( prop_type == "node" )
		{
			node_prop->node = ParseNode( prop , node );
		}else
		{
			string node_value = prop->Attribute( "value" );
			node_prop->SetNodeProperty( node_value );
		}
			
		// get next property
		prop = prop->NextSiblingElement( "Property" );
	}
}

// parse a new node
MaterialNode* MaterialNode::ParseNode( TiXmlElement* element , MaterialNode* node )
{
	string node_type = element->Attribute( "node" );
	// create new material node
	MaterialNode* mat_node = CREATE_TYPE( node_type , MaterialNode );
	if( mat_node == 0 )
	{
		LOG_WARNING<<"Node type "<<node_type<<" is unknown!!"<<ENDL;
		return mat_node;
	}

	// parse node properties
	ParseProperty( element , mat_node );

	return mat_node; 
}

// post process
void MaterialNode::PostProcess()
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			it->second->node->PostProcess();
		++it;
	}
}

// update bsdf
void MaterialNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			it->second->node->UpdateBSDF(bsdf , weight);
		++it;
	}
}

OutputNode::OutputNode()
{
	// register node property
	m_props.insert( make_pair( "Surface" , &output ) );
}

// update bsdf
void OutputNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( output.node )
		output.node->UpdateBSDF( bsdf );
}

LambertNode::LambertNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
}

void LambertNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	Lambert* lambert = SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf) );
	lambert->m_weight = weight;
	bsdf->AddBxdf( lambert );
}

MerlNode::MerlNode()
{
	m_props.insert( make_pair( "Filename" , &merlfile ) );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	merl.m_weight = weight;
	bsdf->AddBxdf( &merl );
}

// post process
void MerlNode::PostProcess()
{
	if( merlfile.str.empty() == false )
		merl.LoadData( merlfile.str );
}

OrenNayarNode::OrenNayarNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "Sigma" , &sigma ) );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( baseColor.value , sigma.value );
	orennayar->m_weight = weight;
	bsdf->AddBxdf( orennayar );
}

MicrofacetNode::MicrofacetNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
	m_props.insert( make_pair( "Fresnel" , &fresnel ) );
}

void MicrofacetNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	MicroFacet* mf = SORT_MALLOC(MicroFacet)( baseColor.value , pFresnel , pMFDist );
	mf->m_weight = weight;
	bsdf->AddBxdf( mf );
}

// post process
void MicrofacetNode::PostProcess()
{
	// parameters are to be exposed through GUI
	if( fresnel.str == "FresnelConductor" )
		pFresnel = new FresnelConductor( 1.0f , 1.0f );
	else if( fresnel.str == "FresnelDielectric" )
		pFresnel = new FresnelDielectric( 1.0f , 1.0f );
	else
		pFresnel = new FresnelNo();

	if( mf_dist.str == "Blinn" )
		pMFDist = new Blinn( 1.0f );
	else
		pMFDist = new Anisotropic( 1.0f , 1.0f );
}

ReflectionNode::ReflectionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "Fresnel" , &fresnel ) );
}

void ReflectionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	Reflection* reflection = SORT_MALLOC(Reflection)( pFresnel , baseColor.value);
	reflection->m_weight = weight;
	bsdf->AddBxdf( reflection );
}

// post process
void ReflectionNode::PostProcess()
{
	// parameters are to be exposed through GUI
	if( fresnel.str == "FresnelConductor" )
		pFresnel = new FresnelConductor( 1.0f , 1.0f );
	else if( fresnel.str == "FresnelDielectric" )
		pFresnel = new FresnelDielectric( 1.0f , 1.0f );
	else
		pFresnel = new FresnelNo();
}

RefractionNode::RefractionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "Fresnel" , &fresnel ) );
	m_props.insert( make_pair( "RefractionIndexOut" , &theta0 ) );
	m_props.insert( make_pair( "RefractionIndexIn" , &theta1 ) );
}

void RefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	Refraction* refraction = SORT_MALLOC(Refraction)( theta0.value , theta1.value , pFresnel , baseColor.value);
	refraction->m_weight = weight;
	bsdf->AddBxdf( refraction );
}

// post process
void RefractionNode::PostProcess()
{
	// parameters are to be exposed through GUI
	if( fresnel.str == "FresnelConductor" )
		pFresnel = new FresnelConductor( theta0.value , theta1.value );
	else if( fresnel.str == "FresnelDielectric" )
		pFresnel = new FresnelDielectric( theta0.value , theta1.value );
	else
		pFresnel = new FresnelNo();
}

AddNode::AddNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

LerpNode::LerpNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
	m_props.insert( make_pair( "Factor" , &factor ) );
}

// update bsdf
void LerpNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	float f = factor.GetPropertyValue(bsdf).x;

	if( src0.node )
		src0.node->UpdateBSDF( bsdf, weight * ( 1.0f - f ) );
	if( src1.node )
		src1.node->UpdateBSDF( bsdf, weight * f );
}

BlendNode::BlendNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
	m_props.insert( make_pair( "Factor1" , &factor0 ) );
	m_props.insert( make_pair( "Factor2" , &factor1 ) );
}

// update bsdf
void BlendNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	float f0 = factor0.GetPropertyValue(bsdf).x;
	float f1 = factor1.GetPropertyValue(bsdf).y;
	if( src0.node )
		src0.node->UpdateBSDF( bsdf, weight * f0 );
	if( src1.node )
		src1.node->UpdateBSDF( bsdf, weight * f1 );
}

GridTexNode::GridTexNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// get property value
MaterialPropertyValue GridTexNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	const Intersection* intesection = bsdf->GetIntersection();
	return grid_tex.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f );
}

// post process
void GridTexNode::PostProcess()
{
	// set grid texture
	grid_tex.SetGridColor( src0.value , src1.value );
}

CheckBoxTexNode::CheckBoxTexNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// get property value
MaterialPropertyValue CheckBoxTexNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	const Intersection* intesection = bsdf->GetIntersection();
	return checkbox_tex.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f );
}

// post process
void CheckBoxTexNode::PostProcess()
{
	// set grid texture
	checkbox_tex.SetCheckBoxColor( src0.value , src1.value );
}

ImageTexNode::ImageTexNode()
{
	m_props.insert( make_pair( "Filename" , &filename ) );
}

// get property value
MaterialPropertyValue ImageTexNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	const Intersection* intesection = bsdf->GetIntersection();
	return image_tex.GetColorFromUV( intesection->u , intesection->v );
}

// post process
void ImageTexNode::PostProcess()
{
	// set grid texture
	image_tex.LoadImageFromFile( filename.str );
}