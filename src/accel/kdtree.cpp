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
		splits.split[i] = new Split[2*count];
	for( int k = 0 ; k < 3 ; k++ )
	{
		for( unsigned i = 0 ; i < count ; i++ )
		{
			const BBox& box = (*m_primitives)[i]->GetBBox();
			Primitive* pri = (*m_primitives)[i];
			if( box.m_Min[k] != box.m_Max[k] )
			{
				splits.split[k][splits.split_c[k]] = Split( box.m_Min[k] , Split_Start , i , pri );
				splits.split[k][splits.split_c[k]+1] = Split( box.m_Max[k] , Split_End , i , pri );
				splits.split_c[k] += 2;
			}else
			{
				splits.split[k][splits.split_c[k]] = Split( box.m_Min[k] , Split_Flat , i , pri );
				splits.split_c[k]++;
			}
		}
	}
	for( int i = 0 ; i < 3 ; i++ )
		sort( splits.split[i] , splits.split[i] + splits.split_c[i] );

	// create root node
	Kd_Node* root = SORT_MALLOC_ID(Kd_Node,KD_NODE_MEMID)();

	// build kd-tree
	_splitNode( root , splits , count , m_BBox , 0 );

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
	m_nodes = 0;
	m_prilist = 0;
	m_temp = 0;
}

// malloc the memory
void KDTree::_mallocMemory()
{
	SORT_PREMALLOC( 16 * 1024 * 1024 , KD_NODE_MEMID );
	SORT_PREMALLOC( 16 * 1024 * 1024 , KD_LEAF_TRILIST_MEMID );
	m_temp = new unsigned char[m_primitives->size()];

	m_nodes = SORT_MEMORY_ID( Kd_Node , KD_NODE_MEMID );
	m_prilist = SORT_MEMORY_ID( Primitive* , KD_LEAF_TRILIST_MEMID );
}

// free temporary memory
void KDTree::_deallocTmpMemory()
{
	// delete temporary memory
	SAFE_DELETE_ARRAY(m_temp);
}

// split node
void KDTree::_splitNode( Kd_Node* node , Splits& splits , unsigned tri_num , const BBox& box , unsigned depth )
{
	if( tri_num < 16 || depth > 28 )
	{
		_makeLeaf( node , splits , tri_num );
		return;
	}

	// ----------------------------------------------------------------------------------------
	// step 1
	// pick best split
	float		split_pos;
	unsigned 	split_Axis;
	bool 		left = false;
	float sah = _pickSplitting( splits , tri_num , box , split_Axis , split_pos , left );
	node->flag = split_Axis;
	node->split = split_pos;
	if( sah >= tri_num )
	{
		_makeLeaf( node , splits , tri_num );
		return;
	}

	// ----------------------------------------------------------------------------------------
	// step 2
	// distribute triangles
	Split* _splits = splits.split[split_Axis];
	unsigned l_num = 0 , r_num = 0 , b_num = tri_num;
	for( unsigned i = 0 ; i < splits.split_c[split_Axis] ; i++ )
		m_temp[splits.split[split_Axis][i].id] = 2;
	for( unsigned i = 0 ; i < splits.split_c[split_Axis] ; i++ )
	{
		if( _splits[i].type == Split_End && _splits[i].pos <= split_pos )
		{
			m_temp[_splits[i].id] = 0;
			b_num--;
			l_num++;
		}else if( _splits[i].type == Split_Start && _splits[i].pos >= split_pos )
		{
			m_temp[_splits[i].id] = 1;
			b_num--;
			r_num++;
		}else if( _splits[i].type == Split_Flat )
		{
			if( _splits[i].pos < split_pos || ( _splits[i].pos == split_pos && left ) )
			{
				m_temp[_splits[i].id] = 0;
				b_num--;
				l_num++;
			}else if( _splits[i].pos > split_pos || ( _splits[i].pos == split_pos && !left ) )
			{
				m_temp[_splits[i].id] = 1;
				b_num--;
				r_num++;
			}
		}
	}

	// ----------------------------------------------------------------------------------------
	// step 3
	// generate new events
	Splits l_splits;
	Splits r_splits;
	for( int k = 0 ; k < 3 ; k++ )
	{
		l_splits.split[k] = new Split[2*(l_num+b_num)];
		r_splits.split[k] = new Split[2*(r_num+b_num)];
	}
	for( int k = 0 ; k < 3 ; k++ )
	{
		unsigned split_count = splits.split_c[k];
		for( unsigned i = 0 ; i < split_count ; i++ )
		{
			const Split& old = splits.split[k][i];
			unsigned id = old.id;
			if( m_temp[id] == 0 || m_temp[id] == 2 )
			{
				l_splits.split[k][l_splits.split_c[k]] = old; 
				l_splits.split_c[k]++;
			}
			if( m_temp[id] == 1 || m_temp[id] == 2 )
			{
				r_splits.split[k][r_splits.split_c[k]] = old;
				r_splits.split_c[k]++;
			}
		}
	}
	splits.Release();

	BBox left_box = box;
	left_box.m_Max[split_Axis] = split_pos;
	Kd_Node* left_node = SORT_MALLOC_ID(Kd_Node,KD_NODE_MEMID)();
	_splitNode( left_node , l_splits , l_num+b_num , left_box , depth + 1 );

	unsigned node_offset = (SORT_OFFSET(KD_NODE_MEMID)/sizeof(Kd_Node));
	node->right |= node_offset << 2;
	BBox right_box = box;
	right_box.m_Min[split_Axis] = split_pos;
	Kd_Node* right_node = SORT_MALLOC_ID(Kd_Node,KD_NODE_MEMID)();
	_splitNode( right_node , r_splits , r_num+b_num , right_box , depth + 1 );
}

