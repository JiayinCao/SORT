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

#include "kdtree.h"
#include "geometry/primitive.h"
#include "geometry/intersection.h"
#include <algorithm>

IMPLEMENT_CREATOR( KDTree );

SORT_STATS_DEFINE_COUNTER(sKDTreeNodeCount)
SORT_STATS_DEFINE_COUNTER(sKDTreeLeafNodeCount)
SORT_STATS_DEFINE_COUNTER(sKDTreeDepth)
SORT_STATS_DEFINE_COUNTER(sKDTreeMaxPriCountInLeaf)
SORT_STATS_DEFINE_COUNTER(sKDTreePrimitiveCount)

SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Total Ray Count", sRayCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Shadow Ray Count", sShadowRayCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Intersection Test", sIntersectionTest );
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Node Count", sKDTreeNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Leaf Node Count", sKDTreeLeafNodeCount);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "KDTree Depth", sKDTreeDepth);
SORT_STATS_COUNTER("Spatial-Structure(KDTree)", "Maximum Primitive in Leaf", sKDTreeMaxPriCountInLeaf);
SORT_STATS_AVG_COUNT("Spatial-Structure(KDTree)", "Average Primitive Count in Leaf", sKDTreePrimitiveCount , sKDTreeLeafNodeCount );
SORT_STATS_AVG_COUNT("Spatial-Structure(KDTree)", "Average Primitive Tested per Ray", sIntersectionTest, sRayCount);

// destructor
KDTree::~KDTree()
{
    SORT_PROFILE("Destructe KD-Tree");

	// delete kd-tree
	deleteKdNode( m_root );
}

// build the acceleration structure
void KDTree::Build()
{
    SORT_PROFILE("Build KdTree");

	if( m_primitives->size() == 0 )
		return;

	// pre-malloc node and leaf primitive list memory
	m_temp = new unsigned char[m_primitives->size()];

	// get the bounding box for the whole primitive list
	computeBBox();
	
	// create the split candidates
	unsigned count = (unsigned)m_primitives->size();
	Splits splits;
	for( int i = 0 ; i < 3 ; i++ )
		splits.split[i] = new Split[2*count];
	for( int k = 0 ; k < 3 ; k++ ){
		for( unsigned i = 0 ; i < count ; i++ ){
			Primitive* pri = (*m_primitives)[i];
			const BBox& box = pri->GetBBox();
            splits.split[k][2*i] = Split(box.m_Min[k], Split_Type::Split_Start, i, pri);
            splits.split[k][2*i+1] = Split(box.m_Max[k], Split_Type::Split_End, i, pri);
		}
	}
	for( int i = 0 ; i < 3 ; i++ )
		std::sort( splits.split[i] , splits.split[i] + 2 * count);

	// create root node
	m_root = new Kd_Node(m_bbox);

	// build kd-tree
	splitNode( m_root , splits , count , 0 );

    SORT_STATS(++sKDTreeNodeCount);

	// delete temporary memory
	SAFE_DELETE_ARRAY(m_temp);
}

// split node
void KDTree::splitNode( Kd_Node* node , Splits& splits , unsigned prinum , unsigned depth )
{
    SORT_STATS(sKDTreeDepth = std::max(sKDTreeDepth, (StatsInt)depth+1));
    
	if( prinum < m_maxPriInLeaf || depth > m_maxDepth ){
		makeLeaf( node , splits , prinum );
		return;
	}

	// ----------------------------------------------------------------------------------------
	// step 1
	// pick best split
	unsigned	split_offset;
	unsigned 	split_Axis;
	float sah = pickSplitting( splits , prinum , node->bbox , split_Axis , split_offset);
	if( sah >= prinum ){
		makeLeaf( node , splits , prinum );
		return;
	}
    node->flag = split_Axis;
    node->split = splits.split[split_Axis][split_offset].pos;

	// ----------------------------------------------------------------------------------------
	// step 2
	// distribute primitives
    const unsigned split_count = prinum * 2;
	Split* _splits = splits.split[split_Axis];
	unsigned l_num = 0 , r_num = 0;
	for( unsigned i = 0 ; i < split_count; i++ )
		m_temp[splits.split[split_Axis][i].id] = 0;
	for( unsigned i = 0 ; i < split_count; i++ )
	{
        if (i < split_offset) {
            if (_splits[i].type == Split_Type::Split_Start) {
                m_temp[_splits[i].id] |= 1;
                ++l_num;
            }
        }
        else if (i > split_offset) {
            if (_splits[i].type == Split_Type::Split_End) {
                m_temp[_splits[i].id] |= 2;
                ++r_num;
            }
        }
	}

	// ----------------------------------------------------------------------------------------
	// step 3
	// generate new events
	Splits l_splits;
	Splits r_splits;
	for( int k = 0 ; k < 3 ; k++ ){
		l_splits.split[k] = new Split[2*l_num];
		r_splits.split[k] = new Split[2*r_num];
	}
	for( int k = 0 ; k < 3 ; k++ ){
        int l_offset = 0, r_offset = 0;
		for( unsigned i = 0 ; i < split_count ; i++ ){
            const Split& old = splits.split[k][i];
            unsigned id = old.id;
            if (m_temp[id] & 0x01)
                l_splits.split[k][l_offset++] = old;
            if (m_temp[id] & 0x02)
                r_splits.split[k][r_offset++] = old;
		}
        sAssert(l_offset == 2 * l_num, SPATIAL_ACCELERATOR);
        sAssert(r_offset == 2 * r_num, SPATIAL_ACCELERATOR);
	}
	splits.Release();

	BBox left_box = node->bbox;
	left_box.m_Max[split_Axis] = node->split;
	node->leftChild = new Kd_Node(left_box);
	splitNode( node->leftChild , l_splits , l_num , depth + 1 );

	BBox right_box = node->bbox;
	right_box.m_Min[split_Axis] = node->split;
	node->rightChild = new Kd_Node(right_box);
	splitNode( node->rightChild , r_splits , r_num , depth + 1 );

    SORT_STATS(sKDTreeNodeCount += 2);
}

