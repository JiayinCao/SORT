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

#include "sort.h"

#if defined(SORT_IN_WINDOWS)

#include "sharedmemory.h"
#include "utility/log.h"
#include "utility/strhelper.h"

// default constructor
WinSharedMemory::WinSharedMemory()
{
	hMapFile = 0;
}

void WinSharedMemory::CreateSharedMemory( const std::string& name , int size , unsigned type )
{
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	// read/write access
		FALSE,					// do not inherit the name
		name.c_str());		// name of mapping object

	if (hMapFile == NULL)
	{
        slog( WARNING , MATERIAL , stringFormat( "Creating shared memory failed %s" , name.c_str() ) );
		return;
	}
	sharedmemory.size = size;
	sharedmemory.bytes = (char*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_WRITE, // read/write permission
		0,
		0,
		size);

	if (sharedmemory.bytes == NULL)
	{
		sharedmemory.size = 0;
		CloseHandle(hMapFile);
        slog( WARNING , MATERIAL , stringFormat( "Creating shared memory failed %s" , name.c_str() ) );
	}
}

// Release share memory resource
void WinSharedMemory::ReleaseSharedMemory()
{
	// unmap the view
	if (sharedmemory.bytes)
		UnmapViewOfFile(sharedmemory.bytes);

	// close file handle
	if (hMapFile)
	{
		CloseHandle(hMapFile);
		hMapFile = 0;
	}
}

#endif
