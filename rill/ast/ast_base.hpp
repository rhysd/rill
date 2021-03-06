//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RILL_AST_AST_BASE_HPP
#define RILL_AST_AST_BASE_HPP

#include <cstddef>
#ifdef RILL_DEBUG
# include <iostream>
#endif

#include "detail/tree_visitor_base.hpp"
#include "detail/visitor_delegator.hpp"
#include "detail/ast_def_macro.hpp"


namespace rill
{
    namespace ast
    {
        typedef std::size_t         ast_id_t;

        namespace detail
        {
            // TODO: supoprt for thread safe
            class ast_id_generator
            {
            public:

#ifdef RILL_DEBUG
                ast_id_generator()
                {
                    std::cout << "AST ID GENERATOR" << std::endl;
                }
#endif

            public:
                auto operator()()
                    -> ast_id_t
                {
                    // TODO: THREAD SAFE
                    return ++generated_counter_;
                }

            private:
                ast_id_t generated_counter_ = 0;
            };
        } // namespace detail


        // base class for all AST
        class ast_base
        {
        public:
            ast_base()
                : id_( igen_() )  // generate id
            {
                rill_dout << "NEW AST @ ID: " << id_ << std::endl;
            }

            virtual ~ast_base()
            {
                rill_dout << "DELETED AST @ ID: " << id_ << std::endl;
            }

        public:
            inline auto get_id() const -> ast_id_t
            {
                return id_;
            }

            template<typename Ptr>
            inline auto after_constructing( Ptr const& self )
                -> void
            {}

        public:
            std::size_t line, column;

        private:
            static detail::ast_id_generator igen_;
            ast_id_t id_;
        };

        //
        typedef std::shared_ptr<ast_base> ast_base_ptr;
        typedef std::shared_ptr<ast_base const> const_ast_base_ptr;

    } // namespace ast
} // namespace rill

#endif /*RILL_AST_AST_BASE_HPP*/
