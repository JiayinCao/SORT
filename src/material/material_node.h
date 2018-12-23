/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "spectrum/spectrum.h"
#include "core/creator.h"
#include "spectrum/rgbspectrum.h"
#include "math/vector4.h"
#include "core/strhelper.h"
#include "stream/stream.h"

#define SORT_PROP_CAT_PROXY(v0, v1)                     v0 ## v1
#define SORT_PROP_CAT(v0, v1)                           SORT_PROP_CAT_PROXY(v0,v1)
#define SORT_STATS_UNIQUE_PROP(var)                     SORT_PROP_CAT(SORT_PROP_CAT(var, __LINE__), var)

#define SORT_MATERIAL_DEFINE_PROP_COMMON(name,prop,T)   T prop; MaterialNodeRegister SORT_STATS_UNIQUE_PROP(prop) = MaterialNodeRegister( name , prop , *this );
#define SORT_MATERIAL_DEFINE_PROP_COLOR(name,prop)      SORT_MATERIAL_DEFINE_PROP_COMMON(name,prop,MaterialNodePropertyColor)
#define SORT_MATERIAL_DEFINE_PROP_STR(name,prop)        SORT_MATERIAL_DEFINE_PROP_COMMON(name,prop,MaterialNodePropertyString)
#define SORT_MATERIAL_DEFINE_PROP_FLOAT(name,prop)      SORT_MATERIAL_DEFINE_PROP_COMMON(name,prop,MaterialNodePropertyFloat)
#define SORT_MATERIAL_DEFINE_PROP_VECTOR(name,prop)     SORT_MATERIAL_DEFINE_PROP_COMMON(name,prop,MaterialNodePropertyVector)
#define SORT_MATERIAL_DEFINE_PROP_BXDF(name,prop)       SORT_MATERIAL_DEFINE_PROP_COMMON(name,prop,MaterialNodePropertyBxdf)

#define SORT_MATERIAL_GET_PROP_COMMON(v,prop,T)         T v; prop.GetMaterialProperty(bsdf,v);
#define SORT_MATERIAL_GET_PROP_FLOAT(v,prop)            SORT_MATERIAL_GET_PROP_COMMON(v,prop,float)
#define SORT_MATERIAL_GET_PROP_COLOR(v,prop)            SORT_MATERIAL_GET_PROP_COMMON(v,prop,Spectrum)
#define SORT_MATERIAL_GET_PROP_STR(v,prop)              SORT_MATERIAL_GET_PROP_COMMON(v,prop,std::string)
#define SORT_MATERIAL_GET_PROP_VECTOR(v,prop)           SORT_MATERIAL_GET_PROP_COMMON(v,prop,Vector)

class Bsdf;
class TiXmlElement;
class MaterialNodeProperty;

enum MATERIAL_NODE_PROPERTY_TYPE{
    MNPT_NONE = 0,
    MNPT_FLOAT,
    MNPT_COLOR,
    MNPT_STR,
    MNPT_VECTOR,
    MNPT_BXDF
};

//! @brief  Material node is the base class for node in material editor.
class MaterialNode : public SerializableObject
{
public:
    //! @brief  Empty virtual destructor.
    virtual ~MaterialNode();
    
    // update bsdf
    virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );
    
    virtual void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) {sAssertMsg(false, MATERIAL, "Get spectrum from wrong data type" );}
    virtual void GetMaterialProperty( Bsdf* bsdf , float& result ) {sAssertMsg(false, MATERIAL, "Get float from wrong data type" );}
    virtual void GetMaterialProperty( Bsdf* bsdf , std::string& result ) {sAssertMsg(false, MATERIAL, "Get string from wrong data type" );}
    virtual void GetMaterialProperty( Bsdf* bsdf , Vector& result ) {sAssertMsg(false, MATERIAL, "Get vector from wrong data type" );}
    
    // get node return type
    virtual MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const { return MNPT_NONE; }
    
    // post process
    virtual void PostProcess();
    
    // whether the node is valid
    inline bool IsNodeValid() const { return m_node_valid; }

