/**
 *  @file
 *  @copyright defined in eosio.cdt/LICENSE.txt
 */
#define BOOST_TEST_MODULE fc_time
#include <boost/test/included/unit_test.hpp>

#include <fc/time.hpp>
#include <fc/exception/exception.hpp>

using namespace fc;
using namespace std::literals;
using std::numeric_limits;

using fc::days;
using fc::hours;
using fc::microseconds;
using fc::milliseconds;
using fc::minutes;
using fc::seconds;
using fc::time_point;
using fc::time_point_sec;

static constexpr int64_t i64min = numeric_limits<int64_t>::min(); // -9223372036854775808
static constexpr int64_t i64max = numeric_limits<int64_t>::max(); //  9223372036854775807

static constexpr uint32_t u32min = numeric_limits<uint32_t>::min(); // 0
static constexpr uint32_t u32max = numeric_limits<uint32_t>::max(); // 4294967295

BOOST_AUTO_TEST_SUITE(fc_time)

// Definitions in `eosio.cdt/libraries/eosio/time.hpp`
BOOST_AUTO_TEST_CASE(microseconds_type_test) try {
   //// explicit microseconds(uint64_t)/int64_t count()
   BOOST_CHECK_EQUAL( microseconds{}.count(), 0ULL );
   BOOST_CHECK_EQUAL( microseconds{i64max}.count(), i64max );
   BOOST_CHECK_EQUAL( microseconds{i64min}.count(), i64min );

   // -----------------------------
   // static microseconds maximum()
   BOOST_CHECK_EQUAL( microseconds::maximum().count(), microseconds{0x7FFFFFFFFFFFFFFFLL}.count() );

   // ------------------------------------------------------------------------
   // friend microseconds operator+(const  microseconds&, const microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{0LL} + microseconds{ 1LL}), microseconds{ 1LL} );
   BOOST_CHECK_EQUAL( (microseconds{1LL} + microseconds{-1LL}), microseconds{ 0LL} );

   // ------------------------------------------------------------------------
   // friend microseconds operator-(const  microseconds&, const microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{0LL} - microseconds{ 1LL}), microseconds{-1LL} );
   BOOST_CHECK_EQUAL( (microseconds{1LL} - microseconds{-1LL}), microseconds{ 2LL} );

   // ----------------------------------------------
   // microseconds& operator+=(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{0LL} += microseconds{ 1LL}), microseconds{ 1LL} );
   BOOST_CHECK_EQUAL( (microseconds{1LL} += microseconds{-1LL}), microseconds{ 0LL} );

   // ----------------------------------------------
   // microseconds& operator-=(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{0LL} -= microseconds{ 1LL}), microseconds{-1LL} );
   BOOST_CHECK_EQUAL( (microseconds{1LL} -= microseconds{-1LL}), microseconds{ 2LL} );

   // -------------------------------------
   // bool operator==(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{1LL} == microseconds{1LL}), true  );
   BOOST_CHECK_EQUAL( (microseconds{0LL} == microseconds{1LL}), false );

   // -------------------------------------
   // bool operator!=(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{1LL} != microseconds{1LL}), false );
   BOOST_CHECK_EQUAL( (microseconds{0LL} != microseconds{1LL}), true  );

   // -------------------------------------
   // bool operator<(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{0LL} <  microseconds{1LL}), true  );
   BOOST_CHECK_EQUAL( (microseconds{1LL} <  microseconds{1LL}), false );

   // -------------------------------------
   // bool operator<=(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{1LL} <= microseconds{1LL}), true  );
   BOOST_CHECK_EQUAL( (microseconds{2LL} <= microseconds{1LL}), false );

   // -------------------------------------
   // bool operator>(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{1LL} >  microseconds{0LL}), true  );
   BOOST_CHECK_EQUAL( (microseconds{1LL} >  microseconds{1LL}), false );

   // -------------------------------------
   // bool operator>=(const  microseconds&)
   BOOST_CHECK_EQUAL( (microseconds{1LL} >= microseconds{1LL}), true  );
   BOOST_CHECK_EQUAL( (microseconds{1LL} >= microseconds{2LL}), false );

   // --------------------
   // int64_t to_seconds()
   BOOST_CHECK_EQUAL( (microseconds{i64max}.to_seconds()), i64max / 1000000 );

   // -----------------------------------------
   // inline microseconds milliseconds(int64_t)
   BOOST_CHECK_EQUAL( milliseconds(0LL),  microseconds{0LL} );
   BOOST_CHECK_EQUAL( milliseconds(1LL),  microseconds{1000LL} );
   BOOST_CHECK_EQUAL( milliseconds(60LL), microseconds{60LL*1000LL} );

   // ------------------------------------
   // inline microseconds seconds(int64_t)
   BOOST_CHECK_EQUAL( seconds(0LL),  microseconds{0LL} );
   BOOST_CHECK_EQUAL( seconds(1LL),  microseconds{1000000LL} );
   BOOST_CHECK_EQUAL( seconds(60LL), microseconds{60LL*1000000LL} );

   // ------------------------------------
   // inline microseconds minutes(int64_t)
   BOOST_CHECK_EQUAL( minutes(0LL),  microseconds{0LL} );
   BOOST_CHECK_EQUAL( minutes(1LL),  microseconds{60LL*1000000LL} );
   BOOST_CHECK_EQUAL( minutes(60LL), microseconds{60LL*60LL*1000000LL} );

   // ----------------------------------
   // inline microseconds hours(int64_t)
   BOOST_CHECK_EQUAL( hours(0LL),  microseconds{0LL} );
   BOOST_CHECK_EQUAL( hours(1LL),  microseconds{60LL*60LL*1000000LL} );
   BOOST_CHECK_EQUAL( hours(60LL), microseconds{60LL*60LL*60LL*1000000LL} );

   // ---------------------------------
   // inline microseconds days(int64_t)
   BOOST_CHECK_EQUAL( days(0LL),  microseconds{0LL} );
   BOOST_CHECK_EQUAL( days(1LL),  microseconds{24LL*60LL*60LL*1000000LL} );
   BOOST_CHECK_EQUAL( days(60LL), microseconds{24LL*60LL*60LL*60LL*1000000LL} );
} FC_LOG_AND_RETHROW();

