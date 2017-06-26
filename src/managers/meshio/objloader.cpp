/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
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

// the maxmium length of a single line
static const unsigned LINE_MAXLENGTH = 4096;

#define CHECK_INDEX(index) {\
	if( index.posIndex < 0 )\
	{\
		LOG_WARNING<<"Negative position index in file "<<str<<ENDL;\
		index.posIndex = 0;\
	}\
	if( index.norIndex < 0 )\
	{\
		LOG_WARNING<<"Negative normal index in file "<<str<<ENDL;\
		index.norIndex = 0;\
	}\
	if( index.texIndex < 0 )\
	{\
		LOG_WARNING<<"Negative texture index in file "<<str<<ENDL;\
		index.texIndex = 0;\
	}\
}
	
// load obj from file
bool ObjLoader::LoadMesh( const string& str , std::shared_ptr<BufferMemory>& mem )
{
	ifstream file( str.c_str() );

	// if the specific file is not loaded return false
	if( false == file.is_open() )
		return false;

	mem->m_filename = str;

	// current trunk
    std::shared_ptr<Trunk>	trunk = nullptr;

	while( true )
	{
		string prefix;
		file>>prefix;

		if( strcmp( prefix.c_str() , "g" ) == 0 )
		{
			// create a new trunk
			string trunkname;
			file>>trunkname;
            trunk = std::make_shared<Trunk>( trunkname );
			mem->m_TrunkBuffer.push_back( trunk );
		}else if( strcmp( prefix.c_str() , "mtllib" ) == 0 )
		{
			string name;
			file>>name;
			MatManager::GetSingleton().ParseMatFile( name );
		}else if( strcmp( prefix.c_str() , "usemtl" ) == 0 )
		{
			string name;
			file>>name;
			if( trunk )
			{
				trunk->m_mat = MatManager::GetSingleton().FindMaterial( name );
				if( 0 == trunk->m_mat )
					LOG_WARNING<<"Material named \'"<<name<<"\' not found, use default material in subset \'"<<trunk->name<<"\' of \'"<<str<<"\'."<<ENDL;
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
			Vector v;
			file>>v.x;
			file>>v.y;
			file>>v.z;
			mem->m_NormalBuffer.push_back( v );
		}else if( strcmp( prefix.c_str() , "vt" ) == 0 )
		{
			float u , v;
			file>>u;
			file>>v;
			mem->m_TexCoordBuffer.push_back( u );
			mem->m_TexCoordBuffer.push_back( v );
		}else if( strcmp( prefix.c_str() , "f" ) == 0 && trunk != 0 )
		{
			string strIndex;
			file>>strIndex;
			VertexIndex vi0 = VertexIndexFromStr( strIndex );
			//CHECK_INDEX(vi0);
			trunk->m_IndexBuffer.push_back( vi0 );
			file>>strIndex;
			VertexIndex vi1 = VertexIndexFromStr( strIndex );
			//CHECK_INDEX(vi1);
			trunk->m_IndexBuffer.push_back( vi1 );
			file>>strIndex;
			VertexIndex vi2 = VertexIndexFromStr( strIndex );
			//CHECK_INDEX(vi2);
			trunk->m_IndexBuffer.push_back( vi2 );

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
				//CHECK_INDEX(vi3);
				trunk->m_IndexBuffer.push_back( vi0 );
				trunk->m_IndexBuffer.push_back( vi2 );
				trunk->m_IndexBuffer.push_back( vi3 );
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
