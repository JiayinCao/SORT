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

#include <OSL/oslexec.h>
#include "core/sassert.h"
#include "texture/imagetexture2d.h"
#include "texture/imagetexture3d.h"

// Since only one single file will include this header file, it is OK to use the namespace in a header file.
using namespace OSL;
using namespace OIIO;

class Mesh;

//! @brief  Texture System per-thread information.
/**
 * This is used to pass data from the host to texture access method.
 */
struct SORTTextureThreadInfo {
    const Mesh* mesh = nullptr;
};

//! @brief  Texture system implementation in SORT.
/**
 * This is the connection between OSL texturing system and SORT texturing system.
 * In order to take full control of the rendering, SORT doesn't use the OSL implementation of texturing system.
 * The current texturing system in SORT is extremely naive and simple.
 *
 * Most of the following interfaces are not implemented for now due to the simplicity of SORT texturing system.
 * There will be more features added in the future.
 */
class SORTTextureSystem : public TextureSystem {
public:
    bool attribute(string_view name, TypeDesc type, const void *val) override { return true; }
    bool attribute(string_view name, int val) override { return true; }
    bool attribute(string_view name, float val) override { return true; }
    bool attribute(string_view name, double val) override { return true; }
    bool attribute(string_view name, string_view val) override { return true; }

    bool getattribute(string_view name, TypeDesc type, void *val) const override { return true; }
    bool getattribute(string_view name, int& val) const override { return true; }
    bool getattribute(string_view name, float& val) const override { return true; }
    bool getattribute(string_view name, double& val) const override { return true; }
    bool getattribute(string_view name, char** val) const override { return true; }
    bool getattribute(string_view name, std::string& val) const override { return true; }

    TextureHandle * get_texture_handle(ustring filename, Perthread *thread_info = NULL) override;
    bool good(TextureHandle* texture_handle) override;

