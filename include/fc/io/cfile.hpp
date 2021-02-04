#pragma once
#include <fc/filesystem.hpp>
#include <fc/io/datastream.hpp>
#include <cstdio>
#include <ios>

#ifndef _WIN32
#define FC_FOPEN(p, m) fopen(p, m)
#else
#define FC_CAT(s1, s2) s1 ## s2
#define FC_PREL(s) FC_CAT(L, s)
#define FC_FOPEN(p, m) _wfopen(p, FC_PREL(m))
#endif

namespace fc {

class cfile_datastream;

/**
 * Wrapper for c-file access that provides a similar interface as fstream without all the overhead of std streams.
 * std::ios_base::failure exception thrown for errors.
 */
class cfile {
private:
   cfile(const cfile &) = delete;
   cfile(const cfile &&) = delete;
   const cfile& operator=(const cfile &) = delete;
   const cfile&& operator=(const cfile &&) = delete;
public:
   cfile():
      _open(false),
      _file_path(""),
      _file_ptr(nullptr)
   {}

   ~cfile() {
      close();
   }

   void set_file_path( const fc::path& file_path ) {
      _file_path = file_path;
   }

   const fc::path& get_file_path() const {
      return _file_path;
   }

   bool is_open() const { return _open; }

   static constexpr const char* create_or_update_rw_mode = "ab+";
   static constexpr const char* update_rw_mode = "rb+";
   static constexpr const char* truncate_rw_mode = "wb+";

   /// @param mode is any mode supported by fopen
   ///        Tested with:
   ///         "ab+" - open for binary update - create if does not exist
   ///         "rb+" - open for binary update - file must exist
   void open( const char* mode ) {
      close();

      _file_ptr = FC_FOPEN( _file_path.generic_string().c_str(), mode );
      if( !_file_ptr ) {
         throw std::ios_base::failure( "cfile unable to open: " +  _file_path.generic_string() + " in mode: " + std::string( mode ) );
      }
      _open = true;
   }

   size_t tellp() const {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      long result = ftell( _file_ptr );
      if (result == -1)
         throw std::ios_base::failure("cfile: " + get_file_path().generic_string() +
                                      " unable to get the current position of the file, error: " + std::to_string( errno ));
      return static_cast<size_t>(result);
   }

   void seek( long loc ) {
      if( 0 != fseek( _file_ptr, loc, SEEK_SET ) ) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to SEEK_SET to: " + std::to_string(loc) );
      }
   }

   void seek_end( long loc ) {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      if( 0 != fseek( _file_ptr, loc, SEEK_END ) ) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                       " unable to SEEK_END to: " + std::to_string(loc) );
      }
   }

   void skip( long loc) {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      if( 0 != fseek( _file_ptr, loc, SEEK_CUR ) ) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                       " unable to SEEK_CUR to: " + std::to_string(loc) );
      }
   }

   void read( char* d, size_t n ) {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      size_t result = fread( d, 1, n, _file_ptr );

      if( result != n ) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to read " + std::to_string( n ) + " bytes; only read " + std::to_string( result ) );
      }
   }

   void write( const char* d, size_t n ) {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      size_t result = fwrite( d, 1, n, _file_ptr );

      if( result != n ) {
          throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to write " + std::to_string( n ) + " bytes; only wrote " + std::to_string( result ) );
      }
   }

   void flush() {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      if( 0 != fflush( _file_ptr ) ) {
         int ec = ferror( _file_ptr );
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to flush file, ferror: " + std::to_string( ec ) );
      }
   }

   void sync() {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      const int fd = fileno(_file_ptr );

      if( -1 == fd ) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to convert file pointer to file descriptor, error: " +
                                      std::to_string( errno ) );
      }

      if( -1 == fsync( fd ) ) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to sync file, error: " + std::to_string( errno ) );
      }
   }

    bool eof() const  {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      if(feof(_file_ptr))
         return true;
    }

   int getc() {
      if(!is_open())
         throw std::ios_base::failure("cfile is not open");

      int ret = fgetc(_file_ptr);

      if (ret == EOF) {
         throw std::ios_base::failure( "cfile: " + _file_path.generic_string() +
                                      " unable to read 1 byte");
      }
      return ret;
   }

   void close() {
      if(is_open()) {
         fclose(_file_ptr);
         _open = false;
      }
   }

   cfile_datastream create_datastream();

private:
   bool                  _open;
   fc::path              _file_path;
   FILE                  *_file_ptr;
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

   size_t tellp() const { return cf.tellp(); }

 private:
   cfile& cf;
};

inline cfile_datastream cfile::create_datastream() {
   return cfile_datastream(*this);
}

template <>
class datastream<fc::cfile, void> : public fc::cfile {
private:
   char mCurrentChar = 'EOF';
 public:
   using fc::cfile::cfile;

   bool seekp(size_t pos) { return this->seek(pos), true; }

   bool get(char& c) {
      c = this->getc();
      mCurrentChar = c;
      return true;
   }

   bool remaining() const {
      return (mCurrentChar != EOF);
   }

   fc::cfile&       storage() { return *this; }
   const fc::cfile& storage() const { return *this; }
};


} // namespace fc

#ifndef _WIN32
#undef FC_FOPEN
#else
#undef FC_CAT
#undef FC_PREL
#undef FC_FOPEN
#endif
