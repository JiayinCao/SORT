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

#include <random>
#include "spectrum/spectrum.h"
#include "core/memory.h"
#include "sampler/sampler.h"
#include "core/rtti.h"
#include "core/stats.h"
#include "core/profile.h"
#include "core/primitive.h"
#include "stream/stream.h"
#include "core/scene.h"

class   Ray;

//! @brief  Integrator is for esitimating radiance in rendering equation.
/**
 * This is the core of ray tracing rendering in SORT. There are different integrators supported in SORT.
 * Path tracing is the most well supported algorithm implemented in SORT for now. Bi-directional path tracing
 * is also well implemented with a few limitations like the lack of SSS and volumetric rendering.
 * There are also other experimental integrators implemented only for learning purposes.
 */
class   Integrator : public SerializableObject{
public:
    //! @brief  Empty virtual destructor.
    virtual ~Integrator(){}

    //! @brief  Evaluate the radiance along the opposite direction of the ray in the scene.
    //!
    //! The exact way of importance sampling could depend on the rendering algorithm itself.
    //! Each way of rendering has their unique features.
    //!
    //! @param  ray     The extent ray in rendering equation.
    //! @param  ps      The pixel samples. Currently not used.
    //! @param  scene   The rendering scene.
    //! @return         The spectrum of the radiance along the opposite direction of the ray.
    virtual Spectrum    Li( const Ray& ray , const PixelSample& ps , const Scene& scene) const = 0;

    //! @brief Pre-process before rendering.
    //!
    //! By default , nothing is done in pre-process some integrator, such as Photon Mapping use pre-process step to
    //! generate some neccessary infomation by latter stage.
    virtual void PreProcess(const Scene& scene) {}

    //! @brief  Some integrator have a post process step.
    virtual void PostProcess() {}

    //! @brief  Though most integrators do support live update in Blender, some doesn't, like light tracing.
    virtual bool NeedRefreshTile() const {
        return true;
    }

    //! @brief      Serializing data from stream
    //!
    //! @param      Stream where the serialization data comes from. Depending on different situation, it could come from different places.
    void    Serialize( IStreamBase& stream ) override {
        stream >> max_recursive_depth;
    }

    //! @brief  This interface is not well supported in SORT for now.
    virtual void GenerateSample(const Sampler* sampler, PixelSample* samples, unsigned ps, const Scene& scene) const {
        auto data = std::make_unique<float[]>(2 * ps);
        sampler->Generate2D(data.get(), ps, true);
        for (unsigned i = 0; i < ps; ++i)
        {
            samples[i].img_u = data[2 * i];
            samples[i].img_v = data[2 * i + 1];
        }

        auto shuffle = std::make_unique<unsigned[]>(ps);
        for (unsigned i = 0; i < ps; i++)
            shuffle[i] = i;
        std::shuffle(shuffle.get(), shuffle.get() + ps, std::default_random_engine(sort_rand()));

        sampler->Generate2D(data.get(), ps);
        for (unsigned i = 0; i < ps; ++i)
        {
            unsigned sid = 2 * shuffle[i];
            samples[i].dof_u = data[sid];
            samples[i].dof_v = data[sid + 1];
        }
    }

    //! @brief  This is not well supported in SORT for now.
    virtual void RequestSample(Sampler* sampler, PixelSample* ps, unsigned ps_num) {}

protected:
    int           max_recursive_depth = 6;      /*< maxium recursive depth. */
    PixelSample   pixel_sample;                 /*< the pixel sample. */
    unsigned      sample_per_pixel;             /*< light sample per pixel sample per light. */
};
