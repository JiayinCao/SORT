/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <OSL/oslexec.h>
#include <OSL/oslcomp.h>
#include <OSL/oslquery.h>
#include "thirdparty/gtest/gtest.h"

#include "core/define.h"

using namespace OSL;

#define OSL_TEST_VERBOSE    0

#if defined(SORT_IN_WINDOWS)
    #define STDOSL_PATH     "..\\dependencies\\osl\\shader\\stdosl.h"
#else
    #define STDOSL_PATH     "../dependencies/osl/shader/stdosl.h"
#endif

class SORTRenderServices : public RendererServices{
public:
    SORTRenderServices(){}
    virtual int supports (string_view feature) const{ return 0; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             TransformationPtr xform,
                             float time){ return true; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             ustring from, float time){ return true; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             TransformationPtr xform){ return true; }
    virtual bool get_matrix (ShaderGlobals *sg, Matrix44 &result,
                             ustring from){ return true; }
    virtual bool get_inverse_matrix (ShaderGlobals *sg, Matrix44 &result,
                                     ustring to, float time) { return true; }

    virtual bool get_array_attribute (ShaderGlobals *sg, bool derivatives, 
                                      ustring object, TypeDesc type, ustring name,
                                      int index, void *val ){ return true; }
    virtual bool get_attribute (ShaderGlobals *sg, bool derivatives, ustring object,
                                TypeDesc type, ustring name, void *val){ return true; }
    virtual bool get_userdata (bool derivatives, ustring name, TypeDesc type, 
                               ShaderGlobals *sg, void *val){ return true;}
};

// Compile OSL source code
bool compile_buffer ( ShadingSystem* shadingsys , const std::string &sourcecode, const std::string &shadername){
    // Compile a OSL shader.
    OSLCompiler compiler;
    std::string osobuffer;
    std::vector<std::string> options;
    if (!compiler.compile_buffer(sourcecode, osobuffer, options, STDOSL_PATH))
        return false;

    // Load shader from compiled object file.
    if (! shadingsys->LoadMemoryCompiledShader (shadername, osobuffer))
        return false;
    
    return true;
}

// This unit test will cover the basics of Open Shading Language, like read input and output from a shader ,
// exectute a shader and verify the result
TEST(OSL, Basic) {
    static std::string shader_source =  "shader TestBasic( color Cin = 0.5 , float exponent = 2.0 , output color Cout = 1.0 ){ Cout = pow( Cin , exponent ); }";

    ErrorHandler errhandler;
    SORTRenderServices rs;

    auto shadingsys = std::make_unique<ShadingSystem>(&rs, nullptr, &errhandler);

    shadingsys->attribute("lockgeom", 1);
#if OSL_TEST_VERBOSE
    errhandler.verbosity (ErrorHandler::VERBOSE);
#endif

    const auto group_name = "Basic_Test_ShaderGroup";
    const auto shader_name = "test_basic_osl";
    const auto shader_layer = "test_basic_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Basic_Test_ShaderGroup");
    const auto ret = compile_buffer( shadingsys.get() , shader_source , shader_name );
    shadingsys->Shader ("surface", shader_name, shader_layer);
    shadingsys->ShaderGroupEnd ();

    // Making sure the shader is successfully compiled.
    EXPECT_TRUE( ret );

    ustring groupname;
    shadingsys->getattribute (shadergroup.get(), "groupname", groupname);
#if OSL_TEST_VERBOSE
    std::cout << "Shader group name:\t" << groupname << std::endl;
#endif
    const auto group_name_match = std::strcmp( groupname.c_str() , group_name ) == 0;
    EXPECT_TRUE( group_name_match );

    // There should be exactly one layer
    int num_layers = 0;
    shadingsys->getattribute (shadergroup.get(), "num_layers", num_layers);
    EXPECT_EQ( num_layers , 1 );

    // Checking the layer name
    const char* layers[1] = { nullptr };
    shadingsys->getattribute (shadergroup.get(), "layer_names", TypeDesc(TypeDesc::STRING, num_layers), &layers[0]);
#if OSL_TEST_VERBOSE
    std::cout<<"Layer name:\t\t"<<layers[0]<<std::endl;
#endif
    const auto layer_name_match = std::strcmp( layers[0] , shader_layer ) == 0;
    EXPECT_TRUE( layer_name_match );

#if OSL_TEST_VERBOSE
    std::cout << layers[0] << "\n";
#endif
    OSLQuery q;
    q.init (shadergroup.get(), 0);
    for (size_t p = 0;  p < q.nparams(); ++p) {
        const OSLQuery::Parameter *param = q.getparam(p);
#if OSL_TEST_VERBOSE
        std::cout << "\t" << (param->isoutput ? "output "  : "input ") << param->type << ' ' << param->name << "\n";
#endif
    }

    static const auto verify_param = [=]( const int index , const std::string& name , const bool input ){
        const auto param = q.getparam(index);
#if OSL_TEST_VERBOSE
        std::cout << "\t" << (param->isoutput ? "output "  : "input ") << param->type << ' ' << param->name << "\n";
#endif
        const auto param_match = std::strcmp( param->name.c_str() , name.c_str() ) == 0;
        EXPECT_TRUE( param_match );
        EXPECT_EQ( param->isoutput , !input );
    };

    verify_param( 0 , "Cin" , true );
    verify_param( 1 , "exponent" , true );
    verify_param( 2 , "Cout" , false );

    // Set attribute
    const std::vector<const char *> aovnames( 1 , { "Cout" } );
    shadingsys->attribute ( nullptr, "renderer_outputs", TypeDesc(TypeDesc::STRING,1), &aovnames[0] );

    // create a new thread and the context
    OSL::PerThreadInfo *thread_info = shadingsys->create_thread_info();
    ShadingContext *ctx = shadingsys->get_context (thread_info);

    // Execute the shader
    ShaderGlobals shaderglobals;
    shadingsys->execute (ctx, *shadergroup, shaderglobals);

    // Getting back the result
    const auto sym = shadingsys->find_symbol (*shadergroup, ustring("Cout") );
    EXPECT_NE( sym , nullptr );

    TypeDesc t;
    const void *data = shadingsys->get_symbol (*ctx, ustring("Cout"), t);
    EXPECT_NE( data , nullptr );

    // It should be exactly 0.25f, there should be no error.
    const float* fdata = ( const float* ) data;
    EXPECT_EQ( fdata[0] , 0.25f );
    EXPECT_EQ( fdata[1] , 0.25f );
    EXPECT_EQ( fdata[2] , 0.25f );

    shadingsys->release_context (ctx);
    shadingsys->destroy_thread_info(thread_info);
}