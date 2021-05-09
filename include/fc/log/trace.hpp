#pragma once

#include <fc/log/zipkin.hpp>
#include <optional>

/// @param TRACE_STR const char* identifier for trace
/// @return implementation defined type RAII object that submits trace on exit of scope
#define fc_create_trace( TRACE_STR ) \
      ::fc::zipkin_config::is_enabled() ? \
        ::std::optional<::fc::zipkin_span>( ::std::in_place, (TRACE_STR) ) \
        : ::std::optional<::fc::zipkin_span>{};

/// @param TRACE_STR const char* identifier for trace
/// @param TRACE_ID fc::sha256 id to use
/// @return implementation defined type RAII object that submits trace on exit of scope
#define fc_create_trace_with_id( TRACE_STR, TRACE_ID ) \
      ::fc::zipkin_config::is_enabled() ? \
        ::std::optional<::fc::zipkin_span>( ::std::in_place, ::fc::zipkin_span::to_id(TRACE_ID), (TRACE_STR) ) \
        : ::std::optional<::fc::zipkin_span>{};

/// @param TRACE_VARNAME variable returned from fc_create_trace
/// @param SPAN_STR const char* indentifier
/// @return implementation defined type RAII object that submits span on exit of scope
#define fc_create_span( TRACE_VARNAME, SPAN_STR ) \
      ( (TRACE_VARNAME) && ::fc::zipkin_config::is_enabled() ) ? \
        (TRACE_VARNAME)->create_span( (SPAN_STR) ) \
        : ::std::optional<::fc::zipkin_span>{};

/// @param TRACE_TOKEN variable returned from trace.get_token()
/// @param SPAN_STR const char* indentifier
/// @return implementation defined type RAII object that submits span on exit of scope
#define fc_create_span_from_token( TRACE_TOKEN, SPAN_STR ) \
      ( (TRACE_TOKEN) && ::fc::zipkin_config::is_enabled() ) ? \
        ::fc::zipkin_span::create_span_from_token( (TRACE_TOKEN), (SPAN_STR) ) \
        : ::std::optional<::fc::zipkin_span>{};

/// @param SPAN_VARNAME variable returned from fc_create_span
/// @param TAG_KEY_STR string key
/// @param TAG_VALUE string value
#define fc_add_tag( SPAN_VARNAME, TAG_KEY_STR, TAG_VALUE_STR) \
  FC_MULTILINE_MACRO_BEGIN \
      if( (SPAN_VARNAME) && ::fc::zipkin_config::is_enabled() ) \
         (SPAN_VARNAME)->add_tag( (TAG_KEY_STR), (TAG_VALUE_STR) ); \
  FC_MULTILINE_MACRO_END

inline fc::zipkin_span::token fc_get_token(const ::std::optional<::fc::zipkin_span>& span) {
  return (span && ::fc::zipkin_config::is_enabled())
     ? span->get_token() : fc::zipkin_span::token(0, 0);
}