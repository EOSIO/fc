#pragma once

#include <fc/log/zipkin.hpp>
#include <fc/log/logger.hpp>
#include <optional>

#include <eosio/chain/abi_serializer.hpp>

/// @param trace_str const char* identifier for trace
/// @return implementation defined type RAII object that submits trace on exit of scope
inline ::std::optional<::fc::zipkin_span> fc_create_trace(const char* trace_str) {
   return ::fc::zipkin_config::is_enabled()
              ? ::std::optional<::fc::zipkin_span>(::std::in_place, ::fc::zipkin_config::get_next_unique_id(),
                                                   (trace_str), 0, 0)
              : ::std::optional<::fc::zipkin_span>{};
}

/// @param trace_str const char* identifier for trace
/// @param trace_id fc::sha256 id to use
/// @return implementation defined type RAII object that submits trace on exit of scope
inline ::std::optional<::fc::zipkin_span> fc_create_trace_with_id(const char* trace_str, const fc::sha256& trace_id) {
   return ::fc::zipkin_config::is_enabled()
              ? ::std::optional<::fc::zipkin_span>(::std::in_place, ::fc::zipkin_span::to_id(trace_id), (trace_str), 0 , 0)
              : ::std::optional<::fc::zipkin_span>{};
}

/// @param condition create the trace only when the condition is true
/// @param trace_str const char* identifier for trace
/// @param trace_id fc::sha256 id to use
/// @return implementation defined type RAII object that submits trace on exit of scope
inline ::std::optional<::fc::zipkin_span> fc_create_trace_with_id_if(bool condition, const char* trace_str, const fc::sha256& trace_id) {
   return (condition && ::fc::zipkin_config::is_enabled())
              ? ::std::optional<::fc::zipkin_span>(::std::in_place, ::fc::zipkin_span::to_id(trace_id), (trace_str), ::fc::zipkin_span::to_id(trace_id) , 0)
              : ::std::optional<::fc::zipkin_span>{};
}

inline ::std::optional<::fc::zipkin_span> fc_create_span_with_id(const char* span_str, uint64_t id, const fc::sha256& trace_id) {
   auto tid = ::fc::zipkin_span::to_id(trace_id);
   return ::fc::zipkin_config::is_enabled()
              ? ::std::optional<::fc::zipkin_span>(::std::in_place, id, span_str, tid, tid)
              : ::std::optional<::fc::zipkin_span>{};
}

inline ::std::optional<::fc::zipkin_span> fc_create_trace_with_start_time(const char* trace_str, fc::time_point start) {
   return ::fc::zipkin_config::is_enabled()
              ? ::std::optional<::fc::zipkin_span>(::std::in_place, trace_str, start)
              : ::std::optional<::fc::zipkin_span>{};
}

/// @param trace variable returned from fc_create_trace
/// @param span_str const char* indentifier
/// @return implementation defined type RAII object that submits span on exit of scope
inline ::std::optional<::fc::zipkin_span> fc_create_span(const ::std::optional<::fc::zipkin_span>& trace,
                                                         const char*                               span_str) {
   return ((trace) && ::fc::zipkin_config::is_enabled()) ? (trace)->create_span((span_str))
                                                         : ::std::optional<::fc::zipkin_span>{};
}

/// @param trace_token variable returned from trace.get_token()
/// @param span_str const char* indentifier
/// @return implementation defined type RAII object that submits span on exit of scope
inline ::std::optional<::fc::zipkin_span> fc_create_span_from_token(fc::zipkin_span::token trace_token,
                                                                    const char*            span_str) {
   return ((trace_token) && ::fc::zipkin_config::is_enabled())
              ? ::fc::zipkin_span::create_span_from_token((trace_token), (span_str))
              : ::std::optional<::fc::zipkin_span>{};
}

/// @param span variable returned from fc_create_span
/// @param tag_key_str string key
/// @param tag_value string value
template <typename Value>
inline void fc_add_tag(::std::optional<::fc::zipkin_span>& span, const char* tag_key_str, Value&& value) {
   if ((span) && ::fc::zipkin_config::is_enabled())
      (span)->add_tag((tag_key_str), std::forward<Value>(value));
}

inline fc::zipkin_span::token fc_get_token(const ::std::optional<::fc::zipkin_span>& span) {
   return (span && ::fc::zipkin_config::is_enabled()) ? span->get_token() : fc::zipkin_span::token(0, 0);
}


