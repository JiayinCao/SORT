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

// somehow, pre-decleration will crash the program on Mac.
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "texture/rendertarget.h"
#include "utility/multithread/multithread.h"
#include <mutex>

// pre-decleration
class RenderTask;

// generate output
class ImageSensor : public PropertySet<ImageSensor>
{
public:
    ImageSensor(){
        _registerAllProperty();
    }
    virtual ~ImageSensor(){}
    
	// pre process
    virtual void PreProcess()
	{
		m_rendertarget.SetSize(m_width, m_height);

		m_mutex = new PlatformSpinlockMutex*[m_width];
		for( int i = 0 ; i < m_width ; ++i )
			m_mutex[i] = new PlatformSpinlockMutex[m_height];
	}

	// set image size
	virtual void SetSensorSize( int w , int h )
	{
		m_width = w;
		m_height = h;
	}

	// finish image tile
	virtual void FinishTile( int tile_x , int tile_y , const RenderTask& rt ){}

    // store pixel information
    virtual void StorePixel( int x , int y , const Spectrum& color , const RenderTask& rt ) = 0;
    
	// post process
    virtual void PostProcess(){
		// delete the mutex
		for( int i = 0 ; i < m_width ; ++i )
			delete[] m_mutex[i];
		delete[] m_mutex;
	}
    
    // get width
    unsigned GetWidth() const {
        return m_width;
    }
    
    // get height
    unsigned GetHeight() const {
        return m_height;
    }

	// add radiance
	virtual void UpdatePixel(int x, int y, const Spectrum& color)
	{
        std::lock_guard<PlatformSpinlockMutex> lock(m_mutex[x][y]);
		Spectrum _color = m_rendertarget.GetColor(x, y);
		m_rendertarget.SetColor(x, y, _color + color);
	}

protected:
	int m_width;
	int m_height;
    
	// the mutex
	PlatformSpinlockMutex**	m_mutex;

	// the render target
	RenderTarget m_rendertarget;
};
