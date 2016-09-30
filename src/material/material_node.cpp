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
#include "bsdf/fresnel.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "thirdparty/tinyxml/tinyxml.h"
#include "geometry/intersection.h"

IMPLEMENT_CREATOR( LambertNode );
IMPLEMENT_CREATOR( MerlNode );
IMPLEMENT_CREATOR( OrenNayarNode );
IMPLEMENT_CREATOR( MicrofacetReflectionNode );
IMPLEMENT_CREATOR( MicrofacetRefractionNode );
IMPLEMENT_CREATOR( AddNode );
IMPLEMENT_CREATOR( LerpNode );
IMPLEMENT_CREATOR( BlendNode );
IMPLEMENT_CREATOR( MutiplyNode );
IMPLEMENT_CREATOR( GridTexNode );
IMPLEMENT_CREATOR( CheckBoxTexNode );
IMPLEMENT_CREATOR( ImageTexNode );
IMPLEMENT_CREATOR( ConstantColorNode );

// get node property
MaterialNodeProperty* MaterialNode::getProperty( const string& name )
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.find( name );
	if( it != m_props.end() )
		return it->second;
	return 0;
}

// set node property
void MaterialNodeProperty::SetNodeProperty( const string& prop )
{
	string str = prop;
	string x = NextToken( str , ' ' );
	string y = NextToken( str , ' ' );
	string z = NextToken( str , ' ' );
	string w = NextToken( str , ' ' );

	value.x = (float)atof( x.c_str() );
	value.y = (float)atof( y.c_str() );
	value.z = (float)atof( z.c_str() );
	value.w = (float)atof( w.c_str() );
}

// get node property
MaterialPropertyValue MaterialNodeProperty::GetPropertyValue( Bsdf* bsdf )
{
	if( node )
		return node->GetNodeValue( bsdf );
	return value;
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
			prop = prop->NextSiblingElement( "Property" );

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

// check validation
bool MaterialNode::CheckValidation()
{
	// get subtree node type
	getNodeType();

	m_node_valid = true;
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			m_node_valid &= it->second->node->CheckValidation();
		++it;
	}

	return m_node_valid;
}

// get sub tree node type
MAT_NODE_TYPE MaterialNode::getNodeType()
{
	MAT_NODE_TYPE type = MAT_NODE_NONE;

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			type |= it->second->node->getNodeType();
		++it;
	}

	// setup sub-tree type
	subtree_node_type = type;

	return type;
}

// post process
void MaterialNode::PostProcess()
{
	if( m_post_processed )
		return;

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		if( it->second->node )
			it->second->node->PostProcess();
		++it;
	}

	m_post_processed = true;
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

MaterialNode::~MaterialNode()
{
	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		delete it->second->node;
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
	// return a default one for invalid material
	if( !m_node_valid )
	{
		Spectrum default_spectrum( 0.3f , 0.0f , 0.0f );
		Lambert* lambert = SORT_MALLOC(Lambert)( default_spectrum );
		lambert->m_weight = weight;
		bsdf->AddBxdf( lambert );
		return;
	}

	if( output.node )
		output.node->UpdateBSDF( bsdf );
}

// check validation
bool OutputNode::CheckValidation()
{
	// it is invalid if there is no node attached
	if( output.node == 0 )
		return false;

	// get node type
	MAT_NODE_TYPE type = output.node->getNodeType();

	// make sure there is bxdf attached !!
	if( ( output.node == 0 ) || !(type & MAT_NODE_BXDF) )
	{
		m_node_valid = false;
		return false;
	}

	m_node_valid = MaterialNode::CheckValidation();

	return m_node_valid;
}

// check validation
bool BxdfNode::CheckValidation()
{
	bool valid = MaterialNode::CheckValidation();

	map< string , MaterialNodeProperty* >::const_iterator it = m_props.begin();
	while( it != m_props.end() )
	{
		MaterialNode* node = it->second->node;
		if( node )
		{
			MAT_NODE_TYPE sub_type = node->getNodeType();

			// attaching bxdf result as an input of another bxdf doesn't make any sense at all
			if( sub_type & MAT_NODE_BXDF )
				return false;
		}
		++it;
	}

	return valid;
}

LambertNode::LambertNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
}

void LambertNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
    Lambert* lambert = SORT_MALLOC(Lambert)( baseColor.GetPropertyValue(bsdf).ToSpectrum() );
	lambert->m_weight = weight;
	bsdf->AddBxdf( lambert );
}

MerlNode::MerlNode()
{
	m_props.insert( make_pair( "Filename" , &merlfile ) );
}

void MerlNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	merl.m_weight = weight;
	bsdf->AddBxdf( &merl );
}

// post process
void MerlNode::PostProcess()
{
	if( m_post_processed )
		return;

	if( merlfile.str.empty() == false )
		merl.LoadData( merlfile.str );
}

