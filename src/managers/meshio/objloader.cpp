/*
 * filename :	objloader.h
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "objloader.h"
#include <fstream>
#include "geometry/point.h"
#include "geometry/vector.h"
#include "managers/meshmanager.h"
#include "utility/strhelper.h"
#include "managers/matmanager.h"

// the maxmium length of a single line
static const unsigned LINE_MAXLENGTH = 4096;

// load obj from file
bool ObjLoader::LoadMesh( const string& str , BufferMemory* mem )
{
	ifstream file( str.c_str() );

	// if the specific file is not loaded return false
	if( false == file.is_open() )
		return false;

	mem->m_filename = str;

	// current trunk
	Trunk*	trunk = 0;

	while( true )
	{
		string prefix;
		file>>prefix;

		if( strcmp( prefix.c_str() , "g" ) == 0 )
		{
			// create a new trunk
			string trunkname;
			file>>trunkname;
			trunk = new Trunk(trunkname);
			trunk->m_mat = MatManager::GetSingleton().FindMaterial( "default_matte1" );
			mem->m_TrunkBuffer.push_back( trunk );
		}if( strcmp( prefix.c_str() , "v" ) == 0 )
		{
			Point p;
			file>>p.x;
			file>>p.y;
			file>>p.z;
			mem->m_PositionBuffer.push_back( p );	
		}else if( strcmp( prefix.c_str() , "vn" ) == 0 )
		{
			Vector v(true);
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
			trunk->m_IndexBuffer.push_back( vi0 );
			file>>strIndex;
			VertexIndex vi1 = VertexIndexFromStr( strIndex );
			trunk->m_IndexBuffer.push_back( vi1 );
			file>>strIndex;
			VertexIndex vi2 = VertexIndexFromStr( strIndex );
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
