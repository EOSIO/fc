#define BOOST_TEST_MODULE cypher_suites
#include <boost/test/included/unit_test.hpp>

#include <fc/crypto/public_key.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/crypto/signature.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/elliptic_r1.hpp>
#include <fc/utility.hpp>

#include <algorithm>

using namespace fc::crypto;
using namespace fc;

BOOST_AUTO_TEST_SUITE(cypher_suites)
BOOST_AUTO_TEST_CASE(test_k1) try {
   auto private_key_string = std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3");
   auto expected_public_key = std::string("EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV");
   auto test_private_key = private_key(private_key_string);
   auto test_public_key = test_private_key.get_public_key();

   BOOST_CHECK_EQUAL(private_key_string, test_private_key.to_string());
   BOOST_CHECK_EQUAL(expected_public_key, test_public_key.to_string());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_r1) try {
   auto private_key_string = std::string("PVT_R1_iyQmnyPEGvFd8uffnk152WC2WryBjgTrg22fXQryuGL9mU6qW");
   auto expected_public_key = std::string("PUB_R1_6EPHFSKVYHBjQgxVGQPrwCxTg7BbZ69H9i4gztN9deKTEXYne4");
   auto test_private_key = private_key(private_key_string);
   auto test_public_key = test_private_key.get_public_key();

   BOOST_CHECK_EQUAL(private_key_string, test_private_key.to_string());
   BOOST_CHECK_EQUAL(expected_public_key, test_public_key.to_string());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_k1_recovery) try {
   auto payload = "Test Cases";
   auto digest = sha256::hash(payload, const_strlen(payload));
   auto key = private_key::generate<ecc::private_key_shim>();
   auto pub = key.get_public_key();
   auto sig = key.sign(digest);

   auto recovered_pub = public_key(sig, digest);
   std::cout << recovered_pub << std::endl;

   BOOST_CHECK_EQUAL(recovered_pub.to_string(), pub.to_string());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_r1_recovery) try {
   auto payload = "Test Cases";
   auto digest = sha256::hash(payload, const_strlen(payload));
   auto key = private_key::generate<r1::private_key_shim>();
   auto pub = key.get_public_key();
   auto sig = key.sign(digest);

   auto recovered_pub = public_key(sig, digest);
   std::cout << recovered_pub << std::endl;

   BOOST_CHECK_EQUAL(recovered_pub.to_string(), pub.to_string());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_k1_recyle) try {
   auto key = private_key::generate<ecc::private_key_shim>();
   auto pub = key.get_public_key();
   auto pub_str = pub.to_string();
   auto recycled_pub = public_key(pub_str);

   std::cout << pub << " -> " << recycled_pub << std::endl;

   BOOST_CHECK_EQUAL(pub.to_string(), recycled_pub.to_string());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_r1_recyle) try {
   auto key = private_key::generate<r1::private_key_shim>();
   auto pub = key.get_public_key();
   auto pub_str = pub.to_string();
   auto recycled_pub = public_key(pub_str);

   std::cout << pub << " -> " << recycled_pub << std::endl;

   BOOST_CHECK_EQUAL(pub.to_string(), recycled_pub.to_string());
} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_k1_serialize) try {
   ecc::public_key_point_data pub_point;
   auto pub_point_hex = std::string("04ad90e5b6bc86b3ec7fac2c5fbda7423fc8ef0d58df594c773fa05e2c281b2bfe877677c668bd13603944e34f4818ee03cadd81a88542b8b4d5431264180e2c28");
   from_hex(pub_point_hex, const_cast<char*>(&pub_point.data[0]), pub_point.size());

   ecc::public_key_data pub;
   auto pub_hex = std::string("02ad90e5b6bc86b3ec7fac2c5fbda7423fc8ef0d58df594c773fa05e2c281b2bfe");
   from_hex(pub_hex, const_cast<char*>(&pub.data[0]), pub.size());
   auto uncompressed = ecc::public_key(pub).serialize_ecc_point();

   BOOST_CHECK(std::equal(const_cast<const char*>(pub_point.begin()), const_cast<const char*>(pub_point.end()), uncompressed.begin()));

} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE(test_r1_serialize) try {
   r1::public_key_point_data pub_point;
   auto pub_point_hex = std::string("04413029cb9a5a4a0b087a9b8a060116d0d32bb22d14aebf7778215744811bb6ce40780d7bb9e2e068879f443e05b21b8fc0b62c9c811008064d988856077e35e7");
   from_hex(pub_point_hex, const_cast<char*>(&pub_point.data[0]), pub_point.size());

   r1::public_key_data pub;
   auto pub_hex = std::string("03413029cb9a5a4a0b087a9b8a060116d0d32bb22d14aebf7778215744811bb6ce");
   from_hex(pub_hex, const_cast<char*>(&pub.data[0]), pub.size());
   auto uncompressed = r1::public_key(pub).serialize_ecc_point();

   BOOST_CHECK(std::equal(const_cast<const char*>(pub_point.begin()), const_cast<const char*>(pub_point.end()), uncompressed.begin()));

} FC_LOG_AND_RETHROW();


BOOST_AUTO_TEST_SUITE_END()
