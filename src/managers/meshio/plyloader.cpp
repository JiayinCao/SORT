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

// include the header file
#include "plyloader.h"
#include "managers/meshmanager.h"
#include "thirdparty/ply/ply.h"

static char x_prop[] = "x";
static char y_prop[] = "y";
static char z_prop[] = "z";
static char vertex_index_prop[] = "vertex_indices";

static PlyProperty vert_props[] = 
{
  {x_prop, PLY_FLOAT, PLY_FLOAT, 0, 0, 0, 0, 0},
  {y_prop, PLY_FLOAT, PLY_FLOAT, 4, 0, 0, 0, 0},
  {z_prop, PLY_FLOAT, PLY_FLOAT, 8, 0, 0, 0, 0},
};

static PlyProperty face_props[] = 
{
  {vertex_index_prop, PLY_UINT, PLY_UINT, 4 , 1, 4, PLY_UINT, 0},
};

// load obj from file
bool PlyLoader::LoadMesh( const std::string& str , std::shared_ptr<BufferMemory>& mem )
{
	// some variable that will be used later
	int nelems;
	char **elist;
	int file_type;
	float version;
	int nprops;
	int num_elems;
	char *elem_name;

	// open file for reading
	PlyFile *ply = ply_open_for_reading( str.c_str() , &nelems, &elist, &file_type, &version);
	if( ply == 0 )
		return false;

	// set the memory
	mem->m_filename = str;

	mem->m_TrunkBuffer.push_back( Trunk() );

	for ( int i = 0; i < nelems; i++) 
	{
		/* get the description of the first element */
		elem_name = elist[i];
		PlyProperty** properties = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

		if (equal_strings ("vertex", elem_name))
		{
			ply_get_property (ply, elem_name, &vert_props[0]);
			ply_get_property (ply, elem_name, &vert_props[1]);
			ply_get_property (ply, elem_name, &vert_props[2]);

			for ( int j = 0; j < num_elems; j++) 
			{
				Point p;
				ply_get_element(ply, (void *)&p);
				mem->m_PositionBuffer.push_back( p );
			}
		}

		/* if we're on face elements, read them in */
		if (equal_strings ("face", elem_name)) 
		{
			/* set up for getting face elements */
			ply_get_property (ply, elem_name, &face_props[0]);

			/* grab all the face elements */
			for ( int j = 0; j < num_elems; j++)
			{
				PlyIndex index;
				ply_get_element (ply, (void *)&index);

				int curId = 1;
				for( unsigned i = 0 ; i < index.count - 2 ; i++ )
				{
					VertexIndex vid;
					vid.posIndex = index.index[0];
					mem->m_TrunkBuffer.back().m_IndexBuffer.push_back( vid );
					vid.posIndex = index.index[curId];
					mem->m_TrunkBuffer.back().m_IndexBuffer.push_back( vid );
					vid.posIndex = index.index[curId+1];
					mem->m_TrunkBuffer.back().m_IndexBuffer.push_back( vid );

					curId++;
				}
			}
		}

		for( int k = 0 ; k < nprops ; k++ )
		{
			delete[] properties[k]->name;
			delete properties[k];
		}
		delete[] properties;
	}

	for( int i = 0 ; i < nelems ; i++ )
		delete[] elist[i];
	delete[] elist;

	// close ply file
	ply_free_file( ply );

	return true;
}
