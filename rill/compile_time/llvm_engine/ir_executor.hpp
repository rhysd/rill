//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RILL_COMPILE_TIME_LLVM_ENGINE_IR_EXECUTOR_HPP
#define RILL_COMPILE_TIME_LLVM_ENGINE_IR_EXECUTOR_HPP

#include <memory>
#include <unordered_set>
#include <cstdint>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#include "../../ast/visitor.hpp"

#include "../../code_generator/llvm_ir_generator.hpp"
#include "../../code_generator/llvm_engine/support.hpp"
#include "../../type/type_detail_factory.hpp"
#include "../../type/type_detail.hpp"

#include "engine_value_holder.hpp"
#include "value_storage.hpp"


namespace rill
{
    namespace compile_time
    {
        namespace llvm_engine
        {
            // this class holds llvm_ir_generator and llvm_ir_execute_engine
            class ir_executor final
                : public ast::readonly_ast_visitor<ir_executor, void*>
            {
                friend code_generator::llvm_ir_generator;
                using self_type = ir_executor;

            public:
                ir_executor(
                    global_environment_ptr const&,
                    std::shared_ptr<code_generator::llvm_ir_generator> const&,
                    std::shared_ptr<llvm::ExecutionEngine> const&,
                    std::shared_ptr<type_detail_factory> const&
                    );

            public:
                RILL_VISITOR_OP_DEFAULT

                //
                RILL_VISITOR_READONLY_OP_DECL( ast::call_expression );
                RILL_VISITOR_READONLY_OP_DECL( ast::binary_operator_expression );
                RILL_VISITOR_READONLY_OP_DECL( ast::id_expression );
                RILL_VISITOR_READONLY_OP_DECL( ast::term_expression );

                RILL_VISITOR_READONLY_OP_DECL( ast::evaluated_type_expression );

                RILL_VISITOR_READONLY_OP_DECL( ast::identifier_value );
                RILL_VISITOR_READONLY_OP_DECL( ast::template_instance_value );

                RILL_VISITOR_READONLY_OP_DECL( ast::intrinsic::int32_value );
                RILL_VISITOR_READONLY_OP_DECL( ast::intrinsic::boolean_value );
                RILL_VISITOR_READONLY_OP_DECL( ast::intrinsic::string_value );
                RILL_VISITOR_READONLY_OP_DECL( ast::intrinsic::array_value );

            public:
                // TODO: fix this...
                auto value_holder() const
                    -> std::shared_ptr<engine_value_holder>
                {
                    return value_holder_;
                }

            private:
                template<typename T, typename... Args>
                auto make_object( Args&&... args ) const
                    -> raw_value_holder*
                {
                    auto storage = make_dynamic_storage( sizeof( T ), alignof( T ) );
                    new( storage.get() ) T( std::forward<Args>( args )... );

                    return type_detail_factory_->construct_raw_value_holder(
                        storage
                        );
                }

                auto eval_args(
                    ast::expression_list const& arguments,
                    const_environment_base_ptr const& parent_env,
                    llvm::Function const* const target_function
                    ) -> std::vector<llvm::GenericValue>;

                auto normalize_generic_value(
                    llvm::GenericValue const& arguments,
                    type_id_t const& semantic_type_id,
                    llvm::Function const* const target_function
                    ) -> void*;

                auto map_intrinsic_function(
                    llvm::Function const* const target_function
                    ) -> bool;

                auto update_intrinsic_functions()
                    -> void;

            private:
                global_environment_ptr g_env_;
                std::shared_ptr<code_generator::llvm_ir_generator> ir_generator_;
                std::shared_ptr<llvm::ExecutionEngine> execution_engine_;

                std::shared_ptr<engine_value_holder> value_holder_;
                std::shared_ptr<type_detail_factory> type_detail_factory_;

                std::unordered_set<std::string> mapped_intrinsic_function_names_;
                std::size_t mapped_intrinsic_functions_num_ = 0;
            };

        } // namespace llvm_engine
    } // namespace compile_time
} // namespace rill

#endif /*RILL_COMPILE_TIME_LLVM_ENGINE_IR_EXECUTOR_HPP*/
