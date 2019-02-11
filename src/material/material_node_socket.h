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

#include <unordered_map>
#include "core/sassert.h"
#include "math/vector4.h"
#include "spectrum/spectrum.h"

#define SORT_PROP_CAT_PROXY(v0, v1)                     v0 ## v1
#define SORT_PROP_CAT(v0, v1)                           SORT_PROP_CAT_PROXY(v0,v1)
#define SORT_STATS_UNIQUE_PROP(var)                     SORT_PROP_CAT(SORT_PROP_CAT(var, __LINE__), var)

#define SORT_MATERIAL_DEFINE_PROP_COMMON(prop,T)        T prop; MaterialNode::MaterialNodeInputSocketRegister SORT_STATS_UNIQUE_PROP(prop) = MaterialNode::MaterialNodeInputSocketRegister( prop , *this );
#define SORT_MATERIAL_DEFINE_PROP_COLOR(prop)           SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyColor)
#define SORT_MATERIAL_DEFINE_PROP_STR(prop)             SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyString)
#define SORT_MATERIAL_DEFINE_PROP_FLOAT(prop)           SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyFloat)
#define SORT_MATERIAL_DEFINE_PROP_VECTOR(prop)          SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyVector)
#define SORT_MATERIAL_DEFINE_PROP_BXDF(prop)            SORT_MATERIAL_DEFINE_PROP_COMMON(prop,MaterialNodePropertyBxdf)

#define SORT_MATERIAL_GET_PROP_COMMON(v,prop,T)         T v; node->prop.GetMaterialProperty(bsdf,v);
#define SORT_MATERIAL_GET_PROP_FLOAT(v,prop)            SORT_MATERIAL_GET_PROP_COMMON(v,prop,float)
#define SORT_MATERIAL_GET_PROP_COLOR(v,prop)            SORT_MATERIAL_GET_PROP_COMMON(v,prop,Spectrum)
#define SORT_MATERIAL_GET_PROP_STR(v,prop)              SORT_MATERIAL_GET_PROP_COMMON(v,prop,std::string)
#define SORT_MATERIAL_GET_PROP_VECTOR(v,prop)           SORT_MATERIAL_GET_PROP_COMMON(v,prop,Vector)

#define SORT_MATERIAL_GET_PROP_COMMON_TMP(v,prop,T)     T v; prop.GetMaterialProperty(bsdf,v);
#define SORT_MATERIAL_GET_PROP_FLOAT_TMP(v,prop)        SORT_MATERIAL_GET_PROP_COMMON_TMP(v,prop,float)
#define SORT_MATERIAL_GET_PROP_COLOR_TMP(v,prop)        SORT_MATERIAL_GET_PROP_COMMON_TMP(v,prop,Spectrum)
#define SORT_MATERIAL_GET_PROP_STR_TMP(v,prop)          SORT_MATERIAL_GET_PROP_COMMON_TMP(v,prop,std::string)
#define SORT_MATERIAL_GET_PROP_VECTOR_TMP(v,prop)       SORT_MATERIAL_GET_PROP_COMMON_TMP(v,prop,Vector)

#define DEFINE_OUTPUT_CHANNEL( CHANNEL , NODE )     \
    class CHANNEL : public MaterialNodeOutputSocket{\
        public:\
            using  NODE##CHANNEL = NODE::CHANNEL;\
            DEFINE_RTTI( NODE##CHANNEL , MaterialNodeOutputSocket );\
            void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ) override;\
    };\
    void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ) override;
