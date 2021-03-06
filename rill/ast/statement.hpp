//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <vector>
#include <string>

#include <boost/filesystem/path.hpp>

#include "ast_base.hpp"
#include "statement_fwd.hpp"
#include "elements.hpp"

#include "value.hpp"
#include "expression.hpp"


namespace rill
{
    namespace ast
    {
        // ----------------------------------------------------------------------
        // ----------------------------------------------------------------------
        // statements
        // ----------------------------------------------------------------------
        // ----------------------------------------------------------------------
        RILL_AST_GROUP_BEGIN( statement )
        RILL_AST_GROUP_END


        //
        RILL_AST_BEGIN(
            statements, statement,
            (( element::statement_list, statements_ ))
            )
        public:
            statements( element::statement_list const& s )
                : statements_( s )
            {}
        RILL_AST_END


        //
        RILL_AST_BEGIN(
            module, statement,
            (( std::shared_ptr<native_string_t>, name ))
            (( statements_ptr, program ))
            (( boost::filesystem::path, fullpath ))
            )
        public:
            module(
                std::shared_ptr<native_string_t> const& module_name,
                statements_ptr const& p
                )
                : name( module_name )
                , program( p )
            {}

            module( statements_ptr const& p )
                : name( nullptr )
                , program( p )
            {}
        RILL_AST_END


        RILL_AST_BEGIN(
            block_statement, statement,
            (( statements_ptr, statements_ ))
            )
        public:
            block_statement( statements_ptr const& ss )
                : statements_( ss )
            {}

            explicit block_statement( statement_ptr const& s )
                : statements_( std::make_shared<statements>( element::statement_list{ s } ) )
            {}
        RILL_AST_END


        RILL_AST_BEGIN(
            expression_statement, statement,
            (( expression_ptr, expression_ ))
            )
        public:
            expression_statement( expression_ptr const& expr )
                : expression_( expr )
            {}
        RILL_AST_END


        RILL_AST_BEGIN(
            empty_statement, statement
            )
        RILL_AST_END


        RILL_AST_BEGIN(
            can_be_template_statement, statement,
            (( identifier_value_ptr, identifier_ ))
            (( bool, is_template_layout_ ))
            (( bool, is_templated_clone_ ))
            )
        public:
            can_be_template_statement( identifier_value_ptr const& id )
                : identifier_( id )
                , is_template_layout_( false )
                , is_templated_clone_( false )
            {}

        public:
            auto get_identifier() const
                -> identifier_value_ptr
            {
                return identifier_;
            }

            void mark_as_template_layout()
            {
                is_template_layout_ = true;
            }

            void mark_as_nontemplate_layout()
            {
                is_template_layout_ = false;
            }

            auto is_template_layout() const
                -> bool
            {
                return is_template_layout_;
            }

            void mark_as_templated_clone()
            {
                is_templated_clone_ = true;
            }

            void mark_as_nontemplated_clone()
            {
                is_templated_clone_ = false;
            }

            auto is_templated_clone() const
                -> bool
            {
                return is_templated_clone_;
            }
        RILL_AST_END




        // inner statement will be templated
        RILL_AST_BEGIN(
            template_statement, statement,
            (( parameter_list_t, parameter_list_ ))
            (( can_be_template_statement_ptr, inner_ ))
            )
        public:
            template_statement(
                parameter_list_t const& parameter_list,
                can_be_template_statement_ptr const& inner
                )
                : parameter_list_( parameter_list )
                , inner_( inner )
            {}

        public:
            auto get_identifier() const
                -> identifier_value_ptr
            {
                return get_inner_statement()->get_identifier();
            }

            //
            auto get_inner_statement() const
                -> can_be_template_statement_ptr const&
            {
                return inner_;
            }

            auto clone_inner_node()
                -> can_be_template_statement_ptr
            {
                auto const& cloned = clone( inner_ );
                cloned->mark_as_nontemplate_layout();
                cloned->mark_as_templated_clone();

                cloned_inners_.push_back( cloned );

                return cloned;
            }

