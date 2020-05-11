#pragma once
#include <stdint.h>
#include <fc/string.hpp>
#include <fc/optional.hpp>
//#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/chrono/duration.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/convert.hpp>
#include <boost/convert/stream.hpp>

#ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable : 4244)
#endif //// _MSC_VER

#define TIME_IS_STD_CHRONO

namespace fc {
#ifdef TIME_IS_STD_CHRONO
  namespace chrono = std::chrono;

  using     chrono::microseconds;
  using     chrono::milliseconds;
  //using     chrono::seconds;
  typedef chrono::duration<int32_t, std::ratio<1, 1>> seconds;
  using chrono::hours;
  using chrono::minutes;
  typedef   chrono::duration<int16_t, std::ratio<60 * 60 * 24>>      days;
  typedef   chrono::duration<int16_t, std::ratio<60 * 60 * 24 * 7>>  weeks;
/*/
  typedef   duration<uint64_t, std::ratio<1, 1000000>> microseconds;  // at least 55 bits needed
  typedef   duration<uint64_t, std::ratio<1,    1000>> milliseconds;  // at least 45 bits needed
  typedef   duration<uint64_t, std::ratio<1,       1>> seconds;       // at least 35 bits needed
  typedef   duration<uint32_t, std::ratio<  60> >      minutes;       // at least 29 bits needed
  typedef   duration<uint32_t, std::ratio<3600> >      hours;         // at least 23 bits needed
  typedef   duration<uint16_t, std::ratio<60 * 60 * 24>>      days;
  typedef   duration<uint16_t, std::ratio<60 * 60 * 24 * 7>>  weeks;
*/
  using     chrono::duration_cast;
  using     chrono::time_point_cast;

  //typedef   chrono::system_clock  clock;
  typedef   chrono::time_point<chrono::system_clock, fc::microseconds>  time_point;
  typedef   chrono::time_point<chrono::system_clock, fc::seconds>       time_point_sec;

  template<typename Duration = fc::microseconds>
  auto now() { return fc::time_point_cast<Duration>( chrono::system_clock::now() ); }
#else
  class microseconds {
    public:
        constexpr explicit microseconds( int64_t c = 0) :_count(c){}
        static constexpr microseconds maximum() { return microseconds(0x7fffffffffffffffll); }
        friend constexpr microseconds operator + (const  microseconds& l, const microseconds& r ) { return microseconds(l._count+r._count); }
        friend constexpr microseconds operator - (const  microseconds& l, const microseconds& r ) { return microseconds(l._count-r._count); }

        constexpr bool operator==(const microseconds& c)const { return _count == c._count; }
        constexpr bool operator!=(const microseconds& c)const { return _count != c._count; }
        friend constexpr bool operator>(const microseconds& a, const microseconds& b){ return a._count > b._count; }
        friend constexpr bool operator>=(const microseconds& a, const microseconds& b){ return a._count >= b._count; }
        constexpr friend bool operator<(const microseconds& a, const microseconds& b){ return a._count < b._count; }
        constexpr friend bool operator<=(const microseconds& a, const microseconds& b){ return a._count <= b._count; }
        constexpr microseconds& operator+=(const microseconds& c) { _count += c._count; return *this; }
        constexpr microseconds& operator-=(const microseconds& c) { _count -= c._count; return *this; }
        constexpr int64_t count()const { return _count; }
        constexpr int64_t to_seconds()const { return _count/1000000; }
    private:
        friend class time_point;
        int64_t      _count;
  };

  inline constexpr microseconds seconds( int64_t s ) { return microseconds( s * 1000000 ); }
  inline constexpr microseconds milliseconds( int64_t s ) { return microseconds( s * 1000 ); }
  inline constexpr microseconds minutes(int64_t m) { return seconds(60*m); }
  inline constexpr microseconds hours(int64_t h) { return minutes(60*h); }
  inline constexpr microseconds days(int64_t d) { return hours(24*d); }

  class variant;
  void to_variant( const microseconds&,  fc::variant&  );
  void from_variant( const fc::variant&, microseconds& );

  template<class OStream>
  OStream & operator<<(OStream & os, const microseconds & usec ) {
    return os << usec.count() << " usec";
  }

  typedef  boost::chrono::system_clock  clock;
  typedef  clock::time_point            time_point;
  class time_point {
    public:
        constexpr explicit time_point( microseconds e = microseconds() ) :elapsed(e){}
        static time_point now();
        static constexpr time_point maximum() { return time_point( microseconds::max() ); }
        static constexpr time_point min() { return time_point();                      }

        operator fc::string()const;
        static time_point from_iso_string( const fc::string& s );

