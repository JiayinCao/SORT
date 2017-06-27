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

#include "kdtree.h"
#include "geometry/primitive.h"
#include "geometry/intersection.h"
#include <algorithm>

IMPLEMENT_CREATOR( KDTree );

// destructor
KDTree::~KDTree()
{
	// delete kd-tree
	deleteKdNode( m_root );
}

// build the acceleration structure
void KDTree::Build()
{
	if( m_primitives->size() == 0 )
		return;

	// pre-malloc node and leaf triangle list memory
	m_temp = new unsigned char[m_primitives->size()];

	// get the bounding box for the whole primitive list
	computeBBox();
	
	// create the split candidates
	unsigned count = (unsigned)m_primitives->size();
	Splits splits;
	for( int i = 0 ; i < 3 ; i++ )
		splits.split[i] = new Split[2*count];
	for( int k = 0 ; k < 3 ; k++ )
	{
		for( unsigned i = 0 ; i < count ; i++ )
		{
			Primitive* pri = (*m_primitives)[i];
			const BBox& box = pri->GetBBox();
			if( box.m_Min[k] != box.m_Max[k] )
			{
                splits.split[k][splits.split_c[k]] = Split( box.m_Min[k] , Split_Type::Split_Start , i , pri );
                splits.split[k][splits.split_c[k]+1] = Split( box.m_Max[k] , Split_Type::Split_End , i , pri );
				splits.split_c[k] += 2;
			}else
			{
                splits.split[k][splits.split_c[k]] = Split( box.m_Min[k] , Split_Type::Split_Flat , i , pri );
				splits.split_c[k]++;
			}
		}
	}
	for( int i = 0 ; i < 3 ; i++ )
		sort( splits.split[i] , splits.split[i] + splits.split_c[i] );

	// create root node
	m_root = new Kd_Node(m_bbox);

	// build kd-tree
	splitNode( m_root , splits , count , 0 );

	if( m_leaf != 0 )
		m_fAvgLeafTri /= m_leaf;

	// delete temporary memory
	SAFE_DELETE_ARRAY(m_temp);
}

// output log
void KDTree::OutputLog() const
{
	LOG_HEADER( "Accelerator" );
	LOG<<"Accelerator Type :\tK-Dimensional Tree"<<ENDL;
	LOG<<"KD-Tree Depth    :\t"<<m_depth<<ENDL;
	LOG<<"Total Node Count :\t"<<m_total<<ENDL;
	LOG<<"Inner Node Count :\t"<<m_total - m_leaf<<ENDL;
	LOG<<"Leaf Node Count  :\t"<<m_leaf<<ENDL;
	LOG<<"Triangles per leaf:\t"<<m_fAvgLeafTri<<ENDL;
	LOG<<"Max triangles in leaf:\t"<<m_MaxLeafTri<<ENDL<<ENDL;
}

