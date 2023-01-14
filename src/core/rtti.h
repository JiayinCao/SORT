/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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

#include <unordered_map>
#include <algorithm>
#include "core/singleton.h"
#include "core/log.h"
#include "core/strid.h"

//! @brief  This class has no member field. Its only purpose is to instancing class object.
template<class T>
class FactoryMethod{
public:
    virtual std::shared_ptr<T> CreateSharedInstance() const = 0;
    virtual std::unique_ptr<T> CreateUniqueInstance() const = 0;
};

//! @brief      Class Factory is responsible for creating instances based on names.
template<class T>
class Factory : public Singleton<Factory<T>>{
    using FACTORY_MAP = std::unordered_map<StringID,FactoryMethod<T>*>;

public:
    //! @brief  Create a shared instance of a specific type based on class name.
    //!
    //! @return     Return a reference of a newly created instance.
    auto CreateSharedType( const StringID sid ) const{
        auto it = m_factoryMap.find(sid);
        return it == m_factoryMap.end() ? nullptr : it->second->CreateSharedInstance();
    }

    //! @brief  Create an unique instance of a specific type based on class name.
    //!
    //! @return     Return a reference of a newly created instance.
    auto CreateUniqueType( const StringID sid ) const{
        auto it = m_factoryMap.find(sid);
        return it == m_factoryMap.end() ? nullptr : it->second->CreateUniqueInstance();
    }

    //! @brief  Get the container mapping from string to factory method.
    //!
    //! It is not perfect to return a reference of the class member. Since this class is not directly
    //! exposed to upper level code, no code should directly call this interface.
    //!
    //! @return     Return the reference of the container.
    FACTORY_MAP& GetFactoryMap(){
        return m_factoryMap;
    }

private:
    /**< Container for the factory methods. */
    FACTORY_MAP     m_factoryMap;

    //! @brief  Make sure constructor is private.
    Factory(){}
    //! @brief  Make sure copy constructor is private.
    Factory(const Factory<T>&) = delete;

    friend class Singleton<Factory>;
};

//! @brief  Instance a class type based on name.
//!
//! @param  name        Name of the class. This has to match what is defined in python plugin.
//! @return             Shared pointer holding the instance.
template<class T>
std::shared_ptr<T> MakeSharedInstance( const StringID sid){
    return std::shared_ptr<T>(Factory<T>::GetSingleton().CreateSharedType(sid));
}

//! @brief  Instance a class and an unique pointer is returned.
//!
//! @param  name        Name of the class. This has to match what is defined in python plugin.
//! @return             An unique pointer pointing to the instance.
template<class T>
std::unique_ptr<T> MakeUniqueInstance( const StringID sid ) {
    return std::unique_ptr<T>(Factory<T>::GetSingleton().CreateUniqueType(sid));
}

#define DEFINE_RTTI( T , B )     class T##FactoryMethod : public FactoryMethod<B>\
{public: \
    T##FactoryMethod(){\
        StringID sid(#T);\
        auto& factoryMap = Factory<B>::GetSingleton().GetFactoryMap();\
        if( factoryMap.count(sid) ){\
            slog( WARNING , GENERAL , "The class with specific name of %s already exxisted." , #T );\
            return;\
        }\
        factoryMap[sid] = this;\
    }\
    std::shared_ptr<B> CreateSharedInstance() const { return std::make_shared<T>(); }\
    std::unique_ptr<B> CreateUniqueInstance() const { return std::make_unique<T>(); }\
};\
inline static T::T##FactoryMethod g_factoryMethod##T;