        constexpr const microseconds& time_since_epoch()const { return elapsed; }
        constexpr uint32_t            sec_since_epoch()const  { return elapsed.count() / 1000000; }
        constexpr bool   operator > ( const time_point& t )const                              { return elapsed.count > t.elapsed._count; }
        constexpr bool   operator >=( const time_point& t )const                              { return elapsed._count >=t.elapsed._count; }
        constexpr bool   operator < ( const time_point& t )const                              { return elapsed._count < t.elapsed._count; }
        constexpr bool   operator <=( const time_point& t )const                              { return elapsed._count <=t.elapsed._count; }
        constexpr bool   operator ==( const time_point& t )const                              { return elapsed._count ==t.elapsed._count; }
        constexpr bool   operator !=( const time_point& t )const                              { return elapsed._count !=t.elapsed._count; }
        constexpr time_point&  operator += ( const microseconds& m)                           { elapsed+=m; return *this;                 }
        constexpr time_point&  operator -= ( const microseconds& m)                           { elapsed-=m; return *this;                 }
        constexpr time_point   operator + (const microseconds& m) const { return time_point(elapsed+m); }
        constexpr time_point   operator - (const microseconds& m) const { return time_point(elapsed-m); }
        constexpr microseconds operator - (const time_point& m)   const { return microseconds(elapsed.count() - m.elapsed.count()); }
    private:
        microseconds elapsed;
  };

  /**
   *  A lower resolution time_point accurate only to seconds from 1970
   */
  class time_point_sec
  {
    public:
        constexpr time_point_sec()
        :utc_seconds(0){}

        constexpr explicit time_point_sec(uint32_t seconds )
        :utc_seconds(seconds){}

        constexpr time_point_sec( const time_point& t )
        :utc_seconds( t.time_since_epoch().count() / 1000000ll ){}

        static constexpr time_point_sec maximum() { return time_point_sec(0xffffffff); }
        static constexpr time_point_sec min() { return time_point_sec(0); }

        constexpr operator time_point()const { return time_point( fc::seconds( utc_seconds) ); }
        constexpr uint32_t sec_since_epoch()const { return utc_seconds; }

        constexpr time_point_sec operator = ( const fc::time_point& t )
        {
          utc_seconds = t.time_since_epoch().count() / 1000000ll;
          return *this;
        }
        constexpr friend bool      operator <  ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds <  b.utc_seconds; }
        constexpr friend bool      operator >  ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds >  b.utc_seconds; }
        constexpr friend bool      operator <= ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds <= b.utc_seconds; }
        constexpr friend bool      operator >= ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds >= b.utc_seconds; }
        constexpr friend bool      operator == ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds == b.utc_seconds; }
        constexpr friend bool      operator != ( const time_point_sec& a, const time_point_sec& b )  { return a.utc_seconds != b.utc_seconds; }
        constexpr time_point_sec&  operator += ( uint32_t m ) { utc_seconds+=m; return *this; }
        constexpr time_point_sec&  operator += ( microseconds m ) { utc_seconds+=boost::chrono::duration_cast<boost::chrono::seconds>(m); return *this; }
        constexpr time_point_sec&  operator -= ( uint32_t m ) { utc_seconds-=m; return *this; }
        constexpr time_point_sec&  operator -= ( microseconds m ) { utc_seconds-=boost::chrono::duration_cast<boost::chrono::seconds>(m); return *this; }
        constexpr time_point_sec   operator +( uint32_t offset )const { return time_point_sec(utc_seconds + offset); }
        constexpr time_point_sec   operator -( uint32_t offset )const { return time_point_sec(utc_seconds - offset); }

        friend constexpr time_point   operator + ( const time_point_sec& t, const microseconds& m )   { return time_point(t) + m;             }
        friend constexpr time_point   operator - ( const time_point_sec& t, const microseconds& m )   { return time_point(t) - m;             }
        friend constexpr microseconds operator - ( const time_point_sec& t, const time_point_sec& m ) { return time_point(t) - time_point(m); }
        friend constexpr microseconds operator - ( const time_point&     t, const time_point_sec& m ) { return time_point(t) - time_point(m); }

        fc::string to_non_delimited_iso_string()const;
        fc::string to_iso_string()const;

        operator fc::string()const;
        static time_point_sec from_iso_string( const fc::string& s );

    private:
        boost::chrono::seconds  utc_seconds;
  };
