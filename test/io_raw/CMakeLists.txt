add_executable( test_io_raw test_io_raw.cpp )
target_link_libraries( test_io_raw fc )

add_test(NAME test_io_raw COMMAND libraries/fc/test/io_raw/test_io_raw WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
