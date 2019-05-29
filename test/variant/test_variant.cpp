#define BOOST_TEST_MODULE variant
#include <boost/test/included/unit_test.hpp>

#include <fc/variant_object.hpp>
#include <fc/exception/exception.hpp>

#include <string>

using namespace fc;

BOOST_AUTO_TEST_SUITE(variant_test_suite)
BOOST_AUTO_TEST_CASE(mutable_variant_object_test)
{
  // no BOOST_CHECK / BOOST_REQUIRE, just see that this compiles on all supported platforms
  try {
    variant v(42);
    variant_object vo;
    mutable_variant_object mvo;
    variants vs;
    vs.push_back(mutable_variant_object("level", "debug")("color", v));
    vs.push_back(mutable_variant_object()("level", "debug")("color", v));
    vs.push_back(mutable_variant_object("level", "debug")("color", "green"));
    vs.push_back(mutable_variant_object()("level", "debug")("color", "green"));
    vs.push_back(mutable_variant_object("level", "debug")(vo));
    vs.push_back(mutable_variant_object()("level", "debug")(mvo));
    vs.push_back(mutable_variant_object("level", "debug").set("color", v));
    vs.push_back(mutable_variant_object()("level", "debug").set("color", v));
    vs.push_back(mutable_variant_object("level", "debug").set("color", "green"));
    vs.push_back(mutable_variant_object()("level", "debug").set("color", "green"));
  }
  FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_SUITE_END()
