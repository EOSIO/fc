#pragma once

#include <fc/time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <optional>


namespace fc {

/// mock out fc::time_point::now() and provide a mock deadline timer
class mock_time_traits {
   typedef boost::asio::deadline_timer::traits_type source_traits;
public:
   typedef source_traits::time_type time_type;
   typedef source_traits::duration_type duration_type;

   // Requires set_now() to be called before now(), only valid if is_set() is true
   static time_type now() { return *now_; }

   static void set_now( time_type t );

   static bool is_set() { return now_.has_value(); }

   static time_type add( time_type t, duration_type d ) { return source_traits::add( t, d ); }
   static duration_type subtract( time_type t1, time_type t2 ) { return source_traits::subtract( t1, t2 ); }
   static bool less_than( time_type t1, time_type t2 ) { return source_traits::less_than( t1, t2 ); }

   // This function is called by asio to determine how often to check
   // if the timer is ready to fire. By manipulating this function, we
   // can make sure asio detects changes to now_ in a timely fashion.
   static boost::posix_time::time_duration to_posix_duration( duration_type d ) {
      return d < boost::posix_time::milliseconds( 1 ) ? d : boost::posix_time::milliseconds( 1 );
   }

   // return now as fc::time_point, used by fc::time_point::now() if mock_time_traits is_set()
   static fc::time_point fc_now();

private:
   static std::optional<time_type> now_;
};

typedef boost::asio::basic_deadline_timer<boost::posix_time::ptime, mock_time_traits> mock_deadline_timer;

} // namespace fc
