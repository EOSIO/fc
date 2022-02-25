#pragma once
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>

namespace fc { namespace formatter
{
   template<typename T, typename FMT> void format_arg( const T& arg, FMT& fmt_str, FMT& arg_str);

   template<typename T, typename FMT>
   class format_arg_visitor {
   public:
       format_arg_visitor( const T& arg, FMT& fmt_str, FMT& arg_str)
       :_arg(arg), _fmt_str(fmt_str), _arg_str(arg_str) {}

       template<typename Member, class Class, Member (Class::*member)>
       void operator()(const char *name) const {
          _fmt_str += "{}: ";
          _arg_str += "\"" + (string)name + "\"" + ", ";
          format_arg(_arg.*member, _fmt_str, _arg_str);
       }

       const T& _arg;
       FMT& _fmt_str;
       FMT& _arg_str;
   };

   template<typename IsReflected=fc::false_type>
   struct if_enum {
       template<typename T, typename FMT>
       static inline void format_arg( const T& arg, FMT& fmt_str, FMT& arg_str) {
          if constexpr (std::is_same_v<T, eosio::chain::name>) {
             arg_str += const_cast<T&>(arg).to_string() + ", ";
             fmt_str += "{}, ";
          } else {
             fc::reflector<T>::visit(format_arg_visitor<T, FMT>(arg, fmt_str, arg_str));
          }
       }
   };

   template<>
   struct if_enum<fc::true_type> {
       template<typename T, typename FMT>
       static inline void format_arg( const T& arg, FMT& fmt_str, FMT& arg_str) {
          //TODO
       }
   };

   template<typename IsReflected=fc::false_type>
   struct if_reflected {
      template<typename T, typename FMT>
      static inline void format_arg( const T& arg, FMT& fmt_str, FMT& arg_str) {
         fmt_str += "{}, ";
         if constexpr (std::is_same_v<T, fc::time_point_sec>) {
            arg_str += (fc::string) (const_cast<T &>(arg));
         } else if constexpr (std::is_same_v<T, eosio::chain::extensions_type>) {
            fmt_str += "{}, ";
            arg_str += "[]"; //TODO
         } else if constexpr (std::is_same_v<T, std::pair<unsigned short, std::vector<char>>>) {
            fmt_str += "{}, ";
            arg_str += "[]"; //TODO
         } else {
            arg_str += fc::to_string(const_cast<T&>(arg));
         }
         arg_str += ", ";
      }

      template<typename M, typename FMT>
      static inline void format_arg( const std::vector<M>& arg, FMT& fmt_str, FMT& arg_str) {
         for (auto itr = arg.begin(); itr != arg.end(); ++itr) {
            if_reflected<typename fc::reflector<M>::is_defined>::format_arg(*itr, fmt_str, arg_str);
         }
      }

      template<typename FMT>
      static inline void format_arg( const std::vector<char>& arg, FMT& fmt_str, FMT& arg_str) {
         string str(arg.begin(), arg.end());
         fmt_str += "{}, ";
         arg_str += str + ", ";
      }
   };

   template<>
   struct if_reflected<fc::true_type> {
      template<typename T, typename FMT>
      static inline void format_arg( const T& arg, FMT& fmt_str, FMT& arg_str) {
         if_enum<typename fc::reflector<T>::is_enum>::format_arg(arg, fmt_str, arg_str);
      }
   };

   template<typename T, typename FMT>
   void format_arg( const T& arg, FMT& fmt_str, FMT& arg_str)
   {
      if_reflected<typename fc::reflector<T>::is_defined>::format_arg(arg, fmt_str, arg_str);
   }

} }