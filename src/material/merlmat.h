/*
 * filename	:	merlmat.h
 *
 * programmer :	Cao Jiayin
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
	// default constructor
	MerlMat();
	
	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

	// create instance of the brdf
	virtual Material* CreateInstance(){return new MerlMat();}

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
	class MerlProperty : public PropertyHandler<Material>
	{
	public:
		// constructor
		MerlProperty(Material* matte):PropertyHandler(matte){}

		// set value
		void SetValue( const string& str )
		{
			// cast target
			MerlMat* merlmat = dynamic_cast<MerlMat*>(m_target);

			// create a merl bxdf
			Merl* merl = new Merl( str );

			if( merl->IsValid() )
				merlmat->m_bxdf.push_back( merl );
			else
				delete merl;
		}
	};
};

#endif