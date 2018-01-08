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

#define SharedMemory_Read 0x01
#define	SharedMemory_Write 0x02
#define SharedMmeory_All ( SharedMemory_Read | SharedMemory_Write )

struct SharedMemory
{
	char*	bytes;
	int		size;

	SharedMemory()
	{
		bytes = 0;
		size = 0;
	}
};

#if defined(SORT_IN_WINDOWS)
	#include "winsharedmemory.h"
#elif defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)
    #include "mmapsharedmemory.h"
#endif
