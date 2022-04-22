#pragma once
#include <fc/log/logger.hpp>
#include <spdlog/spdlog.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace fc {
   class path;
   struct sink_config {
       sink_config(const string& name = "", const string& type = "", variant args = variant())
          : name(name), type(type), args(fc::move(args)), enabled(true) {}
       string name;
       string type;
       variant args;
       bool enabled;
   };

   namespace sink {
      struct level_color {
          level_color ( string l = "trace", string c = "yellow")
                  : level(l), color(c) {}

          string  level;
          string  color;
      };

      struct stderr_color_sink_st_config {
          std::vector<level_color>      level_colors;
      };

      struct daily_file_sink_mt_config {
         std::string    base_filename;
         int32_t        rotation_hour;
         int32_t        rotation_minute;
         bool           truncate;
         uint32_t       max_files;
      };

      struct rotating_file_sink_mt_config {
         string         base_filename;
         uint32_t       max_size;
         uint32_t       max_files;
      };

   } // namespace sink

   struct logger_config {
      logger_config(const fc::string& name = ""):name(name),enabled(true){}
      string                           name;
      std::optional<log_level>         level;
      bool                             enabled;
      std::vector<string>              sinks;
   };

   struct logging_config {
      static logging_config default_config();
      std::vector<string>          includes;
      std::vector<sink_config>     sinks;
      std::vector<logger_config>   loggers;
   };

   struct log_config {
      static logger get_logger( const fc::string& name );
      static void update_logger( const fc::string& name, logger& log );

      static bool configure_logging( const logging_config& l );

   private:
      static log_config& get();

      friend class logger;

      std::mutex                                                             log_mutex;
      std::unordered_map<std::string, std::shared_ptr<spdlog::sinks::sink>>  sink_map;
      std::unordered_map<std::string, logger>                                logger_map;
   };

   void configure_logging( const fc::path& log_config );
   bool configure_logging( const logging_config& l );

   void set_os_thread_name( const string& name );
   void set_thread_name( const string& name );
   const string& get_thread_name();
}

#include <fc/reflect/reflect.hpp>
FC_REFLECT( fc::sink_config, (name)(type)(args)(enabled) )
FC_REFLECT( fc::sink::level_color, (level)(color) )
FC_REFLECT( fc::sink::stderr_color_sink_st_config, (level_colors) )
FC_REFLECT( fc::sink::daily_file_sink_mt_config, (base_filename)(rotation_hour)(rotation_minute)(truncate)(max_files) )
FC_REFLECT( fc::sink::rotating_file_sink_mt_config, (base_filename)(max_size)(max_files) )
FC_REFLECT( fc::logger_config, (name)(level)(enabled)(sinks) )
FC_REFLECT( fc::logging_config, (includes)(sinks)(loggers) )
