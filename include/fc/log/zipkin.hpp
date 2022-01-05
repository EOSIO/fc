#pragma once

#include <fc/log/logger.hpp>
#include <string>
#include <type_traits>

namespace fc {
/// Active Object that sends zipkin messages in JSON format
/// https://zipkin.io/zipkin-api/
///
/// Span contains following data
///     uint64_t    traceId   - unique id for trace, all children spans shared same id
///     std::string name      - logical operation, should have low cardinality
///     uint64_t    parentId  - The parent span id, or absent if root span
///     uint64_t    id        - unique id for this span
///     int64_t     timestamp - epoch microseconds of start of span
///     int64_t     duration  - microseconds of span
///
/// Enable zipkin by calling zipkin_config::init() from main thread on startup.
/// Use macros defined in trace.hpp.

class zipkin;

class sha256;

class zipkin_config {
public:
   /// Thread safe only if init() called from main thread before spawning of any threads
   static bool is_enabled() { return get_zipkin_() != nullptr; }

   /// Not thread safe, call from main thread before spawning any threads that might use zipkin.
   /// @param url the url endpoint of zipkin server. e.g. http://127.0.0.1:9411/api/v2/spans
   /// @param service_name the service name to include in each zipkin span
   /// @param timeout_us the timeout in microseconds for each http call
   ///        (9 consecutive failures and zipkin is disabled, SIGHUP will reset the failure counter and re-enable zipkin)
   /// @param retry_interval_us the interval in microseconds for connecting to zipkin
   /// @param wait_time_seconds the initial wait time in seconds for connecting to zipkin, an exception is thrown when the connection is not established within the wait time. 
   static void init( const std::string& url, const std::string& service_name, uint32_t timeout_us, uint32_t retry_interval_us, uint32_t wait_time_seconds = 0 );

   /// Thread safe only if init() called from main thread before spawning of any threads
   /// @throw assert_exception if called before init()
   static zipkin& get_zipkin();

   /// Thread safe only if init() called from main thread before spawning of any threads
   /// @throw assert_exception if called before init()
   static void shutdown();

   /// Starts with a random id and increments on each call, will not return 0
   static uint64_t get_next_unique_id();

   /// Signal safe
   static void handle_sighup();

private:
   /// Provide access to initialized zipkin endpoint
   /// Thread safe as long as init() called correctly
   /// @return nullptr if init() not called
   static zipkin* get_zipkin_() { return get().zip.get(); };

   static zipkin_config& get();

private:
   std::unique_ptr<zipkin> zip;
};

struct zipkin_span {
   explicit zipkin_span( std::string name, uint64_t trace_id, uint64_t parent_id )
         : data( std::move( name ), trace_id, parent_id ) {}

   explicit zipkin_span( uint64_t id, std::string name, uint64_t trace_id, uint64_t parent_id )
         : data( id, std::move( name ), trace_id, parent_id ) {}

   explicit zipkin_span( std::string name, fc::time_point start = time_point::now())
         : data( std::move( name ), start ) {}

   zipkin_span( const zipkin_span& ) = delete;
   zipkin_span& operator=( const zipkin_span& ) = delete;
   zipkin_span& operator=( zipkin_span&& ) = delete;

   zipkin_span( zipkin_span&& rhs ) noexcept
         : data( std::move( rhs.data ) ) {
      rhs.data.id = 0;
   }

   ~zipkin_span();

   void add_tag( const std::string& key, const std::string& var ) {
      // zipkin tags are required to be strings
      data.tags( key, var );
   }

   void add_tag( const std::string& key, const char* var ) {
      // zipkin tags are required to be strings
      data.tags( key, var );
   }

   void add_tag( const std::string& key, bool v ) {
      // zipkin tags are required to be strings
      data.tags( key, v ? "true" : "false" );
   }

   template<typename T>
   std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>
   add_tag( const std::string& key, T&& var ) {
      data.tags( key, std::to_string( std::forward<T>( var ) ) );
   }

   template<typename T>
   std::enable_if_t<!std::is_arithmetic_v<std::remove_reference_t<T>>, void>
   add_tag( const std::string& key, T&& var ) {
      data.tags( key, (std::string) var );
   }

   struct token {
      friend zipkin_span;
      friend struct zipkin_trace;
      friend struct optional_trace;
      constexpr explicit operator bool() const noexcept { return id != 0; }
      token( uint64_t id, uint64_t trace_id )
            : id( id ), trace_id( trace_id ) {}
   private:
      uint64_t id;
      uint64_t trace_id;
   };

   token get_token() const { return token{data.id, data.trace_id }; };

   static uint64_t to_id( const fc::sha256& id );

   struct span_data {
      explicit span_data( std::string name, uint64_t trace_id, uint64_t parent_id )
            : id( zipkin_config::get_next_unique_id() ), trace_id( trace_id ), parent_id( parent_id ),
              start( time_point::now() ), name( std::move( name ) ) {}

      explicit span_data( uint64_t id, std::string name, uint64_t trace_id, uint64_t parent_id )
            : id( id ), trace_id( trace_id == 0 ? id : trace_id ), parent_id( parent_id ), start( time_point::now() ), name( std::move( name ) ) {}

      explicit span_data( std::string name, fc::time_point start) 
         : id( zipkin_config::get_next_unique_id() ), trace_id( id ), parent_id( 0 ),
              start( start), name( std::move( name ) ) {}

      span_data( const span_data& ) = delete;
      span_data& operator=( const span_data& ) = delete;
      span_data& operator=( span_data&& ) = delete;
      span_data( span_data&& rhs ) = default;

      uint64_t id;
      const uint64_t             trace_id;
      const uint64_t             parent_id;
      const fc::time_point       start;
      fc::time_point stop;
      std::string name;
      fc::mutable_variant_object tags;
   };

   [[nodiscard]] std::optional<zipkin_span> create_span( std::string name ) const {
      return create_span_from_token(get_token(), std::move(name));
   }

   [[nodiscard]] static std::optional<zipkin_span>
   create_span_from_token( zipkin_span::token token, std::string name ) {
      return zipkin_span{std::move( name ), token.trace_id, token.id};
   }

   std::string trace_id_string() const;

   span_data data;
};

class zipkin {
public:
   zipkin( const std::string& url, const std::string& service_name, uint32_t timeout_us, uint32_t retry_interval_us , uint32_t wait_time_seconds );

   /// finishes logging all queued up spans
   ~zipkin() = default;

   /// Starts with a random id and increments on each call, will not return 0
   uint64_t get_next_unique_id();

   // finish logging all queued up spans, not restartable
   void shutdown();

   // Logs zipkin json via http on separate thread
   void log( zipkin_span::span_data&& span );

   // Post http request to the boost asio queue
   void post_request(zipkin_span::span_data&& span);

private:
   class impl;
   std::unique_ptr<impl> my;
};

} // namespace fc