// split node
void KDTree::splitNode( Kd_Node* node , Splits& splits , unsigned prinum , unsigned depth )
{
	if( prinum < m_maxTriInLeaf || depth > m_maxDepth ){
		makeLeaf( node , splits , prinum );
		return;
	}

	// ----------------------------------------------------------------------------------------
	// step 1
	// pick best split
	float		split_pos;
	unsigned 	split_Axis;
	bool 		left = false;
	float sah = pickSplitting( splits , prinum , node->bbox , split_Axis , split_pos , left );
	node->flag = split_Axis;
	node->split = split_pos;
	if( sah >= prinum ){
		makeLeaf( node , splits , prinum );
		return;
	}

	// ----------------------------------------------------------------------------------------
	// step 2
	// distribute triangles
	Split* _splits = splits.split[split_Axis];
	unsigned l_num = 0 , r_num = 0 , b_num = prinum;
	for( unsigned i = 0 ; i < splits.split_c[split_Axis] ; i++ )
		m_temp[splits.split[split_Axis][i].id] = 2;
	for( unsigned i = 0 ; i < splits.split_c[split_Axis] ; i++ )
	{
        if( _splits[i].type == Split_Type::Split_End && _splits[i].pos <= split_pos )
		{
			m_temp[_splits[i].id] = 0;
			b_num--;
			l_num++;
        }else if( _splits[i].type == Split_Type::Split_Start && _splits[i].pos >= split_pos )
		{
			m_temp[_splits[i].id] = 1;
			b_num--;
			r_num++;
        }else if( _splits[i].type == Split_Type::Split_Flat )
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

	BBox left_box = node->bbox;
	left_box.m_Max[split_Axis] = split_pos;
	node->leftChild = new Kd_Node(left_box);
	splitNode( node->leftChild , l_splits , l_num+b_num , depth + 1 );
	m_total ++;

	BBox right_box = node->bbox;
	right_box.m_Min[split_Axis] = split_pos;
	node->rightChild = new Kd_Node(right_box);
	splitNode( node->rightChild , r_splits , r_num+b_num , depth + 1 );
	m_total ++;

	m_depth = max( m_depth , depth );
}

// evaluate sah value for a specific split plane
float KDTree::sah( unsigned l , unsigned r , unsigned f , unsigned axis , float split , const BBox& box , bool& left )
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
float KDTree::pickSplitting( const Splits& splits , unsigned prinum , const BBox& box ,
							 unsigned& splitAxis , float& split_pos , bool& left )
{
	float min_sah = FLT_MAX;
	for( int k = 0 ; k < 3 ; k++ )
	{
		unsigned n_l = 0 ;
		unsigned n_r = prinum ;
		unsigned split_count = splits.split_c[k];
		unsigned i = 0;
		while( i < split_count )
		{
			unsigned pe = 0 , pf = 0 , ps = 0;
			float split = splits.split[k][i].pos;

            while( i < split_count && splits.split[k][i].pos == split && splits.split[k][i].type == Split_Type::Split_End )
			{i++;pe++;}
            while( i < split_count && splits.split[k][i].pos == split && splits.split[k][i].type == Split_Type::Split_Flat )
			{i++;pf++;}
            while( i < split_count && splits.split[k][i].pos == split && splits.split[k][i].type == Split_Type::Split_Start )
			{i++;ps++;}

			n_r -= pe;
			n_r -= pf;

			// get the sah value
			bool _left = false;
			float sahv = sah( n_l , n_r , pf , k , split , box , _left );
			if( sahv < min_sah )
			{
				min_sah = sahv;
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

// make leaf kd-tree node
void KDTree::makeLeaf( Kd_Node* node , Splits& splits , unsigned prinum )
{
	node->flag = 3;
	for( unsigned i = 0 ; i < splits.split_c[0] ; i++ ){
        if( splits.split[0][i].type == Split_Type::Split_Start || splits.split[0][i].type == Split_Type::Split_Flat ){
			const Primitive* primitive = splits.split[0][i].primitive;
			if( primitive->GetIntersect( node->bbox ) )
				node->trilist.push_back(primitive);
		}
	}
	splits.Release();

	m_leaf++;
	m_fAvgLeafTri += prinum;
	m_MaxLeafTri = max( m_MaxLeafTri , prinum );
}

// get the intersection between the ray and the primitive set
bool KDTree::GetIntersect( const Ray& r , Intersection* intersect ) const
{
	float fmax;
	float fmin = Intersect( r , m_bbox , &fmax );
	if( fmin < 0.0f )
		return false;

	if( traverse( m_root , r , intersect , fmin , fmax ) )
	{
		if( intersect == 0 )
			return true;
		return intersect->primitive != 0;
	}
	return false;
}

// traverse kd-tree node
bool KDTree::traverse( const Kd_Node* node , const Ray& ray , Intersection* intersect , float fmin , float fmax ) const
{
	static const unsigned	mask = 0x00000003;
	static const float		delta = 0.001f;

    if( fmin > fmax )
        return false;
    
	if( intersect && intersect->t < fmin - delta )
		return true;

	// it's a leaf node
	if( (node->flag & mask) == 3 ){
		bool inter = false;
        for( auto tri : node->trilist ){
			inter |= tri->GetIntersect( ray , intersect );
			if( intersect == 0 && inter )
				return true;
		}
		return inter && ( intersect->t < ( fmax + delta ) && intersect->t > ( fmin - delta ) );
	}

	// get the intersection point between the ray and the splitting plane
	const unsigned split_axis = node->flag & mask;
	const float dir = ray.m_Dir[split_axis];
	const float t = (dir==0.0f) ? FLT_MAX : ( node->split - ray.m_Ori[split_axis] ) / dir;
	
	const Kd_Node* first = node->leftChild;
	const Kd_Node* second = node->rightChild;
	if( dir < 0.0f || (dir==0.0f&&ray.m_Ori[split_axis] > node->split) )
        swap(first, second);

	bool inter = false;
	if( t > fmin - delta ){
		inter = traverse( first , ray , intersect , fmin , min( fmax , t ) );
		if( !intersect && inter )
			return true;
	}
	if( !inter && ( fmax + delta ) > t )
		return traverse( second , ray , intersect , max( t , fmin ) , fmax );
	return inter;
}

// delete all kd-tree nodes
void KDTree::deleteKdNode( Kd_Node* node )
{
	if( !node )
		return;
	deleteKdNode( node->leftChild );
	deleteKdNode( node->rightChild );
	delete node;
}
