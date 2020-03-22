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

#include <regex>
#include <math.h>
#include "texturebase.h"
#include "core/sassert.h"
#include "math/interaction.h"
#include "thirdparty/tiny_exr/tinyexr.h"
#include "thirdparty/stb_image/stb_image.h"

bool Texture2DBase::Output( const std::string& name ){
    std::regex exr_reg(".*\\.exr$", std::regex_constants::icase);
    if (std::regex_match(name, exr_reg)) {
        const auto totalXRes = GetWidth();
        const auto totalYRes = GetHeight();
        const auto total = totalXRes * totalYRes;
        const auto data = std::make_unique<float[]>(total * 3);
        for (auto i = 0; i < total; ++i)
        {
            auto x = i % totalXRes;
            auto y = i / totalXRes;
            Spectrum c = GetColor(x, y);

            data[3 * i] = c.r;
            data[3 * i + 1] = c.g;
            data[3 * i + 2] = c.b;
        }

        int ret = SaveEXR(data.get(), GetWidth(), GetHeight(), 3, true, name.c_str());
        if (ret < 0)
            slog(WARNING, MATERIAL, "Fail to save image file %s", name.c_str());
        return ret >= 0;
    }

    sAssertMsg( false , IMAGE , "SORT doesn't support exporting file %s",name.c_str());

    return false;
}

void Texture2DBase::texCoordFilter( int& x , int& y ) const{
    switch( m_TexCoordFilter ){
    case TCF_WARP:
        if( x >= 0 )
            x = x % m_iTexWidth;
        else
            x = m_iTexWidth - ( -x ) % m_iTexWidth - 1;
        if( y >= 0 )
            y = y % m_iTexHeight;
        else
            y = m_iTexHeight - ( -y ) % m_iTexHeight - 1;
        break;
    case TCF_CLAMP:
        x = std::min( (int)m_iTexWidth - 1 , std::max( x , 0 ) );
        y = std::min( (int)m_iTexHeight - 1 , std::max( y , 0 ) );
        break;
    case TCF_MIRROR:
        x = ( x >= 0 )?x:(1-x);
        x = x % ( 2 * m_iTexWidth );
        x -= m_iTexWidth;
        x = ( x >= 0 )?x:(1-x);
        x = m_iTexWidth - 1 - x;
        y = ( y >= 0 )?y:(1-y);
        y = y % ( 2 * m_iTexHeight );
        y -= m_iTexHeight;
        y = ( y >= 0 )?y:(1-y);
        y = m_iTexHeight - 1 - y;
        break;
    }
}

Spectrum Texture2DBase::GetColorFromUV( float u , float v ) const{
    // Before I have time to work on texturing system, using linear sampling by default.
    // This is by no means the most efficient way to implement bilinear sampling, but it works for now.

    const auto fu = u * m_iTexWidth - 0.5f;
    const auto fv = v * m_iTexHeight - 0.5f;
    const auto iu = (int)( fu );
    const auto iv = (int)( fv );
    const auto _fu = fu - floor(fu);
    const auto _fv = fv - floor(fv);

    return  GetColor(iu, iv) * (1.0f - _fu) * (1.0f - _fv) + GetColor(iu + 1, iv) * _fu * (1.0f - _fv) +
            GetColor(iu, iv + 1) * (1.0f - _fu) * _fv + GetColor(iu + 1, iv + 1) * _fu * _fv;
}

float Texture2DBase::GetAlphaFromtUV( float u , float v ) const{
    const auto fu = u * m_iTexWidth - 0.5f;
    const auto fv = v * m_iTexHeight - 0.5f;
    const auto iu = (int)( fu );
    const auto iv = (int)( fv );
    const auto _fu = fu - floor(fu);
    const auto _fv = fv - floor(fv);

    return  GetAlpha(iu, iv) * (1.0f - _fu) * (1.0f - _fv) + GetAlpha(iu + 1, iv) * _fu * (1.0f - _fv) +
            GetAlpha(iu, iv + 1) * (1.0f - _fu) * _fv + GetAlpha(iu + 1, iv + 1) * _fu * _fv;
}