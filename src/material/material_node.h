/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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

#include <unordered_set>
#include "spectrum/spectrum.h"
#include "core/rtti.h"
#include "spectrum/rgbspectrum.h"
#include "math/vector4.h"
#include "stream/stream.h"

#define SORT_PROP_CAT_PROXY(v0, v1)                     v0 ## v1
#define SORT_PROP_CAT(v0, v1)                           SORT_PROP_CAT_PROXY(v0,v1)
#define SORT_STATS_UNIQUE_PROP(var)                     SORT_PROP_CAT(SORT_PROP_CAT(var, __LINE__), var)

#define SORT_MATERIAL_DEFINE_PROP_COMMON(prop,T)        T prop; MaterialNodeRegister SORT_STATS_UNIQUE_PROP(prop) = MaterialNodeRegister( prop , *this );
#define SORT_MATERIAL_DEFINE_PROP_COLOR(prop)           SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyColor)
#define SORT_MATERIAL_DEFINE_PROP_STR(prop)             SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyString)
#define SORT_MATERIAL_DEFINE_PROP_FLOAT(prop)           SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyFloat)
#define SORT_MATERIAL_DEFINE_PROP_VECTOR(prop)          SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyVector)
#define SORT_MATERIAL_DEFINE_PROP_BXDF(prop)            SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyBxdf)

#define SORT_MATERIAL_GET_PROP_COMMON(v,prop,T)         T v; prop.GetMaterialProperty(bsdf,v);
#define SORT_MATERIAL_GET_PROP_FLOAT(v,prop)            SORT_MATERIAL_GET_PROP_COMMON(v,prop,float)
#define SORT_MATERIAL_GET_PROP_COLOR(v,prop)            SORT_MATERIAL_GET_PROP_COMMON(v,prop,Spectrum)
#define SORT_MATERIAL_GET_PROP_STR(v,prop)              SORT_MATERIAL_GET_PROP_COMMON(v,prop,std::string)
#define SORT_MATERIAL_GET_PROP_VECTOR(v,prop)           SORT_MATERIAL_GET_PROP_COMMON(v,prop,Vector)

class Bsdf;

enum MATERIAL_NODE_PROPERTY_TYPE{
    MNPT_NONE = 0,
    MNPT_FLOAT,
    MNPT_COLOR,
    MNPT_STR,
    MNPT_VECTOR,
    MNPT_BXDF
};

//! @brief  Material node is the base class for node in material editor.
class MaterialNode : public SerializableObject{
public:
    //! @brief  Empty virtual destructor.
    virtual ~MaterialNode() = default;
    
    //! @brief  Update BSDF for this node.
    //!
    //! @param bsdf     The BSDF data structure to be filled.
    //! @param weight   The weight for this bsdf sub-tree.
    virtual void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f );
    
    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) {
        sAssertMsg(false, MATERIAL, "Get spectrum from wrong data type" );
    }

    //! @brief  Get float material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , float& result ) {
        sAssertMsg(false, MATERIAL, "Get float from wrong data type" );
    }

    //! @brief  Get string material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , std::string& result ) {
        sAssertMsg(false, MATERIAL, "Get string from wrong data type" );
    }

    //! @brief  Get vector material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , Vector& result ) {
        sAssertMsg(false, MATERIAL, "Get vector from wrong data type" );
    }
    
    //! @brief  Get the type of the material node.
    //!
    //! @return         Material node type.
    virtual MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const { 
        return MNPT_NONE; 
    }
    
    //! @brief  Post process of the material.
    //!
    //! Heavy operations like loading textures, loading measured BRDFs will be done here.
    virtual void PostProcess();
    
    //! @brief Whether the node is valid or not.
    //!
    //! @return         'True' if it is a valid node, otherwise 'False'.
    inline bool IsNodeValid() const { 
        return m_node_valid; 
    }
    
protected:
    /**< All properties in the node. */
    std::unordered_set< class MaterialNodeProperty* > m_props;
    
    /**< Whether the node is valid or not. */
    bool m_node_valid = true;
    
    /**< Whether post processing is done for this node. */
    bool m_post_processed = false;
    
    friend class MaterialNodeRegister;
};

//! @brief  Basic socket property in SORT material node.
/**
 * A socket may be connected with a sub node, which means that 'node' member will not be 'nullptr'.
 * Otherwise, this will be merely a property defined in sub-classes.
 */
class MaterialNodeProperty : public SerializableObject{
public:
    //! @brief Update bsdf, this is for bxdf wrappers like Blend , Coat or any other BXDF that can attach other BXDF as input.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param weight   The weight for this bsdf sub-tree.
    void UpdateBsdf( Bsdf* bsdf , Spectrum weight = Spectrum( 1.0f ) );

    //! @brief Get the type of the node property.
    //!
    //! @return         The type of the node property.
    virtual MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const { 
        return MNPT_NONE; 
    }
    
