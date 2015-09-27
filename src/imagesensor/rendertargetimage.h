/*
   FileName:      imageoutput.h

   Created Time:  2015-09-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_IMAGEOUTPUT
#define SORT_IMAGEOUTPUT

#include "imagesensor.h"
#include "texture/rendertarget.h"

// generate output
class RenderTargetImage : public ImageSensor
{
public:
    // constructor
    RenderTargetImage()
    {
        _registerAllProperty();
    }
	// store pixel information
	virtual void StorePixel( int x , int y , const Spectrum& color , const RenderTask& rt );

    // Pre process
    virtual void PreProcess();
	// post process
	virtual void PostProcess();

private:
    // render target
	RenderTarget m_rendertarget;
    
    // filename
    string      m_filename;
    
    // register property
    void _registerAllProperty()
    {
        _registerProperty( "filename" , new FilenameProperty( this ) );
    }
    
    class FilenameProperty : public PropertyHandler<ImageSensor>
    {
    public:
        PH_CONSTRUCTOR(FilenameProperty,ImageSensor);
        
        // set value
        void SetValue( const string& str )
        {
            RenderTargetImage* rti = CAST_TARGET(RenderTargetImage);
            rti->m_filename = str;
        }
    };
};

#endif