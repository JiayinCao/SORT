/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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

#include <unordered_set>
#include "stream/stream.h"
#include "core/singleton.h"
#include <mutex>

//! @brief  Resource manager keeps track of all resource allocated in SORT.
/**
 * Upon creation of resources, each of them will be registered in their constructor so 
 * that it can be tracked. Resource manager will be responsible for releasing all 
 * allocated memory and data once the program terminates.
 */
class ResourceManager : public Singleton<ResourceManager> {
public:
    //! @brief  Release all allocated resources
    void ReleaseAllResources();

    //! @brief  Register resources.
    //!
    //! @param resource    Register the resource. Crash if it is already registered.
    void Register(class Resource* resource);

    //! @brief  Unregister resources.
    //!
    //! @param resource     Unregister the resource. Crash if there is no such a resource registered.
    void Unregister(class Resource* resource);

private:
    std::unordered_set<class Resource*>     m_resources;    /**< Containers holding all resources allocated in SORT. */
    std::mutex                              m_mutex;        /**< Mutex to make sure it is thread safe. */
};


//! @brief Basic resource unit in SORT.
/**
 * Resource is the basic unit in SORT. Resource can be a mesh, a light, a camera, etc.
 * Resource is maintained by a resource manager, who will be responsible for deallocating
 * the memory allocated. Each sub-system owns the resource has no obligation to release
 * the resources it owns.
 */
class Resource : public SerializableObject
{
public:
    //! @brief  Destructor of Accelerator, nothing is done in it.
    virtual ~Resource() {}

    //! Get the name of the resource
    //!
    //! @return         Current name of the resource.
    std::string     GetName() const { return m_name; }

    //! @brief  Set the name of the resource
    //!
    //! @param name     New name of the resource.
    void            SetName( const std::string& name ) { m_name = name; }

    //! Release allocated resource
    virtual void    Release() {}

private:
    std::string m_name = "unknown";     /**< Name of the resource, it is 'unknown' by default. */
};
