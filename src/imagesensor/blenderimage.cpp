/*
   FileName:      blenderoutput.cpp

   Created Time:  2015-09-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "blenderimage.h"
#include "platform/multithread/multithread.h"
#include "managers/smmanager.h"

// tile size
extern int g_iTileSize;

// allocate memory in sort
void BlenderImage::SetImageSize( int w , int h )
{
	ImageSensor::SetImageSize( w , h );
}

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

	m_sharedMemory = SMManager::GetSingleton().GetSharedMemory("SORTBLEND_SHAREMEM");
}