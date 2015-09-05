/*
   FileName:      merlmat.h

   Created Time:  2011-08-04 12:46:45

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MERLMAT
#define	SORT_MERLMAT

// include material
#include "material.h"
#include <vector>
#include "bsdf/merl.h"

//////////////////////////////////////////////////////////////////////////
//	definition of merl material
class MerlMat : public Material
{
// public method
public:
	DEFINE_CREATOR( MerlMat , "Merl" );

	// default constructor
	MerlMat();
	
	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

// private field
private:
	// the bxdf list
	vector<Merl*>	m_bxdf;

	// register property
	void _registerAllProperty();

	// initialize default value and register property
	void _init();

	// release
	void _release();

	// destructor
	~MerlMat();

// property handler
	/*class MerlProperty : public PropertyHandler<Material>
	{
	public:
		PH_CONSTRUCTOR(MerlProperty,Material);

		// set value
		void SetValue( const string& str )
		{
			// cast target
			MerlMat* merlmat = CAST_TARGET(MerlMat);

			// create a merl bxdf
			Merl* merl = new Merl( str );

			if( merl->IsValid() )
				merlmat->m_bxdf.push_back( merl );
			else
				delete merl;
		}
	};*/
};

#endif
