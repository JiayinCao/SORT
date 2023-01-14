/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <memory>
#include "scene.h"
#include "math/interaction.h"
#include "accel/accelerator.h"
#include "material/matmanager.h"
#include "core/path.h"
#include "core/samplemethod.h"
#include "core/sassert.h"
#include "core/sassert.h"
#include "core/stats.h"
#include "core/strid.h"
#include "core/primitive.h"
#include "entity/visual_entity.h"
#include "entity/visual.h"
#include "stream/fstream.h"
#include "light/light.h"
#include "shape/shape.h"

SORT_STATS_DEFINE_COUNTER(sScenePrimitiveCount)
SORT_STATS_DEFINE_COUNTER(sSceneLightCount)

SORT_STATS_COUNTER("Statistics", "Total Primitive Count", sScenePrimitiveCount);
SORT_STATS_COUNTER("Statistics", "Total Light Count", sSceneLightCount);

ScenePrimitiveIterator::ScenePrimitiveIterator(const Scene& scene):m_scene(scene){
    Reset();
}

void ScenePrimitiveIterator::Reset(){
    m_indices[0] = 0;
    m_indices[1] = 0;
    m_indices[2] = -1;
}

const Primitive* ScenePrimitiveIterator::Next(){
    int eid = m_indices[0];        // entity id
    int vid = m_indices[1];        // visual id
    int pid = m_indices[2] + 1;    // primitive id

    while(eid < m_scene.m_entities.size()){
        m_indices[0] = eid;

        const auto entity = m_scene.m_entities[eid].get();
        while(vid < entity->m_visuals.size()){
            m_indices[1] = vid;

            const auto visual = entity->m_visuals[vid].get();
            if(pid < visual->m_primitives.size()){
                m_indices[2] = pid;
                return visual->m_primitives[pid].get();
            }

            pid = 0;
            ++vid;
        }

        vid = 0;
        ++eid;
    }

    m_indices[0] = eid;
    return nullptr;
}

SceneVisualIterator::SceneVisualIterator(const Scene& scene):m_scene(scene){
    Reset();
}

void SceneVisualIterator::Reset(){
    m_indices[0] = 0;
    m_indices[1] = -1;
}

const Visual* SceneVisualIterator::Next(){
    int eid = m_indices[0];        // entity id
    int vid = m_indices[1] + 1;    // visual id

    while(eid < m_scene.m_entities.size()){
        m_indices[0] = eid;

        const auto entity = m_scene.m_entities[eid].get();
        while(vid < entity->m_visuals.size()){
            m_indices[1] = vid;
            return entity->m_visuals[vid].get();
        }

        vid = 0;
        ++eid;
    }

    m_indices[0] = eid;
    return nullptr;
}

bool Scene::LoadScene( IStreamBase& stream ){
    const StringID verificationBit( "verification bits" );

    StringID checkingBit;
    stream >> checkingBit;
    sAssertMsg( checkingBit == verificationBit , RESOURCE , "Serialization is broken." );

    while( true ){
        StringID class_id;
        stream >> class_id;
        if( SID("End of Entities") == class_id )
            break;

        auto entity = MakeUniqueInstance<Entity>( class_id );
        sAssertMsg( entity , RESOURCE , "Serialization is broken." );

        entity->Serialize(stream);
        m_entities.push_back(std::move(entity));
    }

    // this will populate data in the scene
    for( auto& entity : m_entities )
        entity->FillScene(*this);
    
    // If there is no light in the scene, a default ambient light will be created.
    if (m_lights.empty()) {
        slog(WARNING, LIGHT, "No light is setup in the scene. A default ambient light will be added");
        auto entity = MakeUniqueInstance<Entity>(SID("SkyLightEntity"));
        entity->FillScene(*this);

        m_entities.push_back(std::move(entity));
    }
    
    // get the bounding box of the whole scene
    ScenePrimitiveIterator iter(*this);
    while(auto primitive = iter.Next())
        m_bbox.Union(primitive->GetBBox());

    // enlarge the bounding box a little
    static const auto threshold = 0.001f;
    auto delta = (m_bbox.m_Max - m_bbox.m_Min) * threshold;
    m_bbox.m_Min -= delta;
    m_bbox.m_Max += delta;

    // generate triangle buffer after parsing from stream
    genLightDistribution();

    SORT_STATS(sScenePrimitiveCount=(StatsInt)GetPrimitiveCount());
    SORT_STATS(sSceneLightCount=(StatsInt)m_lights.size());

    // parse the acceleration structure configuration
    StringID accelType;
    stream >> accelType;
    m_accelerator = MakeUniqueInstance<Accelerator>(accelType);
    if (m_accelerator)
        m_accelerator->Serialize(stream);

    if (!m_accelerator) {
#if SIMD_4WAY_ENABLED
        slog(WARNING, SPATIAL_ACCELERATOR, "Acceleration structure not supported. Use QBVH instead.");
        m_accelerator = MakeUniqueInstance<Accelerator>(SID("Qbvh"));
#else
        slog(WARNING, SPATIAL_ACCELERATOR, "Acceleration structure not supported. Use BVH instead.");
        m_accelerator = MakeUniqueInstance<Accelerator>(SID("Bvh"));
#endif
    }

    return true;
}

