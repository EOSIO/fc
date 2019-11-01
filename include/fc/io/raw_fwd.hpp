#pragma once
#include <fc/container/flat_fwd.hpp>
#include <fc/container/deque_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/array.hpp>
#include <fc/safe.hpp>
#include <deque>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <set>

namespace fc {
   class time_point;
   class time_point_sec;
   class variant;
   class variant_object;
   class path;
   template<typename... Types> class static_variant;

   template<typename IntType, typename EnumType> class enum_type;
   namespace ip { class endpoint; }

   namespace ecc { class public_key; class private_key; }
   template<typename Storage> class fixed_string;

using block_1024_option_t = boost::container::deque_options< boost::container::block_size<1024u> >::type;
template<typename T>
using deque_1024 = boost::container::deque<T, void, block_1024_option_t >;

template<typename T, typename... U>
void to_variant( const boost::container::deque<T, U...>& vec, fc::variant& vo ) {
   FC_ASSERT( vec.size() <= MAX_NUM_ARRAY_ELEMENTS );
   variants vars;
   for( const auto& item : vec ) {
      vars.emplace_back( item );
   }
   vo = std::move(vars);
}

template<typename T, typename... U>
void from_variant( const fc::variant& v, boost::container::deque<T, U...>& vec ) {
   const variants& vars = v.get_array();
   FC_ASSERT( vars.size() <= MAX_NUM_ARRAY_ELEMENTS );
   vec.clear();
   vec.resize( vars.size() );
   for( uint32_t i = 0; i < vars.size(); ++i ) {
      from_variant( vars[i], vec[i] );
   }
}

namespace raw {
    template<typename T>
    constexpr bool is_trivial_array = std::is_scalar<T>::value == true && std::is_pointer<T>::value == false;

    template<typename T>
    inline size_t pack_size(  const T& v );

    template<typename Stream, typename Storage> inline void pack( Stream& s, const fc::fixed_string<Storage>& u );
    template<typename Stream, typename Storage> inline void unpack( Stream& s, fc::fixed_string<Storage>& u );

    template<typename Stream, typename IntType, typename EnumType>
    inline void pack( Stream& s, const fc::enum_type<IntType,EnumType>& tp );
    template<typename Stream, typename IntType, typename EnumType>
    inline void unpack( Stream& s, fc::enum_type<IntType,EnumType>& tp );