#endif
  template <class Clock, class Duration>
  inline boost::posix_time::ptime  to_ptime(const chrono::time_point<Clock, Duration> &  tp) {
    namespace posix = boost::posix_time;
    posix::microseconds  usec( chrono::duration_cast<fc::microseconds>( tp.time_since_epoch() ).count() );
    return  posix::from_time_t(0) + usec;
  }

  template <class Clock, class Duration>
  inline std::string to_iso_string(const chrono::time_point<Clock, Duration> &  tp, bool extend_to_usec = false)
  {
    std::string res = boost::posix_time::to_iso_extended_string( to_ptime(tp) );
    if ( std::is_same<Duration, fc::microseconds>() ) {
      if ( res.length() < 20 )
        return res + (extend_to_usec ? ".000000" :  ".000") ;
      else {
        // 1970-01-01T00:00:00.000000
        // ^    ^    ^    ^    ^    ^
        // 1    6    11   16   21   25
        if ( !extend_to_usec )  res.resize( 23 );
        return res;
      }
    }
    else {
      return res;
    }
  }

  template <class Clock, class Duration>
  inline std::string to_non_delimited_iso_string(const chrono::time_point<Clock, Duration> &  tp)
  {
    std::string res = boost::posix_time::to_iso_string( to_ptime(tp) );
    if ( std::is_same<Duration, fc::microseconds>() ) {
      if ( res.length() < 16 )
        return res + ".000";
      else {
        // 19700101T000000.000
        // ^    ^    ^    ^  ^
        // 1    6    11   16 19
        res.resize( 19 );
        return res;
      }
    }
    else {
      return res;
    }
  }

  template <class Duration>
  inline std::string to_string(const Duration &  time) {
    #define TRY_TO_STRING(TYPE, TOKEN) \
      if (std::is_same<Duration, TYPE>::value) \
        return to_string( (int64_t)time.count() ) + TOKEN;

    TRY_TO_STRING( fc::microseconds, " microsec" );
    TRY_TO_STRING( fc::milliseconds, " millisec" );
    TRY_TO_STRING( fc::seconds,      " sec"      );
    TRY_TO_STRING( fc::minutes,      " min"      );
    TRY_TO_STRING( fc::hours,        " hours"    );
    TRY_TO_STRING( fc::days,         " days"     );
    TRY_TO_STRING( fc::weeks,        " weeks"    );
    return to_string( uint64_t(time.count()) );
  }

  void from_iso_string( const fc::string& s, fc::time_point_sec &  tp   );
  void from_iso_string( const fc::string& s, fc::time_point     &  tp   );
  fc::microseconds from_string_to_usec( const fc::string& str );
  template<class Dur>
  Dur from_string( const fc::string& str ) {
    return fc::duration_cast<Dur>( from_string_to_usec(str) );
  }

  template<class TimePoint>
  TimePoint from_iso_string( const fc::string& s )
  {
    TimePoint  res;
    from_iso_string(s, res);
    return res;
  }

  class variant;
  void from_variant( const fc::variant& v, fc::time_point& tp );
  void from_variant( const fc::variant& v, fc::time_point_sec& tp );
  void to_variant( const fc::time_point& t, variant& v );
  void to_variant( const fc::time_point_sec& t, variant& v );

  /** return a human-readable approximate time, relative to now()
   * e.g., "4 hours ago", "2 months ago", etc.
   */
  string get_approximate_relative_time_string(const time_point_sec& event_time,
                                              const time_point_sec& relative_to_time = fc::now<fc::seconds>(),
                                              const std::string& ago = " ago");
  string get_approximate_relative_time_string(const time_point& event_time,
                                              const time_point& relative_to_time = fc::now<fc::microseconds>(),
                                              const std::string& ago = " ago");
}

namespace std {
  template <class CharT, class Traits, class Rep, class Period>
  std::basic_ostream<CharT, Traits> &
  operator<<( std::basic_ostream<CharT, Traits>&        os,
              const fc::chrono::duration<Rep, Period>&  d ) {
    return os << fc::to_string(d);
  }

  template<class CharT, class Traits, class Clock, class Duration>
  std::basic_ostream<CharT, Traits> &
  operator<<( std::basic_ostream<CharT, Traits>&       os,
              const fc::chrono::time_point<Clock, Duration> &  tp ) {
    return os << fc::to_iso_string(tp, true);
  }

  template<class Clock, class Duration>
  struct less<fc::chrono::time_point<Clock, Duration>> {
    public:
      typedef fc::chrono::time_point<Clock, Duration> TP;
      constexpr bool operator()( const TP &  left, const TP &  right) const {
        return left.time_since_epoch().count() < right.time_since_epoch().count();
      }
  };

}

#include <fc/reflect/reflect.hpp>
FC_REFLECT_TYPENAME( fc::microseconds )
FC_REFLECT_TYPENAME( fc::time_point )
FC_REFLECT_TYPENAME( fc::time_point_sec )

#ifdef _MSC_VER
  #pragma warning (pop)
#endif /// #ifdef _MSC_VER
