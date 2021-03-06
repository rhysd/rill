//
// Copyright yutopp 2013 - .
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RILL_TYPE_ATTRIBUTE_HPP
#define RILL_TYPE_ATTRIBUTE_HPP

#include <bitset>
#include <iostream>


namespace rill
{
    namespace attribute
    {
        enum class holder_kind
        {
            k_suggest,
            k_val,
            k_ref,
            last
        };

        enum class modifiability_kind
        {
            k_mutable,
            k_const,
            k_immutable,
            k_none,
            last
        };

        // ---------------------------------------|
        // | k_auto                               |
        // ---------------------------------------|
        // | k_managed   | k_full_managed   |  |  |
        // | k_static    |                  |  |  |
        // ---------------------------------|  |  |
        // | k_scoped    | k_partial_managed   |  | <- default
        // ------------------------------------|  |
        // | k_unmanaged | k_unmanaged            |
        // ---------------------------------------|
        enum class lifetime_kind
        {
            k_managed,
            k_static,
            k_unmanaged,
            k_scoped,
            k_full_managed,
            k_partial_managed,
            k_none,
            last
        };

        struct type_attributes
        {
            attribute::holder_kind quality; // TODO: rename
            attribute::modifiability_kind modifiability;
            attribute::lifetime_kind lifetime;
        };

        inline auto operator==( type_attributes const& lhs, type_attributes const& rhs )
            -> bool
        {
            return lhs.quality == rhs.quality
                && lhs.modifiability == rhs.modifiability
                && lhs.lifetime == rhs.lifetime;
        }

        inline auto operator<<( std::ostream& os, holder_kind const& k )
            -> std::ostream&
        {
            switch( k ) {
            case holder_kind::k_suggest:
                os << "suggest";
                break;
            case holder_kind::k_val:
                os << "val";
                break;
            case holder_kind::k_ref:
                os << "ref";
                break;
            default:
                assert( false );
            }

            return os;
        }

        inline auto operator<<( std::ostream& os, modifiability_kind const& k )
            -> std::ostream&
        {
            switch( k ) {
            case modifiability_kind::k_none:
                os << "none";
                break;
            case modifiability_kind::k_mutable:
                os << "mutable";
                break;
            case modifiability_kind::k_const:
                os << "const";
                break;
            case modifiability_kind::k_immutable:
                os << "immutable";
                break;
            default:
                assert( false );
            }

            return os;
        }

        inline auto operator<<( std::ostream& os, lifetime_kind const& k )
            -> std::ostream&
        {
            switch( k ) {
            case lifetime_kind::k_scoped:
                os << "scoped";
                break;
            case lifetime_kind::k_managed:
                os << "managed";
                break;
            case lifetime_kind::k_unmanaged:
                os << "unmagaged";
                break;
            case lifetime_kind::k_static:
                os << "static";
                break;
            default:
                assert( false );
            }

            return os;
        }

        inline auto operator<<( std::ostream& os, type_attributes const& attr )
            -> std::ostream&
        {
            os << "type_attributes_bit" << std::endl
               << " q: " << attr.quality << std::endl
               << " m: " << attr.modifiability << std::endl
               << " l: " << attr.lifetime << std::endl;

            return os;
        }


        namespace detail
        {
            template<typename E>
            constexpr std::size_t min_bitwidth( E const& e )
            {
                std::size_t i = 1, t = 1;
                std::size_t n = static_cast<std::size_t>( e );

                while( n > i ) {
                    i *= 2;
                    ++t;
                }

                return t;
            }
        }

        constexpr auto attributes_width_max
            = detail::min_bitwidth( holder_kind::last )
            + detail::min_bitwidth( modifiability_kind::last )
            + detail::min_bitwidth( lifetime_kind::last );
        typedef std::bitset<attributes_width_max> attributes_bit_t;

        namespace detail
        {
            auto inline make_type_attributes_bit( type_attributes const& attr )
                -> attributes_bit_t
            {
                using ull = unsigned long long;

                ull bits = 0;
                std::size_t offset = 0;

                bits |= static_cast<ull>( attr.quality ) << offset;
                offset += detail::min_bitwidth( holder_kind::last );

                bits |= static_cast<ull>( attr.modifiability ) << offset;
                offset += detail::min_bitwidth( modifiability_kind::last );

                bits |= static_cast<ull>( attr.lifetime ) << offset;
                offset += detail::min_bitwidth( lifetime_kind::last );

                rill_dout << attr
                          << " bits: " << attributes_bit_t( bits ).to_string() << std::endl;;

                return attributes_bit_t( bits );
            }
        } // namespace detail


        namespace detail
        {
            inline void set_type_attribute(
                type_attributes& attr,
                holder_kind const& k
                )
            {
                attr.quality = k;
            }

            inline void set_type_attribute(
                type_attributes& attr,
                modifiability_kind const& k
                )
            {
                attr.modifiability = k;
            }

            inline void set_type_attribute(
                type_attributes& attr,
                lifetime_kind const& k
                )
            {
                attr.lifetime = k;
            }

            template<typename T>
            void set(
                type_attributes& attr,
                T const& arg
                )
            {
                set_type_attribute( attr, arg );
            }

            template<typename T, typename... Args>
            void set(
                type_attributes& attr,
                T const& arg,
                Args const&... args
                )
            {
                set_type_attribute( attr, arg );
                set( attr, args... );
            }
        } // namespace detail

        inline auto operator+=(
            type_attributes& a,
            type_attributes const& b
            )
            -> type_attributes&
        {
            detail::set(
                a,
                b.quality,
                b.modifiability
                );

            return a;
        }

