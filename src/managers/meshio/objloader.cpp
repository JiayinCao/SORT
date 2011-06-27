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
#include "utility/path.h"

// the maxmium length of a single line
#define	LINE_MAXLENGTH		4096

// load obj from file
bool ObjLoader::LoadMesh( const string& str , BufferMemory* mem )
{
	ifstream file( GetFullPath(str).c_str() );

	// if the specific file is not loaded return false
	if( false == file.is_open() )
		return false;

	mem->m_filename = str;
	mem->m_iTriNum = 0;

	while( true )
	{
		string prefix;
		file>>prefix;

		if( strcmp( prefix.c_str() , "v" ) == 0 )
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
		}else if( strcmp( prefix.c_str() , "f" ) == 0 )
		{
			for( int i = 0 ; i < 3 ; i++ )
			{
				string strIndex;
				file>>strIndex;

				mem->m_IndexBuffer.push_back( Str2VertexIndex( strIndex ) );
			}

			mem->m_iTriNum++;
		}

		// skip the rest of the line
		file.ignore( LINE_MAXLENGTH , '\n' );

		if( file.eof() )
			break;
	}

	file.close();

	mem->m_iVBCount = mem->m_PositionBuffer.size();
	mem->m_iTBCount = mem->m_TexCoordBuffer.size();
	mem->m_iNBCount = mem->m_NormalBuffer.size();

	return true;
}
