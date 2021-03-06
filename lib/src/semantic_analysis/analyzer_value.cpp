//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <rill/semantic_analysis/semantic_analysis.hpp>
#include <rill/semantic_analysis/message_code.hpp>

#include <rill/environment/environment.hpp>

#include <rill/ast/ast.hpp>


namespace rill
{
    namespace semantic_analysis
    {
        //
        //
        //
        RILL_VISITOR_OP( analyzer, ast::captured_value, v, parent_env )
        {
            rill_dregion {
                std::cout << v->get_id() << std::endl;
                if ( captured_type_details_.find( v->get_id() ) == captured_type_details_.end() ) {
                    assert( false && "id not found" );
                }
            }

            auto ty_d = captured_type_details_.at( v->get_id() );
            assert( ty_d != nullptr );

            rill_dout << "found" << std::endl;
            return ty_d;
        }


        //
        //
        //
        RILL_VISITOR_OP( analyzer, ast::identifier_value, v, parent_env )
        {
            auto&& td = solve_identifier(
                static_cast<ast::const_identifier_value_ptr>( v ),
                parent_env
                );
            if ( td == nullptr ) {
                semantic_error(
                    message_code::e_id_not_found,
                    v,
                    format( "Identifier '%1%' was not declared." ) % v->get_inner_symbol()->to_native_string()
                    );
            }

            return bind_type( v, td );
        }



        //
        //
        //
        RILL_VISITOR_OP( analyzer, ast::template_instance_value, v, parent_env )
        {
            auto&& td = solve_identifier(
                static_cast<ast::const_template_instance_value_ptr>( v ),
                parent_env
                );
            if ( td == nullptr ) {
                semantic_error(
                    message_code::e_id_not_found,
                    v,
                    format( "Identifier '%1%' was not declared." ) % v->get_inner_symbol()->to_native_string()
                    );
            }

            return bind_type( v, td );
        }


        RILL_VISITOR_OP( analyzer, ast::intrinsic::int32_value, v, parent_env )
        {
            auto const& class_env = get_primitive_class_env( v->get_native_typename_string() );
            assert( class_env != nullptr );  // literal type must exist

            //
            return bind_type(
                v,
                type_detail_pool_->construct(
                    g_env_->make_type_id( class_env, attribute::make_default() ),
                    nullptr,    // unused
                    nullptr,    // unused
                    nullptr,    // unused
                    true,       // xvalue
                    type_detail::evaluate_mode::k_runtime
                    )
                );
        }

        RILL_VISITOR_OP( analyzer, ast::intrinsic::float_value, v, parent_env )
        {
            auto const& class_env
                = get_primitive_class_env( v->get_native_typename_string() );
            assert( class_env != nullptr );  // literal type must exist

            return bind_type(
                v,
                type_detail_pool_->construct(
                    g_env_->make_type_id( class_env, attribute::make_default() ),
                    nullptr,    // unused
                    nullptr,    // unused
                    nullptr,    // unused
                    true,       // xvalue
                    type_detail::evaluate_mode::k_runtime
                    )
                );
        }

        RILL_VISITOR_OP( analyzer, ast::intrinsic::boolean_value, v, parent_env )
        {
            auto const& class_env = get_primitive_class_env( v->get_native_typename_string() );
            assert( class_env != nullptr );  // literal type must exist

            //
            return bind_type(
                v,
                type_detail_pool_->construct(
                    g_env_->make_type_id( class_env, attribute::make_default_type_attributes() ),
                    nullptr,    // unused
                    nullptr,    // unused
                    nullptr,    // unused
                    true,       // xvalue
                    type_detail::evaluate_mode::k_runtime
                    )
                );
        }

        RILL_VISITOR_OP( analyzer, ast::intrinsic::string_value, v, parent_env )
        {
            // first, create pointer to static area
            auto const& int8_c_env = get_primitive_class_env( "int8" );
            auto const& ty_id = g_env_->make_type_id(
                    int8_c_env,
                    attribute::make(
                        attribute::holder_kind::k_val,
                        attribute::modifiability_kind::k_immutable,
                        attribute::lifetime_kind::k_static
                        )
                    );
            auto const& ty_d = make_pointer_type( ty_id, v, parent_env );

            // TODO: wrap pointer by string class

            //
            return bind_type( v, ty_d );
        }

        RILL_VISITOR_OP( analyzer, ast::intrinsic::array_value, v, parent_env )
        {
            auto const& ar
                = std::static_pointer_cast<ast::intrinsic::array_value>( v );

            // TODO: check if all values are copyable
            for( auto&& elem : v->elements_list_ ) {
                dispatch( elem, parent_env );
            }

            // abyaaa
            // TODO: support various types
            ast::expression_list args = {
                ast::helper::make_id_expression(
                    ast::make_identifier( "int" )
                    ),
                std::make_shared<ast::term_expression>(
                    std::make_shared<ast::intrinsic::int32_value>(
                        ar->elements_list_.size()
                        )
                    )
            };

            auto const& i
                = ast::helper::make_id_expression(
                    std::make_shared<ast::term_expression>(
                        std::make_shared<ast::template_instance_value>(
                            "array", args/*, true*/
                            )
                        )
                    );

            // solve array type...
            auto const& ty_d
                = resolve_type(
                    i,
                    attribute::holder_kind::k_val,
                    parent_env->root_env(),
                    [&]( type_detail_ptr const& ty_d,
                         type const& ty,
                         class_symbol_environment_ptr const& class_env
                        ) {
                        assert( class_env != nullptr );
                        assert( class_env->is_array() );

                        // connect fron LITARAL VALUE
                        class_env->connect_from_ast( v );
                    } );

            //
            return bind_type( v, ty_d );
        }

    } // namespace semantic_analysis
} // namespace rill
