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

IMPLEMENT_CREATOR( LambertNode );
IMPLEMENT_CREATOR( MerlNode );
IMPLEMENT_CREATOR( OrenNayarNode );
IMPLEMENT_CREATOR( MicrofacetNode );
IMPLEMENT_CREATOR( ReflectionNode );
IMPLEMENT_CREATOR( RefractionNode );

// get node property
MaterialNodeProperty* MaterialNode::getProperty( const string& name )
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.find( name );
	if( it != m_props.end() )
		return it->second;
	return 0;
}

// set node property
void MaterialNodePropertyColor::SetNodeProperty( const string& prop )
{
	value = SpectrumFromStr( prop );
}

// set node property
void MaterialNodePropertyFloat::SetNodeProperty( const string& prop )
{
	value = atof( prop.c_str() );
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

OutputNode::OutputNode()
{
	// register node property
	m_props.insert( make_pair( "Surface" , &output ) );
}

// update bsdf
void OutputNode::UpdateBSDF( Bsdf* bsdf )
{
	if( output.node )
		output.node->UpdateBSDF( bsdf );
}

LambertNode::LambertNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
}

void LambertNode::UpdateBSDF( Bsdf* bsdf )
{
	Lambert* lambert = SORT_MALLOC(Lambert)( baseColor.value );
	bsdf->AddBxdf( lambert );
}

MerlNode::MerlNode()
{
	m_props.insert( make_pair( "Filename" , &merlfile ) );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf )
{
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

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf )
{
	OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( baseColor.value , sigma.value );
	bsdf->AddBxdf( orennayar );
}

MicrofacetNode::MicrofacetNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
	m_props.insert( make_pair( "Fresnel" , &fresnel ) );
}

void MicrofacetNode::UpdateBSDF( Bsdf* bsdf )
{
	MicroFacet* mf = SORT_MALLOC(MicroFacet)( baseColor.value , pFresnel , pMFDist );
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

void ReflectionNode::UpdateBSDF( Bsdf* bsdf )
{
	Reflection* reflection = SORT_MALLOC(Reflection)( pFresnel , baseColor.value);
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

void RefractionNode::UpdateBSDF( Bsdf* bsdf )
{
	Refraction* refraction = SORT_MALLOC(Refraction)( theta0.value , theta1.value , pFresnel , baseColor.value);
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