bool Scene::GetIntersect( RenderContext& rc, const Ray& r , SurfaceInteraction& intersect ) const{
    intersect.t = FLT_MAX;
    return m_accelerator->GetIntersect( rc, r , intersect );
}

#ifndef ENABLE_TRANSPARENT_SHADOW
bool Scene::IsOccluded(const Ray& r) const{
    return m_accelerator->IsOccluded(r);
}
#else
Spectrum Scene::GetAttenuation( const Ray& const_ray , RenderContext& rc , MediumStack* ms ) const{
    auto ray = const_ray;

    Spectrum attenuation( 1.0f );
    while( !attenuation.IsBlack() ){
        Spectrum att;
        if( !m_accelerator->GetAttenuation(ray, att, rc, ms) )
            break;

        if( att.IsBlack() )
            return att;

        attenuation *= att;
    }
    
    return attenuation;
}
#endif

void Scene::RestoreMediumStack( const Point& p , RenderContext& rc, MediumStack& ms ) const{
    if (IS_PTR_INVALID(m_accelerator))
        return;

    Ray ray;
    ray.m_Ori = p;
    ray.m_Dir = Vector( 0.0f , 1.0f , 0.0f );        // shoot the ray through a random direction.
    while (m_accelerator->UpdateMediumStack(ray, ms, rc, true)) {}
}

void Scene::GetIntersect( const Ray& r , BSSRDFIntersections& intersect , RenderContext& rc, const StringID matID ) const{
    // no brute force support in BSSRDF
    if(IS_PTR_VALID(m_accelerator))
        m_accelerator->GetIntersect( r , intersect , rc , matID );
}

void Scene::genLightDistribution(){
    unsigned count = (unsigned)m_lights.size();
    if( count == 0 )
        return ;

    std::unique_ptr<float[]> pdf = std::make_unique<float[]>(count);
    for( unsigned i = 0 ; i < count ; i++ )
        pdf[i] = m_lights[i]->Power().GetIntensity();

    float total_pdf = 0.0f;
    for( unsigned i = 0 ; i < count ; i++ )
        total_pdf += pdf[i];

    for( unsigned i = 0 ; i < count ; i++ )
        m_lights[i]->SetPickPDF( pdf[i] / total_pdf );

    m_lightsDis = std::make_unique<Distribution1D>( pdf.get() , count );
}

const Light* Scene::SampleLight( float u , float* pdf ) const{
    sAssert( u >= 0.0f && u <= 1.0f , SAMPLING );
    sAssertMsg(IS_PTR_VALID(m_lightsDis), SAMPLING , "No light in the scene." );

    float _pdf;
    int id = m_lightsDis->SampleDiscrete( u , &_pdf );
    if( id >= 0 && id < (int)m_lights.size() && _pdf != 0.0f ){
        if( pdf ) *pdf = _pdf;
        return m_lights[id];
    }
    return nullptr;
}

float Scene::LightProperbility( unsigned i ) const{
    sAssert(IS_PTR_VALID(m_lightsDis), LIGHT );
    return m_lightsDis->GetProperty( i );
}

Spectrum Scene::Le( const Ray& ray ) const{
    if( m_skyLight ){
        Spectrum r;
        m_skyLight->Le( ray , 0 , r );
        return r;
    }
    return 0.0f;
}

void Scene::AddLight( Light* light ){
    if( light ){
        m_lights.push_back( light );
        light->SetupScene( this );
    }
}

void Scene::BuildAccelerationStructure() {
    m_accelerator->Build(*this);
}

unsigned Scene::GetPrimitiveCount() const{
    unsigned int cnt = 0;
    for(const auto& entity: m_entities)
        cnt += entity->GetPrimitiveCount();
    return cnt;
}