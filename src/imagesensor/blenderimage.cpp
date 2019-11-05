/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <mutex>
#include "blenderimage.h"
#include "core/globalconfig.h"
#include "core/path.h"

static std::mutex g_cntLock;

void BlenderImage::StorePixel( int x , int y , const Spectrum& color , const Render_Task& rt ){
    if (!m_sharedMemory.sharedmemory.bytes)
        return;

    int tile_w = rt.GetTileSize().x;
    int tile_size = g_tileSize * g_tileSize;
    int x_off = (int)(rt.GetTopLeft().x / g_tileSize);
    int y_off = (int)(floor((m_height - 1 - rt.GetTopLeft().y) / (float)g_tileSize));
    int tile_offset = y_off * m_tilenum_x + x_off;
    int offset = 4 * tile_offset * tile_size;

    // get the data pointer
    float* data = (float*)(m_sharedMemory.sharedmemory.bytes + m_header_offset);

    // get offset
    int inner_offset = offset + 4 * (x - rt.GetTopLeft().x + (g_tileSize - 1 - (y - rt.GetTopLeft().y)) * tile_w);

    // copy data
    data[ inner_offset ] = color.GetR();
    data[ inner_offset + 1 ] = color.GetG();
    data[ inner_offset + 2 ] = color.GetB();
    data[ inner_offset + 3 ] = 1.0f;

    // for final update
    {
        std::lock_guard<spinlock_mutex> lock(m_mutex[y*m_width+x]);
        Spectrum _color = m_rendertarget.GetColor(x,y);
        m_rendertarget.SetColor(x, y, color+_color);
    }
}

void BlenderImage::FinishTile( int tile_x , int tile_y , const Render_Task& rt ){
    if (!m_sharedMemory.sharedmemory.bytes)
        return;

    m_sharedMemory.sharedmemory.bytes[tile_y * m_tilenum_x + tile_x] = 1;

	std::lock_guard<std::mutex> lock(g_cntLock);
	m_sharedMemory.sharedmemory.bytes[m_sharedMemory.sharedmemory.size - 2] = (int)((++m_finishedTileCnt) / (float)( m_tilenum_x * m_tilenum_y ) * 100.0f);
}

void BlenderImage::PreProcess(){
    // create shared memory
    m_tilenum_x = (int)(ceil(g_resultResollutionWidth / (float)g_tileSize));
    m_tilenum_y = (int)(ceil(g_resultResollutionHeight / (float)g_tileSize));
    m_header_offset = m_tilenum_x * m_tilenum_y;
    m_final_update_flag_offset = m_header_offset * g_tileSize * g_tileSize * 4 * sizeof(float) * 2 + m_header_offset + 1;

    int size = m_header_offset * g_tileSize * g_tileSize * 4 * sizeof(float) * 2    // image size
             + m_header_offset                                                      // header size
             + 2;                                                                   // progress data and final update flag

    m_sharedMemory.CreateSharedMemory(GetFilePathInResourceFolder("sharedmem.bin"), size, SharedMmeory_All);
    auto& sm = m_sharedMemory.sharedmemory;
    // clear the memory first
    if (sm.bytes)
        memset(sm.bytes, 0, sm.size);
}

void BlenderImage::PostProcess(){
    // perform a copy from render target to shared memory
    float* data = (float*)(m_sharedMemory.sharedmemory.bytes + m_header_offset + m_header_offset * g_tileSize * g_tileSize * 4 * sizeof(float));

    auto offset = 0;
    for (auto i = 0; i < m_rendertarget.GetHeight(); ++i)
        for (auto j = 0; j < m_rendertarget.GetWidth(); ++j){
            const Spectrum& c = m_rendertarget.GetColor(j, m_rendertarget.GetHeight() - i - 1);
            data[offset++] = c.GetR();
            data[offset++] = c.GetG();
            data[offset++] = c.GetB();
            data[offset++] = 1.0f;
        }

    // signal a final update
    m_sharedMemory.sharedmemory.bytes[m_final_update_flag_offset] = 1;

    ImageSensor::PostProcess();
}
