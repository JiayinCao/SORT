/*
 * filename :	kdtree.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "kdtree.h"
#include "managers/memmanager.h"

// the memory id used in kdtree
static unsigned	KDTREE_MEM0 = 1;
static unsigned KDTREE_MEM1 = 2;

// split type
enum Split_Type
{
	Split_Start = 1 ,
	Split_End = 2 ,
	Split_Left = 4 , 
	Split_Right = 8 ,
	Split_Mask = Split_Start | Split_End
};

// split candidates
struct Split
{
public:
	// the position of the split
	float		pos;
	// the type of the split , start or end
	Split_Type	type;
};

// default constructor
KDTree::KDTree()
{
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
	// get the bounding box for the whole primitive list
	_computeBBox();
	
	// allocate some memory to be used in kdtree construction
	SORT_PREMALLOC( 4 * 1024 * 1024 , KDTREE_MEM0 );
	SORT_PREMALLOC( 4 * 1024 * 1024 , KDTREE_MEM1 );

	// release the memory
	SORT_DEALLOC( KDTREE_MEM0 );
	SORT_DEALLOC( KDTREE_MEM1 );
}

// output log
void KDTree::OutputLog() const
{
	LOG_HEADER( "Accelerator" );
	LOG<<"KD-Tree is not implemented yet."<<ENDL<<ENDL;
}
