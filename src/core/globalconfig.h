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
#include "accel/accelerator.h"
#include "integrator/integrator.h"
#include "core/creator.h"

//! @brief	This needs to be update every time the content of GlobalConfiguration chagnes.
constexpr unsigned int GLOBAL_CONFIGURATION_VERSION	= 0;

//! @brief	GlobalConfiguration saves some global state.
class GlobalConfiguration : public Singleton<GlobalConfiguration> , SerializableObject {
public:
	//! @brief	Get render tile size.
	//!
	//! A tile is a group of pixels to be traced by one thread unit.
	unsigned int	GetTileSize() const { return m_tileSize; }
	
	//! @brief	Whether SORT is ran in Blender mode.
	//!
	//! Blender mode will stream the result directly to shared memory through IPC.
	bool			GetBlenderMode() const { return m_blenderMode; }

	//! @brief	Update blender mode
	//!
	//! @param	mode		Whether blender mode is true or not.
	void			SetBlenderMode( bool mode ) { m_blenderMode = mode; }

	//! @brief      Get the spatial accelerator structure.
    //!
    //! @return     The spatial acceleration structure. Could be 'nullptr', meaning a bruteforce workaround will be used.
    std::shared_ptr<Accelerator>    GetAccelerator() { return m_accelerator; }

	//! @brief      Get the integrator of the renderer.
    //!
	//! @return		Integrator used to evaluate rendering equation.
    std::shared_ptr<Integrator>    	GetIntegrator() { return m_integrator; }

	//! @brief		Get the number of worker threads.
	//!
	//! @return		Number of worker thread. Default value is 16.
	unsigned int					GetThreadCnt() const { return m_threadCnt; }

	//! @brief		Get sampler per pixel.
	//!
	//! @return		Number of sample per pixel.
	unsigned int					GetSamplePerPixel() const { return m_samplePerPixel; }

	//! @brief		Get full path of the resource.
	//!
	//! @return		Full path of the resources files.
	const std::string&				GetResourcePath() const { return m_resourcePath; }

	//! @brief		Get output file name.
	//!
	//! @return		Name of the output file.
	const std::string&				GetOutputFileName() const { return m_outputFile; }

	//! @brief		Get resolution of the final result.
	//!
	//! @return 	Resolution of the final result.
	const Vector2i					GetResultResolution() const { return Vector2i( m_resWidth , m_resHeight ); }

	//! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
		unsigned int version = 0;
		stream >> version;
		sAssertMsg( GLOBAL_CONFIGURATION_VERSION == version , GENERAL , "Incompatible resource file with this version SORT.");
		stream >> m_resourcePath;
		stream >> m_outputFile;
		stream >> m_tileSize;
		stream >> m_threadCnt;
		stream >> m_samplePerPixel;
		stream >> m_resWidth >> m_resHeight;
        stream >> m_accelType;
        m_accelerator = MakeInstance<Accelerator>(m_accelType);
		if( m_accelerator )
			m_accelerator->Serialize( stream );
		stream >> m_integratorType;
		m_integrator = MakeInstance<Integrator>(m_integratorType);
		if( m_integrator != nullptr )
			m_integrator->Serialize( stream );
	};
	
private:
	std::string						m_resourcePath = "";			/**< Full path of the resource files. */
	std::string						m_outputFile;					/**< Name of the output file. */
	unsigned int		            m_tileSize = 64;				/**< Size of tile for tasks to render each time. */
	bool				            m_blenderMode = false;			/**< Whether the current running instance is attached with Blender. */
	unsigned int					m_resWidth = 1024;				/**< Width of the result resolution. */
	unsigned int					m_resHeight = 1024;				/**< Height of the result resolution. */
    unsigned int					m_threadCnt = 16;				/**< Number of worker thread ( including the main thread as a woker thread ). */
	unsigned int					m_samplePerPixel = 4;			/**< Sample of per-pixel. Default value is 4 for fast iteration. */
	std::shared_ptr<Accelerator>    m_accelerator = nullptr;    	/**< Spatial accelerator for accelerating primitive/ray intersection test. */
	std::shared_ptr<Integrator>		m_integrator = nullptr;			/**< Integrator used to evaluate rendering equation. */
	
    std::string                     m_accelType;            		/**< Local cache of accelerator type. This is not exposed to other systems.*/
	std::string						m_integratorType;				/**< Integrator type. */

	//! @brief	Make constructor private
	GlobalConfiguration(){}
	//! @brief	Make copy constructor private
	GlobalConfiguration( const GlobalConfiguration& ){}

	friend class Singleton<GlobalConfiguration>;
};

#define	g_tileSize			GlobalConfiguration::GetSingleton().GetTileSize()
#define	g_blenderMode		GlobalConfiguration::GetSingleton().GetBlenderMode()
#define g_accelerator		GlobalConfiguration::GetSingleton().GetAccelerator()
#define g_integrator		GlobalConfiguration::GetSingleton().GetIntegrator()
#define g_threadCnt			GlobalConfiguration::GetSingleton().GetThreadCnt()
#define g_samplePerPixel	GlobalConfiguration::GetSingleton().GetSamplePerPixel()
#define g_resourcePath		GlobalConfiguration::GetSingleton().GetResourcePath()
#define g_outputFileName	GlobalConfiguration::GetSingleton().GetOutputFileName()
#define g_resultResollution GlobalConfiguration::GetSingleton().GetResultResolution()