//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RILL_AST_DETAIL_TREE_VISITOR_BASE_HPP
#define RILL_AST_DETAIL_TREE_VISITOR_BASE_HPP

#include <memory>
#include <iostream>
#include <cassert>  // for assert
#include <typeinfo>

#include "../../config/macros.hpp"
#include "../../message/message.hpp"

#include "../../environment/environment_fwd.hpp"

#include "../value_fwd.hpp"
#include "../expression_fwd.hpp"
#include "../statement_fwd.hpp"

#include "dispatch_functions.hpp"
#include "ast_node_type_seq.hpp"


//
// For visitor
//

#define RILL_VISITOR_OP_INDIRECT( node_type, node_name, env_name, result_scope, prefix ) \
    typename result_scope result<node_type>::type \
    prefix operator()( \
        std::shared_ptr<node_type> const& node_name, \
        environment_base_ptr const& env_name \
        )

#define RILL_VISITOR_READONLY_OP_INDIRECT( node_type, node_name, env_name, result_scope, prefix ) \
    typename result_scope result<node_type>::type \
    prefix operator()( \
        std::shared_ptr<node_type const> const& node_name, \
        const_environment_base_ptr const& env_name \
        )


#define RILL_VISITOR_OP( class_name, node_type, node_name, env_name ) \
    RILL_VISITOR_OP_INDIRECT( node_type, node_name, env_name, class_name::, class_name:: )

#define RILL_VISITOR_READONLY_OP( class_name, node_type, node_name, env_name ) \
    RILL_VISITOR_READONLY_OP_INDIRECT( node_type, node_name, env_name, class_name::, class_name:: )


#define RILL_VISITOR_OP_DECL( node_type ) RILL_VISITOR_OP_INDIRECT( node_type, , , , )

#define RILL_VISITOR_READONLY_OP_DECL( node_type ) RILL_VISITOR_READONLY_OP_INDIRECT( node_type, , , , )


#define RILL_VISITOR_OP_DECL_INNER( node_type, node_name, env_name ) \
    RILL_VISITOR_OP_INDIRECT( node_type, node_name, env_name, self_type::template, )

#define RILL_VISITOR_READONLY_OP_DECL_INNER( node_type, node_name, env_name ) \
    RILL_VISITOR_READONLY_OP_INDIRECT( node_type, node_name, env_name, self_type::template, )


// filter outbound object
#define RILL_VISITOR_OP_FAIL                                        \
    template<typename NodeT>                                        \
    RILL_VISITOR_OP_DECL_INNER( NodeT, , )                          \
    {                                                               \
        this->template failed_to_dispatch<NodeT>();                 \
        using R = typename self_type::template result<NodeT>::type; \
        return R();                                                 \
    }                                                               \
    template<typename NodeT>                                        \
    RILL_VISITOR_OP_DECL_INNER( NodeT, , ) const                    \
    {                                                               \
        this->template failed_to_dispatch<NodeT>();                 \
        using R = typename self_type::template result<NodeT>::type; \
        return R();                                                 \
    }                                                               \
    template<typename NodeT>                                        \
    RILL_VISITOR_READONLY_OP_DECL_INNER( NodeT, , )                 \
    {                                                               \
        this->template failed_to_dispatch<NodeT const>();           \
        using R = typename self_type::template result<NodeT>::type; \
        return R();                                                 \
    }                                                               \
    template<typename NodeT>                                        \
    RILL_VISITOR_READONLY_OP_DECL_INNER( NodeT, , ) const           \
    {                                                               \
        this->template failed_to_dispatch<NodeT const>();           \
        using R = typename self_type::template result<NodeT>::type; \
        return R();                                                 \
    }



//
// For invoker
//

#define RILL_VISITOR_DELEGATOR_SIGNATURE( node_type, qual ) \
    auto callback_from_node(                              \
        std::shared_ptr<node_type> const& node,           \
        environment_base_ptr const& env,                  \
        tree_visitor_base<ReturnT> qual* const v          \
        void* storage                                     \
        ) qual                                            \
        -> void

