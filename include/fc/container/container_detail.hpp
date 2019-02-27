#pragma once

#include <fc/variant.hpp>
#include <fc/io/raw_fwd.hpp>

namespace fc {

   namespace detail {

      template<template<typename...> class Set, typename T, typename... U>
      void to_variant_from_set( const Set<T, U...>& s, fc::variant& vo ) {
         FC_ASSERT( s.size() <= MAX_NUM_ARRAY_ELEMENTS );
         variants vars;
         vars.reserve( s.size() );
         for( const auto& item : s ) {
            vars.emplace_back( item );
         }
         vo = std::move( vars );
      }

      template<template<typename...> class Set, typename T, typename... U>
      void from_variant_to_set( const fc::variant& v, Set< T, U... >& s ) {
         const variants& vars = v.get_array();
         FC_ASSERT( vars.size() <= MAX_NUM_ARRAY_ELEMENTS );
         s.clear();
         for( const auto& var : vars ) {
            const auto& item = var.as<T>();
            s.insert( s.end(), item ); // Avoid moving since it needs to use the allocator of s.
         }
      }

      template<template<typename...> class Set, typename T, typename... U>
      void from_variant_to_flat_set( const fc::variant& v, Set< T, U... >& s ) {
         const variants& vars = v.get_array();
         FC_ASSERT( vars.size() <= MAX_NUM_ARRAY_ELEMENTS );
         s.clear();
         s.reserve( vars.size() );
         for( const auto& var : vars ) {
            const auto& item = var.as<T>();
            s.insert( s.end(), item ); // Avoid moving since it needs to use the allocator of s.
         }
      }

      template<template<typename...> class Map, typename K, typename V, typename... U >
      void to_variant_from_map( const Map< K, V, U... >& m, fc::variant& vo ) {
         FC_ASSERT( m.size() <= MAX_NUM_ARRAY_ELEMENTS );
         variants vars;
         vars.reserve( m.size() );
         for( const auto& item : m ) {
            vars.emplace_back( item );
         }
         vo = std::move( vars );
      }

      template<template<typename...> class Map, typename K, typename V, typename... U>
      void from_variant_to_map( const variant& v, Map<K, V, U...>& m ) {
         const variants& vars = v.get_array();
         FC_ASSERT( vars.size() <= MAX_NUM_ARRAY_ELEMENTS );
         m.clear();
         for( const auto& var : vars ) {
          const auto& item = var.as< std::pair<K,V> >();
          m.insert( m.end(), item ); // Avoid moving since it needs to use the allocator of m.
         }
      }

      template<template<typename...> class Map, typename K, typename V, typename... U>
      void from_variant_to_flat_map( const variant& v, Map<K, V, U...>& m ) {
         const variants& vars = v.get_array();
         FC_ASSERT( vars.size() <= MAX_NUM_ARRAY_ELEMENTS );
         m.clear();
         m.reserve( vars.size() );
         for( const auto& var : vars ) {
          const auto& item = var.as< std::pair<K,V> >();
          m.insert( m.end(), item ); // Avoid moving since it needs to use the allocator of m.
         }
      }

   }

   namespace raw {

      namespace detail {

         template<template<typename...> class Set, typename Stream, typename T, typename... U>
         inline void pack_set( Stream& s, const Set<T, U...>& value ) {
            FC_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
            pack( s, unsigned_int((uint32_t)value.size()) );
            for( const auto& item : value ) {
               pack( s, item );
            }
         }

         template<template<typename...> class Set, typename Stream, typename T, typename... U>
         inline void unpack_set( Stream& s, Set<T, U...>& value ) {
            unsigned_int size; unpack( s, size );
            FC_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
            value.clear();
            for( uint32_t i = 0; i < size.value; ++i ) {
               T tmp;
               unpack( s, tmp );
               value.insert( value.end(), tmp ); // Avoid moving since it needs to use the allocator of value.
            }
         }

         template<template<typename...> class Set, typename Stream, typename T, typename... U>
         inline void unpack_flat_set( Stream& s, Set<T, U...>& value ) {
            unsigned_int size; unpack( s, size );
            FC_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
            value.clear();
            value.reserve( size.value );
            for( uint32_t i = 0; i < size.value; ++i ) {
               T tmp;
               unpack( s, tmp );
               value.insert( value.end(), tmp ); // Avoid moving since it needs to use the allocator of value.
            }
         }

         template<template<typename...> class Map, typename Stream, typename K, typename V, typename... U>
         inline void pack_map( Stream& s, const Map<K, V, U...>& value ) {
            FC_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
            pack( s, unsigned_int((uint32_t)value.size()) );
            for( const auto& item : value ) {
               pack( s, item );
            }
         }

         template<template<typename...> class Map, typename Stream, typename K, typename V, typename... U>
         inline void unpack_map( Stream& s, Map<K, V, U...>& value ) {
            unsigned_int size; unpack( s, size );
            FC_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
            value.clear();
            for( uint32_t i = 0; i < size.value; ++i ) {
               std::pair<K,V> tmp;
               unpack( s, tmp );
               value.insert( value.end(), tmp ); // Avoid moving since it needs to use the allocator of value.
            }
         }

         template<template<typename...> class Map, typename Stream, typename K, typename V, typename... U>
         inline void unpack_flat_map( Stream& s, Map<K, V, U...>& value ) {
            unsigned_int size; unpack( s, size );
            FC_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
            value.clear();
            value.reserve( size.value );
            for( uint32_t i = 0; i < size.value; ++i ) {
               std::pair<K,V> tmp;
               unpack( s, tmp );
               value.insert( value.end(), tmp ); // Avoid moving since it needs to use the allocator of value.
            }
         }

      }

   }

}
