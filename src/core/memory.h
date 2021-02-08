/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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

#include <list>
#include <memory>
#include "core/sassert.h"

// 32KB memory for each memory block by default.
#define MEM_BLOCK_SIZE                  32768
// Memory alignment size
#define MEM_ALIGN_SIZE                  4u
#define MEM_SIZE_ALIGNED(s)             (((s+MEM_ALIGN_SIZE-1)/MEM_ALIGN_SIZE) * MEM_ALIGN_SIZE)

//! @brief  Memory block allocated in MemoryAllocator.
class MemoryBlock {
public:
    /**< Real data of the memory block. */
    std::unique_ptr<char[]> m_data = std::make_unique<char[]>(MEM_BLOCK_SIZE);
    /**< Current position of available memory. */
    unsigned int            m_start = 0;
};

//! @brief   MemoryAllocator is responsible for allocating small trunk of memory in a fast way.
/**
 * The main purpose of MemoryAllocator is to allocate small trunk of memory in a faster way than
 * naive 'new' method. MemoryAllocator achieves this by allocating a memory pool beforehand.
 * With a memory pool, memory allocation through MemoryAllocator benefits from avoiding page
 * allocation under the hood. And the other benefit it gets is memory deallocation is not needed
 * any more. There is no way to release allocated memory block for simplicity. Although SORT is
 * memory protected by std::unique_ptrs, there is still a possibility for it to leak memory if
 * a std::unique_ptr is allocated through this memory allocator. It is up to the higher level
 * code to make sure it doesn't happen.
 */
class MemoryAllocator {
public:
    //! @brief  Allocate memory from memory pool.
    //!
    //! @param  cnt     Number of instance it needs allocate.
    //! @return         The pointer pointing to memory that could hold the instance(s).
    template<class T>
    T*  Allocate(unsigned int cnt = 1u) {
        unsigned int size_to_allocate = (unsigned int)(sizeof(T) * cnt);
        sAssert( size_to_allocate <= MEM_BLOCK_SIZE , MEMORY );
        MemoryBlock* currentBlock = m_availableBlocks.size() ? m_availableBlocks.front().get() : nullptr;
        if (IS_PTR_INVALID(currentBlock) || (currentBlock->m_start + size_to_allocate > MEM_BLOCK_SIZE)) {
            if (currentBlock) {
                auto block = std::move(m_availableBlocks.front());
                m_availableBlocks.pop_front();
                m_usedBlocks.push_back(std::move(block));
            }

            if( m_availableBlocks.empty() )
                m_availableBlocks.push_front(std::make_unique<MemoryBlock>());
            currentBlock = m_availableBlocks.front().get();
        }
        auto ret = currentBlock->m_data.get() + currentBlock->m_start;
        currentBlock->m_start += MEM_SIZE_ALIGNED(size_to_allocate);
        return (T*)ret;
    }

    //! @brief  Reset the memory allocator.
    void Reset() {
        m_availableBlocks.splice(m_availableBlocks.begin(), m_usedBlocks);
        for (auto& block : m_availableBlocks)
            block->m_start = 0;
    }

private:
    /**< Currently availably blocks, the front one is the one being used. */
    std::list<std::unique_ptr<MemoryBlock>>  m_availableBlocks;
    /**< Blocks that has been consumed. */
    std::list<std::unique_ptr<MemoryBlock>>  m_usedBlocks;
};

//! @brief Get static allocator.
//!
//! @return Thread based memory allocator.
SORT_FORCEINLINE ::MemoryAllocator& GetStaticAllocator() {
    // Each thread has their own memory allocator.
    static thread_local ::MemoryAllocator memoryAllocator;
    return memoryAllocator;
}

#define SORT_MALLOC(T)              new (GetStaticAllocator().Allocate<T>()) T
#define SORT_MALLOC_ARRAY(T,cnt)    new (GetStaticAllocator().Allocate<T>(cnt)) T
#define SORT_CLEAR_MEMPOOL()        GetStaticAllocator().Reset()

#define SORT_MALLOC_PROXY(A, T)     new (A->Allocate<T>()) T

//! @brief  A helper utility function that allocate memory with alignment.
//!
//! @param size         The size of the memory to be allocated.
//! @param alignment    The bytes to be aligned.
//! @return             The returned pointer pointing to allocated memory.
SORT_FORCEINLINE void* malloc_aligned( unsigned int size , unsigned int alignment ){
    void* ret = nullptr;
    if( 0 == size )
        return ret;

#ifdef SORT_IN_WINDOWS
    ret = _aligned_malloc( size , alignment );
#else
    if( 0 != posix_memalign( &ret , alignment , size ) )
        return nullptr;
#endif

    sAssert( ( ((uintptr_t)ret) & (alignment-1) ) == 0 , MEMORY );
    
    return ret;
}

//! @brief  A helper function that frees the memory allocated with the interface defined above.
//!
//! @param  p           The address of memory allocated.
SORT_FORCEINLINE void free_aligned( void* p ){
    if( p ){
#ifdef SORT_IN_WINDOWS
        _aligned_free(p);
#else
        free(p);
#endif
    }
}