// evaluate sah for a specific split plane
float KDTree::sah( unsigned l , unsigned r , unsigned axis , float split , const BBox& box )
{
	float inv_sarea = 1.0f / box.HalfSurfaceArea();

	Vector delta = box.m_Max - box.m_Min;
	delta[axis] = split - box.m_Min[axis];
	float l_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;
	delta[axis] = box.m_Max[axis] - split;
	float r_sarea = delta.x * delta.y + delta.y * delta.z + delta.z * delta.x;

	return ( l * l_sarea + r * r_sarea ) * inv_sarea;
}

// pick best splitting
float KDTree::pickSplitting( const Splits& splits , unsigned prinum , const BBox& box ,
							 unsigned& splitAxis , unsigned& split_offset )
{
	float min_sah = FLT_MAX;
	for( int k = 0 ; k < 3 ; k++ )
	{
		unsigned n_l = 0 ;
		unsigned n_r = prinum ;
		unsigned split_count = prinum * 2;
		unsigned i = 0;
		while( i < split_count ){
            if (splits.split[k][i].pos <= box.m_Min[k] ){
                ++i;
                continue;
            }
            if( splits.split[k][i].pos >= box.m_Max[k] )
                break;
            
            if (splits.split[k][i].type == Split_Type::Split_End)
                --n_r;

			// get the sah
			float sahv = sah( n_l , n_r , k , splits.split[k][i].pos , box );
			if( sahv < min_sah ){
				min_sah = sahv;
				splitAxis = k;
                split_offset = i;
			}
			
            if (splits.split[k][i].type == Split_Type::Split_Start)
                ++n_l;
            
            ++i;
		}
	}

	return min_sah;
}

// make leaf kd-tree node
void KDTree::makeLeaf( Kd_Node* node , Splits& splits , unsigned prinum )
{
	node->flag = 3;
	for( unsigned i = 0 ; i < prinum * 2; i++ ){
        if( splits.split[0][i].type == Split_Type::Split_Start ){
			const Primitive* primitive = splits.split[0][i].primitive;
			if( primitive->GetIntersect( node->bbox ) )
				node->primitivelist.push_back(primitive);
		}
	}
	splits.Release();

    SORT_STATS(++sKDTreeLeafNodeCount);
    SORT_STATS(++sKDTreeNodeCount);
    SORT_STATS(sKDTreePrimitiveCount += prinum);
    SORT_STATS(sKDTreeMaxPriCountInLeaf = std::max(sKDTreeMaxPriCountInLeaf, (StatsInt)prinum));
}

// get the intersection between the ray and the primitive set
bool KDTree::GetIntersect( const Ray& r , Intersection* intersect ) const
{
    SORT_PROFILE("Traverse KD-Tree");
    SORT_STATS(++sRayCount);
    SORT_STATS(sShadowRayCount += (intersect == nullptr));

	float fmax;
	float fmin = Intersect( r , m_bbox , &fmax );
	if( fmin < 0.0f )
		return false;

	if( traverse( m_root , r , intersect , fmin , fmax ) ){
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
        for( auto primitive : node->primitivelist ){
            SORT_STATS(++sIntersectionTest);
			inter |= primitive->GetIntersect( ray , intersect );
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
        std::swap(first, second);

	bool inter = false;
	if( t > fmin - delta ){
		inter = traverse( first , ray , intersect , fmin , std::min( fmax , t ) );
		if( !intersect && inter )
			return true;
	}
	if( !inter && ( fmax + delta ) > t )
		return traverse( second , ray , intersect , std::max( t , fmin ) , fmax );
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
