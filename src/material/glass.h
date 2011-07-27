/*
 * filename :	glass.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_GLASS
#define	SORT_GLASS

#include "material.h"

///////////////////////////////////////////////////////////////////
// definition of glass
class	Glass : public Material
{
// public method
public:
	DEFINE_CREATOR( Glass );
	
	// constructor
	Glass();
	// destructor
	~Glass(){}

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

// private field
private:
	float eta_i;
	float eta_t;

	// register property
	void _registerAllProperty();

// property handler
	class EtaIProperty : public PropertyHandler<Material>
	{
	public:
		// constructor
		EtaIProperty(Material* mat):PropertyHandler(mat){}

		// set value
		void SetValue( const string& str )
		{
			Glass* glass = CAST_TARGET(Glass);
			glass->eta_i = (float)atof( str.c_str() );
		}
	};
	class EtaTProperty : public PropertyHandler<Material>
	{
	public:
		EtaTProperty(Material* mat):PropertyHandler(mat){}

		// set value
		void SetValue( const string& str )
		{
			Glass* glass = CAST_TARGET(Glass);
			glass->eta_t = (float)atof( str.c_str() );
		}
	};
};

#endif
