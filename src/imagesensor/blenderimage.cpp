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

#include "blenderimage.h"
#include "utility/multithread/multithread.h"
#include "managers/smmanager.h"
#include <mutex>

// tile size
extern int g_iTileSize;

// store pixel information
void BlenderImage::StorePixel( int x , int y , const Spectrum& color , const RenderTask& rt )
{
	if (!m_sharedMemory.bytes)
		return;

	int tile_w = rt.size.x;
	int tile_size = g_iTileSize * g_iTileSize;
	int x_off = (int)(rt.ori.x / g_iTileSize);
	int y_off = (int)(floor((m_height - 1 - rt.ori.y) / (float)g_iTileSize));
	int tile_offset = y_off * m_tilenum_x + x_off;
	int offset = 4 * tile_offset * tile_size;

	// get the data pointer
	float* data = (float*)(m_sharedMemory.bytes + m_header_offset);

	// get offset
	int inner_offset = offset + 4 * (x - rt.ori.x + (g_iTileSize - 1 - (y - rt.ori.y)) * tile_w);

	// copy data
	data[ inner_offset ] = color.GetR();
	data[ inner_offset + 1 ] = color.GetG();
	data[ inner_offset + 2 ] = color.GetB();
	data[ inner_offset + 3 ] = 1.0f;

	// for final update
    {
        std::lock_guard<PlatformSpinlockMutex> lock(m_mutex[x][y]);
        Spectrum _color = m_rendertarget.GetColor(x,y);
        m_rendertarget.SetColor(x, y, color+_color);
    }
}

// finish image tile
void BlenderImage::FinishTile( int tile_x , int tile_y , const RenderTask& rt )
{
	if (!m_sharedMemory.bytes)
		return;

	m_sharedMemory.bytes[tile_y * m_tilenum_x + tile_x] = 1;
}

// pre process
void BlenderImage::PreProcess()
{
	m_tilenum_x = (int)(ceil(m_width / (float)g_iTileSize));
	m_tilenum_y = (int)(ceil(m_height / (float)g_iTileSize));
	m_header_offset = m_tilenum_x * m_tilenum_y;
	m_final_update_flag_offset = m_header_offset * g_iTileSize * g_iTileSize * 4 * sizeof(float) * 2 + m_header_offset + 1;

	m_sharedMemory = SMManager::GetSingleton().GetSharedMemory("sharedmem.bin");

	ImageSensor::PreProcess();
}

// post process
void BlenderImage::PostProcess()
{
	// perform a copy from render target to shared memory
	float* data = (float*)(m_sharedMemory.bytes + m_header_offset + m_header_offset * g_iTileSize * g_iTileSize * 4 * sizeof(float));

	int offset = 0;
	for (int i = 0; i < (int)m_rendertarget.GetHeight(); ++i)
		for (int j = 0; j < (int)m_rendertarget.GetWidth(); ++j)
		{
			const Spectrum& c = m_rendertarget.GetColor(j, m_rendertarget.GetHeight() - i - 1);
			data[offset++] = c.GetR();
			data[offset++] = c.GetG();
			data[offset++] = c.GetB();
			data[offset++] = 1.0f;
		}

	// signal a final update
	m_sharedMemory.bytes[m_final_update_flag_offset] = 1;

	ImageSensor::PostProcess();
}
