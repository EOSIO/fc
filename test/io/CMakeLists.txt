add_executable( test_io test_cfile.cpp)
target_link_libraries( test_io fc )

add_test(NAME test_io COMMAND libraries/fc/test/io/test_io WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