// Definitions in `eosio.cdt/libraries/eosio/time.hpp`
BOOST_AUTO_TEST_CASE(time_point_type_test) try {
   static const microseconds ms0 { 0LL};
   static const microseconds ms1 { 1LL};
   static const microseconds msn1{-1LL};
   static const microseconds ms_min{i64min};
   static const microseconds ms_max{i64max};

   //// explicit time_point(microseconds)
   // microseconds& time_since_epoch()
   BOOST_CHECK_EQUAL( time_point{ms0}.time_since_epoch(), ms0 );
   BOOST_CHECK_EQUAL( time_point{ms1}.time_since_epoch(), ms1 );
   BOOST_CHECK_EQUAL( time_point{ms_min}.time_since_epoch(), ms_min );
   BOOST_CHECK_EQUAL( time_point{ms_max}.time_since_epoch(), ms_max );

   // --------------------------
   // uint32_t sec_since_epoch()
   BOOST_CHECK_EQUAL( time_point{ms0}.sec_since_epoch(), 0 );
   BOOST_CHECK_EQUAL( time_point{ms1}.sec_since_epoch(), 0 );
   BOOST_CHECK_EQUAL( time_point{microseconds{1000000}}.sec_since_epoch(),   1 );
   BOOST_CHECK_EQUAL( time_point{microseconds{10000000}}.sec_since_epoch(), 10 );

   // -----------------------------------------
   // time_point operator+(const microseconds&)
   BOOST_CHECK_EQUAL( (time_point{ms0}  + ms1), time_point{ms1} );
   BOOST_CHECK_EQUAL( (time_point{msn1} + ms1), time_point{ms0} );

   // -----------------------------------------
   // time_point operator-(const microseconds&)
   BOOST_CHECK_EQUAL( (time_point{ms0} - ms1), time_point{msn1} );
   BOOST_CHECK_EQUAL( (time_point{ms0} - msn1), time_point{ms1} );

   // -------------------------------------------
   // time_point& operator+=(const microseconds&)
   BOOST_CHECK_EQUAL( (time_point{ms0} += ms1), time_point{ms1}  );
   BOOST_CHECK_EQUAL( (time_point{msn1} += ms1), time_point{ms0} );

   // -------------------------------------------
   // time_point& operator-=(const microseconds&)
   BOOST_CHECK_EQUAL( (time_point{ms0} -= ms1), time_point{msn1} );
   BOOST_CHECK_EQUAL( (time_point{ms0} -= msn1), time_point{ms1} );

   // ----------------------------------
   // bool operator==(const time_point&)
   BOOST_CHECK_EQUAL( (time_point{ms0} == time_point{ms0}), true  );
   BOOST_CHECK_EQUAL( (time_point{ms0} == time_point{ms1}), false );

   // ----------------------------------
   // bool operator!=(const time_point&)
   BOOST_CHECK_EQUAL( (time_point{ms0} != time_point{ms0}), false );
   BOOST_CHECK_EQUAL( (time_point{ms0} != time_point{ms1}), true  );

   // ---------------------------------
   // bool operator<(const time_point&)
   BOOST_CHECK_EQUAL( (time_point{ms0} <  time_point{ms1}), true  );
   BOOST_CHECK_EQUAL( (time_point{ms1} <  time_point{ms1}), false );

   // ----------------------------------
   // bool operator<=(const time_point&)
   BOOST_CHECK_EQUAL( (time_point{ms1} <= time_point{ms1}), true  );
   BOOST_CHECK_EQUAL( (time_point{ms1} <= time_point{ms0}), false );

   // ---------------------------------
   // bool operator>(const time_point&)
   BOOST_CHECK_EQUAL( (time_point{ms1} >  time_point{ms0}), true  );
   BOOST_CHECK_EQUAL( (time_point{ms1} >  time_point{ms1}), false );

   // ----------------------------------
   // bool operator>=(const time_point&)
   BOOST_CHECK_EQUAL( (time_point{ms1} >= time_point{ms1}), true  );
   BOOST_CHECK_EQUAL( (time_point{ms0} >= time_point{ms1}), false );
} FC_LOG_AND_RETHROW();

