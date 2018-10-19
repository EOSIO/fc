#pragma once
#include <fc/optional.hpp>
#include <fc/log/logger.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <future>
#include <mutex>

namespace fc {

/// Simple thread_pool which provides a specified number of threads
/// started in init() and terminated in stop() for processing lambdas
/// passed to async().
class thread_pool {

public:
   ~thread_pool() {
      stop();
   }

   /// Spawn specified number of threads for pool
   void init( int num_threads ) {
      std::lock_guard<std::mutex> g{_mtx};
      _num_threads = num_threads;
      _stop();

      _ioc.reset( new boost::asio::io_context{_num_threads} );

      auto& ioc = *_ioc;
      work_guard.emplace( boost::asio::make_work_guard( ioc ));

      for( auto i = 0; i < _num_threads; ++i ) {
         _threads.emplace_back( [&ioc] {
            ilog( "start thread_pool thread" );
            ioc.run();
            ilog( "end thread_pool thread" );
         } );
      }
   }

   /// submit lambda f to pool and return future for the packaged task
   /// @param f lambda to execute in thread pool
   /// @return std::future<f()>
   template<typename F>
   auto async( F&& f ) {
      auto task = std::make_shared<std::packaged_task<decltype( f() )()>>( std::forward<F>(f) );
      _ioc->post( [task]() { (*task)(); } );
      return task->get_future();
   }

   /// @return the io_context running in pool
   boost::asio::io_context& get_io_context() {
      return *_ioc;
   }

   /// Stops joins all threads and stops io_context
   void stop() {
      std::lock_guard<std::mutex> g{_mtx};
      _stop();
   }

private:

   void _stop() {
      if( _ioc && !_ioc->stopped()) {
         work_guard.reset();

         // join_all
         for( auto& t : _threads ) {
            if( t.joinable()) {
               t.join();
            }
         }
         _threads.clear();

         _ioc->stop();
      }
   }

private:
   std::mutex _mtx;
   int _num_threads = 1;
   std::vector<std::thread> _threads;
   std::unique_ptr<boost::asio::io_context> _ioc;
   fc::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard;
};


} // namespace fc
