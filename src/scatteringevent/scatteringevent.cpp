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

#include "scatteringevent.h"
#include "bsdf/bxdf.h"
#include "bssrdf/bssrdf.h"

ScatteringEvent::ScatteringEvent( const Intersection& intersection , const SE_Flag flag )
: m_flag(flag), m_intersection( intersection ){
    m_n = Normalize(intersection.normal);
    m_bt = Normalize(Cross( m_n , intersection.tangent ));
    m_t = Normalize(Cross( m_bt , m_n ));
}

Vector ScatteringEvent::worldToLocal( const Vector& v , bool forceTransform ) const{
    if( ( m_flag & SE_SUB_EVENT ) && !forceTransform ) return v;
    return Vector( Dot(v,m_t) , Dot(v,m_n) , Dot(v,m_bt) );
}

Vector ScatteringEvent::localToWorld( const Vector& v ) const{
    if( m_flag & SE_SUB_EVENT ) return v;
    return Vector(  v.x * m_t.x + v.y * m_n.x + v.z * m_bt.x ,
                    v.x * m_t.y + v.y * m_n.y + v.z * m_bt.y ,
                    v.x * m_t.z + v.y * m_n.z + v.z * m_bt.z );
}

void ScatteringEvent::Sample_SE( const Vector& wo , const class BsdfSample& bs , ScatteringResult& sr ) const{
    // to be implemented
}