#define fc_trace_log( TRACE_OR_SPAN, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (fc::logger::get(DEFAULT_LOGGER)).is_enabled( fc::log_level::info ) ) \
      (fc::logger::get(DEFAULT_LOGGER)).log( FC_LOG_MESSAGE( info, FORMAT " traceID=${_the_trace_id_}", ("_the_trace_id_", TRACE_OR_SPAN->trace_id_string()) __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

template <typename T, typename = void>
struct has_fmt_to_string : std::false_type {};

template <typename T>
struct has_fmt_to_string<T, std::void_t<decltype(fmt::to_string(std::declval<T>()))>> : std::true_type{};

template<typename T>
struct member_pointer_value {
    typedef T type;
};

template<typename Class, typename Value>
struct member_pointer_value<Value Class::*> {
    typedef Value type;
};

template<class T, template<class...> class Primary>
struct is_specialization_of : std::false_type {};

template<template<class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type{ };

template<class T, template<class...> class Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;


template<typename T>
class to_string_visitor
{
public:
    to_string_visitor( T& v, std::string& out)
            : obj(v)
            , out(out)
    {
       out += "{";
    }

    ~to_string_visitor() {
       out += "}";
    }

    template<typename X>
    void append_str(std::string& out, const char* name, const X& t)const {
       out += "\"";
       out += name;
       out += "\":\"";
       if constexpr (std::is_integral_v<X>) {
          out += std::to_string(t);
       } else if constexpr(std::is_same_v<X, std::string>) {
          out += t;
       } else if constexpr(std::is_convertible_v<X, std::string>) {
          out += (std::string)t;
       } else if constexpr(std::is_same_v<X, eosio::chain::name>) {
          out += t.to_string();
       } else {
          out += "~unknown~";
       }
       out += "\"";
    }

    /**
     * Visit a single member and extract it from the variant object
     * @tparam Member - the member to visit
     * @tparam Class - the class we are traversing
     * @tparam member - pointer to the member
     * @param name - the name of the member
     */
    template<typename Member, class Class, Member (Class::*member)>
    void operator()( const char* name )const
    {
       using mem_type = std::decay_t<typename member_pointer_value<decltype(this->obj.*member)>::type>;

       if( ++depth > 1 ) {
          out += ",";
       }
       if constexpr (std::is_integral_v<mem_type>) {
          append_str(out, name, this->obj.*member);
       } else if constexpr(std::is_same_v<mem_type, std::string>) {
          append_str(out, name, this->obj.*member);
       } else if constexpr(std::is_same_v<mem_type, fc::unsigned_int> || std::is_same_v<mem_type, fc::signed_int>) {
          append_str(out, name, (this->obj.*member).value);
       } else if constexpr(std::is_same_v<mem_type, eosio::chain::name>) {
          append_str(out, name, (this->obj.*member).to_string());
       } else if constexpr(std::is_convertible_v<mem_type, std::string>) {
          append_str(out, name, this->obj.*member);
       } else if constexpr(std::is_convertible_v<mem_type, eosio::chain::action>) {
          auto& act = this->obj.*member;
          if( act.account == eosio::chain::config::system_account_name && act.name.to_string() == "setcode" ) {
             append_str(out, "account", act.account); out += ",";
             append_str(out, "name", act.name); out += ",";
             out += "\"authorization\":";
             out += fmt::to_string(act.authorization);
             out += ",";
             auto setcode_act = act.template data_as<eosio::chain::setcode>();
             if( setcode_act.code.size() > 0 ) {
                fc::sha256 code_hash = fc::sha256::hash(setcode_act.code.data(), (uint32_t) setcode_act.code.size());
                append_str(out, "code_hash", code_hash); out += ",";
             }
             out += "\""; out += "data"; out += "\""; out += ":{";
             const auto& m = act.data;
             append_str(out, "size", m.size()); out += ",";
             if( m.size() > eosio::chain::impl::hex_log_max_size ) {
                out += "\"trimmed_hex\":\"" + fc::to_hex(&m[0], eosio::chain::impl::hex_log_max_size) + "\"";
             } else if (m.size() > 0) {
                out += "\"hex\":\"" + fc::to_hex(&m[0], m.size()) + "\"";
             } else {
                out += "\"hex\":\"\"";
             }
             out += "}";
          } else {
             append_str(out, name, fmt::to_string(this->obj.*member));
          }
       } else if constexpr (is_specialization_of_v<mem_type, std::optional>) {
          if( (this->obj.*member).has_value() ) {
             append_str( out, name, *(this->obj.*member) );
          } else {
             out += "\""; out += name; out += "\":null";
          }
       } else if constexpr (is_specialization_of_v<mem_type, std::shared_ptr>) {
          if( !!(this->obj.*member) ) {
             append_str( out, name, *(this->obj.*member) );
          } else {
             out += "\""; out += name; out += "\":null";
          }
       } else if constexpr (std::is_same_v<mem_type, std::vector<char>>) {
          out += "\""; out += name; out += "\""; out += ":{";
          const auto& m = this->obj.*member;
          if( m.size() > eosio::chain::impl::hex_log_max_size ) {
             out += "\"trimmed_hex\":\"" + fc::to_hex(&m[0], eosio::chain::impl::hex_log_max_size) + "\"";
          } else if (m.size() > 0) {
             out += "\"hex\":\"" + fc::to_hex(&m[0], m.size()) + "\"";
          } else {
             out += "\"hex\":\"\"";
          }
          out += "}";
       } else if constexpr(has_fmt_to_string<mem_type>::value) {
          append_str(out, name, fmt::to_string(this->obj.*member));
       } else if constexpr(std::is_same_v<typename fc::reflector<mem_type>::is_defined, fc::true_type>) {
          out += "\""; out += name; out += "\""; out += ":";
          to_string_visitor<mem_type> v(this->obj.*member, out);
          fc::reflector<mem_type>::visit(v);
       } else {
          //static_assert(std::is_integral_v<mem_type>, "Not FC_REFLECT and no fmt::to_string");
          append_str(out, name, "~unknown~");
       }
    }

private:
    T& obj;
    std::string& out;
    mutable uint32_t depth = 0;
};