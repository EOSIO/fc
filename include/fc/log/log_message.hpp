#pragma once
/**
 * @file log_message.hpp
 * @brief Defines types and helper macros necessary for generating log messages.
 */
#include <fc/time.hpp>
#include <fc/variant_object.hpp>
#include <memory>
#include <boost/preprocessor.hpp>
#include <spdlog/fmt/fmt.h>

// get number of arguments with __NARG__
#define __NARG__(...)  __NARG_I_(__VA_ARGS__,__RSEQ_N())
#define __NARG_I_(...) __ARG_N(__VA_ARGS__)
#define __ARG_N( \
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
     _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
     _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
     _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
     _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
     _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
     _61,_62,_63,N,...) N
#define __RSEQ_N() \
     63,62,61,60,                   \
     59,58,57,56,55,54,53,52,51,50, \
     49,48,47,46,45,44,43,42,41,40, \
     39,38,37,36,35,34,33,32,31,30, \
     29,28,27,26,25,24,23,22,21,20, \
     19,18,17,16,15,14,13,12,11,10, \
     9,8,7,6,5,4,3,2,1,0

// this is to deal with -Wgnu-zero-variadic-macro-arguments
// there are duplicates of SWITCH_MACRO for nested calls. otherwise preprocessor stops expanding macro
#define GET_FUNCTION(FN_0, FN_1, MIN_NUM_FN_1, NUM_ARG) BOOST_PP_IF(BOOST_PP_GREATER(NUM_ARG, MIN_NUM_FN_1), FN_1, FN_0)
#define SWITCH_MACRO(FN_0, FN_1, MIN_NUM_FN_1, ...) GET_FUNCTION(FN_0, FN_1, MIN_NUM_FN_1, __NARG__(__VA_ARGS__))(__VA_ARGS__)
#define SWITCH_MACRO1(FN_0, FN_1, MIN_NUM_FN_1, ...) GET_FUNCTION(FN_0, FN_1, MIN_NUM_FN_1, __NARG__(__VA_ARGS__))(__VA_ARGS__)
#define SWITCH_MACRO2(FN_0, FN_1, MIN_NUM_FN_1, ...) GET_FUNCTION(FN_0, FN_1, MIN_NUM_FN_1, __NARG__(__VA_ARGS__))(__VA_ARGS__)

namespace fc
{
   namespace detail 
   { 
       class log_context_impl; 
       class log_message_impl; 
   }

   /**
    * Named scope for log_level enumeration.
    */
   class log_level
   {
      public:
         /**
          * @brief Define's the various log levels for reporting.  
          *
          * Each log level includes all higher levels such that 
          * Debug includes Error, but Error does not include Debug.
          */
         enum values
         {
             all, 
             debug, 
             info, 
             warn, 
             error, 
             off  
         };
         log_level( values v = off ):value(v){}
         explicit log_level( int v ):value( static_cast<values>(v)){}
         operator int()const { return value; }
         string to_string()const;
         values value;
   };

   void to_variant( log_level e, variant& v );
   void from_variant( const variant& e, log_level& ll );

   /**
    *  @brief provides information about where and when a log message was generated.
    *  @ingroup AthenaSerializable
    *
    *  @see FC_LOG_CONTEXT
    */
   class log_context 
   {
      public:
        log_context();
        log_context( log_level ll,
                    const char* file, 
                    uint64_t line, 
                    const char* method );
        ~log_context();
        explicit log_context( const variant& v );
        variant to_variant()const;

        string        get_file()const;
        uint64_t      get_line_number()const;
        string        get_method()const;
        string        get_thread_name()const;
        string        get_task_name()const;
        string        get_host_name()const;
        time_point    get_timestamp()const;
        log_level     get_log_level()const;
        string        get_context()const;

        void          append_context( const fc::string& c );

        string        to_string()const;
      private:
        std::shared_ptr<detail::log_context_impl> my;
   };

   void to_variant( const log_context& l, variant& v );
   void from_variant( const variant& l, log_context& c );

