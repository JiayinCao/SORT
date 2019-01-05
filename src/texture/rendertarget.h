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

#include "texture.h"

class	RenderTarget : public Texture{
public:
	RenderTarget( int w , int h ) : Texture( w , h ){
		m_pData = std::make_unique<Spectrum[]>( w * h );
	}

	void SetColor( int x , int y , const Spectrum& c );
	Spectrum GetColor( int x , int y ) const;

private:
	std::unique_ptr<Spectrum[]>	m_pData;
};
