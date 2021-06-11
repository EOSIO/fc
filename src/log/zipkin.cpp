#include <fc/log/zipkin.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/network/http/http_client.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/variant.hpp>

#include <boost/asio.hpp>

#include <thread>
#include <random>

namespace fc {

struct local_endpoint_resolver {
   using tcp        = boost::asio::ip::tcp;
   using error_code = boost::system::error_code;

   local_endpoint_resolver(boost::asio::io_context& ctx)
       : resolver(ctx)
       , sock(ctx)
       , timer(ctx) {}
   boost::asio::io_context      ctx;
   tcp::resolver                resolver;
   tcp::socket                  sock;
   boost::asio::deadline_timer  timer;
   std::string                  remote;
   tcp::resolver::results_type  endpoints;
   std::optional<tcp::endpoint> local_endpoint;

   void async_resolve(std::string remote_host, std::string port) {
      remote = remote_host + ":" + port;
      resolver.async_resolve(remote_host, port, [this](const error_code& ec, tcp::resolver::results_type resolved) {
         if (ec)
            throw boost::system::system_error(ec);
         endpoints = resolved;
         do_connect();
      });
   }

   void do_connect() {
      boost::asio::async_connect(sock, endpoints, [this](const error_code& ec, const tcp::endpoint& endpoint) {
         if (ec) {
            wlog("failed to connect to ${remote}, retry in 5 seconds", ("remote", remote));
            timer.expires_from_now(boost::posix_time::seconds(5));
            timer.async_wait([this](const error_code& ec) {
               if (!ec)
                  do_connect();
            });
            return;
         }
         local_endpoint = sock.local_endpoint();
         ilog("connected to ${remote}", ("remote", remote));
      });
   }
};

zipkin_config& zipkin_config::get() {
   static zipkin_config the_one;
   return the_one;
}

void zipkin_config::init( const std::string& url, const std::string& service_name, uint32_t timeout_us, uint32_t wait_time_seconds ) {
   get().zip = std::make_unique<zipkin>( url, service_name, timeout_us, wait_time_seconds);
}

zipkin& zipkin_config::get_zipkin() {
   if( !get().zip ) {
      FC_THROW_EXCEPTION( fc::assert_exception, "uninitialized zipkin" );
   }
   return *get().zip;
}

void zipkin_config::shutdown() {
   if( zipkin* z = get_zipkin_() ) {
      z->shutdown();
   }
}

uint64_t zipkin_config::get_next_unique_id() {
   if( !get().zip ) {
      FC_THROW_EXCEPTION( fc::assert_exception, "uninitialized zipkin" );
   }
   return get().zip->get_next_unique_id();
}

class zipkin::impl {
public:
   static constexpr uint32_t max_consecutive_errors = 9;

   const std::string zipkin_url;
   const std::string service_name;
   const uint32_t timeout_us;
   std::mutex mtx;
   uint64_t next_id = 0;
   http_client http;
   std::atomic<uint32_t> consecutive_errors = 0;
   std::atomic<unsigned char> stopped = 0;
   std::optional<url> endpoint;
   std::thread thread;
   boost::asio::io_context ctx;
   boost::asio::io_context::strand work_strand{ctx};
   boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ctx);
   std::optional<boost::asio::ip::tcp::endpoint>  local_endpoint;


   impl( std::string url, std::string service_name, uint32_t timeout_us )
         : zipkin_url( std::move(url) )
         , service_name( std::move(service_name) )
         , timeout_us( timeout_us ) {
   }

   void init(uint32_t wait_time_seconds);
   void shutdown();

   void log( zipkin_span::span_data&& span );

   ~impl();
};

void zipkin::impl::init(uint32_t wait_time_seconds) {
   if (wait_time_seconds > 0) {
      endpoint = url( zipkin_url );
      if (!endpoint->host() || endpoint->host()->empty())
         FC_THROW("Invalid url ${url}", ("url", zipkin_url));

      local_endpoint_resolver resolver;
      resolver.async_resolve(*endpoint->host(), std::to_string(*endpoint->port()));
      auto deadline =  std::chrono::system_clock::now() + std::chrono::seconds(wait_time_seconds);
      resolver.ctx.run_until(deadline);

      local_endpoint = resolver.local_endpoint;

      if (!local_endpoint) {
         FC_THROW("Unable to connect to ${url} within ${wait_time_seconds} seconds", ("url", zipkin_url)("wait_time_seconds", wait_time_seconds));
      }
   }

   thread = std::thread( [this]() {
      fc::set_os_thread_name( "zipkin" );
      while( true ) {
         try {
            ctx.run();
            break;
         } FC_LOG_AND_DROP();
      }
   } );
}

zipkin::impl::~impl() {
   try {
      shutdown();
   } catch (...) {}
}

void zipkin::impl::shutdown() {
   if( stopped ^= 1 ) return;
   work_guard.reset(); // drain the queue
   thread.join();
}

