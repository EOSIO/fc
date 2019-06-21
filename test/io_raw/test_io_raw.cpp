#define BOOST_TEST_MODULE io_raw
#include <boost/test/included/unit_test.hpp>

#include <fc/variant_object.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/bitutil.hpp>
#include <fc/variant.hpp>
#include <string>

using namespace fc;

bool expect_assert_message(const fc::exception& ex, string expected) {
  BOOST_TEST_MESSAGE("LOG : " << "expected: " << expected << ", actual: " << ex.get_log().at(0).get_message());
  return (ex.get_log().at(0).get_message().find(expected) != std::string::npos);
}

BOOST_AUTO_TEST_SUITE(test_io_raw)

BOOST_AUTO_TEST_CASE(varint_unpack) {
     struct stream {
      char   buffer[4096];
      size_t read_pos = 0;
      size_t write_pos = 0;
      stream() {
         memset(buffer, 0, sizeof(buffer));
      }
      void get(char &b) {
         ++read_pos;
         b = buffer[read_pos - 1];
      }
      void write(char *data, size_t len) {
         memcpy(&(buffer[write_pos]), data, len);
         write_pos += len;
      }
   };

   std::vector<fc::signed_int> slist = {
      0, 1, -1, 63, -63, 64, -64, 127, -127, 128, -128, 255, -255, 256, -256, 32768, -32768, 65535, -65535, 
      65536, -65536, -2147483648, -2147483647, 2147483646, 2147483647};

   std::vector<fc::unsigned_int> ulist = {
      0, 1, 2, 3, 63, 64, 127, 128, 65535, 65536, 2147483646, 2147483647, 2147483648, 4294967294, 4294967295};

   stream s1;
   fc::raw::pack(s1, slist);
   std::vector<fc::signed_int> slist2;
   fc::raw::unpack(s1, slist2);

   BOOST_CHECK_EQUAL(slist.size(), slist2.size());
   BOOST_CHECK_EQUAL(s1.read_pos, s1.write_pos);
   for (int i = 0; i < slist.size(); ++i) {
      BOOST_CHECK_EQUAL(slist[i], slist2[i]);
   }

   stream s2;
   fc::raw::pack(s2, ulist);
   std::vector<fc::unsigned_int> ulist2;
   fc::raw::unpack(s2, ulist2);

   BOOST_CHECK_EQUAL(ulist.size(), ulist2.size());
   BOOST_CHECK_EQUAL(s2.read_pos, s2.write_pos);
   for (int i = 0; i < ulist.size(); ++i) {
      BOOST_CHECK_EQUAL(ulist[i], ulist2[i]);
   }

   auto unpack_uint = [](unsigned char *buf, size_t len, uint32_t expect_value) {
      stream s;
      s.write((char *)buf, len);
      unsigned_int ui;
      fc::raw::unpack(s, ui);
      BOOST_CHECK_EQUAL((uint32_t)ui, expect_value);
      BOOST_CHECK_EQUAL(s.read_pos, len);
   };

   auto unpack_int = [](unsigned char *buf, size_t len, int32_t expect_value) {
      stream s;
      s.write((char *)buf, len);
      signed_int si;
      fc::raw::unpack(s, si);
      BOOST_CHECK_EQUAL((int32_t)si, expect_value);
      BOOST_CHECK_EQUAL(s.read_pos, len);
   };

   unsigned char buf[] = { 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00};
   unpack_uint(buf, 5, UINT_MAX);
   unpack_int(buf, 5, INT_MIN);

   unsigned char buf2[] = { 0xff, 0xff, 0xff, 0xff, 0x8f, 0x00}; // invalid stop bit 
   BOOST_CHECK_EXCEPTION( unpack_uint(buf2, 5, 0) , fc::assert_exception, [](const auto& e) {
      return expect_assert_message(e, "unsigned_int out of bounds");
   });
   BOOST_CHECK_EXCEPTION( unpack_int(buf2, 5, 0) , fc::assert_exception, [](const auto& e) {
      return expect_assert_message(e, "signed_int out of bounds");
   });

   unsigned char buf3[] = { 0xff, 0xff, 0xff, 0xff, 0x10, 0x00}; // data out of bound
   BOOST_CHECK_EXCEPTION( unpack_uint(buf3, 5, 0) , fc::assert_exception, [](const auto& e) {
      return expect_assert_message(e, "unsigned_int out of bounds");
   });
   BOOST_CHECK_EXCEPTION( unpack_int(buf3, 5, 0) , fc::assert_exception, [](const auto& e) {
      return expect_assert_message(e, "signed_int out of bounds");
   });
}

BOOST_AUTO_TEST_SUITE_END()