OrenNayarNode::OrenNayarNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "Roughness" , &roughness ) );
}

void OrenNayarNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , roughness.GetPropertyValue(bsdf).x );
	orennayar->m_weight = weight;
	bsdf->AddBxdf( orennayar );
}

MicrofacetReflectionNode::MicrofacetReflectionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
	m_props.insert( make_pair( "Visibility" , &mf_vis ) );
	m_props.insert( make_pair( "Roughness" , &roughness ) );
	m_props.insert( make_pair( "eta" , &eta ) );
	m_props.insert( make_pair( "k" , &k ) );
}

void MicrofacetReflectionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	float rn = clamp( roughness.GetPropertyValue(bsdf).x , 0.05f , 1.0f );
	MicroFacetDistribution* dist = 0;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( rn );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( rn );
	else
		dist = SORT_MALLOC(GGX)( rn );	// GGX is default

	VisTerm* vis = 0;
	if( mf_vis.str == "Neumann" )
		vis = SORT_MALLOC(VisNeumann)();
	else if( mf_vis.str == "Kelemen" )
		vis = SORT_MALLOC(VisKelemen)();
	else if( mf_vis.str == "Schlick" )
		vis = SORT_MALLOC(VisSchlick)( rn );
	else if( mf_vis.str == "Smith" )
		vis = SORT_MALLOC(VisSmith)( rn );
	else if( mf_vis.str == "SmithJointApprox" )
		vis = SORT_MALLOC(VisSmithJointApprox)( rn );
	else if( mf_vis.str == "CookTorrance" )
		vis = SORT_MALLOC(VisCookTorrance)();
	else
		vis = new VisImplicit();	// implicit visibility term is default

	Fresnel* frenel = SORT_MALLOC( FresnelConductor )( eta.GetPropertyValue(bsdf).ToSpectrum() , k.GetPropertyValue(bsdf).ToSpectrum() );

	MicroFacetReflection* mf = SORT_MALLOC(MicroFacetReflection)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , frenel , dist , vis);
	mf->m_weight = weight;
	bsdf->AddBxdf( mf );
}

MicrofacetRefractionNode::MicrofacetRefractionNode()
{
	m_props.insert( make_pair( "BaseColor" , &baseColor ) );
	m_props.insert( make_pair( "MicroFacetDistribution" , &mf_dist ) );
	m_props.insert( make_pair( "Visibility" , &mf_vis ) );
	m_props.insert( make_pair( "Roughness" , &roughness ) );
	m_props.insert( make_pair( "in_ior" , &in_ior ) );
	m_props.insert( make_pair( "ext_ior" , &ext_ior ) );
}

void MicrofacetRefractionNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	float rn = clamp( roughness.GetPropertyValue(bsdf).x , 0.05f , 1.0f );
	MicroFacetDistribution* dist = 0;
	if( mf_dist.str == "Blinn" )
		dist = SORT_MALLOC(Blinn)( rn );
	else if( mf_dist.str == "Beckmann" )
		dist = SORT_MALLOC(Beckmann)( rn );
	else
		dist = SORT_MALLOC(GGX)( rn );	// GGX is default

	VisTerm* vis = 0;
	if( mf_vis.str == "Neumann" )
		vis = SORT_MALLOC(VisNeumann)();
	else if( mf_vis.str == "Kelemen" )
		vis = SORT_MALLOC(VisKelemen)();
	else if( mf_vis.str == "Schlick" )
		vis = SORT_MALLOC(VisSchlick)( rn );
	else if( mf_vis.str == "Smith" )
		vis = SORT_MALLOC(VisSmith)( rn );
	else if( mf_vis.str == "SmithJointApprox" )
		vis = SORT_MALLOC(VisSmithJointApprox)( rn );
	else if( mf_vis.str == "CookTorrance" )
		vis = SORT_MALLOC(VisCookTorrance)();
	else
		vis = new VisImplicit();	// implicit visibility term is default

	float in_eta = in_ior.GetPropertyValue(bsdf).x;
	float ext_eta = ext_ior.GetPropertyValue(bsdf).x;
	Fresnel* frenel = SORT_MALLOC( FresnelDielectric )( in_eta , ext_eta );

	MicroFacetRefraction* mf = SORT_MALLOC(MicroFacetRefraction)( baseColor.GetPropertyValue(bsdf).ToSpectrum() , frenel , dist , vis , in_eta , ext_eta );
	mf->m_weight = weight;
	bsdf->AddBxdf( mf );
}

AddNode::AddNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// check validation
bool AddNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// if one of the parameters is a bxdf, the other should be exactly the same
	if( ( type0 & MAT_NODE_BXDF ) != ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
}

