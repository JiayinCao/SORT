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

#include "resource.h"
#include "core/strhelper.h"
#include "core/log.h"

void ResourceManager::ReleaseAllResources() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto resource : m_resources) {
        slog(INFO, RESOURCE, stringFormat("Releasing resource %s.", resource->GetName().c_str()));
        resource->Release();
    }
    m_resources.clear();
}

void ResourceManager::Register(class Resource* resource){
    sAssert(resource != nullptr, RESOURCE);
    std::lock_guard<std::mutex> lock(m_mutex);
    sAssert(0 == m_resources.count(resource), RESOURCE);
    m_resources.insert(resource);
}

void ResourceManager::Unregister(class Resource* resource) {
    sAssert(resource != nullptr, RESOURCE);
    std::lock_guard<std::mutex> lock(m_mutex);
    sAssert(0 != m_resources.count(resource), RESOURCE);
    m_resources.erase(resource);
}