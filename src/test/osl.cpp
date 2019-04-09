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
#include <OSL/genclosure.h>
#include <OSL/oslclosure.h>
#include "thirdparty/gtest/gtest.h"
#include "core/define.h"
#include "unittest_common.h"
#include "material/osl_system.h"
#include "bsdf/orennayar.h"

using namespace OSL;

#define OSL_TEST_VERBOSE    0

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
bool compile_buffer_test( ShadingSystem* shadingsys , const std::string &sourcecode, const std::string &shadername){
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

// Build a shader from source code
void build_shader_test( ShadingSystem* shadingsys , const std::string& shader_source, const std::string& shader_name, const std::string& shader_layer , const std::string& shader_group_name = "" ) {
    auto ret = compile_buffer_test(shadingsys, shader_source, shader_name);
    shadingsys->Shader("surface", shader_name, shader_layer);
    EXPECT_TRUE(ret);
};

// Create shading system
std::unique_ptr<ShadingSystem>  MakeOSLShadingSys(){
    static ErrorHandler errhandler;
    static SORTRenderServices rs;
    auto shadingsys = std::make_unique<ShadingSystem>(&rs, nullptr, &errhandler);

    // Making sure shading system is created already.
    EXPECT_NE( shadingsys , nullptr );

#if OSL_TEST_VERBOSE
    errhandler.verbosity (ErrorHandler::VERBOSE);
#endif

    return std::move( shadingsys );
}

// This unit test will cover the basics of Open Shading Language, like read input and output from a shader ,
// execute a shader and verify the result
TEST(OSL, CheckingSymbol) {
    static const std::string shader_source =  
        "shader TestBasic("
        "   color Cin = 0.5 ,"
        "   float exponent = 2.0 ,"
        "   output color Cout = 1.0 )"
        "{"
        "   Cout = pow( Cin , exponent );"
        "}";

    auto shadingsys = MakeOSLShadingSys();

    const auto group_name = "Basic_Test_ShaderGroup";
    const auto shader_name = "test_basic_osl";
    const auto shader_layer = "test_basic_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Basic_Test_ShaderGroup");
    build_shader_test( shadingsys.get() , shader_source , shader_name , shader_layer , group_name );
    shadingsys->ShaderGroupEnd ();

    ustring groupname;
    shadingsys->getattribute (shadergroup.get(), "groupname", groupname);
#if OSL_TEST_VERBOSE
    std::cout << "Shader group name:\t" << groupname << std::endl;
#endif
    const auto group_name_match = std::strcmp( groupname.c_str() , group_name );
    EXPECT_EQ( group_name_match , 0 );

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
    const auto layer_name_match = std::strcmp( layers[0] , shader_layer );
    EXPECT_EQ( layer_name_match , 0 );

#if OSL_TEST_VERBOSE
    std::cout << layers[0] << std::endl;
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
        const auto param_match = std::strcmp( param->name.c_str() , name.c_str() );
        EXPECT_EQ( param_match , 0 );
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

// Check basic closure operations
TEST(OSL, CheckingClosure) {
    static const std::string shader_source =  
        "shader TestBasic()"
        "{"
        "    color tint = color( 0.25 , 1.25 , 0.125 );"
        "    float gamma = 0.1;"
        "    Ci = lambert(tint,N) * tint + orenNayar(tint,gamma,N);"
        "}";

    // Create OSL shading system
    auto shadingsys = MakeOSLShadingSys();

    // Register closures
    register_closures( shadingsys.get() );

    const auto group_name = "default_shader_group";
    const auto shader_name = "default_shader";
    const auto shader_layer = "default_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Closure_Test_ShaderGroup");
    build_shader_test( shadingsys.get() , shader_source , shader_name , shader_layer , group_name );
    shadingsys->ShaderGroupEnd ();

    ShadingContextWrapper sc;
    auto ctx = sc.GetShadingContext( shadingsys.get() );

    ShaderGlobals shaderglobals;
    shadingsys->execute (ctx, *shadergroup, shaderglobals);

    const auto closure = shaderglobals.Ci;

    EXPECT_NE( closure , nullptr );
    EXPECT_EQ( closure->id , ClosureColor::ADD );

    const auto closureA = closure->as_add()->closureA;
    const auto closureB = closure->as_add()->closureB;

    EXPECT_NE( closureA , nullptr );
    EXPECT_NE( closureB , nullptr );
    EXPECT_EQ( closureA->id , CLOSURE_LAMBERT );
    EXPECT_EQ( closureB->id , CLOSURE_OREN_NAYAR );

    const auto compA = closureA->as_comp();
    const auto cwA = compA->w;

    EXPECT_EQ( cwA.x , 0.25f );
    EXPECT_EQ( cwA.y , 1.25f );
    EXPECT_EQ( cwA.z , 0.125f );

    // 0.1 can't be represented in 32 bits float, but it is picked for the purpose to check IEEE implementation of OSL.
    // It should be exactly the same with C++ compiler if it does what IEEE standard requests.
    const auto compB = closureB->as_comp();
    const auto& params = *compB->as<OrenNayar::Params>();
    const float expected_sigam = 0.1f;
    EXPECT_EQ( params.sigma , expected_sigam );
}