#define IMPLEMENT_OUTPUT_CHANNEL_BEGIN( CHANNEL , NODE )  \
    using NODE##CHANNEL = NODE::CHANNEL; \
    IMPLEMENT_RTTI( NODE##CHANNEL );\
    void NODE::UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){\
        sAssert( false , MATERIAL );\
    }\
    void NODE::CHANNEL::UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){\
        auto node = dynamic_cast<NODE*>(m_node);
#define IMPLEMENT_OUTPUT_CHANNEL_END  }

#define DEFINE_OUTPUT_BSDF_SOCKET( CHANNEL , NODE )  \
    class CHANNEL : public MaterialNodeOutputSocket{\
    public:\
        using NODE##CHANNEL = NODE::CHANNEL;\
        DEFINE_RTTI( NODE##CHANNEL , MaterialNodeOutputSocket );\
        void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ) override;\
    } SORT_STATS_UNIQUE_PROP(channel);\
    MaterialNode::MaterialNodeOutputSocketRegister SORT_STATS_UNIQUE_PROP(reg) = MaterialNode::MaterialNodeOutputSocketRegister( SORT_STATS_UNIQUE_PROP(channel) , #NODE#CHANNEL , *this );
#define IMPLEMENT_OUTPUT_BSDF_SOCKET_BEGIN( CHANNEL , NODE )  \
    using NODE##CHANNEL = NODE::CHANNEL; \
    IMPLEMENT_RTTI( NODE##CHANNEL );\
    void NODE::CHANNEL::UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){\
        auto node = dynamic_cast<NODE*>(m_node);
#define IMPLEMENT_OUTPUT_BSDF_SOCKET_END  }

#define DEFINE_OUTPUT_SOCKET_COMMON( CHANNEL , NODE , TYPE )\
    class CHANNEL : public MaterialNodeOutputSocket{\
    public:\
        using NODE##CHANNEL = NODE::CHANNEL;\
        DEFINE_RTTI( NODE##CHANNEL , MaterialNodeOutputSocket );\
        void GetMaterialProperty( Bsdf* bsdf , TYPE& result ) override;\
    } SORT_STATS_UNIQUE_PROP(channel);\
    MaterialNode::MaterialNodeOutputSocketRegister SORT_STATS_UNIQUE_PROP(reg) = MaterialNode::MaterialNodeOutputSocketRegister( SORT_STATS_UNIQUE_PROP(channel) , #NODE#CHANNEL , *this );
#define DEFINE_OUTPUT_SOCKET_COMMON_BEGIN( CHANNEL , NODE , TYPE )  \
    using NODE##CHANNEL = NODE::CHANNEL; \
    IMPLEMENT_RTTI( NODE##CHANNEL );\
    void NODE::CHANNEL::GetMaterialProperty( Bsdf* bsdf , TYPE& result ){\
        auto node = dynamic_cast<NODE*>(m_node);

#define DEFINE_OUTPUT_COLOR_SOCKET( CHANNEL , NODE )            DEFINE_OUTPUT_SOCKET_COMMON( CHANNEL , NODE , Spectrum )
#define IMPLEMENT_OUTPUT_COLOR_SOCKET_BEGIN( CHANNEL , NODE )   DEFINE_OUTPUT_SOCKET_COMMON_BEGIN( CHANNEL , NODE , Spectrum )
#define IMPLEMENT_OUTPUT_COLOR_SOCKET_END                       }

#define DEFINE_OUTPUT_FLOAT_SOCKET( CHANNEL , NODE )            DEFINE_OUTPUT_SOCKET_COMMON( CHANNEL , NODE , float )
#define IMPLEMENT_OUTPUT_FLOAT_SOCKET_BEGIN( CHANNEL , NODE )   DEFINE_OUTPUT_SOCKET_COMMON_BEGIN( CHANNEL , NODE , float )
#define IMPLEMENT_OUTPUT_FLOAT_SOCKET_END                       }

#define DEFINE_OUTPUT_VEC_SOCKET( CHANNEL , NODE )            DEFINE_OUTPUT_SOCKET_COMMON( CHANNEL , NODE , Vector )
#define IMPLEMENT_OUTPUT_VEC_SOCKET_BEGIN( CHANNEL , NODE )   DEFINE_OUTPUT_SOCKET_COMMON_BEGIN( CHANNEL , NODE , Vector )
#define IMPLEMENT_OUTPUT_VEC_SOCKET_END                       }

using MaterialNodeCache = std::unordered_map<std::string,std::unique_ptr<class MaterialNode>>;

class Bsdf;
class IStreamBase;

enum MATERIAL_NODE_PROPERTY_TYPE{
    MNPT_NONE = 0,
    MNPT_FLOAT,
    MNPT_COLOR,
    MNPT_STR,
    MNPT_VECTOR,
    MNPT_BXDF
};

//! @brief  Material node socket.
/**
 * A socket is the connection window to other nodes. There are two different type of sockets,
 * input socket and output socket. Input socket is generally for connecting to another output
 * socket in other nodes. It is also possible that an input socket connects to nothing, in which
 * case a value will be defined. Output socket is more of defining the exact behavior of material 
 * node.
 */
class MaterialNodeSocket{
public:
    //! @brief  Empty virtual function.
    ~MaterialNodeSocket() = default;

    //! @brief  Update BSDF for this node.
    //!
    //! @param bsdf     The BSDF data structure to be filled.
    //! @param weight   The weight for this bsdf sub-tree.
    virtual void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ){
        sAssertMsg(false, MATERIAL, "Update BSDF from a wrong data type." );
    }
    
    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) {
        sAssertMsg(false, MATERIAL, "Get spectrum from a wrong data type." );
    }

    //! @brief  Get float material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , float& result ) {
        sAssertMsg(false, MATERIAL, "Get float from a wrong data type." );
    }

    //! @brief  Get string material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , std::string& result ) {
        sAssertMsg(false, MATERIAL, "Get string from a wrong data type." );
    }

    //! @brief  Get vector material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    virtual void GetMaterialProperty( Bsdf* bsdf , Vector& result ) {
        sAssertMsg(false, MATERIAL, "Get vector from a wrong data type." );
    }
};

//! @brief  Output socket of a node.
/**
 * An output socket defines the exact behavior of the node. Each material node could have multiple output
 * sockets as output channels.
 */
class MaterialNodeOutputSocket : public MaterialNodeSocket{
public:
    //! @brief  Get owning node.
    //!
    //! @return     Node owning this socket.
    class MaterialNode*   GetOwningNode(){
        return m_node;
    }

    //! @brief  Update owning node.
    //!
    //! @param  Node owning this socket.
    void UpdateOwningNode( MaterialNode* node ){
        m_node = node;
    }

protected:
    // temporary
    MaterialNode*    m_node = nullptr;
};

//! @brief  Input socket property in SORT material node.
/**
 * An input socket could be connected with another output socket in SORT.
 */
class MaterialNodeInputSocket : public MaterialNodeSocket{
public:
    //! @brief  Update BSDF for this node.
    //!
    //! @param bsdf     The BSDF data structure to be filled.
    //! @param weight   The weight for this bsdf sub-tree.
    //void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ) override;

    //! @brief Get the type of the node property.
    //!
    //! @return         The type of the node property.
    virtual MATERIAL_NODE_PROPERTY_TYPE GetNodeReturnType() const { 
        return MNPT_NONE; 
    }
    
    //! @brief  Get the node attached to the current socket.
    //!
    //! @return         Node attached to the current socket. 'nullptr' means no node attached.
    inline MaterialNode*    GetConnectedNode() { 
        return m_fromSocket != nullptr ? m_fromSocket->GetOwningNode() : nullptr;
    }

    //! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    //! @param  cache       Cache for avoiding creating duplicated node.
    virtual void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) = 0;

protected:
    /**< The from socket if this socket is connected with an input socket. */
    MaterialNodeOutputSocket*	m_fromSocket = nullptr;

    friend class MaterialNode;
};

//! @brief  Color property in SORT material node.
class MaterialNodePropertyColor : public MaterialNodeInputSocket{
public:
    //! @brief  Get spectrum material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   Spectrum data structure to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Spectrum& result ) override;

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
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override;

private:
    /**< The color value of the node property. */
    Spectrum color;
};

//! @brief  Float property in SORT material node.
class MaterialNodePropertyFloat : public MaterialNodeInputSocket{
public:
    //! @brief  Get float material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    void GetMaterialProperty( Bsdf* bsdf , float& result ) override;

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
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override;

private:
    /**< The float value of the node property. */
    float m_value;
};

//! @brief  String property in SORT material node.
class MaterialNodePropertyString : public MaterialNodeInputSocket{
public:
    //! @brief  Get string material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    void GetMaterialProperty( Bsdf* bsdf , std::string& result ) override;
    
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
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override;

private:
    /**< The string value of the node property. */
	std::string	m_str;
};

//! @brief  BXDF property in SORT material node.
class MaterialNodePropertyBxdf : public MaterialNodeInputSocket{
public:
    //! @brief  Update BSDF based on the sub-tree of the node.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param weight   The weight for the BSDF sub-tree.
    void UpdateBSDF( Bsdf* bsdf , const Spectrum& weight ) override;
    
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
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override;
};

//! @brief  Vector property of SORT material node.
class MaterialNodePropertyVector : public MaterialNodeInputSocket{
public:
    //! @brief  Get vector material property.
    //!
    //! @param bsdf     The BSDF data structure.
    //! @param result   The result to be filled.
    void GetMaterialProperty( Bsdf* bsdf , Vector& result ) override;
    
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
    //! @param  cache       Cache for avoiding creating duplicated node.
    void Serialize( IStreamBase& stream , MaterialNodeCache& cache ) override;

private:
    /**< The vector value of the node property. */
    Vector    m_vec;
};