        inline auto operator+(
            type_attributes a,  // copy
            type_attributes const& b
            )
            -> type_attributes
        {
            a += b;

            return a;
        }


        namespace detail
        {
            inline void unset_type_attribute(
                type_attributes& attr,
                holder_kind const& k
                )
            {
                if ( k != holder_kind::k_suggest ) {
                    attr.quality = holder_kind::k_suggest;
                }
            }

            inline void unset_type_attribute(
                type_attributes& attr,
                modifiability_kind const& k
                )
            {
                // modifiability can NOT be none
                if ( k != modifiability_kind::k_none ) {
                    attr.modifiability = k;
                }
            }

            template<typename T>
            void unset(
                type_attributes& attr,
                T const& arg
                )
            {
                unset_type_attribute( attr, arg );
            }

            template<typename T, typename... Args>
            void unset(
                type_attributes& attr,
                T const& arg,
                Args const&... args
                )
            {
                unset_type_attribute( attr, arg );
                unset( attr, args... );
            }
        } // namespace detail

        inline auto operator-=(
            type_attributes& a,
            type_attributes const& b
            )
            -> type_attributes&
        {
            detail::unset(
                a,
                b.quality,
                b.modifiability
                );

            return a;
        }

        inline auto operator-(
            type_attributes a,  // copy
            type_attributes const& b
            )
            -> type_attributes
        {
            a -= b;

            return a;
        }


        namespace detail
        {
            inline void overlap_empty(
                attribute::type_attributes& attr,
                attribute::holder_kind const& k
                )
            {
                if ( attr.quality == holder_kind::k_suggest ) {
                    attr.quality = k;
                }
            }

            inline void overlap_empty(
                attribute::type_attributes& attr,
                attribute::modifiability_kind const& k
                )
            {
                if ( attr.modifiability == attribute::modifiability_kind::k_none ) {
                    attr.modifiability = k;
                }
            }

            inline void overlap_empty(
                attribute::type_attributes& attr,
                attribute::lifetime_kind const& k
                )
            {
                if ( attr.lifetime == attribute::lifetime_kind::k_none ) {
                    attr.lifetime = k;
                }
            }

            template<typename T>
            void set_overlap_empty_by(
                attribute::type_attributes& attr,
                T const& arg
                )
            {
                overlap_empty( attr, arg );
            }

            template<typename T, typename... Args>
            void set_overlap_empty_by(
                attribute::type_attributes& attr,
                T const& arg,
                Args const&... args
                )
            {
                overlap_empty( attr, arg );
                set_overlap_empty_by( attr, args... );
            }
        } // namespace detail

        inline auto overlap_empty_attr(
            attribute::type_attributes a,  // copy
            attribute::type_attributes const& b
            )
            -> attribute::type_attributes
        {
            detail::set_overlap_empty_by(
                a,
                b.quality,
                b.modifiability
                );

            return a;
        }


        namespace detail
        {
            inline auto overwrite_by_nonempty(
                type_attributes& attr,
                holder_kind const& k
                ) -> type_attributes&
            {
                if ( k != holder_kind::k_suggest ) {
                    attr.quality = k;
                }
                return attr;
            }

            inline auto overwrite_by_nonempty(
                type_attributes& attr,
                modifiability_kind const& k
                ) -> type_attributes&
            {
                if ( k != modifiability_kind::k_none ) {
                    attr.modifiability = k;
                }
                return attr;
            }

            inline auto overwrite_by_nonempty(
                type_attributes& attr,
                lifetime_kind const& k
                ) -> type_attributes&
            {
                if ( k != lifetime_kind::k_none ) {
                    attr.lifetime = k;
                }
                return attr;
            }

            template<typename T>
            void set_overwrite_by_nonempty(
                attribute::type_attributes& attr,
                T const& arg
                )
            {
                overwrite_by_nonempty( attr, arg );
            }

            template<typename T, typename... Args>
            void set_overwrite_by_nonempty(
                attribute::type_attributes& attr,
                T const& arg,
                Args const&... args
                )
            {
                overwrite_by_nonempty( attr, arg );
                set_overwrite_by_nonempty( attr, args... );
            }
        } // namespace detail

        inline auto overwrite_by_nonempty_attr(
            attribute::type_attributes a,  // copy
            attribute::type_attributes const& b
            )
            -> attribute::type_attributes
        {
            detail::set_overwrite_by_nonempty(
                a,
                b.quality,
                b.modifiability
                );

            return a;
        }





        template<typename T>
        auto operator<<=( type_attributes& attr, T const& t )
            -> type_attributes&
        {
            detail::set( attr, t );
            return attr;
        }

        auto inline make_default_type_attributes()
            -> type_attributes
        {
            return { holder_kind::k_val, modifiability_kind::k_immutable };
        }

        auto inline make_empty_type_attributes()
            -> type_attributes
        {
            return { holder_kind::k_suggest, modifiability_kind::k_none };
        }

        template<typename... Args>
        auto inline make_type_attributes( Args const&... args )
            -> type_attributes
        {
            type_attributes attr = make_default_type_attributes();
            detail::set( attr, args... );
            return attr;
        }

        auto inline make_default()
            -> type_attributes
        {
            return { holder_kind::k_val, modifiability_kind::k_immutable };
        }

        auto inline make_value_default()
            -> type_attributes
        {
            return {
                holder_kind::k_val,
                modifiability_kind::k_none
            };
        }

        template<typename... Args>
        auto inline make( Args const&... args )
            -> type_attributes
        {
            type_attributes attr = make_empty_type_attributes();
            detail::set( attr, args... );
            return attr;
        }

    } // namespace attribute
} // namespace rill

#endif /*RILL_TYPE_ATTRIBUTE_HPP*/