// Check multiple layers
TEST(OSL, CheckingMultipleLayers) {
    static const std::string shader0_source =
        "shader Shader0("
        "   color   iColor_0 = color( 0.1 , 0.2 , 0.3 ),"
        "   output  color oColor_0 = color( 0.2 , 0.3 , 0.4 ),"
        "   output  float oScaler_0 = 0.3)"
        "{"
        "    oColor_0 = iColor_0 * 0.5;"
        "    oScaler_0 = 0.123;"
        "}";
    static const std::string shader1_source =
        "shader Shader1("
        "   color   iColor_0 = color( 0.125 , 0.25 , 0.5 ),"
        "   float   iScale_0 = 0.5,"
        "   output  color oColor_0 = color( 0.2 , 0.3 , 0.4 ))"
        "{"
        "    oColor_0 = iColor_0 * iScale_0 + 0.5; /* oColor_0 should be ( 0.5625 , 0.625 , 0.75 ) */"
        "}";
    static const std::string shader2_source =
        "shader Shader2("
        "   float iScale_0 = 0.1,"
        "   float iScale_1 = 0.2,"
        "   color iColor_0 = color( 0.2 , 0.2 , 0.2 ))"
        "{"
        "    Ci = lambert( iColor_0 , N ) * iScale_0 + orenNayar( iColor_0 , iScale_1 , N ) * iColor_0;"
        "}";
    static const std::string shader3_source =
        "shader Extract("
        "   color iColor_0 = color( 0.2 , 0.0 , 0.0 ),"
        "   output float oRed = 0.0,"
        "   output float oGreen = 0.0,"
        "   output float oBlue = 0.0)"
        "{"
        "   oRed = iColor_0[0];"
        "   oGreen = iColor_0[1];"
        "   oBlue = iColor_0[2];"
        "}";

    // Create OSL shading system
    auto shadingsys = MakeOSLShadingSys();

    // Register closures
    register_closures( shadingsys.get() );

    const auto shadergroup = shadingsys->ShaderGroupBegin("Default_Shader_Group");
    build_shader_test(shadingsys.get(), shader0_source, "shader0", "shader0_layer");
    build_shader_test(shadingsys.get(), shader1_source, "shader1", "shader1_layer");
    build_shader_test(shadingsys.get(), shader3_source, "shader3_0", "extract0");
    build_shader_test(shadingsys.get(), shader3_source, "shader3_1", "extract1");
    build_shader_test(shadingsys.get(), shader2_source, "shader2", "shader2_layer");
    shadingsys->ConnectShaders("shader0_layer", "oColor_0", "extract0", "iColor_0");
    shadingsys->ConnectShaders("shader1_layer", "oColor_0", "extract1", "iColor_0");
    shadingsys->ConnectShaders("extract0", "oRed", "shader2_layer", "iScale_0");
    shadingsys->ConnectShaders("extract1", "oGreen", "shader2_layer", "iScale_1");
    shadingsys->ConnectShaders("shader1_layer", "oColor_0", "shader2_layer", "iColor_0");
    shadingsys->ShaderGroupEnd();

    ShadingContextWrapper sc;
    auto ctx = sc.GetShadingContext( shadingsys.get() );

    ShaderGlobals shaderglobals;
    shadingsys->execute(ctx, *shadergroup, shaderglobals);

    const auto closure = shaderglobals.Ci;

    EXPECT_NE(closure , nullptr);
    EXPECT_EQ(closure->id , ClosureColor::ADD);

    const auto closureA = closure->as_add()->closureA;
    const auto closureB = closure->as_add()->closureB;

    EXPECT_NE(closureA , nullptr);
    EXPECT_EQ(closureA->id, ClosureColor::MUL);

    const auto cw = closureA->as_mul()->weight;
    EXPECT_EQ(cw.x, 0.05f);
    EXPECT_EQ(cw.y, 0.05f);
    EXPECT_EQ(cw.z, 0.05f);

    EXPECT_NE(closureB , nullptr);
    EXPECT_EQ(closureB->id, CLOSURE_OREN_NAYAR);

    const auto compB = closureB->as_comp();
    const auto w = compB->w;
    EXPECT_EQ(w.x, 0.5625f);
    EXPECT_EQ(w.y, 0.625f);
    EXPECT_EQ(w.z, 0.75f);
    
    const auto& params = *compB->as<OrenNayar::Params>();
    const float expected_sigam = 0.625f;
    EXPECT_EQ(params.sigma, expected_sigam);

    const auto closureC = closureA->as_mul()->closure;
    EXPECT_EQ(closureC->id, CLOSURE_LAMBERT);
}