zipkin::zipkin( const std::string& url, const std::string& service_name, uint32_t timeout_us, uint32_t wait_time_seconds ) :
      my( new impl( url, service_name, timeout_us ) ) {
   my->init(wait_time_seconds);
}

uint64_t zipkin::get_next_unique_id() {
   std::scoped_lock g( my->mtx );
   if( my->next_id == 0 ) {
      std::mt19937_64 engine( std::random_device{}() );
      std::uniform_int_distribution<uint64_t> distribution(1);
      my->next_id = distribution( engine );
   }
   return my->next_id++;
}

void zipkin::shutdown() {
   my->shutdown();
}

fc::variant create_zipkin_variant( zipkin_span::span_data&& span, const std::string& service_name, std::optional<boost::asio::ip::tcp::endpoint>& local_endpoint ) {
   // https://zipkin.io/zipkin-api/
   //   std::string traceId;  // [a-f0-9]{16,32} unique id for trace, all children spans shared same id
   //   std::string name;     // logical operation, should have low cardinality
   //   std::string parentId; // The parent span id, or absent if root span
   //   std::string id        // a-f0-9]{16}
   //   int64_t     timestamp // epoch microseconds of start of span
   //   int64_t     duration  // microseconds of span

   fc::mutable_variant_object mvo;
   mvo( "id", fc::to_hex( reinterpret_cast<const char*>(&span.id), sizeof( span.id ) ) );
   mvo( "traceId", fc::to_hex( reinterpret_cast<const char*>(&span.trace_id), sizeof( span.trace_id ) ) );
   if( span.parent_id != 0 ) {
      mvo( "parentId", fc::to_hex( reinterpret_cast<const char*>(&span.parent_id), sizeof( span.parent_id ) ) );
   }
   mvo( "name", std::move( span.name ) );
   mvo( "timestamp", span.start.time_since_epoch().count() );
   mvo( "duration", (span.stop - span.start).count() );

   mutable_variant_object local_endpoint_mvo("serviceName", service_name);
   if (local_endpoint) {
      const auto &address = local_endpoint->address();
      local_endpoint_mvo( address.is_v4() ? "ipv4": "ipv6", address.to_string());
   }
   mvo( "localEndpoint", local_endpoint_mvo );

   mvo( "tags", std::move( span.tags ) );
   span.id = 0; // stop destructor of span from calling log again

   // /api/v2/spans takes an array of spans
   fc::variants result;
   result.emplace_back( std::move( mvo ) );

   return result;
}

void zipkin::log( zipkin_span::span_data&& span ) {
   if( my->consecutive_errors > my->max_consecutive_errors || my->stopped )
      return;

   boost::asio::post(my->work_strand, [this, span{std::move(span)}]() mutable {
      my->log( std::move( span ) );
   });
}

void zipkin::impl::log( zipkin_span::span_data&& span ) {
   if (consecutive_errors > max_consecutive_errors) {
      wlog("consecutive_errors=${consecutive_errors} exceeds "
            "limit($max_consecutive_errors)",
            ("consecutive_errors", consecutive_errors.load())("max_consecutive_errors",
                                                      max_consecutive_errors));
      return;
   }

   try {
      auto deadline = fc::time_point::now() + fc::microseconds( timeout_us );
      if( !endpoint ) {
         endpoint = url( zipkin_url );
         dlog( "connecting to zipkin: ${p}", ("p", *endpoint) );
      }

      http.post_sync(*endpoint, create_zipkin_variant(std::move(span), service_name, local_endpoint), deadline, fc::json::output_formatting::legacy_generator);

      consecutive_errors = 0;
      return;
   } catch( const fc::exception& e ) {
      wlog( "unable to connect to zipkin: ${u}, error: ${e}", ("u", zipkin_url)("e", e.to_detail_string()) );
   } catch( const std::exception& e ) {
      wlog( "unable to connect to zipkin: ${u}, error: ${e}", ("u", zipkin_url)("e", e.what()) );
   } catch( ... ) {
      wlog( "unable to connect to zipkin: ${u}, error: unknown", ("u", zipkin_url) );
   }
   ++consecutive_errors;
}

uint64_t zipkin_span::to_id( const fc::sha256& id ) {
   // avoid 0 since id of 0 is used as a flag
   return id._hash[3] == 0 ? 1 : id._hash[3];
}

zipkin_span::~zipkin_span() {
   if( data.id == 0 )
      return;
   try {
      if( zipkin_config::is_enabled() ) {
         data.stop = time_point::now();
         zipkin_config::get_zipkin().log( std::move( data ) );
      }
   } catch( ... ) {}
}

std::string zipkin_span::trace_id_string() const {
   return fc::to_hex(reinterpret_cast<const char *>(&data.trace_id), sizeof(data.trace_id));
}

} // fc
