/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include <tsl_system.h>
#include "thirdparty/gtest/gtest.h"

using namespace Tsl_Namespace;

TEST(ShaderGroup, ShaderGroupWithoutClosure) {
    // global tsl shading system
    auto& shading_system = ShadingSystem::get_instance();

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = shading_system.make_shading_context();

    // the root shader node, this usually matches to the output node in material system
    auto root_shader_template = shading_context->begin_shader_unit_template("root_shader");
    const auto root_shader_compiling = root_shader_template->compile_shader_source(R"(
        shader output_node( float in_bxdf , out float out_bxdf ){
            out_bxdf = in_bxdf * 1231.0f;
        }
    )");
    shading_context->end_shader_unit_template(root_shader_template.get());
    EXPECT_NE((void*)nullptr, (void*)root_shader_template.get());
    EXPECT_EQ(true, root_shader_compiling);

    // a bxdf node
    auto bxdf_shader_template = shading_context->begin_shader_unit_template("bxdf_shader");
    const auto bxdf_shader_compiling = bxdf_shader_template->compile_shader_source(R"(
        shader lambert_node( float in_bxdf , out float out_bxdf , out float dummy ){
            out_bxdf = in_bxdf;
            // dummy = 1.0f;
        }
    )");
    shading_context->end_shader_unit_template(bxdf_shader_template.get());
    EXPECT_NE((void*)nullptr, (void*)bxdf_shader_template.get());
    EXPECT_EQ(true, bxdf_shader_compiling);

    // make a shader group
    auto shader_group = shading_context->begin_shader_group_template("first shader");
    EXPECT_NE((void*)nullptr, (void*)shader_group.get());

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit("root_shader", root_shader_template, true);
    EXPECT_EQ(true, ret);
    ret = shader_group->add_shader_unit("bxdf_shader", bxdf_shader_template);
    EXPECT_EQ(true, ret);

    // setup connections between shader units
    shader_group->connect_shader_units("bxdf_shader", "out_bxdf", "root_shader", "in_bxdf");

    // expose the shader interface
    shader_group->expose_shader_argument("root_shader", "out_bxdf");
    shader_group->expose_shader_argument("bxdf_shader", "in_bxdf", false);

    // resolve the shader group
    auto status = shading_context->end_shader_group_template(shader_group.get());
    EXPECT_EQ(TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    auto shader_instance = shader_group->make_shader_instance();
    status = shader_instance->resolve_shader_instance();
    EXPECT_EQ(TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    // get the function pointer
    auto raw_function = (void(*)(float*, float))shader_instance->get_function();
    EXPECT_NE((void*)nullptr, (void*)raw_function);

    // execute the shader
    float closure, in_bxdf = 0.5f;
    raw_function(&closure, in_bxdf);
    EXPECT_EQ(1231.0f * 0.5f, closure);
}