/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header
#include "objloader.h"
#include <fstream>
#include "math/point.h"
#include "math/vector3.h"
#include "managers/meshmanager.h"
#include "utility/strhelper.h"
#include "managers/matmanager.h"
#include "utility/log.h"

// the maxmium length of a single line
static const unsigned LINE_MAXLENGTH = 4096;

#define CHECK_INDEX(index) {\
	if( index.posIndex < 0 )\
	{\
        slog( WARNING , GENERAL , "Negative position index in file." );\
		index.posIndex = 0;\
	}\
	if( index.norIndex < 0 )\
	{\
        slog( WARNING , GENERAL , "Negative normal index in file." );\
		index.norIndex = 0;\
	}\
	if( index.texIndex < 0 )\
	{\
        slog( WARNING , GENERAL , "Negative texture index in file." );\
		index.texIndex = 0;\
	}\
}
	
// load obj from file
bool ObjLoader::LoadMesh( const std::string& str , std::shared_ptr<BufferMemory>& mem )
{
	std::ifstream file( str.c_str() );

	// if the specific file is not loaded return false
	if( false == file.is_open() )
		return false;

	mem->m_filename = str;

	while( true )
	{
		std::string prefix;
		file>>prefix;

		if( strcmp( prefix.c_str() , "g" ) == 0 )
		{
			// create a new trunk
			std::string trunkname;
			file>>trunkname;
			mem->m_TrunkBuffer.push_back( Trunk() );
		}else if( strcmp( prefix.c_str() , "usemtl" ) == 0 )
		{
			std::string name;
			file>>name;
			if( mem->m_TrunkBuffer.size() )
			{
				mem->m_TrunkBuffer.back().m_mat = MatManager::GetSingleton().FindMaterial( name );
				if( 0 == mem->m_TrunkBuffer.back().m_mat )
                    slog( WARNING , MATERIAL , stringFormat("Material named %s not found, use default material in subset \"%s\"." , name.c_str() , str.c_str() ) );
			}
		}else if( strcmp( prefix.c_str() , "v" ) == 0 )
		{
			Point p;
			file>>p.x;
			file>>p.y;
			file>>p.z;
			mem->m_PositionBuffer.push_back( p );	
		}else if( strcmp( prefix.c_str() , "vn" ) == 0 )
		{
			mem->m_hasInitNormal = true;
			Vector v;
			file>>v.x;
			file>>v.y;
			file>>v.z;
			mem->m_NormalBuffer.push_back( v );
		}else if( strcmp( prefix.c_str() , "vt" ) == 0 )
		{
			mem->m_hasInitTexCoord = true;
			float u , v;
			file>>u;
			file>>v;
			mem->m_TexCoordBuffer.push_back( u );
			mem->m_TexCoordBuffer.push_back( v );
		}else if( strcmp( prefix.c_str() , "f" ) == 0 && mem->m_TrunkBuffer.size() )
		{
			std::string strIndex;
			file>>strIndex;
			VertexIndex vi0 = VertexIndexFromStr( strIndex );
			file>>strIndex;
			VertexIndex vi1 = VertexIndexFromStr( strIndex );
			file>>strIndex;
			VertexIndex vi2 = VertexIndexFromStr( strIndex );

            if (mem->m_PositionBuffer[vi0.posIndex] != mem->m_PositionBuffer[vi1.posIndex] &&
                mem->m_PositionBuffer[vi1.posIndex] != mem->m_PositionBuffer[vi2.posIndex] &&
                mem->m_PositionBuffer[vi2.posIndex] != mem->m_PositionBuffer[vi0.posIndex]) {
                mem->m_TrunkBuffer.back().m_IndexBuffer.push_back(vi0);
                mem->m_TrunkBuffer.back().m_IndexBuffer.push_back(vi1);
                mem->m_TrunkBuffer.back().m_IndexBuffer.push_back(vi2);
            }

			// check if there is another index
			char t = file.peek();
			while( t == ' ' || t == '\t' )
			{
				file.get();
				t = file.peek();
			}
			if( t >= '0' && t <= '9' )
			{
				file>>strIndex;
				VertexIndex vi3 = VertexIndexFromStr( strIndex );

                if (mem->m_PositionBuffer[vi0.posIndex] != mem->m_PositionBuffer[vi2.posIndex] &&
                    mem->m_PositionBuffer[vi2.posIndex] != mem->m_PositionBuffer[vi3.posIndex] &&
                    mem->m_PositionBuffer[vi3.posIndex] != mem->m_PositionBuffer[vi0.posIndex]) {
                    mem->m_TrunkBuffer.back().m_IndexBuffer.push_back(vi0);
                    mem->m_TrunkBuffer.back().m_IndexBuffer.push_back(vi2);
                    mem->m_TrunkBuffer.back().m_IndexBuffer.push_back(vi3);
                }
			}
		}

		// skip the rest of the line
		file.ignore( LINE_MAXLENGTH , '\n' );

		if( file.eof() )
			break;
	}

	file.close();

	return true;
}