    bool texture(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOpt &options,
        float s, float t, float dsdx, float dtdx,
        float dsdy, float dtdy,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override;

    bool texture3d(ustring filename, TextureOpt& options,
        const Imath::V3f& P, const Imath::V3f& dPdx,
        const Imath::V3f& dPdy, const Imath::V3f& dPdz,
        int nchannels, float* result,
        float* dresultds = NULL, float* dresultdt = NULL,
        float* dresultdr = NULL) override;

    std::string resolve_filename(const std::string &filename) const override {
        return "";
    }

    bool get_texture_info(ustring filename, int subimage,
        ustring dataname, TypeDesc datatype, void *data) override {
        return false;
    }
    bool get_texture_info(TextureHandle *texture_handle,
        Perthread *thread_info, int subimage,
        ustring dataname, TypeDesc datatype, void *data) override {
        return false;
    }

    bool get_texture_info(TextureHandle *texture_handle, int subimage,
        ustring dataname, TypeDesc datatype, void *data) override {
        return false;
    }

    bool get_imagespec(ustring filename, int subimage, ImageSpec &spec) override {
        return false;
    }
    bool get_imagespec(TextureHandle *texture_handle, Perthread *thread_info, int subimage, ImageSpec &spec) override {
        return false;
    }

    const ImageSpec *imagespec(ustring filename, int subimage = 0) override {
        return nullptr;
    }
    const ImageSpec *imagespec(TextureHandle *texture_handle, Perthread *thread_info = NULL, int subimage = 0) override {
        return nullptr;
    }

    bool get_texels(ustring filename, TextureOpt &options,
        int miplevel, int xbegin, int xend,
        int ybegin, int yend, int zbegin, int zend,
        int chbegin, int chend,
        TypeDesc format, void *result) override {
        return false;
    }
    bool get_texels(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOpt &options,
        int miplevel, int xbegin, int xend,
        int ybegin, int yend, int zbegin, int zend,
        int chbegin, int chend,
        TypeDesc format, void *result) override {
        return false;
    }

    std::string geterror() const override { return ""; }
    std::string getstats(int level = 1, bool icstats = true) const override { return ""; }
    void invalidate(ustring filename) override {}
    void invalidate_all(bool force = false) override {}
    void reset_stats() override {}
    void close(ustring filename) override {}
    void close_all() override {}

    // ---------------------------------------------------------------------------------------------------------
    // Unsupported interfaces in SORT
    bool texture3d(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOpt &options,
        const Imath::V3f &P, const Imath::V3f &dPdx,
        const Imath::V3f &dPdy, const Imath::V3f &dPdz,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL,
        float *dresultdr = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling 3d texture for now.");
        return false;
    }

    bool texture3d(ustring filename,
        TextureOptBatch &options, Tex::RunMask mask,
        const float *P, const float *dPdx,
        const float *dPdy, const float *dPdz,
        int nchannels, float *result,
        float *dresultds = nullptr, float *dresultdt = nullptr,
        float *dresultdr = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling 3d texture for now.");
        return false;
    }

    bool texture3d(TextureHandle *texture_handle,
        Perthread *thread_info,
        TextureOptBatch &options, Tex::RunMask mask,
        const float *P, const float *dPdx,
        const float *dPdy, const float *dPdz,
        int nchannels, float *result,
        float *dresultds = nullptr, float *dresultdt = nullptr,
        float *dresultdr = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling 3d texture for now.");
        return false;
    }

    bool texture3d(ustring filename, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<Imath::V3f> P,
        VaryingRef<Imath::V3f> dPdx,
        VaryingRef<Imath::V3f> dPdy,
        VaryingRef<Imath::V3f> dPdz,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL,
        float *dresultdr = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling 3d texture for now.");
        return false;
    }

    bool texture3d(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<Imath::V3f> P,
        VaryingRef<Imath::V3f> dPdx,
        VaryingRef<Imath::V3f> dPdy,
        VaryingRef<Imath::V3f> dPdz,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL,
        float *dresultdr = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling 3d texture for now.");
        return false;
    }

    bool shadow(ustring filename, TextureOpt &options,
        const Imath::V3f &P, const Imath::V3f &dPdx,
        const Imath::V3f &dPdy, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling shadow texture in SORT.");
        return false;
    }

    bool shadow(TextureHandle *texture_handle, Perthread *thread_info,
        TextureOpt &options,
        const Imath::V3f &P, const Imath::V3f &dPdx,
        const Imath::V3f &dPdy, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling shadow texture in SORT.");
        return false;
    }

    bool shadow(ustring filename,
        TextureOptBatch &options, Tex::RunMask mask,
        const float *P, const float *dPdx, const float *dPdy,
        float *result, float *dresultds = nullptr, float *dresultdt = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling shadow texture in SORT.");
        return false;
    }
    bool shadow(TextureHandle *texture_handle, Perthread *thread_info,
        TextureOptBatch &options, Tex::RunMask mask,
        const float *P, const float *dPdx, const float *dPdy,
        float *result, float *dresultds = nullptr, float *dresultdt = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling shadow texture in SORT.");
        return false;
    }

    bool shadow(ustring filename, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<Imath::V3f> P,
        VaryingRef<Imath::V3f> dPdx,
        VaryingRef<Imath::V3f> dPdy,
        float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling shadow texture in SORT.");
        return false;
    }
    bool shadow(TextureHandle *texture_handle, Perthread *thread_info,
        TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<Imath::V3f> P,
        VaryingRef<Imath::V3f> dPdx,
        VaryingRef<Imath::V3f> dPdy,
        float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling shadow texture in SORT.");
        return false;
    }

    bool environment(ustring filename, TextureOpt &options,
        const Imath::V3f &R, const Imath::V3f &dRdx,
        const Imath::V3f &dRdy, int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling environment texture in SORT.");
        return false;
    }

    bool environment(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOpt &options,
        const Imath::V3f &R, const Imath::V3f &dRdx,
        const Imath::V3f &dRdy, int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling environment texture in SORT.");
        return false;
    }

    bool environment(ustring filename,
        TextureOptBatch &options, Tex::RunMask mask,
        const float *R, const float *dRdx, const float *dRdy,
        int nchannels, float *result,
        float *dresultds = nullptr, float *dresultdt = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling environment texture in SORT.");
        return false;
    }
    bool environment(TextureHandle *texture_handle, Perthread *thread_info,
        TextureOptBatch &options, Tex::RunMask mask,
        const float *R, const float *dRdx, const float *dRdy,
        int nchannels, float *result,
        float *dresultds = nullptr, float *dresultdt = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling environment texture in SORT.");
        return false;
    }

    bool environment(ustring filename, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<Imath::V3f> R,
        VaryingRef<Imath::V3f> dRdx,
        VaryingRef<Imath::V3f> dRdy,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling environment texture in SORT.");
        return false;
    }
    bool environment(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<Imath::V3f> R,
        VaryingRef<Imath::V3f> dRdx,
        VaryingRef<Imath::V3f> dRdy,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling environment texture in SORT.");
        return false;
    }

    Perthread* get_perthread_info(Perthread* thread_info = NULL) override;
    Perthread* create_thread_info() override;
    void destroy_thread_info(Perthread* threadinfo) override;

    bool texture(ustring filename, TextureOpt &options,
        float s, float t, float dsdx, float dtdx,
        float dsdy, float dtdy,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling texture without texture handle, which is very slow.");
        return false;
    }

    bool texture(ustring filename, TextureOptBatch &options,
        Tex::RunMask mask, const float *s, const float *t,
        const float *dsdx, const float *dtdx,
        const float *dsdy, const float *dtdy,
        int nchannels, float *result,
        float *dresultds = nullptr,
        float *dresultdt = nullptr) override {
        sAssertMsg(false, MATERIAL, "No support for sampling texture without texture handle, which is very slow.");
        return false;
    }

    bool texture(ustring filename, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<float> s, VaryingRef<float> t,
        VaryingRef<float> dsdx, VaryingRef<float> dtdx,
        VaryingRef<float> dsdy, VaryingRef<float> dtdy,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "No support for sampling texture without texture handle, which is very slow.");
        return false;
    }

    bool texture(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOptBatch &options,
        Tex::RunMask mask, const float *s, const float *t,
        const float *dsdx, const float *dtdx,
        const float *dsdy, const float *dtdy,
        int nchannels, float *result,
        float *dresultds = nullptr,
        float *dresultdt = nullptr) override {
        sAssertMsg(false, MATERIAL, "Unimplemented interface of texture sampling.");
        return true;
    }

    bool texture(TextureHandle *texture_handle,
        Perthread *thread_info, TextureOptions &options,
        Runflag *runflags, int beginactive, int endactive,
        VaryingRef<float> s, VaryingRef<float> t,
        VaryingRef<float> dsdx, VaryingRef<float> dtdx,
        VaryingRef<float> dsdy, VaryingRef<float> dtdy,
        int nchannels, float *result,
        float *dresultds = NULL, float *dresultdt = NULL) override {
        sAssertMsg(false, MATERIAL, "Unimplemented interface of texture sampling.");
        return true;
    }

private:
    void operator delete(void* todel) { ::delete ((char*)todel); }

    // The texture pool
    std::unordered_map<std::string, std::unique_ptr<ImageTexture2D>> m_Texture2DPool;
};