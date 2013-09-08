//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once


#include <cassert>
#include <memory>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <utility>
#include <boost/range/adaptor/transformed.hpp>

#include <boost/algorithm/string/join.hpp>

//#include <boost/detail/bitmask.hpp>
//#include <boost/optional.hpp>

#include "../config/macros.hpp"

#include "single_identifier_environment_base.hpp"
#include "has_parameter_environment.hpp"


namespace rill
{
    //
    // function
    //
    class function_symbol_environment RILL_CXX11_FINAL
        : public single_identifier_environment_base
    {
    public:
        static kind::type_value const KindValue = kind::type_value::function_e;

    public:
        // pre construct
        function_symbol_environment( environment_id_t const& id, weak_env_pointer const& parent, environment_id_t const& parameter_wrapper_env_id )
            : single_identifier_environment_base( id, parent )
            , parameter_wrapper_env_id_( parameter_wrapper_env_id )
            , return_type_env_id_( environment_id_undefined )
        {}

    public:
        auto get_symbol_kind() const
            -> kind::type_value RILL_CXX11_OVERRIDE
        {
            return KindValue;
        }

        auto is_incomplete() const
            -> bool RILL_CXX11_OVERRIDE
        {
            return return_type_env_id_ == environment_id_undefined;
        }

        auto get_parameter_wrapper_env()
            -> std::shared_ptr<has_parameter_environment<function_symbol_environment>>
        {
            return std::static_pointer_cast<has_parameter_environment<function_symbol_environment>>( get_env_at( parameter_wrapper_env_id_ ).lock() );
        }

        auto get_parameter_decl_ids() const
            -> std::vector<environment_id_t> const&
        {
            return parameter_decl_ids_;
        }

        auto get_parameter_type_ids() const
            -> std::vector<environment_id_t> const&
        {
            return parameter_type_ids_;
        }

        auto complete( const_environment_ptr const& return_type_env )
            -> void
        {
            return_type_env_id_ = return_type_env->get_id();
        }

        auto get_return_type_environment()
            -> class_symbol_environment_ptr
        {
            auto const& p = get_env_at( return_type_env_id_ );

            return std::dynamic_pointer_cast<class_symbol_environment>( p.lock() );
        }

        auto dump( std::ostream& os, std::string const& indent ) const
            -> std::ostream& RILL_CXX11_OVERRIDE
        {
            os  << indent << "function_symbol_environment" << std::endl;
            return dump_include_env( os, indent );
        }

        auto parameter_variable_construct(
            /* ,*/
            intrinsic::single_identifier_value_base_ptr const& name,
            const_class_symbol_environment_ptr const& type_env
            )
            -> variable_symbol_environment_ptr;

    private:
        environment_id_t parameter_wrapper_env_id_;

        // parameter variable environments
        std::vector<environment_id_t> parameter_decl_ids_;

        // types
        std::vector<environment_id_t> parameter_type_ids_;
        environment_id_t return_type_env_id_;
    };

} // namespace rill