// evaluate sah value for the kdtree node
float KDTree::_sah( unsigned l , unsigned r , unsigned f , unsigned axis , float split , const BBox& box , bool& left )
{
	float inv_sarea = 1.0f / box.HalfSurfaceArea();

	Vector delta = box.m_Max - box.m_Min;
	delta[axis] = split - box.m_Min[axis];
	float l_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;
	delta[axis] = box.m_Max[axis] - split;
	float r_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;

	if( f != 0 )
	{
		l_sarea *= inv_sarea;
		r_sarea *= inv_sarea;
		float sah0 = l_sarea * ( l + f ) + r_sarea * r;
		float sah1 = l_sarea * l + r_sarea * ( r + f );
		left = (sah0<=sah1);
		return min( sah0 , sah1 );
	}

	return ( l * l_sarea + r * r_sarea ) * inv_sarea;
}

// pick best splitting
float KDTree::_pickSplitting( const Splits& splits , unsigned tri_num , const BBox& box , 
							 unsigned& splitAxis , float& split_pos , bool& left )
{
	float min_sah = FLT_MAX;
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
			bool _left = false;
			float sah = _sah( n_l , n_r , pf , k , split , box , _left );
			if( sah < min_sah )
			{
				min_sah = sah;
				splitAxis = k;
				split_pos = split;
				left = _left;
			}
			
			n_l += ps;
			n_l += pf;
		}
	}

	return min_sah;
}

// make leaf
void KDTree::_makeLeaf( Kd_Node* node , Splits& splits , unsigned tri_num )
{
	node->flag = 3;
	node->trinum |= tri_num<<2;
	node->offset = SORT_OFFSET(KD_LEAF_TRILIST_MEMID)/sizeof(Primitive*);

	Primitive** primitives = SORT_MALLOC_ARRAY_ID(Primitive*,tri_num,KD_LEAF_TRILIST_MEMID)();
	unsigned offset = 0;
	for( unsigned i = 0 ; i < splits.split_c[0] ; i++ )
	{
		if( splits.split[0][i].type == Split_Start || splits.split[0][i].type == Split_Flat )
		{
			primitives[offset] = splits.split[0][i].primitive;
			offset++;
		}
	}
	splits.Release();
}

// get the intersection between the ray and the primitive set
bool KDTree::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	float fmax;
	float fmin = Intersect( r , m_BBox , &fmax );
	if( fmin < 0.0f )
		return false;

	return _traverse( m_nodes , r , intersect , fmin , fmax );
}

// tranverse kd-tree node
bool KDTree::_traverse( Kd_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const
{
	const unsigned mask = 0x00000003;

	// it's a leaf node
	if( (node->flag & mask) == 3 )
	{
		unsigned tri_num = node->flag>>2;
		bool inter = false;
		for( unsigned i = 0 ; i < tri_num ; i++ )
			inter |= m_prilist[i+node->offset]->GetIntersect( ray , intersect );
		return inter;
	}

	// get the intersection point between the ray and the splitting plane
	unsigned split_axis = node->flag & mask;
	float dir = ray.m_Dir[split_axis];
	float t = ( node->split - ray.m_Ori[split_axis] ) / dir;
	
	Kd_Node* first = node + 1 ;
	Kd_Node* second = m_nodes + (node->right>>2);
	if( dir < 0.0f )
	{
		Kd_Node* temp = first;
		first = second;
		second = temp;
	}

	bool inter = false;
	if( t > fmin )
		inter = _traverse( first , ray , intersect , fmin , min( fmax , t ) );
	if( inter == false && fmax > t )
		return _traverse( second , ray , intersect , max( t , fmin ) , fmax );
	return inter;
}