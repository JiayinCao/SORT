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

#include "mesh.h"
#include "entity/visual.h"
#include "stream/stream.h"
#include "material/matmanager.h"
#include "entity/entity.h"
#include "stream/stream.h"
#include "scatteringevent/bsdf/bxdf_utils.h"

void Mesh::ApplyTransform( const Transform& transform ){
    for (MeshVertex& mv : m_vertices) {
        mv.m_position = transform.TransformPoint(mv.m_position);
        mv.m_normal = transform.TransformNormal((mv.m_normal).Normalize());

        // Warning this function seems to cause quite some trouble on MacOS during the first renderer somehow.
        // And this problem only exists on MacOS not the other two OS.
        // Since there is not a low hanging fruit solution for now, it is disabled by default
        // generate tangent if there is UV, there seems to always be true in Blender 2.8, but not in 2.7x
        //if(m_hasUV)
        //    mv.m_tangent = transform(mv.m_tangent).Normalize();
    }

    m_world2Volume = m_local2Volume * transform.invMatrix;
}

void Mesh::GenSmoothTagent(){
    // generate tangent for each triangle
    std::vector<std::vector<Vector>> tangent(m_vertices.size());
    for (auto mi : m_indices) {
        const auto t = genTagentForTri(mi);

        tangent[mi.m_id[0]].push_back(t);
        tangent[mi.m_id[1]].push_back(t);
        tangent[mi.m_id[2]].push_back(t);
    }
    for (auto i = 0u; i < m_vertices.size(); ++i) {
        Vector t;
        for (auto v : tangent[i])
            t += v;
        m_vertices[i].m_tangent = t.Normalize();
    }
}

void Mesh::GenUV(){
    if (m_hasUV || m_vertices.empty())
        return;

    Point center;
    for( const MeshVertex& mv : m_vertices )
        center = center + mv.m_position;
    center /= (float)m_vertices.size();

    for (MeshVertex& mv : m_vertices) {
        Vector diff = mv.m_position - center;
        diff.Normalize();
        mv.m_texCoord.x = sphericalTheta(diff) * INV_PI;
        mv.m_texCoord.y = sphericalPhi(diff) * INV_TWOPI;
    }
}

Vector Mesh::genTagentForTri( const MeshFaceIndex& mi ) const{
    const auto& _v0 = m_vertices[mi.m_id[0]];
    const auto& _v1 = m_vertices[mi.m_id[1]];
    const auto& _v2 = m_vertices[mi.m_id[2]];

    // get three vertexes
    const auto& p0 = _v0.m_position ;
    const auto& p1 = _v1.m_position ;
    const auto& p2 = _v2.m_position ;

    const auto u0 = _v0.m_texCoord.x;
    const auto u1 = _v1.m_texCoord.x;
    const auto u2 = _v2.m_texCoord.x;
    const auto v0 = _v0.m_texCoord.y;
    const auto v1 = _v1.m_texCoord.y;
    const auto v2 = _v2.m_texCoord.y;

    const auto du1 = u0 - u2;
    const auto du2 = u1 - u2;
    const auto dv1 = v0 - v2;
    const auto dv2 = v1 - v2;
    const auto dp1 = p0 - p2;
    const auto dp2 = p1 - p2;

    const auto determinant = du1 * dv2 - dv1 * du2 ;
    if( determinant == 0.0f ){
        const auto n = normalize( cross( p0 - p1 , p0 - p2 ) );
        Vector t0 , t1;
        coordinateSystem( n , t0 , t1 );
        return t0;
    }
    return ( dv2 * dp1 - dv1 * dp2 ) / determinant;
}

void Mesh::Serialize(IStreamBase& stream) {
    stream >> m_hasUV;
    unsigned int vb_cnt, ib_cnt;
    stream >> vb_cnt;
    m_vertices.resize(vb_cnt);
    for (MeshVertex& mv : m_vertices)
        stream >> mv.m_position >> mv.m_normal >> mv.m_texCoord;

    // mapping from original material to material proxy
    std::unordered_map<const MaterialBase*, const MaterialBase*> mapping;

    stream >> ib_cnt;
    m_indices.resize(ib_cnt);
    for (auto& mi : m_indices) {
        stream >> mi.m_id[0] >> mi.m_id[1] >> mi.m_id[2];
        int mat_id = -1;
        stream >> mat_id;
        mi.m_mat = MatManager::GetSingleton().GetMaterial(mat_id);

        // If there is SSS in the material or volume is attached to the material, it is necessary to create a material proxy to
        // prevent the same material used in multiple places being recognized as the same one.
        //
        // This doesn't handle the corner cases that the same material used in two separate parts in a same mesh, the two parts
        // will still have SSS bleeding together. But it doesn't prevent a same material used by two meshes being bleeding from
        // each other.
        if (mi.m_mat->HasSSS() || mi.m_mat->HasVolumeAttached()) {
            // material proxy of this material is not created yet.
            if (0 == mapping.count(mi.m_mat))
                mapping[mi.m_mat] = MatManager::GetSingleton().CreateMaterialProxy(*mi.m_mat);

            mi.m_mat = mapping[mi.m_mat];
        }
    }

    static const StringID has_volume_sid("has_volume");
    static const StringID no_volume_sid("no_volume");

    // serialize volume data if needed
    StringID volume_sid;
    stream >> volume_sid;
    if (volume_sid == has_volume_sid){
        m_volumeDensity = std::make_unique<MediumDensity>();
        m_volumeDensity->Serialize(stream);

        m_volumeColor = std::make_unique<MediumColor>();
        m_volumeColor->Serialize(stream);

        // this doesn't need to be done if there is no volume data
        BBox bbox;
        for (const auto& v : m_vertices)
            bbox.Union(v.m_position);
        const auto extent = bbox.m_Max - bbox.m_Min;
        const auto ie_x = 1.0f / extent[0];
        const auto ie_y = 1.0f / extent[1];
        const auto ie_z = 1.0f / extent[2];
        m_local2Volume = Matrix(ie_x, 0.0f, 0.0f, -bbox.m_Min[0] * ie_x,
            0.0f, ie_y, 0.0f, -bbox.m_Min[1] * ie_y,
            0.0f, 0.0f, ie_z, -bbox.m_Min[2] * ie_z,
            0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        sAssert(volume_sid == no_volume_sid, VOLUME);
    }

    static const StringID end_of_mesh("end of mesh");
    StringID eom_sid;
    stream >> eom_sid;
    sAssert(eom_sid == end_of_mesh, GENERAL);
}

float Mesh::SampleVolumeDensity(const Point& pos) const {
    if (IS_PTR_INVALID(m_volumeDensity))
        return 0.0f;
    const auto uvw = m_world2Volume.TransformPoint(pos);
    return m_volumeDensity->Sample(uvw);
}

Spectrum Mesh::SampleVolumeColor(const Point& pos) const {
    if (IS_PTR_INVALID(m_volumeColor))
        return 0.0f;
    const auto uvw = m_world2Volume.TransformPoint(pos);
    return m_volumeColor->Sample(uvw);
}