    //! @brief  Get the node attached to the current socket.
    //!
    //! @return         Node attached to the current socket. 'nullptr' means no node attached.
    inline MaterialNode*    GetNode() { 
        return m_node.get(); 
    }

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {}

protected:
	/**< sub node if it has value. */
    std::unique_ptr<MaterialNode>	m_node = nullptr;
};

//! @brief  Color property in SORT material node.
class MaterialNodePropertyColor : public MaterialNodeProperty{
public:
    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) {
        if( m_node )
            m_node->GetMaterialProperty(bsdf, result);
        else
            result = color;
    }
    
    //! @brief  Get the type of the material node property.
    //!
    //! @return         The type of the material node property.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override { 
        return MNPT_COLOR; 
    }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        m_node = MakeUniqueInstance<MaterialNode>( class_id );
        if( m_node )
            m_node->Serialize( stream );
        else
            stream >> color;
	}

private:
    /**< The color value of the node property. */
    Spectrum color;
};

//! @brief  Float property in SORT material node.
class MaterialNodePropertyFloat : public MaterialNodeProperty{
public:
    //! @brief  Get float material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    void GetMaterialProperty( Bsdf* bsdf , float& result ) {
        if( m_node )
            m_node->GetMaterialProperty(bsdf, result);
        else
            result = m_value;
    }
    
    //! @brief  Get the type of the material node property.
    //!
    //! @return         The type of the material node property.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_FLOAT;
    }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        m_node = MakeUniqueInstance<MaterialNode>( class_id );
        if( m_node )
            m_node->Serialize( stream );
        else
            stream >> m_value;
	}

private:
    /**< The float value of the node property. */
    float m_value;
};

//! @brief  String property in SORT material node.
class MaterialNodePropertyString : public MaterialNodeProperty{
public:
    //! @brief  Get string material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    void GetMaterialProperty( Bsdf* bsdf , std::string& result ) {
        result = m_str;
    }
    
    //! @brief  Get the type of the material node property.
    //!
    //! @return         The type of the material node property.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_STR;
    }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string dummy;
        stream >> dummy;
        stream >> m_str;
	}

private:
    /**< The string value of the node property. */
	std::string	m_str;
};

//! @brief  BXDF property in SORT material node.
class MaterialNodePropertyBxdf : public MaterialNodeProperty{
public:
    //! @brief  Update BSDF based on the sub-tree of the node.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param weight   The weight for the BSDF sub-tree.
    void UpdateBsdf( Bsdf* bsdf , Spectrum weight = Spectrum(1.0f) ){
        if( m_node )
            m_node->UpdateBSDF(bsdf,weight);
    }
    
    //! @brief  Get the type of the material node property.
    //!
    //! @return         The type of the material node property.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_BXDF;
    }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        m_node = MakeUniqueInstance<MaterialNode>( class_id );
        if( m_node )
            m_node->Serialize( stream );
	}
};

//! @brief  Vector property of SORT material node.
class MaterialNodePropertyVector : public MaterialNodeProperty{
public:
    //! @brief  Get vector material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Vector& result ) {
        if( m_node )
            m_node->GetMaterialProperty(bsdf, result);
        else
            result = m_vec;
    }
    
    //! @brief  Get the type of the material node property.
    //!
    //! @return         The type of the material node property.
    MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const override {
        return MNPT_VECTOR;
    }
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
        std::string class_id;
        stream >> class_id;
        m_node = MakeUniqueInstance<MaterialNode>( class_id );
        if( m_node )
            m_node->Serialize( stream );
        else
            stream >> m_vec;
	}

private:
    /**< The vector value of the node property. */
    Vector    m_vec;
};

//! @brief  Auto register for node socket properties.
/**
 * This is a class hidden behind the macros. It should not be explicitly defined anywhere else in the code.
 */
class MaterialNodeRegister{
public:
    //! @brief  Constructor.
    //!
    //! @param  prop        Material node property.
    //! @param  node        Reference of the node that this material property belongs to.
    MaterialNodeRegister( MaterialNodeProperty& prop , MaterialNode& node ){
        node.m_props.insert( &prop );
    }
};

//! @brief Mateiral output node.
/**
 * By default, each material will have exactly one output node. For those who doesn't have any OutputNode,
 * it is defined as invalid material, a default material will be used to replace this material.
 */
class OutputNode : public MaterialNode{
public:
    //! @brief  Update BSDF for this node.
    //!
    //! @param bsdf     The BSDF data structure to be filled.
    //! @param weight   The weight for this bsdf sub-tree.
    void UpdateBSDF( Bsdf* bsdf , Spectrum weight = 1.0f ) override;
    
    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void Serialize( IStreamBase& stream ) override {
		output.Serialize( stream );
	}

private:
    SORT_MATERIAL_DEFINE_PROP_BXDF( output );
};