#define RILL_VISITOR_DELEGATOR_READONLY_SIGNATURE( node_type, qual )    \
    auto callback_from_node(                              \
        std::shared_ptr<node_type const> const& node,     \
        const_environment_base_ptr const& env,            \
        tree_visitor_base<ReturnT> qual* const v          \
        void* storage                                     \
        ) qual                                            \
        -> void


#define RILL_VISITOR_DELEGATOR_INTERFACE( node_type ) \
    virtual RILL_VISITOR_INVOKER_OP_HEADER( node_type, ) =0; \
    virtual RILL_VISITOR_INVOKER_OP_HEADER( node_type, const ) =0; \
    virtual RILL_VISITOR_INVOKER_READONLY_OP_HEADER( node_type, ) =0; \
    virtual RILL_VISITOR_INVOKER_READONLY_OP_HEADER( node_type, const ) =0;


#define RILL_VISITOR_DELEGATOR_DEFINITION( node_type )                  \
    RILL_VISITOR_DELEGATOR_SIGNATURE( node_type, ) override             \
    {                                                                   \
        return static_cast<Visitor&>( *v ).operator()( node, env );     \
    }                                                                   \
    RILL_VISITOR_DELEGATOR_SIGNATURE( node_type, const ) override       \
    {                                                                   \
        return static_cast<Visitor const&>( *v ).operator()( node, env ); \
    }                                                                   \
    RILL_VISITOR_DELEGATOR_READONLY_SIGNATURE( node_type, ) override    \
    {                                                                   \
        return static_cast<Visitor&>( *v ).operator()( node, env );     \
    }                                                                   \
    RILL_VISITOR_DELEGATOR_READONLY_SIGNATURE( node_type, const ) override \
    {                                                                   \
        return static_cast<Visitor const&>( *v ).operator()( node, env ); \
    }


namespace rill
{
    namespace ast
    {
        namespace detail
        {
            class visitor_delegator_base
            {
            public:
                BOOST_PP_SEQ_FOR_EACH( RILL_VISITOR_INVOKER_OP_PURE_VIRTUAL_GEN, _, RILL_AST_NODE_TYPE_SEQ )
            };

            //
            template<typename Visitor, typename ReturnT>
            class visitor_delegator
                : public visitor_invoker_base
            {
            public:
                template<typename NodeT>
                using result = visitor_result_traits<ReturnT, NodeT>;

            public:
                BOOST_PP_SEQ_FOR_EACH( RILL_VISITOR_INVOKER_OP_GEN, _, RILL_AST_NODE_TYPE_SEQ )
            };
        } // namespace detail
    } // namespace ast
} // namespace rill


#define RILL_VISITOR_INVOKER_OP_GEN( r, _, node_class )    \
    RILL_VISITOR_INVOKER_OP( ast :: node_class )

#define RILL_VISITOR_INVOKER_OP_PURE_VIRTUAL_GEN( r, _, node_class ) \
    RILL_VISITOR_INVOKER_OP_PURE_VIRTUAL( ast :: node_class )


namespace rill
{
    namespace ast
    {
        namespace detail
        {
            // forward declaration
            template<typename ReturnT>
            struct tree_visitor_base;

            // --
            template<typename R, typename BaseNode>
            struct tree_visitor_result;
            template<typename R> struct tree_visitor_result<R, ast::statement>
            { typedef void type; };
            template<typename R> struct tree_visitor_result<R, ast::expression>
            { typedef R type; };
            template<typename R> struct tree_visitor_result<R, ast::value>
            { typedef R type; };

            //
            template<typename ReturnT, typename NodeT>
            struct visitor_result_traits
            {
                typedef typename tree_visitor_result<
                    ReturnT,
                    typename base_type_specifier<typename std::decay<NodeT>::type>::type
                >::type type;
            };


