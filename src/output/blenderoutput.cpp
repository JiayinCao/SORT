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

#include "blenderoutput.h"
#include "multithread/taskqueue.h"

#include <Windows.h>

float*	result_buffer = 0;
int		result_offset = 0;
HANDLE hMapFile;

// allocate memory in sort
void BlenderOutput::SetImageSize( int w , int h )
{
	SORTOutput::SetImageSize( w , h );
}

// store pixel information
void BlenderOutput::StorePixel( int x , int y , const Spectrum& color , const RenderTask& rt )
{
	int w = m_width;
	int h = m_height;
	int mod = h % 64;
	int w_tile = ceil( w / 64.0f );
	int h_tile = ceil( h / 64.0f );

	int tile_w = rt.width;
	int tile_size = 64 * 64;
	int x_off = rt.ori_x / 64;
	int y_off = floor( (h - 1 - rt.ori_y ) / 64.0f ) ;
	int tile_offset = y_off * w_tile + x_off;
	int offset = 4 * tile_offset * tile_size;
	float* data = (float*)((char*)result_buffer+result_offset);

	int inner_offset = offset + 4 * ( x - rt.ori_x + ( 63 - ( y - rt.ori_y ) ) * tile_w );

	data[ inner_offset ] = color.GetR();
	data[ inner_offset + 1 ] = color.GetG();
	data[ inner_offset + 2 ] = color.GetB();
	data[ inner_offset + 3 ] = 1.0f;
}

// finish image tile
void BlenderOutput::FinishTile( int tile_x , int tile_y , const RenderTask& rt )
{
	int w_tile = ceil( m_width / 64.0f );
	int h_tile = ceil( m_height / 64.0f );
	int tile_offset = tile_y * w_tile + tile_x;

	((char*)result_buffer)[ tile_offset ] = 1;
}

// pre process
void BlenderOutput::PreProcess()
{
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		"SORTBLEND_SHAREMEM");               // name of mapping object

	if (hMapFile == NULL)
	{
		cout<<GetLastError()<<endl;
		cout<<"Create."<<endl;
		return;
	}
	int x_tile = ceil( m_width / 64.0f );
	int y_tile = ceil( m_height / 64.0f );
	result_offset = x_tile * y_tile;
	int size = result_offset * 64 * 64 * 4 * sizeof(float) + result_offset;
	result_buffer = (float*) MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_WRITE, // read/write permission
		0,
		0,
		size );

	if (result_buffer == NULL)
	{
		cout<<GetLastError()<<endl;
		cout<<"Map."<<endl;
		CloseHandle(hMapFile);

		return;
	}

	// clear header first
	memset( result_buffer , 0 , result_offset );
}

// post process
void BlenderOutput::PostProcess()
{
	UnmapViewOfFile(result_buffer);

	CloseHandle(hMapFile);
}