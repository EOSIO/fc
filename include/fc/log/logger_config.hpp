#pragma once
#include <fc/log/logger.hpp>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace fc {
   class path;

   struct logger_config {
      logger_config(const fc::string& name = ""):name(name),enabled(true),additivity(false){}
      string                           name;
      ostring                          parent;
      /// if not set, then parents level is used.
      std::optional<log_level>         level;
      bool                             enabled;
      /// if any appenders are sepecified, then parent's appenders are not set.
      bool                             additivity;
   };

   struct logging_config {
      static logging_config default_config();
      std::vector<string>          includes;
      std::vector<logger_config>   loggers;
   };

   struct log_config {

      static logger get_logger( const fc::string& name );
      static void update_logger( const fc::string& name, logger& log );

      static bool configure_logging( const logging_config& l );

   private:
      static log_config& get();

      friend class logger;

      std::mutex                                               log_mutex;
      std::unordered_map<std::string, logger>                  logger_map;
   };

   void configure_logging( const fc::path& log_config );
   bool configure_logging( const logging_config& l );

   void set_os_thread_name( const string& name );
   void set_thread_name( const string& name );
   const string& get_thread_name();
}

#include <fc/reflect/reflect.hpp>
FC_REFLECT( fc::logger_config, (name)(parent)(level)(enabled)(additivity) )
FC_REFLECT( fc::logging_config, (includes)(loggers) )
