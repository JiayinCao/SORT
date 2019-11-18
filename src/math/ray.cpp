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

#include "ray.h"

static SORT_FORCEINLINE float sign( const float x ){
    return x < 0.0f ? -1.0f : 1.0f;
}

static SORT_FORCEINLINE int majorAxis(const Vector3f& v) {
	if (abs(v[0]) > abs(v[1]) && abs(v[0]) > abs(v[2]))
		return 0;
	return abs(v[1]) > abs(v[2]) ? 1 : 2;
}

static SORT_FORCEINLINE Vector3f permuteAxis(const Vector3f& v, int ax, int ay, int az) {
	return Vector3f(v[ax], v[ay], v[az]);
}

Ray::Ray(){
    m_Depth = 0;
    m_fMin = 0.0f;
    m_fMax = FLT_MAX;
    m_fPdfW = 0.0f;
    m_fPdfA = 0.0f;
    m_we = 0.0f;
    m_fCosAtCamera = 0.0f;
}

Ray::Ray( const Point& p , const Vector& dir , unsigned depth , float fmin , float fmax){
    m_Ori = p;
    m_Dir = dir;
    m_Depth = depth;
    m_fMin = fmin;
    m_fMax = fmax;
    m_fPdfW = 0.0f;
    m_fPdfA = 0.0f;
    m_we = 0.0f;
    m_fCosAtCamera = 0.0f;
}

Ray::Ray( const Ray& r ){
    m_Ori = r.m_Ori;
    m_Dir = r.m_Dir;
    m_Depth = r.m_Depth;
    m_fMin = r.m_fMin;
    m_fMax = r.m_fMax;
    m_fPdfW = r.m_fPdfW;
    m_fPdfA = r.m_fPdfA;
    m_we = r.m_we;
    m_fCosAtCamera = r.m_fCosAtCamera;
}

void Ray::Prepare() const{
	m_local_y = majorAxis(m_Dir);
	m_local_z = (m_local_y + 1) % 3;
	m_local_x = (m_local_z + 1) % 3;

	const auto d = permuteAxis(m_Dir, m_local_x, m_local_y, m_local_z);
	m_scale_x = -d.x / d.y;
	m_scale_z = -d.z / d.y;
	m_scale_y = 1.0f / d.y;

#ifdef SSE_ENABLED
    constexpr float delta = 0.00001f;
    const auto dir_x = fabs(m_Dir[0]) < delta ? sign(m_Dir[0]) * delta : m_Dir[0];
    const auto dir_y = fabs(m_Dir[1]) < delta ? sign(m_Dir[1]) * delta : m_Dir[1];
    const auto dir_z = fabs(m_Dir[2]) < delta ? sign(m_Dir[2]) * delta : m_Dir[2];
    m_rcp_dir_x	= _mm_set_ps1( 1.0f/dir_x );
    m_rcp_dir_y = _mm_set_ps1( 1.0f/dir_y );
    m_rcp_dir_z = _mm_set_ps1( 1.0f/dir_z );
    m_ori_dir_x = _mm_set_ps1( -m_Ori[0]/dir_x );
    m_ori_dir_y = _mm_set_ps1( -m_Ori[1]/dir_y ); 
    m_ori_dir_z = _mm_set_ps1( -m_Ori[2]/dir_z ); 

	m_ori_x = _mm_set_ps1( m_Ori.x );
	m_ori_y = _mm_set_ps1( m_Ori.y );
	m_ori_z = _mm_set_ps1( m_Ori.z );

    m_dir_x = _mm_set_ps1( m_Dir.x );
    m_dir_y = _mm_set_ps1( m_Dir.y );
    m_dir_z = _mm_set_ps1( m_Dir.z );

	m_sse_scale_x = _mm_set_ps1( m_scale_x );
	m_sse_scale_y = _mm_set_ps1( m_scale_y );
	m_sse_scale_z = _mm_set_ps1( m_scale_z );
#endif
}