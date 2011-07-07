/*
 * filename :	matte.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_MATTE
#define	SORT_MATTE

// include header file
#include "material.h"
#include "texture/texture.h"

//////////////////////////////////////////////////////////////////////
// definition of matte material
class Matte : public Material , public PropertySet<Matte>
{
// public method
public:
	// default constructor
	Matte();
	// destructor
	~Matte();

	// get bsdf
	virtual Bsdf* GetBsdf( const Intersection* intersect ) const;

	// set scaled color
	void SetDiffuse( Texture* tex ) { m_d = tex; }

	// create instance of the brdf
	virtual Material* CreateInstance(){return new Matte();}

	// set property
	// para 'nane'  : name for the property
	// para 'value' : value for the property
	virtual void SetProperty( const string& name , const string& value ){SetProp( name , value );}

// private field
private:
	// the scaled color for the material
	Texture* m_d;

	// register property
	void _registerAllProperty();

// property handler
	class DiffuseProperty : public PropertyHandler<Matte>
	{
	public:
		// constructor
		DiffuseProperty(Matte* matte):PropertyHandler(matte){}

		// set value
		void SetValue( const string& value )
		{
			//temp
			cout<<value<<endl;
		}
	};
};

#endif