// Definitions in `eosio.cdt/libraries/eosio/time.hpp`
BOOST_AUTO_TEST_CASE(time_point_sec_type_test) try {
   static const microseconds ms0 { 0LL};
   static const microseconds ms1 { 1LL};
   static const microseconds msn1{-1LL};
   static const microseconds ms_min{i64min};
   static const microseconds ms_max{i64max};

   static const time_point tp0{ms0};
   static const time_point tp1{ms1};
   static const time_point tpn1{msn1};
   static const time_point tp_min{ms_min};
   static const time_point tp_max{ms_max};

   //// time_point_sec()
   //uint32_t sec_since_epoch()const
   BOOST_CHECK_EQUAL( time_point_sec{}.sec_since_epoch(), 0 );

   //// explicit time_point_sec(uint32_t)
   BOOST_CHECK_EQUAL( time_point_sec{u32min}.sec_since_epoch(), 0 );
   BOOST_CHECK_EQUAL( time_point_sec{u32max}.sec_since_epoch(), u32max );
   BOOST_CHECK_EQUAL( time_point_sec{u32max + 1}.sec_since_epoch(), 0 );

   //// time_point_sec(const time_point&)
   BOOST_CHECK_EQUAL( time_point_sec{tp0}.sec_since_epoch(), ms0.count() / 1000000 );
   BOOST_CHECK_EQUAL( time_point_sec{tp1}.sec_since_epoch(), ms1.count() / 1000000 );
   BOOST_CHECK_EQUAL( time_point_sec{tpn1}.sec_since_epoch(), msn1.count() / 1000000 );

   // ---------------------------
   // static time_point_sec min()
   BOOST_CHECK_EQUAL( time_point_sec{}.min().sec_since_epoch() == 0, true );
   BOOST_CHECK_EQUAL( time_point_sec{}.min().sec_since_epoch() != 1, true );

   // -------------------------------
   // static time_point_sec maximum()
   BOOST_CHECK_EQUAL( time_point_sec{}.maximum().sec_since_epoch() == 0xFFFFFFFF, true );
   BOOST_CHECK_EQUAL( time_point_sec{}.maximum().sec_since_epoch() != 1, true );

   // --------------------------
   // operator time_point()const
   BOOST_CHECK_EQUAL( time_point_sec{u32min}.operator time_point(), time_point{microseconds{static_cast<int64_t>(u32min)*1000000}} );
   BOOST_CHECK_EQUAL( time_point_sec{u32max}.operator time_point(), time_point{microseconds{static_cast<int64_t>(u32max)*1000000}} );

   // -------------------------------------------
   // time_point_sec operator=(const time_point&)
   BOOST_CHECK_EQUAL( (time_point_sec{} = tp0), time_point_sec{} );
   BOOST_CHECK_EQUAL( (time_point_sec{} = tp1), time_point_sec{} );
   BOOST_CHECK_EQUAL( (time_point_sec{} = tp_max), time_point_sec{tp_max} );

   // ---------------------------------------
   // time_point_sec operator+(uint32_t)const
   BOOST_CHECK_EQUAL( (time_point_sec{} + u32min), time_point_sec{u32min} );
   BOOST_CHECK_EQUAL( (time_point_sec{} + u32max), time_point_sec{u32max} );

   // -----------------------------------------------------------------------
   // friend time_point operator+(const time_point_sec&, const microseconds&)
   BOOST_CHECK_EQUAL( (time_point_sec{0} + microseconds{ 1000000LL}), time_point{microseconds{ 1000000LL}} );
   BOOST_CHECK_EQUAL( (time_point_sec{0} + microseconds{-1000000LL}), time_point{microseconds{-1000000LL}} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} + microseconds{ 1000000LL}), time_point{microseconds{ 2000000LL}} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} + microseconds{-1000000LL}), time_point{microseconds{       0LL}} );

   // -----------------------------------------------------------------------
   // friend time_point operator-(const time_point_sec&, const microseconds&)
   BOOST_CHECK_EQUAL( (time_point_sec{0} - microseconds{ 1000000LL}), time_point{microseconds{-1000000LL}} );
   BOOST_CHECK_EQUAL( (time_point_sec{0} - microseconds{-1000000LL}), time_point{microseconds{ 1000000LL}} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} - microseconds{ 1000000LL}), time_point{microseconds{       0LL}} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} - microseconds{-1000000LL}), time_point{microseconds{ 2000000LL}} );

   // ---------------------------------------------------------------------------
   // friend microseconds operator-(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{0} - time_point_sec{0}), microseconds{       0LL} );
   BOOST_CHECK_EQUAL( (time_point_sec{0} - time_point_sec{1}), microseconds{-1000000LL} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} - time_point_sec{0}), microseconds{ 1000000LL} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} - time_point_sec{1}), microseconds{       0LL} );

   // -----------------------------------------------------------------------
   // friend microseconds operator-(const time_point&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point{microseconds{0}} - time_point_sec{0}), microseconds{       0LL} );
   BOOST_CHECK_EQUAL( (time_point{microseconds{0}} - time_point_sec{1}), microseconds{-1000000LL} );
   BOOST_CHECK_EQUAL( (time_point{microseconds{1}} - time_point_sec{0}), microseconds{       1LL} );
   BOOST_CHECK_EQUAL( (time_point{microseconds{1}} - time_point_sec{1}), microseconds{ -999999LL} );

   // ------------------------------------
   // time_point_sec& operator+=(uint32_t)
   BOOST_CHECK_EQUAL( (time_point_sec{0} += u32min), time_point_sec{u32min} );
   BOOST_CHECK_EQUAL( (time_point_sec{0} += u32max), time_point_sec{u32max} );

   // ----------------------------------------
   // time_point_sec& operator+=(microseconds)
   BOOST_CHECK_EQUAL( (time_point_sec{0} += microseconds{      1LL}), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{0} += microseconds{1000000LL}), time_point_sec{1} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} += microseconds{      1LL}), time_point_sec{1} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} += microseconds{1000000LL}), time_point_sec{2} );

   // ------------------------------------
   // time_point_sec& operator-=(uint32_t)
   BOOST_CHECK_EQUAL( (time_point_sec{u32min} -= u32min), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{u32max} -= u32max), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{u32max} -= u32min), time_point_sec{u32max} );

   // ----------------------------------------
   // time_point_sec& operator-=(microseconds)
   BOOST_CHECK_EQUAL( (time_point_sec{0} -= microseconds{      1LL}), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{1} -= microseconds{1000000LL}), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{2} -= microseconds{1000000LL}), time_point_sec{1} );
   BOOST_CHECK_EQUAL( (time_point_sec{3} -= microseconds{1000000LL}), time_point_sec{2} );

   // ---------------------------------------
   // time_point_sec operator+(uint32_t)const
   BOOST_CHECK_EQUAL( (time_point_sec{0} + u32min), time_point_sec{u32min} );
   BOOST_CHECK_EQUAL( (time_point_sec{0} + u32max), time_point_sec{u32max} );

   // ---------------------------------------
   // time_point_sec operator-(uint32_t)const
   BOOST_CHECK_EQUAL( (time_point_sec{u32min} - u32min), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{u32max} - u32max), time_point_sec{0} );
   BOOST_CHECK_EQUAL( (time_point_sec{u32max} - u32min), time_point_sec{u32max} );

   // --------------------------------------------------------------------
   // friend bool operator==(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{1} == time_point_sec{1}), true  );
   BOOST_CHECK_EQUAL( (time_point_sec{0} == time_point_sec{1}), false );

   // --------------------------------------------------------------------
   // friend bool operator!=(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{1} != time_point_sec{1}), false );
   BOOST_CHECK_EQUAL( (time_point_sec{0} != time_point_sec{1}), true  );

   // -------------------------------------------------------------------
   // friend bool operator<(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{0} <  time_point_sec{1}), true  );
   BOOST_CHECK_EQUAL( (time_point_sec{1} <  time_point_sec{1}), false );

   // --------------------------------------------------------------------
   // friend bool operator<=(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{1} <= time_point_sec{1}), true  );
   BOOST_CHECK_EQUAL( (time_point_sec{2} <= time_point_sec{1}), false );

   // -------------------------------------------------------------------
   // friend bool operator>(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{1} >  time_point_sec{0}), true  );
   BOOST_CHECK_EQUAL( (time_point_sec{1} >  time_point_sec{1}), false );

   // --------------------------------------------------------------------
   // friend bool operator>=(const time_point_sec&, const time_point_sec&)
   BOOST_CHECK_EQUAL( (time_point_sec{1} >= time_point_sec{1}), true  );
   BOOST_CHECK_EQUAL( (time_point_sec{1} >= time_point_sec{2}), false );
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(time_point_sec_test) try {
    time_point_sec tp0(0);
    BOOST_CHECK_EQUAL( "1970-01-01T00:00:00", tp0.to_iso_string() );
    BOOST_CHECK_EQUAL( "19700101T000000", tp0.to_non_delimited_iso_string() );

    time_point_sec tp1(1);
    BOOST_CHECK_EQUAL( "1970-01-01T00:00:01", tp1.to_iso_string() );
    BOOST_CHECK_EQUAL( "19700101T000001", tp1.to_non_delimited_iso_string() );

    time_point_sec tp256(0x100);
    BOOST_CHECK_EQUAL( "1970-01-01T00:04:16", tp256.to_iso_string() );
    BOOST_CHECK_EQUAL( "19700101T000416", tp256.to_non_delimited_iso_string() );

    time_point_sec tp64k(0x10000);
    BOOST_CHECK_EQUAL( "1970-01-01T18:12:16", tp64k.to_iso_string() );
    BOOST_CHECK_EQUAL( "19700101T181216", tp64k.to_non_delimited_iso_string() );

    time_point_sec tp16m(0x1000000);
    BOOST_CHECK_EQUAL( "1970-07-14T04:20:16", tp16m.to_iso_string() );
    BOOST_CHECK_EQUAL( "19700714T042016", tp16m.to_non_delimited_iso_string() );

    time_point_sec tp2gm1(0x7fffffffU);
    BOOST_CHECK_EQUAL( "2038-01-19T03:14:07", tp2gm1.to_iso_string() );
    BOOST_CHECK_EQUAL( "20380119T031407", tp2gm1.to_non_delimited_iso_string() );

    time_point_sec tp2g(0x80000000U);
    BOOST_CHECK_EQUAL( "2038-01-19T03:14:08", tp2g.to_iso_string() );
    BOOST_CHECK_EQUAL( "20380119T031408", tp2g.to_non_delimited_iso_string() );

    time_point_sec tp3g(0xc0000000U);
    if (BOOST_VERSION >= BOOST_VERSION_NUMBER(1,64,0)) {
        BOOST_CHECK_EQUAL( "2072-01-28T16:51:12", tp3g.to_iso_string() );
        BOOST_CHECK_EQUAL( "20720128T165112", tp3g.to_non_delimited_iso_string() );
    }

    BOOST_CHECK( tp0 == time_point_sec() );
    BOOST_CHECK( tp0 < tp1 );
    BOOST_CHECK( tp0 < tp256 );
    BOOST_CHECK( tp0 < tp64k );
    BOOST_CHECK( tp0 < tp16m );
    BOOST_CHECK( tp0 < tp2gm1 );
    BOOST_CHECK( tp0 < tp2g );
    BOOST_CHECK( tp0 < tp3g );
    BOOST_CHECK( tp1 > tp0 );
    BOOST_CHECK( tp1 < tp256 );
    BOOST_CHECK( tp1 < tp64k );
    BOOST_CHECK( tp1 < tp16m );
    BOOST_CHECK( tp1 < tp2gm1 );
    BOOST_CHECK( tp1 < tp2g );
    BOOST_CHECK( tp1 < tp3g );
    BOOST_CHECK( tp256 > tp0 );
    BOOST_CHECK( tp256 > tp1 );
    BOOST_CHECK( tp256 < tp64k );
    BOOST_CHECK( tp256 < tp16m );
    BOOST_CHECK( tp256 < tp2gm1 );
    BOOST_CHECK( tp256 < tp2g );
    BOOST_CHECK( tp256 < tp3g );
    BOOST_CHECK( tp64k > tp0 );
    BOOST_CHECK( tp64k > tp1 );
    BOOST_CHECK( tp64k > tp256 );
    BOOST_CHECK( tp64k < tp16m );
    BOOST_CHECK( tp64k < tp2gm1 );
    BOOST_CHECK( tp64k < tp2g );
    BOOST_CHECK( tp64k < tp3g );
    BOOST_CHECK( tp16m > tp0 );
    BOOST_CHECK( tp16m > tp1 );
    BOOST_CHECK( tp16m > tp256 );
    BOOST_CHECK( tp16m > tp64k );
    BOOST_CHECK( tp16m < tp2gm1 );
    BOOST_CHECK( tp16m < tp2g );
    BOOST_CHECK( tp16m < tp3g );
    BOOST_CHECK( tp2gm1 > tp0 );
    BOOST_CHECK( tp2gm1 > tp1 );
    BOOST_CHECK( tp2gm1 > tp256 );
    BOOST_CHECK( tp2gm1 > tp64k );
    BOOST_CHECK( tp2gm1 > tp16m );
    BOOST_CHECK( tp2gm1 < tp2g );
    BOOST_CHECK( tp2gm1 < tp3g );
    BOOST_CHECK( tp2g > tp0 );
    BOOST_CHECK( tp2g > tp1 );
    BOOST_CHECK( tp2g > tp256 );
    BOOST_CHECK( tp2g > tp64k );
    BOOST_CHECK( tp2g > tp16m );
    BOOST_CHECK( tp2g > tp2gm1 );
    BOOST_CHECK( tp2g < tp3g );
    BOOST_CHECK( tp3g > tp0 );
    BOOST_CHECK( tp3g > tp1 );
    BOOST_CHECK( tp3g > tp256 );
    BOOST_CHECK( tp3g > tp64k );
    BOOST_CHECK( tp3g > tp2gm1 );
    BOOST_CHECK( tp3g > tp2g );
    BOOST_CHECK( tp3g > tp16m );
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_SUITE_END()

/*
int main(int argc, char* argv[]) {
   bool verbose = false;
   if( argc >= 2 && std::strcmp( argv[1], "-v" ) == 0 ) {
      verbose = true;
   }
   silence_output(!verbose);

   EOSIO_TEST(microseconds_type_test);
   EOSIO_TEST(time_point_type_test);
   EOSIO_TEST(time_point_sec_type_test);
   EOSIO_TEST(block_timestamp_type_test);
   return has_failed();
}*/