        private:
            std::vector<can_be_template_statement_ptr> cloned_inners_;
        RILL_AST_END


        RILL_AST_BEGIN(
            function_definition_statement_base, can_be_template_statement,
            (( parameter_list_t, parameter_list ))
            (( attribute::decl::type, decl_attr_ ))
            (( id_expression_ptr, return_type_ ))
            (( statements_ptr, inner_ ))
            )
        public:
            function_definition_statement_base(
                identifier_value_ptr const& id,
                parameter_list_t pl,
                attribute::decl::type const& decl_attr,
                boost::optional<id_expression_ptr> const& return_type,
                statements_ptr const& inner
                )
                : can_be_template_statement( id )
                , parameter_list( std::move( pl ) )
                , decl_attr_( decl_attr )
                , return_type_( return_type != boost::none ? *return_type : nullptr )
                , inner_( inner )
            {}

            function_definition_statement_base(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                boost::optional<id_expression_ptr> const& return_type
                )
                : function_definition_statement_base(
                    id, std::move( parameter_list ), decl_attr, return_type, nullptr
                    )
            {}

        public:
            virtual auto is_class_function() const
                -> bool = 0;
        RILL_AST_END


        RILL_AST_BEGIN(
            function_definition_statement, function_definition_statement_base
            )
        public:
            function_definition_statement(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                boost::optional<id_expression_ptr> const& return_type,
                statements_ptr const& inner
                )
                : function_definition_statement_base(
                    id, std::move( parameter_list ), decl_attr, return_type, inner
                    )
            {}

            auto is_class_function() const
                -> bool override final
            {
                return false;
            }
        RILL_AST_END


        RILL_AST_BEGIN(
            class_function_definition_statement, function_definition_statement_base,
            (( boost::optional<element::class_variable_initializers>, initializers ))
            )
        public:
            class_function_definition_statement(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                boost::optional<element::class_variable_initializers>&& inits,
                boost::optional<id_expression_ptr> const& return_type,
                statements_ptr const& inner
                )
                : function_definition_statement_base(
                    id, std::move( parameter_list ), decl_attr, return_type, inner
                    )
                , initializers( std::move( inits ) )
            {}

            auto is_class_function() const
                -> bool override final
            {
                return true;
            }
        RILL_AST_END


        RILL_AST_BEGIN(
            class_virtual_function_definition_statement, class_function_definition_statement
            )
        public:
            class_virtual_function_definition_statement(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                id_expression_ptr const& return_type,
                statements_ptr const& inner
                )
                : class_function_definition_statement(
                    id,
                    std::move( parameter_list ),
                    decl_attr,
                    boost::none, // initializer
                    return_type,
                    inner
                    )
            {}

            class_virtual_function_definition_statement(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                id_expression_ptr const& return_type
                )
                : class_function_definition_statement(
                    id,
                    std::move( parameter_list ),
                    decl_attr,
                    boost::none, // initializer
                    return_type,
                    nullptr
                    )
            {}

            class_virtual_function_definition_statement(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                statements_ptr const& inner
                )
                : class_function_definition_statement(
                    id,
                    std::move( parameter_list ),
                    decl_attr,
                    boost::none, // initializer
                    boost::none,
                    inner
                    )
            {}
        RILL_AST_END


        RILL_AST_BEGIN(
            extern_function_declaration_statement, function_definition_statement_base,
            (( native_string_t, extern_symbol_name_ ))
            )
        public:
            extern_function_declaration_statement(
                identifier_value_ptr const& id,
                parameter_list_t parameter_list,
                attribute::decl::type const& decl_attr,
                id_expression_ptr const& return_type,
                native_string_t const& extern_symbol_name
                )
                : function_definition_statement_base(
                    id, std::move( parameter_list ), decl_attr, return_type, nullptr
                    )
                , extern_symbol_name_( extern_symbol_name )
            {}

        public:
            auto get_extern_symbol_name() const
                -> native_string_t const&
            {
                return extern_symbol_name_;
            }

            auto is_class_function() const
                -> bool override final
            {
                return false;
            }
        RILL_AST_END



