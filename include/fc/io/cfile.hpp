#pragma once
#include <fc/filesystem.hpp>
#include <cstdio>

namespace fc {

namespace detail {
   using unique_file = std::unique_ptr<FILE, decltype( &fclose )>;
}

class cfile_datastream;

/**
 * Wrapper for c-file access that provides a similar interface as fstream without all the overhead of std streams.
 * std::runtime_error exception thrown for errors.
 */
class cfile {
public:
   cfile()
     : _file(nullptr, &fclose)
   {}

   void set_file_path( fc::path file_path ) {
      _file_path = std::move( file_path );
   }

   fc::path get_file_path() const {
      return _file_path;
   }

   bool is_open() const { return _open; }

   size_t file_size() const {
      return fc::file_size( _file_path );
   }

   /// @param mode is any mode supported by fopen
   ///        Tested with:
   ///         "ab+" - open for binary update - create if does not exist
   ///         "rb+" - open for binary update - file must exist
   void open( const char* mode ) {
      _file.reset( fopen( _file_path.generic_string().c_str(), mode ) );
      if( !_file ) {
         throw std::runtime_error( "cfile unable to open: " +  _file_path.generic_string() + " in mode: " + std::string( mode ) );
      }
      _open = true;
   }

   long tellp() const {
      return ftell( _file.get() );
   }

   void seek( long loc ) {
      if( 0 != fseek( _file.get(), loc, SEEK_SET ) ) {
         throw std::runtime_error( "cfile: " + _file_path.generic_string() +
                                   " unable to SEEK_SET to: " + std::to_string(loc) );
      }
   }

   void seek_end( long loc ) {
      if( 0 != fseek( _file.get(), loc, SEEK_END ) ) {
         throw std::runtime_error( "cfile: " + _file_path.generic_string() +
                                   " unable to SEEK_END to: " + std::to_string(loc) );
      }
   }

   void read( char* d, size_t n ) {
      size_t result = fread( d, 1, n, _file.get() );
      if( result != n ) {
         throw std::runtime_error( "cfile: " + _file_path.generic_string() +
                                   " unable to read " + std::to_string( n ) + " only read " + std::to_string( result ) );
      }
   }

   void write( const char* d, size_t n ) {
      size_t result = fwrite( d, 1, n, _file.get() );
      if( result != n ) {
         throw std::runtime_error( "cfile: " + _file_path.generic_string() +
                                   " unable to write " + std::to_string( n ) + " only wrote " + std::to_string( result ) );
      }
   }

   void flush() {
      if( 0 != fflush( _file.get() ) ) {
         throw std::runtime_error( "cfile: " + _file_path.generic_string() + " unable to flush file." );
      }
   }

   void close() {
      _file.reset();
      _open = false;
   }

   void remove() {
      if( _open ) {
         throw std::runtime_error( "cfile: " + _file_path.generic_string() + " Unable to remove as file is open" );
      }
      fc::remove_all( _file_path );
   }

   cfile_datastream create_datastream();

private:
   bool                  _open = false;
   fc::path              _file_path;
   detail::unique_file   _file;
};

/*
 *  @brief datastream adapter that adapts cfile for use with fc unpack
 *
 *  This class supports unpack functionality but not pack.
 */
class cfile_datastream {
public:
   explicit cfile_datastream( cfile& cf ) : cf(cf) {}

   void skip( size_t s ) {
      std::vector<char> d( s );
      read( &d[0], s );
   }

   bool read( char* d, size_t s ) {
      cf.read( d, s );
      return true;
   }

   bool get( unsigned char& c ) { return get( *(char*)&c ); }

   bool get( char& c ) { return read(&c, 1); }

private:
   cfile& cf;
};

inline cfile_datastream cfile::create_datastream() {
   return cfile_datastream(*this);
}


} // namespace fc