// get property value
MaterialPropertyValue AddNode::GetNodeValue( Bsdf* bsdf )
{
	return src0.GetPropertyValue(bsdf) + src1.GetPropertyValue(bsdf);
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

// get property value
MaterialPropertyValue LerpNode::GetNodeValue( Bsdf* bsdf )
{
	float f = factor.GetPropertyValue( bsdf ).x;
	return src0.GetPropertyValue(bsdf) * ( 1.0f - f ) + src1.GetPropertyValue(bsdf) * f;
}

// check validation
bool LerpNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// if one of the parameters is a bxdf, the other should be exactly the same
	if( ( type0 & MAT_NODE_BXDF ) != ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
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

// get property value
MaterialPropertyValue BlendNode::GetNodeValue( Bsdf* bsdf )
{
	float f0 = factor0.GetPropertyValue( bsdf ).x;
	float f1 = factor1.GetPropertyValue( bsdf ).x;
	return src0.GetPropertyValue(bsdf) * f0 + src1.GetPropertyValue(bsdf) * f1;
}

// check validation
bool BlendNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// if one of the parameters is a bxdf, the other should be exactly the same
	if( ( type0 & MAT_NODE_BXDF ) != ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
}

MutiplyNode::MutiplyNode()
{
	m_props.insert( make_pair( "Color1" , &src0 ) );
	m_props.insert( make_pair( "Color2" , &src1 ) );
}

// update bsdf
void MutiplyNode::UpdateBSDF( Bsdf* bsdf , Spectrum weight )
{
	if( weight.IsBlack() )
		return;

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	if( type0 & MAT_NODE_BXDF )
		src0.node->UpdateBSDF( bsdf , weight * src1.GetPropertyValue(bsdf).x );
	else if( type1 & MAT_NODE_BXDF )
		src1.node->UpdateBSDF( bsdf , weight * src0.GetPropertyValue(bsdf).x );
}

// get property value
MaterialPropertyValue MutiplyNode::GetNodeValue( Bsdf* bsdf )
{
	return src0.GetPropertyValue(bsdf) * src1.GetPropertyValue(bsdf);
}

// check validation
bool MutiplyNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type0 = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	MAT_NODE_TYPE type1 = (src1.node)?src1.node->getNodeType():MAT_NODE_CONSTANT;

	// Can't multiply two bxdfs
	if( ( type0 & MAT_NODE_BXDF ) && ( type1 & MAT_NODE_BXDF ) )
		m_node_valid = false;

	return m_node_valid;
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
	return FromSpectrum( grid_tex.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f ) );
}

// post process
void GridTexNode::PostProcess()
{
	if( m_post_processed )
		return;

	// set grid texture
	grid_tex.SetGridColor( src0.GetPropertyValue(0).ToSpectrum(), src1.GetPropertyValue(0).ToSpectrum());

	MaterialNode::PostProcess();
}

// check validation
bool GridTexNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();
	
	MAT_NODE_TYPE type = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	if( type & MAT_NODE_BXDF ||
		type & MAT_NODE_VARIABLE )
		m_node_valid = false;

	return m_node_valid;
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
	return FromSpectrum( checkbox_tex.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f ) );
}

// post process
void CheckBoxTexNode::PostProcess()
{
	if( m_post_processed )
		return;

	// set grid texture
	checkbox_tex.SetCheckBoxColor( src0.GetPropertyValue(0).ToSpectrum() , src1.GetPropertyValue(0).ToSpectrum() );

	MaterialNode::PostProcess();
}

// check validation
bool CheckBoxTexNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();
	
	MAT_NODE_TYPE type = (src0.node)?src0.node->getNodeType():MAT_NODE_CONSTANT;
	if( type & MAT_NODE_BXDF ||
		type & MAT_NODE_VARIABLE )
		m_node_valid = false;

	return m_node_valid;
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
	return FromSpectrum( image_tex.GetColorFromUV( intesection->u , intesection->v ) );
}

// post process
void ImageTexNode::PostProcess()
{
	if( m_post_processed )
		return;

	// set grid texture
	image_tex.LoadImageFromFile( filename.str );
}

ConstantColorNode::ConstantColorNode()
{
	m_props.insert( make_pair( "Color" , &src ) );
}

// get property value
MaterialPropertyValue ConstantColorNode::GetNodeValue( Bsdf* bsdf )
{
	// get intersection
	return src.GetPropertyValue(bsdf);
}

// check validation
bool ConstantColorNode::CheckValidation()
{
	m_node_valid = MaterialNode::CheckValidation();

	MAT_NODE_TYPE type = (src.node)?src.node->getNodeType():MAT_NODE_CONSTANT;
	if( type & MAT_NODE_BXDF ||
		type & MAT_NODE_VARIABLE )
		m_node_valid = false;

	return m_node_valid;
}
