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

#include "sort.h"

#if defined(SORT_IN_WINDOWS)

#include <Windows.h>

class WinSharedMemory
{
public:
	// default constructor
	WinSharedMemory();

	// Initialize shared memory
	void CreateSharedMemory(const std::string& sm_name, int size, unsigned type);

	// Release share memory resource
	void ReleaseSharedMemory();

	// shared memory data
	SharedMemory	sharedmemory;

// platform dependent fields, still invisible from others
private:
	HANDLE	hMapFile;
};

#endif

#define PlatformSharedMemory	WinSharedMemory
