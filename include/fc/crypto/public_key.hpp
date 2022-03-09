#pragma once
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/elliptic_r1.hpp>
#include <fc/crypto/elliptic_webauthn.hpp>
#include <fc/crypto/signature.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/static_variant.hpp>

namespace fc { namespace crypto {
   namespace config {
      constexpr const char* public_key_legacy_prefix = "EOS";
      constexpr const char* public_key_base_prefix = "PUB";
      constexpr const char* public_key_prefix[] = {
         "K1",
         "R1",
         "WA"
      };
   };

   class public_key
   {
      public:
         using storage_type = std::variant<ecc::public_key_shim, r1::public_key_shim, webauthn::public_key>;

         public_key() = default;
         public_key( public_key&& ) = default;
         public_key( const public_key& ) = default;
         public_key& operator= (const public_key& ) = default;

         public_key( const signature& c, const sha256& digest, bool check_canonical = true );

         public_key( storage_type&& other_storage )
         :_storage(forward<storage_type>(other_storage))
         {}

         bool valid()const;

         int which()const;

         // serialize to/from string
         explicit public_key(const string& base58str);
         std::string to_string(const fc::yield_function_t& yield = fc::yield_function_t()) const;

         storage_type _storage;

      private:
         friend std::ostream& operator<< (std::ostream& s, const public_key& k);
         friend bool operator == ( const public_key& p1, const public_key& p2);
         friend bool operator != ( const public_key& p1, const public_key& p2);
         friend bool operator < ( const public_key& p1, const public_key& p2);
         friend struct reflector<public_key>;
         friend class private_key;
   }; // public_key

} }  // fc::crypto

namespace fc {
   void to_variant(const crypto::public_key& var, variant& vo, const fc::yield_function_t& yield = fc::yield_function_t());

   void from_variant(const variant& var, crypto::public_key& vo);
} // namespace fc

namespace fmt {
   template<>
   struct formatter<fc::crypto::public_key::storage_type> {
      template<typename ParseContext>
      constexpr auto parse( ParseContext& ctx ) { return ctx.begin(); }

      template<typename FormatContext>
      auto format( const fc::crypto::public_key::storage_type& p, FormatContext& ctx ) {
         std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, fc::ecc::public_key_shim> || std::is_same_v<T, fc::crypto::r1::public_key_shim>)
               fmt::formatter<T>().format(arg, ctx);
            else if constexpr( std::is_same_v<T, fc::crypto::webauthn::public_key> )
               format_to( ctx.out(), ""); //TODO: log contents?
         }, p);
         return format_to( ctx.out(), "");
      }
   };
}

FC_REFLECT(fc::crypto::public_key, (_storage) )
