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

#include "stream/stream.h"
#include "core/singleton.h"
#include "core/classtype.h"

//! @brief	This needs to be update every time the content of GlobalConfiguration chagnes.
constexpr unsigned int GLOBAL_CONFIGURATION_VERSION	= 0;

//! @brief	GlobalConfiguration saves some global state.
class GlobalConfiguration : public Singleton<GlobalConfiguration> , SerializableObject {
public:
	//! @brief	Get render tile size.
	//!
	//! A tile is a group of pixels to be traced by one thread unit.
	unsigned int	GetTileSize() const { return m_tileSize; }
	
	//! @brief	Update tile size.
	//!
	//! @param	tile_size	Size of each tile.
	void			SetTileSize( unsigned int tile_size ) { m_tileSize = tile_size; }

	//! @brief	Whether SORT is ran in Blender mode.
	//!
	//! Blender mode will stream the result directly to shared memory through IPC.
	bool			GetBlenderMode() const { return m_blenderMode; }

	//! @brief	Update blender mode
	//!
	//! @param	mode		Whether blender mode is true or not.
	void			SetBlenderMode( bool mode ) { m_blenderMode = mode; }

	//! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
		unsigned int version = 0;
		stream >> version;
		sAssertMsg( GLOBAL_CONFIGURATION_VERSION == version , GENERAL , "Incompatible resource file with this version SORT.");
		stream >> m_tileSize;
        stream >> m_accelType;
        m_accelerator = MakeAccelerator(m_accelType);
	};

    //! @brief      Serializing data to stream
    //!
    //! @param      Stream where the serialization data goes to. Depending on different situation, it could come from different places.#pragma endregion
    void    Serialize( OStreamBase& stream ) override {
		stream << GLOBAL_CONFIGURATION_VERSION;
		stream << m_tileSize;
        stream << m_accelType;
	};

    //! @brief      Get the spatial accelerator structure.
    //!
    //! @return     The spatial acceleration structure. Could be 'nullptr', meaning a bruteforce workaround will be used.
    std::shared_ptr<Accelerator>    GetAccelerator() { return m_accelerator; }

private:
	unsigned int		            m_tileSize = 64;			/**< Size of tile for tasks to render each time. */
	bool				            m_blenderMode = false;		/**< Whether the current running instance is attached with Blender. */
    std::shared_ptr<Accelerator>    m_accelerator = nullptr;    /**< Spatial accelerator for accelerating primitive/ray intersection test. */

    unsigned int                    m_accelType = 0;            /**< Local cache of accelerator type. This is not exposed to other systems.*/
	//! @brief	Make constructor private
	GlobalConfiguration(){}
	//! @brief	Make copy constructor private
	GlobalConfiguration( const GlobalConfiguration& ){}

	friend class Singleton<GlobalConfiguration>;
};

#define	g_tile_size		GlobalConfiguration::GetSingleton().GetTileSize()
#define	g_blender_mode	GlobalConfiguration::GetSingleton().GetBlenderMode()