// Checking reset default value for input parameters
TEST(OSL, CheckingDefaultValue) {
    static const std::string shader_source =  
        "shader TestBasic("
        "   color iColor_0 = color( 0.125 , 0.5 , 2.5 ) ,"
        "   float iScale_0 = 12.0 )"
        "{"
        "    Ci = lambert( iColor_0 , N ) * iColor_0 + orenNayar( iColor_0, iScale_0, N );"
        "}";

    // Create OSL shading system
    auto shadingsys = MakeOSLShadingSys();

    // Register closures
    register_closures( shadingsys.get() );
    
    const auto group_name = "default_shader_group";
    const auto shader_name = "default_shader";
    const auto shader_layer = "default_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Closure_Test_ShaderGroup");
    const auto ret = compile_buffer_test( shadingsys.get() , shader_source , shader_name );
    const auto iScale_0 = 2.0f;
    shadingsys->Parameter( "iScale_0" , TypeDesc::TypeFloat, &iScale_0);
    const float iColor_0[3] = { 0.0125f , 1.25f , 5.0f };
    shadingsys->Parameter( "iColor_0" , TypeDesc::TypeColor, &iColor_0);
    shadingsys->Shader ("surface", shader_name, shader_layer);
    shadingsys->ShaderGroupEnd ();

    ShadingContextWrapper sc;
    auto ctx = sc.GetShadingContext( shadingsys.get() );

    ShaderGlobals shaderglobals;
    shadingsys->execute (ctx, *shadergroup, shaderglobals);

    const auto closure = shaderglobals.Ci;

    EXPECT_NE( closure , nullptr );
    EXPECT_EQ( closure->id , ClosureColor::ADD );

    const auto closureA = closure->as_add()->closureA;
    const auto closureB = closure->as_add()->closureB;

    EXPECT_NE( closureA , nullptr );
    EXPECT_NE( closureB , nullptr );
    EXPECT_EQ( closureA->id , CLOSURE_LAMBERT);
    EXPECT_EQ( closureB->id , CLOSURE_OREN_NAYAR);

    const auto compA = closureA->as_comp();
    const auto cwA = compA->w;

    EXPECT_EQ( cwA.x , iColor_0[0] );
    EXPECT_EQ( cwA.y , iColor_0[1] );
    EXPECT_EQ( cwA.z , iColor_0[2] );

    const auto compB = closureB->as_comp();
    const auto& params = *compB->as<OrenNayar::Params>();
    EXPECT_EQ( params.sigma , iScale_0 );
}

