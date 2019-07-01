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

#include "core/define.h"

#if defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)

#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "sharedmemory.h"
#include "core/log.h"

void MmapSharedMemory::CreateSharedMemory( const std::string& name , int size , unsigned type ){
    fd = open(name.c_str(), O_RDWR , 0 );
    if( fd == -1 )
    {
        slog( WARNING , GENERAL , "Failed to load shared memory file %s " , name.c_str() );
        return;
    }

    // map a new file
    sharedmemory.bytes = (char*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
    if( sharedmemory.bytes == MAP_FAILED)
        sharedmemory.bytes = 0;
    sharedmemory.size = size;
}

// Release share memory resource
MmapSharedMemory::~MmapSharedMemory(){
    if( sharedmemory.bytes != 0 )
        munmap(sharedmemory.bytes, sharedmemory.size);
    if( fd != -1 )
        close(fd);
}

#endif
