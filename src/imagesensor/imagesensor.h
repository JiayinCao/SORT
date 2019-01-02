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

#include "spectrum/spectrum.h"
#include "texture/rendertarget.h"
#include "task/render_task.h"
#include "core/thread.h"
#include <mutex>

class RenderTask;

// generate output
class ImageSensor
{
public:
	ImageSensor( int w , int h ) : m_width(w) , m_height(h) {}
	virtual ~ImageSensor(){}
    
	// pre process
    virtual void PreProcess(){
		m_rendertarget.SetSize(m_width, m_height);
		m_mutex = std::make_unique<spinlock_mutex[]>( m_width * m_height );
	}

	// finish image tile
	virtual void FinishTile( int tile_x , int tile_y , const Render_Task& rt ){}

    // store pixel information
    virtual void StorePixel( int x , int y , const Spectrum& color , const Render_Task& rt ) = 0;
    
    // get width
    unsigned GetWidth() const {
        return m_width;
    }
    
    // get height
    unsigned GetHeight() const {
        return m_height;
    }

	// post process
    virtual void PostProcess(){}

	// add radiance
	virtual void UpdatePixel(int x, int y, const Spectrum& color){
        std::lock_guard<spinlock_mutex> lock(m_mutex[y * m_width + x]);
		Spectrum _color = m_rendertarget.GetColor(x, y);
		m_rendertarget.SetColor(x, y, _color + color);
	}

protected:
	const int m_width;
	const int m_height;
    
	// the mutex
	std::unique_ptr<spinlock_mutex[]>	m_mutex;

	// the render target
	RenderTarget m_rendertarget;
};
