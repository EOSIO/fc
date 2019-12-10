#include <fc/network/message_buffer.hpp>

#include <thread>

#define BOOST_TEST_MODULE message_buffer
#include <boost/test/included/unit_test.hpp>

// Make sure that the memory allocation is thread-safe.
// A previous version used boost::object_pool without synchronization.
BOOST_AUTO_TEST_CASE(test_message_buffer) {
   bool done = false;
   std::vector<std::thread> threads;
   constexpr int num_threads = 4;
   constexpr int iterations = 10000;
   for(int i = 0; i < num_threads; ++i) {
      threads.emplace_back([]{
         for(int i = 0; i < iterations; ++i) {
            fc::message_buffer<1024*1024> buf;
         }
      });
   }
   for(std::thread& t : threads) {
      t.join();
   }
}