   /**
    *  @brief aggregates a message along with the context and associated meta-information.
    *  @ingroup AthenaSerializable
    *
    *  @note log_message has reference semantics, all copies refer to the same log message
    *  and the message is read-only after construction.
    *
    *  When converted to JSON, log_message has the following form:
    *  @code
    *  {
    *     "context" : { ... },
    *     "format"  : "string with ${keys}",
    *     "data"    : { "keys" : "values" }
    *  }
    *  @endcode
    *
    *  @see FC_LOG_MESSAGE
    */
   class log_message
   {
      public:
         log_message();
         /**
          *  @param ctx - generally provided using the FC_LOG_CONTEXT(LEVEL) macro 
          */
         log_message( log_context ctx, std::string format, variant_object args = variant_object() );
         ~log_message();

         log_message( const variant& v );
         variant        to_variant()const;
                              
         string         get_message()const;
         /**
          * A faster version of get_message which does limited formatting and excludes large variants
          * @return formatted message according to format and variant args
          */
         string         get_limited_message()const;
                              
         log_context    get_context()const;
         string         get_format()const;
         variant_object get_data()const;

      private:
         std::shared_ptr<detail::log_message_impl> my;
   };

   void    to_variant( const log_message& l, variant& v );
   void    from_variant( const variant& l, log_message& c );

   typedef std::vector<log_message> log_messages;


} // namespace fc

FC_REFLECT_TYPENAME( fc::log_message )

#ifndef __func__
#define __func__ __FUNCTION__
#endif

/**
 * @def FC_LOG_CONTEXT(LOG_LEVEL)
 * @brief Automatically captures the File, Line, and Method names and passes them to
 *        the constructor of fc::log_context along with LOG_LEVEL
 * @param LOG_LEVEL - a valid log_level::Enum name.
 */
#define FC_LOG_CONTEXT(LOG_LEVEL) \
   fc::log_context( fc::log_level::LOG_LEVEL, __FILE__, __LINE__, __func__ )

#define FC_NARGS(...) __FC_NARGS(0, ## __VA_ARGS__, 9,8,7,6,5,4,3,2,1,0)
#define __FC_NARGS(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,N,...) N

#define FC_FMT( FORMAT, ... ) \
   fmt::format( FORMAT BOOST_PP_IF(FC_NARGS(__VA_ARGS__), BOOST_PP_COMMA, BOOST_PP_EMPTY)() FC_ADD_FMT_ARGS( __VA_ARGS__ ) )

#define FC_ADD_FMT_ARGS(SEQ)           \
   BOOST_PP_SEQ_FOR_EACH_I(            \
     FC_ADD_FMT_ARG, _,                \
     BOOST_PP_VARIADIC_SEQ_TO_SEQ(SEQ) \
   )

#define FC_ADD_FMT_ARG( r, data, index, elem )      \
   BOOST_PP_COMMA_IF(index) fmt::arg(BOOST_PP_TUPLE_ENUM(elem))

/**
 * @def FC_LOG_MESSAGE(LOG_LEVEL,FORMAT,...)
 *
 * @brief A helper method for generating log messages.
 *
 * @param LOG_LEVEL a valid log_level::Enum name to be passed to the log_context
 * @param FORMAT A const char* string containing zero or more references to keys as "${key}"
 * @param ...  A set of key/value pairs denoted as ("key",val)("key2",val2)...
 */
#define FC_LOG_MESSAGE_1( LOG_LEVEL, FORMAT, ... ) \
   fc::log_message( FC_LOG_CONTEXT(LOG_LEVEL), FORMAT, fc::mutable_variant_object()__VA_ARGS__ )

#define FC_LOG_MESSAGE_0(LOG_LEVEL, FORMAT) FC_LOG_MESSAGE_1(LOG_LEVEL, FORMAT,)
#define FC_LOG_MESSAGE(...) SWITCH_MACRO(FC_LOG_MESSAGE_0, FC_LOG_MESSAGE_1, 2, __VA_ARGS__)