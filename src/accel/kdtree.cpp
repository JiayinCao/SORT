/*
 * filename :	kdtree.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "kdtree.h"
#include "managers/memmanager.h"
#include "geometry/primitive.h"
#include <algorithm>

static const unsigned KD_NODE_MEMID = 1;
static const unsigned KD_LEAF_TRILIST_MEMID = 2;

// default constructor
KDTree::KDTree()
{
	_init();
}

// constructor from a primitive list
KDTree::KDTree( vector<Primitive*>* l ) : Accelerator(l)
{
}
// destructor
KDTree::~KDTree()
{
}

// get the intersection between the ray and the primitive set
bool KDTree::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	//to be modified
	return Intersect( r , m_BBox ) > 0.0f ;
}

// build the acceleration structure
void KDTree::Build()
{
	// pre-malloc node and leaf triangle list memory
	_mallocMemory();

	// get the bounding box for the whole primitive list
	_computeBBox();
	
	// create the split candidates
	unsigned count = m_primitives->size();
	Splits splits;
	for( int i = 0 ; i < 3 ; i++ )
	{
		splits.split[i] = new Split[2*count];
		splits.split_c[i] = 0;
	}
	for( int k = 0 ; k < 3 ; k++ )
	{
		for( unsigned i = 0 ; i < count ; i++ )
		{
			const BBox& box = (*m_primitives)[i]->GetBBox();
			unsigned id = (*m_primitives)[i]->GetID();

			if( box.m_Min[k] != box.m_Max[k] )
			{
				splits.split[k][splits.split_c[k]] = Split( box.m_Min[k] , Split_Start , id );
				splits.split[k][splits.split_c[k]+1] = Split( box.m_Max[k] , Split_End , id );
				splits.split_c[k] += 2;
			}else
			{
				splits.split[k][splits.split_c[k]] = Split( box.m_Min[k] , Split_Flat , id );
				splits.split_c[k]++;
			}
		}
	}
	for( int i = 0 ; i < 3 ; i++ )
		sort( splits.split[i] , splits.split[i] + splits.split_c[i] );

	// create root node
	m_root = SORT_MALLOC(Kd_Node)();

	// build kd-tree
	_splitNode( m_root , splits , count , m_BBox );

	// dealloc temporary memory
	_deallocTmpMemory();
}

// output log
void KDTree::OutputLog() const
{
	LOG_HEADER( "Accelerator" );
	LOG<<"KD-Tree is not finished yet."<<ENDL<<ENDL;
}

// initialize
void KDTree::_init()
{
	m_root = 0;
	m_temp = 0;
}

// malloc the memory
void KDTree::_mallocMemory()
{
	SORT_PREMALLOC( 4 * 1024 * 1024 , KD_NODE_MEMID );
	SORT_PREMALLOC( 4 * 1024 * 1024 , KD_LEAF_TRILIST_MEMID );
	m_temp = new unsigned char[m_primitives->size()];
}

// free temporary memory
void KDTree::_deallocTmpMemory()
{
	// delete temporary memory
	SAFE_DELETE_ARRAY(m_temp);
}

// split node
void KDTree::_splitNode( Kd_Node* node , Splits& splits , unsigned tri_num , const BBox& box )
{
	// pick best split
	float min_sah = FLT_MAX;
	unsigned best_axis , best_split_id;
	for( int k = 0 ; k < 3 ; k++ )
	{
		unsigned n_l = 0 ;
		unsigned n_r = tri_num ;
		unsigned split_count = splits.split_c[k];
		unsigned i = 0;
		while( i < split_count )
		{
			unsigned pe = 0 , pf = 0 , ps = 0;
			float split = splits.split[k][i].pos;

			while( i < split_count && splits.split[k][i].pos == split && splits.split[k][i].type == Split_End )
			{i++;pe++;}
			while( i < split_count && splits.split[k][i].pos == split && splits.split[k][i].type == Split_Flat )
			{i++;pf++;}
			while( i < split_count && splits.split[k][i].pos == split && splits.split[k][i].type == Split_Start )
			{i++;ps++;}

			n_r -= pe;
			n_r -= pf;

			// get the sah value
			float sah = _sah( n_l , n_r , k , split , box );
			if( sah < min_sah )
			{
				min_sah = sah;
				best_axis = k;
				best_split_id = i;
			}
			
			n_l += ps;
			n_l += pf;
		}
	}

	cout<<best_axis<<" "<<best_split_id<<" "<<min_sah<<endl;
}

// evaluate sah value for the kdtree node
float KDTree::_sah( unsigned l , unsigned r , unsigned axis , float split , const BBox& box )
{
	float sarea = box.HalfSurfaceArea();

	Vector delta = box.m_Max - box.m_Min;
	delta[axis] = split - box.m_Min[axis];
	float l_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;
	delta[axis] = box.m_Max[axis] - split;
	float r_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;

	return ( l * l_sarea + r * r_sarea ) / sarea;
}