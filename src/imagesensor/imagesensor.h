/*
   FileName:      sortoutput.h

   Created Time:  2015-09-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_SORTOUTPUT
#define SORT_SORTOUTPUT

// somehow, pre-decleration will crash the program on Mac.
#include "spectrum/spectrum.h"
#include "utility/propertyset.h"
#include "texture/rendertarget.h"
#include "utility/multithread/multithread.h"

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

		m_mutex = new PlatformMutex*[m_width];
		for( int i = 0 ; i < m_width ; ++i )
			m_mutex[i] = new PlatformMutex[m_height];
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
		m_mutex[x][y].Lock();
		Spectrum _color = m_rendertarget.GetColor(x, y);
		m_rendertarget.SetColor(x, y, _color + color);
		m_mutex[x][y].Unlock();
	}

protected:
	int m_width;
	int m_height;
    
	// the mutex
	PlatformMutex**	m_mutex;

	// the render target
	RenderTarget m_rendertarget;
};

#endif