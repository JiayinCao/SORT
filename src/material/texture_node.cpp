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

#include "texture_node.h"
#include "bsdf/bsdf.h"
#include "texture/checkerboxtexture.h"
#include "texture/gridtexture.h"

IMPLEMENT_CREATOR( GridTexNode );
IMPLEMENT_CREATOR( CheckBoxTexNode );
IMPLEMENT_CREATOR( ImageTexNode );

void GridTexNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    const Intersection* intesection = bsdf->GetIntersection();

    SORT_MATERIAL_GET_PROP_COLOR(s0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(s1,src1);

    GridTexture texture( s0 , s1 );
    result = texture.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f );
}

void CheckBoxTexNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    const Intersection* intesection = bsdf->GetIntersection();

    SORT_MATERIAL_GET_PROP_COLOR(s0,src0);
    SORT_MATERIAL_GET_PROP_COLOR(s1,src1);

    CheckerBoxTexture texture( s0 , s1 );
    result = texture.GetColorFromUV( intesection->u * 10.0f , intesection->v * 10.0f );
}

void ImageTexNode::GetMaterialProperty( Bsdf* bsdf , Spectrum& result ){
    const Intersection* intesection = bsdf->GetIntersection();
    result = m_imageTexture.GetColorFromUV( intesection->u , intesection->v );
}

void ImageTexNode::PostProcess(){
	if( m_post_processed )
		return;

    Bsdf* bsdf = nullptr;
    SORT_MATERIAL_GET_PROP_STR(file,filename);
	m_imageTexture.LoadImageFromFile( file );
}