            //
            template<typename ReturnT>
            struct visitor_result_default_value_functor
            {
                auto operator()() const
                    -> ReturnT
                {
                    // TODO: fix
                    return nullptr;
                }
            };
            template<>
            struct visitor_result_default_value_functor<void>
            {
                auto operator()() const
                    -> void
                {}
            };




            //
            // base class of ast visitors
            template<typename ReturnT>
            struct tree_visitor_base
            {
            public:
                typedef tree_visitor_base           self_type;
                typedef self_type const             const_self_type;

                template<typename NodeT>
                using result = visitor_result_traits<ReturnT, NodeT>;

                template<typename NodeT>
                using result_value_functor = visitor_result_default_value_functor<typename result<NodeT>::type>;

            public:
                tree_visitor_base( visitor_invoker_base<ReturnT>& invoker )
                    : invoker_( std::ref( invoker ) )
                {}

                virtual ~tree_visitor_base() {};

            public:
                // called from dispacher of AST
                template<typename Node, typename Env>
                auto invoke( std::shared_ptr<Node> const& node, std::shared_ptr<Env> const& env )
                    -> typename result<Node>::type
                try {
                    return invoker_( node, env, this );

                } catch( message::message_object const& e ) {
                    save_message( e );
                    return result_value_functor<Node>()();      // implies failed...
                }

                template<typename Node, typename Env>
                auto invoke( std::shared_ptr<Node> const& node, std::shared_ptr<Env> const& env ) const
                    -> typename result<Node>::type
                try {
                    return invoker_( node, env, this );

                } catch( message::message_object const& e ) {
                    save_message( e );
                    return result_value_functor<Node>()();      // implies failed...
                }

            public:
                template<typename NodeT>
                auto failed_to_dispatch() const
                    -> void
                {
                    std::cerr
                        << "!!! DEBUG: this AST node was not implemented" << std::endl
                        << "VISITOR -> " << typeid( *this ).name() << " / is_const: " << std::is_const<decltype( *this )>::value << std::endl
                        << "AST     -> " << typeid( NodeT ).name() << " / is_const: " << std::is_const<NodeT>::value << std::endl;
                }

            public:
                template<typename... Args>
                auto send_error( Args&&... args ) const
                    -> void
                {
                    assert( false );

                    throw message::message_object(
                        message::message_level::e_error,
                        std::forward<Args>( args )...
                        );
                }

                template<typename... Args>
                auto send_warning( Args&&... args ) const
                    -> void
                {
                    assert( false );

                    // TODO: see compile switch
                    save_message(
                        message::message_object(
                            message::message_level::e_warning,
                            std::forward<Args>( args )...
                            )
                        );
                }

                template<typename... Args>
                auto send_message( Args&&... args ) const
                    -> void
                {
                    assert( false );

                    save_message(
                        message::message_object(
                            message::message_level::e_normal,
                            std::forward<Args>( args )...
                            )
                        );
                }

                auto has_message() const
                    -> bool
                {
                    return !messages_.empty();
                }

                auto has_error() const
                    -> bool
                {
                    return false;       // TODO: implement
                }

            private:
                template<typename T>
                auto save_message( T&& m ) const
                    -> void
                {
                    messages_.push_back( std::forward<T>( m ) );
                }

            private:
                std::reference_wrapper<visitor_invoker_base<ReturnT>> invoker_;

            private:
                mutable std::vector<message::message_object> messages_;
            };

        } // namespace detail
    } // namespace ast
} // namespace rill


#undef RILL_VISITOR_INVOKER_OP_HEADER
#undef RILL_VISITOR_INVOKER_READONLY_OP_HEADER
#undef RILL_VISITOR_INVOKER_OP_PURE_VIRTUAL
#undef RILL_VISITOR_INVOKER_OP

#undef RILL_VISITOR_INVOKER_OP_GEN
#undef RILL_VISITOR_INVOKER_OP_PURE_VIRTUAL_GEN

#endif /*RILL_AST_DETAIL_TREE_VISITOR_BASE_HPP*/