        RILL_AST_BEGIN(
            class_definition_statement, can_be_template_statement,
            (( id_expression_ptr, base_type ))
            (( boost::optional<id_expression_list>, mixin_traits_list ))
            (( attribute::decl::type, decl_attr_ ))
            (( statements_ptr, inner_ ))
            )
        public:
            class_definition_statement(
                identifier_value_ptr const& id,
                boost::optional<id_expression_ptr> const& bt,
                boost::optional<id_expression_list>&& mtl,
                attribute::decl::type const& decl_attr
                )
                : class_definition_statement(
                    id, bt, std::move( mtl ), decl_attr, nullptr
                    )
            {}

            class_definition_statement(
                identifier_value_ptr const& id,
                boost::optional<id_expression_ptr> const& bt,
                boost::optional<id_expression_list>&& mtl,
                attribute::decl::type const& decl_attr,
                statements_ptr const& inner
                )
                : can_be_template_statement( id )
                , base_type( bt ? *bt : nullptr )
                , mixin_traits_list( std::move( mtl ) )
                , decl_attr_( decl_attr )
                , inner_( inner )
            {}
        RILL_AST_END


        RILL_AST_BEGIN(
            extern_class_declaration_statement, class_definition_statement,
            (( native_string_t, extern_symbol_name_ ))
            )
        public:
            extern_class_declaration_statement(
                identifier_value_ptr const& id,
                attribute::decl::type const& decl_attr,
                native_string_t const& extern_symbol_name
                )
                : class_definition_statement( id, boost::none, boost::none, decl_attr )
                , extern_symbol_name_( extern_symbol_name )
            {}

        public:
            auto get_extern_symbol_name() const
                -> native_string_t const&
            {
                return extern_symbol_name_;
            }
        RILL_AST_END


        RILL_AST_BEGIN(
            variable_declaration_statement, statement,
            (( variable_declaration, declaration_ ))
            )
        public:
            variable_declaration_statement( variable_declaration&& decl )
                : declaration_( std::move( decl ) )
            {}

        public:
            auto get_identifier() const
                -> const_identifier_value_base_ptr const&
            {
                return declaration_.decl_unit.name;
            }
        RILL_AST_END



        RILL_AST_BEGIN(
            class_variable_declaration_statement, variable_declaration_statement
            )
        public:
            class_variable_declaration_statement( variable_declaration&& decl )
                : variable_declaration_statement( std::move( decl ) )
            {}
        RILL_AST_END





        RILL_AST_BEGIN(
            return_statement, statement,
            (( expression_ptr, expression_ ))
            )
        public:
            return_statement( expression_ptr const& expr )
                : expression_( expr )
            {}
        RILL_AST_END



        RILL_AST_BEGIN(
            while_statement, statement,
            (( expression_ptr, conditional_ ))
            (( statement_ptr, body_statement_ ))
            )
        public:
            while_statement(
                expression_ptr const& cond,
                statement_ptr const& body_statement
                )
                : conditional_( cond )
                , body_statement_( body_statement )
            {}
        RILL_AST_END



        RILL_AST_BEGIN(
            if_statement, statement,
            (( expression_ptr, conditional_ ))
            (( statement_ptr, then_statement_ ))
            (( statement_ptr, else_statement_ ))
            )
        public:
            if_statement(
                expression_ptr const& cond,
                statement_ptr const& then_statement,
                boost::optional<statement_ptr> const& else_statement
                )
                : conditional_( cond )
                , then_statement_( then_statement )
                , else_statement_( else_statement ? *else_statement : nullptr)
            {}
        RILL_AST_END

        // namespace element {
        struct import_decl_unit
        {
            std::string name;
        };
        using import_decl_unit_list = std::vector<import_decl_unit>;

        // } // namespace element

        RILL_AST_BEGIN(
            import_statement, statement,
            (( import_decl_unit_list, module_decls ))
            )
        public:
            import_statement(
                import_decl_unit_list const& decls
                )
                : module_decls( decls )
            {}
        RILL_AST_END

    } // namespace ast
} // namespace rill
