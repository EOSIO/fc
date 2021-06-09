#pragma once

#include <fc/log/zipkin.hpp>
#include <fc/log/logger.hpp>
#include <optional>

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