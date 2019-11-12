#pragma once
#include <fc/crypto/ripemd160.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/io/raw.hpp>
#include <fc/utility.hpp>
#include <fc/static_variant.hpp>
#include <openssl/bn.h>

namespace fc { namespace crypto {
    /**
     *  @class big_number
     *  @brief a C++ wrapper over BIGNUM from openssl
     */
    class big_number {
       public:
         big_number() = default;
         big_number(const char* bytes, std::size_t len) { from_bytes(bytes, len); }
         big_number(const std::string& hex) { from_hex(hex); }
         big_number(const big_number& bn) {
            _data = BN_dup(bn._data);
            FC_ASSERT(_data);
         }
         big_number(big_number&& bn) : _data(bn._data) {
            bn._data = nullptr;
         }
         ~big_number() {
            if (_data)
               BN_free(_data);
         }

         operator BIGNUM*() { return _data; }
         operator const BIGNUM*()const { return _data; }
         BIGNUM* data() { return _data; }
         const BIGNUM* data()const { return _data; }

         void from_bytes(const char* bytes, std::size_t len) {
            _data = BN_bin2bn((const unsigned char*)bytes, len, nullptr);
            FC_ASSERT(_data, "invalid binary to big number");
         }

         void from_hex(const std::string& hex) {
            char buff[1024];
            FC_ASSERT(hex.size() < 1024);
            int sz = from_hex(hex, buff, 1024);
            _data = BN_bin2bn((const unsigned char*)buff, sz, nullptr);
            FC_ASSERT(_data, "invalid binary to big number");
         }

         std::size_t size()const { return BN_num_bytes(_data); }
         std::vector<char> to_bytes()const {
            std::vector<char> bytes;
            bytes.resize(size());
            BN_bn2bin(_data, (unsigned char*)bytes.data());
            return bytes;
         }
         std::string to_hex()const { return BN_bn2hex(_data); }
       private:
         uint8_t from_hex( char c ) {
            if( c >= '0' && c <= '9' )
            return c - '0';
            if( c >= 'a' && c <= 'f' )
               return c - 'a' + 10;
            if( c >= 'A' && c <= 'F' )
               return c - 'A' + 10;
            FC_ASSERT(false, "from_hex failed");
         }

         int from_hex( const std::string& hex_str, char* out_data, size_t out_data_len ) {
            int j=0;
            for (int i=0; i < hex_str.size(); i += 2, j++)
               out_data[j] = (from_hex(hex_str[i]) << 4) | from_hex(hex_str[i+1]);
            return j;
         }
         BIGNUM* _data = nullptr;
    };

   template<typename DataType>
   struct checksummed_data {
      checksummed_data() {}
      uint32_t     check = 0;
      DataType     data;

      static auto calculate_checksum(const DataType& data, const char *prefix = nullptr) {
         auto encoder = ripemd160::encoder();
         raw::pack(encoder, data);

         if (prefix != nullptr) {
            encoder.write(prefix, const_strlen(prefix));
         }
         return encoder.result()._hash[0];
      }
   };

   inline bool prefix_matches(const char* prefix, const std::string& str) {
      auto prefix_len = const_strlen(prefix);
      return str.size() > prefix_len && str.substr(0, prefix_len).compare(prefix) == 0;
   }

   template<typename, const char * const *, int, typename ...>
   struct base58_str_parser_impl;

   template<typename Result, const char * const * Prefixes, int Position, typename KeyType, typename ...Rem>
   struct base58_str_parser_impl<Result, Prefixes, Position, KeyType, Rem...> {
      static Result apply(const std::string& prefix_str, const std::string& data_str)
      {
         using data_type = typename KeyType::data_type;
         using wrapper = checksummed_data<data_type>;
         constexpr auto prefix = Prefixes[Position];

         if (prefix == prefix_str) {
            auto bin = fc::from_base58(data_str);
            fc::datastream<const char*> unpacker(bin.data(), bin.size());
            wrapper wrapped;
            fc::raw::unpack(unpacker, wrapped);
            FC_ASSERT(!unpacker.remaining(), "decoded base58 length too long");
            auto checksum = wrapper::calculate_checksum(wrapped.data, prefix);
            FC_ASSERT(checksum == wrapped.check);
            return Result(KeyType(wrapped.data));
         }

         return base58_str_parser_impl<Result, Prefixes, Position + 1, Rem...>::apply(prefix_str, data_str);
      }
   };