    template<typename Stream, typename T> inline void pack( Stream& s, const std::set<T>& value );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::set<T>& value );
    template<typename Stream, typename T> inline void pack( Stream& s, const std::unordered_set<T>& value );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::unordered_set<T>& value );

    template<typename Stream, typename... T> void pack( Stream& s, const static_variant<T...>& sv );
    template<typename Stream, typename... T> void unpack( Stream& s, static_variant<T...>& sv );

    template<typename Stream, typename T> inline void pack( Stream& s, const std::deque<T>& value );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::deque<T>& value );

    template<typename Stream, typename K, typename V> inline void pack( Stream& s, const std::unordered_map<K,V>& value );
    template<typename Stream, typename K, typename V> inline void unpack( Stream& s, std::unordered_map<K,V>& value );

    template<typename Stream, typename K, typename V> inline void pack( Stream& s, const std::map<K,V>& value );
    template<typename Stream, typename K, typename V> inline void unpack( Stream& s, std::map<K,V>& value );

    template<typename Stream, typename K, typename V> inline void pack( Stream& s, const std::pair<K,V>& value );
    template<typename Stream, typename K, typename V> inline void unpack( Stream& s, std::pair<K,V>& value );

    template<typename Stream, typename T, std::size_t S> inline auto pack( Stream& s, const std::array<T,S>& value ) -> std::enable_if_t<is_trivial_array<T>>;
    template<typename Stream, typename T, std::size_t S> inline auto pack( Stream& s, const std::array<T,S>& value ) -> std::enable_if_t<!is_trivial_array<T>>;
    template<typename Stream, typename T, std::size_t S> inline auto unpack( Stream& s, std::array<T,S>& value ) -> std::enable_if_t<is_trivial_array<T>>;
    template<typename Stream, typename T, std::size_t S> inline auto unpack( Stream& s, std::array<T,S>& value ) -> std::enable_if_t<!is_trivial_array<T>>;

    template<typename Stream> inline void pack( Stream& s, const variant_object& v );
    template<typename Stream> inline void unpack( Stream& s, variant_object& v );
    template<typename Stream> inline void pack( Stream& s, const variant& v );
    template<typename Stream> inline void unpack( Stream& s, variant& v );

    template<typename Stream> inline void pack( Stream& s, const path& v );
    template<typename Stream> inline void unpack( Stream& s, path& v );
    template<typename Stream> inline void pack( Stream& s, const ip::endpoint& v );
    template<typename Stream> inline void unpack( Stream& s, ip::endpoint& v );


    template<typename Stream, typename T> void unpack( Stream& s, fc::optional<T>& v );
    template<typename Stream, typename T> void unpack( Stream& s, const T& v );
    template<typename Stream, typename T> void pack( Stream& s, const fc::optional<T>& v );
    template<typename Stream, typename T> void pack( Stream& s, const safe<T>& v );
    template<typename Stream, typename T> void unpack( Stream& s, fc::safe<T>& v );

    template<typename Stream> void unpack( Stream& s, time_point& );
    template<typename Stream> void pack( Stream& s, const time_point& );
    template<typename Stream> void unpack( Stream& s, time_point_sec& );
    template<typename Stream> void pack( Stream& s, const time_point_sec& );
    template<typename Stream> void unpack( Stream& s, std::string& );
    template<typename Stream> void pack( Stream& s, const std::string& );
    template<typename Stream> void unpack( Stream& s, fc::ecc::public_key& );
    template<typename Stream> void pack( Stream& s, const fc::ecc::public_key& );
    template<typename Stream> void unpack( Stream& s, fc::ecc::private_key& );
    template<typename Stream> void pack( Stream& s, const fc::ecc::private_key& );

    template<typename Stream, typename T> inline void pack( Stream& s, const T& v );
    template<typename Stream, typename T> inline void unpack( Stream& s, T& v );

    template<typename Stream, typename T> inline void pack( Stream& s, const std::vector<T>& v );
    template<typename Stream, typename T> inline void unpack( Stream& s, std::vector<T>& v );

    template<typename Stream> inline void pack( Stream& s, const signed_int& v );
    template<typename Stream> inline void unpack( Stream& s, signed_int& vi );

    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v );
    template<typename Stream> inline void unpack( Stream& s, unsigned_int& vi );

    template<typename Stream> inline void pack( Stream& s, const char* v );
    template<typename Stream> inline void pack( Stream& s, const std::vector<char>& value );
    template<typename Stream> inline void unpack( Stream& s, std::vector<char>& value );

    template<typename Stream, typename T, std::size_t N> inline auto pack( Stream& s, const fc::array<T,N>& v) -> std::enable_if_t<is_trivial_array<T>>;
    template<typename Stream, typename T, std::size_t N> inline auto pack( Stream& s, const fc::array<T,N>& v) -> std::enable_if_t<!is_trivial_array<T>>;
    template<typename Stream, typename T, std::size_t N> inline auto unpack( Stream& s, fc::array<T,N>& v) -> std::enable_if_t<is_trivial_array<T>>;
    template<typename Stream, typename T, std::size_t N> inline auto unpack( Stream& s, fc::array<T,N>& v) -> std::enable_if_t<!is_trivial_array<T>>;

    template<typename Stream> inline void pack( Stream& s, const bool& v );
    template<typename Stream> inline void unpack( Stream& s, bool& v );

    template<typename T> inline std::vector<char> pack( const T& v );
    template<typename T> inline T unpack( const std::vector<char>& s );
    template<typename T> inline T unpack( const char* d, uint32_t s );
    template<typename T> inline void unpack( const char* d, uint32_t s, T& v );

   template<typename Stream, typename T>
   inline void pack( Stream& s, const fc::deque_1024<T>& value ) {
      FC_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fc::raw::pack( s, unsigned_int( (uint32_t) value.size() ) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
         fc::raw::pack( s, *itr );
         ++itr;
      }
   }

   template<typename Stream, typename T>
   inline void unpack( Stream& s, fc::deque_1024<T>& value ) {
      unsigned_int size;
      fc::raw::unpack( s, size );
      FC_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.resize( size.value );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
         fc::raw::unpack( s, *itr );
         ++itr;
      }
   }


   } }
