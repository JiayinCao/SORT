/*
   FileName:      glass.h

   Created Time:  2011-08-04 12:46:29

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_GLASS
#define	SORT_GLASS

#include "material.h"
#include "spectrum/spectrum.h"
#include "utility/strhelper.h"

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
	Spectrum color;

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
	class ColorProperty : public PropertyHandler<Material>
	{
	public:
		ColorProperty(Material* mat):PropertyHandler(mat){}

		// set value
		void SetValue( const string& str )
		{
			Glass* glass = CAST_TARGET(Glass);
			glass->color = SpectrumFromStr( str );
		}
	};
};

#endif
