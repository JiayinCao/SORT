/*
   FileName:      blenderoutput.h

   Created Time:  2015-09-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_BLENDEROUTPUT
#define SORT_BLENDEROUTPUT

#include "imagesensor.h"
#include "texture/rendertarget.h"
#include "managers/smmanager.h"
#include "platform/multithread/multithread.h"

// generate output
class BlenderImage : public ImageSensor
{
public:
	// store pixel information
	virtual void StorePixel( int x , int y , const Spectrum& color , const RenderTask& rt );

	// finish image tile
	virtual void FinishTile( int tile_x , int tile_y , const RenderTask& rt );

	// pre process
	virtual void PreProcess();

	// post process
	virtual void PostProcess();

	// add radiance
	virtual void UpdatePixel(int x, int y, const Spectrum& color)
	{
		m_mutex[x][y].Lock();
		Spectrum _color = m_rendertarget.GetColor(x, y);
		m_rendertarget.SetColor(x, y, _color + color);
		m_mutex[x][y].Unlock();
	}

private:
	int				m_header_offset;
	int				m_final_update_flag_offset;
	int				m_tilenum_x;
	int				m_tilenum_y;

	SharedMemory	m_sharedMemory;

	// the mutex
	PlatformMutex**	m_mutex;
};

#endif