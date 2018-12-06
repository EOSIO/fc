add_executable( test_cypher_suites test_cypher_suites.cpp )

find_package(Boost 1.67 REQUIRED COMPONENTS unit_test_framework)

target_link_libraries( test_cypher_suites fc Boost::boost ${Boost_LIBRARIES})
if(NOT Boost_USE_STATIC_LIBS)
  target_compile_definitions(test_cypher_suites PRIVATE "BOOST_TEST_DYN_LINK=1")
endif()

add_test(NAME test_cypher_suites COMMAND libraries/fc/test/crypto/test_cypher_suites WORKING_DIRECTORY ${CMAKE_BINARY_DIR})