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

/*

#include <OSL/oslexec.h>

using namespace OSL;


class SORTTextureSystem : public OSL::TextureSystem{
public:
    SORTTextureSystem() = default;
    ~SORTTextureSystem() = default;

    bool attribute (OSL::string_view name, OSL::TypeDesc type, const void *val) override{ return true; }
    // Shortcuts for common types
    bool attribute (OSL::string_view name, int val) override{ return true; }
    bool attribute (OSL::string_view name, float val) override{ return true; }
    bool attribute (OSL::string_view name, double val) override{ return true; }
    bool attribute (OSL::string_view name, OSL::string_view val) override{ return true; }

    /// Get the named attribute, store it in value.
    bool getattribute (OSL::string_view name, OSL::TypeDesc type, void *val) const override{ return true; }
    // Shortcuts for common types
    bool getattribute(OSL::string_view name, int& val) const override{ return true; }
    bool getattribute(OSL::string_view name, float& val) const override{ return true; }
    bool getattribute(OSL::string_view name, double& val) const override{ return true; }
    bool getattribute(OSL::string_view name, char** val) const override{ return true; }
    bool getattribute(OSL::string_view name, std::string& val) const override{ return true; }

    Perthread* get_perthread_info(Perthread* thread_info = NULL) override { return nullptr; }

    Perthread* create_thread_info() override { return nullptr; }
    void destroy_thread_info(Perthread* threadinfo) override {}

    TextureHandle * get_texture_handle (OSL::ustring filename, Perthread *thread_info=NULL) override {
        return nullptr;
    }
    bool good(TextureHandle* texture_handle) override {
        return true;
    }

    bool texture (ustring filename, TextureOpt &options,
                    float s, float t, float dsdx, float dtdx,
                    float dsdy, float dtdy,
                    int nchannels, float *result,
                    float *dresultds=NULL, float *dresultdt=NULL) override {
                    return true;
    }

    bool texture (TextureHandle *texture_handle,
                Perthread *thread_info, TextureOpt &options,
                float s, float t, float dsdx, float dtdx,
                float dsdy, float dtdy,
                int nchannels, float *result,
                float *dresultds=NULL, float *dresultdt=NULL) override {
                    return true;
                }

    bool texture (ustring filename, OIIO::TextureOptBatch &options,
                          OIIO::Tex::RunMask mask, const float *s, const float *t,
                          const float *dsdx, const float *dtdx,
                          const float *dsdy, const float *dtdy,
                          int nchannels, float *result,
                          float *dresultds=nullptr,
                          float *dresultdt=nullptr) override {
                              return true;
                          }
    bool texture (TextureHandle *texture_handle,
                          Perthread *thread_info, OIIO::TextureOptBatch &options,
                          OpenEXR::Tex::RunMask mask, const float *s, const float *t,
                          const float *dsdx, const float *dtdx,
                          const float *dsdy, const float *dtdy,
                          int nchannels, float *result,
                          float *dresultds=nullptr,
                          float *dresultdt=nullptr) override {
                              return true;
                          }

    bool texture (ustring filename, TextureOptions &options,
                          Runflag *runflags, int beginactive, int endactive,
                          VaryingRef<float> s, VaryingRef<float> t,
                          VaryingRef<float> dsdx, VaryingRef<float> dtdx,
                          VaryingRef<float> dsdy, VaryingRef<float> dtdy,
                          int nchannels, float *result,
                          float *dresultds=NULL, float *dresultdt=NULL) {
                              return true;
                          }
    bool texture (TextureHandle *texture_handle,
                          Perthread *thread_info, TextureOptions &options,
                          Runflag *runflags, int beginactive, int endactive,
                          VaryingRef<float> s, VaryingRef<float> t,
                          VaryingRef<float> dsdx, VaryingRef<float> dtdx,
                          VaryingRef<float> dsdy, VaryingRef<float> dtdy,
                          int nchannels, float *result,
                          float *dresultds=NULL, float *dresultdt=NULL) {
                              return true;
                          }

private:
    void operator delete(void* todel) { ::delete ((char*)todel); }
};

*/