// Checking mutli-thread execution of multiple shader group instance
TEST(OSL, CheckingMultiThread) {
    static const std::string shader_source =  
        "shader TestBasic("
        "   color iColor_0 = color( 0.125 , 0.5 , 2.5 ) ,"
        "   float iScale_0 = 12.0 )"
        "{"
        "    Ci = lambert(iColor_0,N) * iColor_0 + orenNayar(iColor_0,iScale_0,N);"
        "}";

    // Create OSL shading system
    auto shadingsys = MakeOSLShadingSys();

    // Register closures
    register_closures( shadingsys.get() );

    constexpr int TN = 8;
    ShaderGroupRef shadergroup[TN];
    for( auto i = 0 ; i < TN ; ++i ){
        const auto group_name = "default_shader_group" + std::to_string( i );
        const auto shader_name = "default_shader" + std::to_string( i );
        const auto shader_layer = "default_layer" + std::to_string( i );
        shadergroup[i] = shadingsys->ShaderGroupBegin("Default_Shader_Group");
        build_shader_test( shadingsys.get() , shader_source , shader_name , shader_layer , group_name );
        shadingsys->ShaderGroupEnd();
    }

    std::unique_ptr<ShadingContextWrapper> sc[TN];
    for( int i = 0 ; i < TN ; ++i )
        sc[i] = std::make_unique<ShadingContextWrapper>();

    ParrallRun<8,16>([&](int tid){
        ShaderGlobals shaderglobals;
        shadingsys->execute (sc[tid]->GetShadingContext(shadingsys.get()), *shadergroup[tid], shaderglobals);

        const auto closure = shaderglobals.Ci;

        EXPECT_NE( closure , nullptr );
        EXPECT_EQ( closure->id , ClosureColor::ADD );

        const auto closureA = closure->as_add()->closureA;
        const auto closureB = closure->as_add()->closureB;

        EXPECT_NE( closureA , nullptr );
        EXPECT_NE( closureB , nullptr );
        EXPECT_EQ( closureA->id , CLOSURE_LAMBERT);
        EXPECT_EQ( closureB->id , CLOSURE_OREN_NAYAR);

        const auto compA = closureA->as_comp();
        const auto cwA = compA->w;

        EXPECT_EQ( cwA.x , 0.125f );
        EXPECT_EQ( cwA.y , 0.5f );
        EXPECT_EQ( cwA.z , 2.5f );

        const auto compB = closureB->as_comp();
        const auto& params = *compB->as<OrenNayar::Params>();
        EXPECT_EQ( params.sigma , 12.0f );
    });
}

// Checking global data
TEST(OSL, CheckingGlobalContext) {
    static const std::string shader_source =  
        "shader TestBasic("
        "   color iColor_0 = color( 0.125 , 0.5 , 2.5 ) ,"
        "   float iScale_0 = 12.0 )"
        "{"
        "    Ci = u * lambert(iColor_0,N) + v * orenNayar(iColor_0,iScale_0,N);"
        "}";

    // Create OSL shading system
    auto shadingsys = MakeOSLShadingSys();

    // Register closures
    register_closures( shadingsys.get() );
    
    const auto group_name = "default_shader_group";
    const auto shader_name = "default_shader";
    const auto shader_layer = "default_layer";
    const auto shadergroup = shadingsys->ShaderGroupBegin ("Closure_Test_ShaderGroup");
    const auto ret = compile_buffer_test( shadingsys.get() , shader_source , shader_name );
    shadingsys->Shader ("surface", shader_name, shader_layer);
    shadingsys->ShaderGroupEnd ();

    ShadingContextWrapper sc;
    auto ctx = sc.GetShadingContext( shadingsys.get() );

    ShaderGlobals shaderglobals;
    shaderglobals.P = Vec3( 0.25f , 1.25f , 4.0 );
    shaderglobals.N = Vec3( 0.0f , 1.0f , 0.0f );
    shaderglobals.I = Vec3( 1.0f , 0.0f , 0.0f );
    shaderglobals.u = 0.5f;
    shaderglobals.v = 0.25f;
    shadingsys->execute (ctx, *shadergroup, shaderglobals);

    const auto closure = shaderglobals.Ci;

    EXPECT_NE( closure , nullptr );
    EXPECT_EQ( closure->id , ClosureColor::ADD );

    const auto closureA = closure->as_add()->closureA;
    const auto closureB = closure->as_add()->closureB;

    EXPECT_NE( closureA , nullptr );
    EXPECT_NE( closureB , nullptr );
    EXPECT_EQ( closureA->id , ClosureColor::MUL );
    EXPECT_EQ( closureB->id , ClosureColor::MUL );

    const auto compA = closureA->as_mul();
    const auto cwA = compA->weight;

    EXPECT_EQ( cwA.x , shaderglobals.u );
    EXPECT_EQ( cwA.y , shaderglobals.u );
    EXPECT_EQ( cwA.z , shaderglobals.u );

    const auto compB = closureB->as_mul();
    const auto cwB = compB->weight;

    EXPECT_EQ( cwB.x , shaderglobals.v );
    EXPECT_EQ( cwB.y , shaderglobals.v );
    EXPECT_EQ( cwB.z , shaderglobals.v );
}