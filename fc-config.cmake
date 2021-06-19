include(CMakeFindDependencyMacro)
find_dependency( Boost COMPONENTS
   date_time
   filesystem
   chrono
   iostreams)
find_dependency( OpenSSL )
find_dependency( ZLIB )
find_dependency( secp256k1 )
include ("${CMAKE_CURRENT_LIST_DIR}/fc-targets.cmake")