   template<typename Result, const char * const * Prefixes, int Position>
   struct base58_str_parser_impl<Result, Prefixes, Position> {
      static Result apply(const std::string& prefix_str, const std::string& data_str ) {
         FC_ASSERT(false, "No matching suite type for ${prefix}_${data}", ("prefix", prefix_str)("data",data_str));
      }
   };

   template<typename, const char * const * Prefixes>
   struct base58_str_parser;

   /**
    * Destructure a variant and call the parse_base58str on it
    * @tparam Ts
    * @param base58str
    * @return
    */
   template<const char * const * Prefixes, typename ...Ts>
   struct base58_str_parser<fc::static_variant<Ts...>, Prefixes> {
      static fc::static_variant<Ts...> apply(const std::string& base58str) {
         const auto pivot = base58str.find('_');
         FC_ASSERT(pivot != std::string::npos, "No delimiter in data, cannot determine suite type: ${str}", ("str", base58str));

         const auto prefix_str = base58str.substr(0, pivot);
         auto data_str = base58str.substr(pivot + 1);
         FC_ASSERT(!data_str.empty(), "Data only has suite type prefix: ${str}", ("str", base58str));

         return base58_str_parser_impl<fc::static_variant<Ts...>, Prefixes, 0, Ts...>::apply(prefix_str, data_str);
      }
   };

   template<typename Storage, const char * const * Prefixes, int DefaultPosition = -1>
   struct base58str_visitor : public fc::visitor<std::string> {
      template< typename KeyType >
      std::string operator()( const KeyType& key ) const {
         using data_type = typename KeyType::data_type;
         constexpr int position = Storage::template position<KeyType>();
         constexpr bool is_default = position == DefaultPosition;

         checksummed_data<data_type> wrapper;
         wrapper.data = key.serialize();
         wrapper.check = checksummed_data<data_type>::calculate_checksum(wrapper.data, !is_default ? Prefixes[position] : nullptr);
         auto packed = raw::pack( wrapper );
         auto data_str = to_base58( packed.data(), packed.size() );
         if (!is_default) {
            data_str = string(Prefixes[position]) + "_" + data_str;
         }

         return data_str;
      }
   };

   template<typename T>
   struct eq_comparator {
      static bool apply(const T& a, const T& b) {
         return a.serialize() == b.serialize();
      }
   };

   template<typename ... Ts>
   struct eq_comparator<fc::static_variant<Ts...>> {
      using variant_type = fc::static_variant<Ts...>;
      struct visitor : public fc::visitor<bool> {
         visitor(const variant_type &b)
            : _b(b) {}

         template<typename KeyType>
         bool operator()(const KeyType &a) const {
            const auto &b = _b.template get<KeyType>();
            return eq_comparator<KeyType>::apply(a,b);
         }

         const variant_type &_b;
      };

      static bool apply(const variant_type& a, const variant_type& b) {
         return a.which() == b.which() && a.visit(visitor(b));
      }
   };

   template<typename T>
   struct less_comparator {
      static bool apply(const T& a, const T& b) {
         return a.serialize() < b.serialize();
      }
   };

   template<typename ... Ts>
   struct less_comparator<fc::static_variant<Ts...>> {
      using variant_type = fc::static_variant<Ts...>;
      struct visitor : public fc::visitor<bool> {
         visitor(const variant_type &b)
            : _b(b) {}

         template<typename KeyType>
         bool operator()(const KeyType &a) const {
            const auto &b = _b.template get<KeyType>();
            return less_comparator<KeyType>::apply(a,b);
         }

         const variant_type &_b;
      };

      static bool apply(const variant_type& a, const variant_type& b) {
         return a.which() < b.which() || (a.which() == b.which() && a.visit(visitor(b)));
      }
   };

   template<typename Data>
   struct shim {
      using data_type = Data;

      shim()
      {}

      shim(data_type&& data)
      :_data(forward<data_type>(data))
      {}

      shim(const data_type& data)
      :_data(data)
      {}

      const data_type& serialize() const {
         return _data;
      }

      data_type _data;
   };

} }

FC_REFLECT_TEMPLATE((typename T), fc::crypto::checksummed_data<T>, (data)(check) )
FC_REFLECT_TEMPLATE((typename T), fc::crypto::shim<T>, (_data) )
