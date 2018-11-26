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

#include "material_node.h"
#include "stream/stream.h"

class Bsdf;
class Intersection;

//! @brief 	A thin layer of material definition.
/**
 * SORT supports node-graph based material system so that it could be flexible enough to support varies features.
 * This is no pre-defined material in SORT. Every material is a combination of material node forming a tree, the topology
 * of the tree defines the behavior of the material. With this design, it is very easy to drive different parameters by 
 * textures or any other information. For invalid material node graph tree, a red default material is returned as default.
 * Material class just holds a root node of material node tree. The exact definition of materials are defined in Blender.
 */
class Material : public SerializableObject
{
public:
	//! @brief	Get the BSDF.
	//!
	//! BSDF is a set of BXDF combined together. This function parse the material and fill the BSDF data structure.
	//! A red warning material will be returned if this material is invalid.
	//!
	//! @param		intersect		The intersection information at the point to be shaded.
	//! @return						A BSDF holding BXDF information will be returned. The BSDF is allocated in the memory pool,
	//!								meaning this is no need to release the memory in BSDF.
	Bsdf* GetBsdf( const Intersection* intersect ) const;

	//! @brief	Set the name of the material.
	//!
	//! Higher level code should make sure the name is unique in the scene.
	//!
	//! @param	name	Name to be set to this material.
	inline void SetName( const std::string& name ) { m_name = name; }

	//! @brief	Get the unique name of this material.
	//!
	//! @return		The name of this material.
	inline const std::string& GetName() const { return m_name; }

	//! @brief	Get the root material node.
	//!
	//! @return		The root of the material node graph tree.
	inline MaterialNode* GetRootNode() { return &m_root; }

	//! @brief	Parse material from XML.
	//!
	//! This interface is to be deprecated after serialization is fully supported.
	//! 
	//! @param	element		Root node for XML node.
	void	ParseMaterial( TiXmlElement* element );

	//! @brief  Serialization interface. Loading data from stream.
    //!
    //! Serialize the material. Loading from an IStreamBase, which could be coming from file, memory or network.
    //!
    //! @param  stream      Input stream for data.
    void        Serialize( IStreamBase& stream ) override {
		// to be implemented
	}

    //! @brief  Serialization interface. Saving data to stream.
    //!
    //! Serialize the material. Saving to an OStreamBase, which could be file, memory or network streaming.
    //!
    //! @param  stream      Output stream.
    void        Serialize( OStreamBase& stream ) override {
		// to be implemented
	}

private:
	std::string			m_name;		/**< Unique name of the material. */
	mutable OutputNode	m_root;		/**< Root node of material node graph tree. */
};
