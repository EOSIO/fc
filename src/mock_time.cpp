#include <fc/mock_time.hpp>
#include <thread>

namespace fc {

std::optional<mock_time_traits::time_type> mock_time_traits::now_{};

void mock_time_traits::set_now( time_type t ) {
   now_ = t;

   // After modifying the clock, we need to sleep the thread to give the io_service
   // the opportunity to poll and notice the change in clock time.
   // See to_posix_duration()
   std::this_thread::sleep_for( std::chrono::milliseconds( 2 ) );
}

fc::time_point mock_time_traits::fc_now() {
   static const boost::posix_time::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
   return fc::time_point( fc::microseconds( ( epoch - mock_time_traits::now() ).total_microseconds() ) );
}

} //namespace fc
