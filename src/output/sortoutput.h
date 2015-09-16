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

// pre-decleration
class RenderTask;

// generate output
class SORTOutput
{
public:
    SORTOutput(){}
    virtual ~SORTOutput(){}
    
	// pre process
	virtual void PreProcess() = 0;

	// allocate memory in sort
	virtual void SetImageSize( int w , int h ){ m_width = w; m_height = h; }

	// finish image tile
	virtual void FinishTile( int tile_x , int tile_y , const RenderTask& rt ){}

    // store pixel information
    virtual void StorePixel( int x , int y , const Spectrum& color , const RenderTask& rt ) = 0;
    
	// post process
	virtual void PostProcess() = 0;

public:
	int m_width;
	int m_height;
};

#endif