protected:
    // node properties
    std::unordered_map< std::string , MaterialNodeProperty * > m_props;
    
    // get node property
    MaterialNodeProperty*    getProperty( const std::string& name );
    
    // valid node
    bool m_node_valid = true;
    
    // already post processed
    bool m_post_processed = false;
    
    friend class MaterialNodeRegister;
};

//! @brief  Basic socket property in material node.
/**
 * A socket may be connected with a sub node, which means that 'node' member will not be 'nullptr'.
 * Otherwise, this will be merely a property defined in sub-classes.
 */
class MaterialNodeProperty : public SerializableObject
{
public:
    //! @brief  Empty virtual destructor.
    virtual ~MaterialNodeProperty() {}

    // update bsdf, this is for bxdf wrappers like Blend , Coat or any other BXDF that can attach other BXDF as input
    void UpdateBsdf( Bsdf* bsdf , Spectrum weight = Spectrum( 1.0f ) );

    // get node return type
    virtual MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const { return MNPT_NONE; }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    virtual void Serialize( IStreamBase& stream ) = 0;

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    virtual void Serialize( OStreamBase& stream ) = 0;

    //! @brief  Get the node attached to the current socket.
    //!
    //! @return Node attached to the current socket. 'nullptr' means no node attached.
    inline std::shared_ptr<MaterialNode>    GetNode() { 
        return node; 
    }
protected:
	// sub node if it has value
    std::shared_ptr<MaterialNode>	node = nullptr;
};

class MaterialNodePropertyColor : public MaterialNodeProperty
{
public:
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) {
        if( node )
            node->GetMaterialProperty(bsdf, result);
        else
            result = color;
    }
    
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_COLOR; }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        node = MakeInstance<MaterialNode>( class_id );
        if( node )
            node->Serialize( stream );
        else
            stream >> color;
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    // value
    Spectrum color;
};

class MaterialNodePropertyFloat : public MaterialNodeProperty
{
public:
    void GetMaterialProperty( Bsdf* bsdf , float& result ) {
        if( node )
            node->GetMaterialProperty(bsdf, result);
        else
            result = value;
    }
    
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_FLOAT; }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        node = MakeInstance<MaterialNode>( class_id );
        if( node )
            node->Serialize( stream );
        else
            stream >> value;
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    float value;
};

class MaterialNodePropertyString : public MaterialNodeProperty
{
public:
    void GetMaterialProperty( Bsdf* bsdf , std::string& result ) { result = str; }
    
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_STR; }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string dummy;
        stream >> dummy;
        stream >> str;
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
        std::string dummy;
        stream >> dummy;
        stream << str;
	}

	// color value
	std::string	str;
};

class MaterialNodePropertyBxdf : public MaterialNodeProperty
{
public:
    void UpdateBsdf( Bsdf* bsdf , Spectrum weight = Spectrum(1.0f) ){
        if( node )
            node->UpdateBSDF(bsdf,weight);
    }
    
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_BXDF; }
    
    void GetMaterialProperty( Bsdf* bsdf , std::shared_ptr<MaterialNode>& result ) { result = node; }

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        node = MakeInstance<MaterialNode>( class_id );
        if( node )
            node->Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}
};

class MaterialNodePropertyVector : public MaterialNodeProperty
{
public:
    void GetMaterialProperty( Bsdf* bsdf , Vector& result ) {
        if( node )
            node->GetMaterialProperty(bsdf, result);
        else
            result = vec;
    }
    
    // get node return type
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { return MNPT_VECTOR; }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        node = MakeInstance<MaterialNode>( class_id );
        if( node )
            node->Serialize( stream );
        else
            stream >> vec;
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    Vector    vec;
};

// Register a node automatically
class MaterialNodeRegister{
public:
    MaterialNodeRegister( const std::string& name , MaterialNodeProperty& prop , MaterialNode& node ){
        node.m_props.insert( make_pair( name , &prop ) );
    }
};

// Mateiral output node
class OutputNode : public MaterialNode
{
public:
    // update bsdf
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
		output.Serialize( stream );
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF( "Surface" , output );
};
