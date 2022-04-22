#include <fc/log/logger_config.hpp>
#include <fc/io/json.hpp>
#include <fc/filesystem.hpp>
#include <unordered_map>
#include <string>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>

namespace fc {

   log_config& log_config::get() {
      // allocate dynamically which will leak on exit but allow loggers to be used until the very end of execution
      static log_config* the = new log_config;
      return *the;
   }

   logger log_config::get_logger( const fc::string& name ) {
      std::lock_guard g( log_config::get().log_mutex );
      return log_config::get().logger_map[name];
   }

   void log_config::update_logger( const fc::string& name, logger& log ) {
      std::lock_guard g( log_config::get().log_mutex );
      if( log_config::get().logger_map.find( name ) != log_config::get().logger_map.end() ) {
         log = log_config::get().logger_map[name];
      } else {
         // no entry for logger, so setup with default logger if it exists, otherwise do nothing since default logger not configured
         if( log_config::get().logger_map.find( DEFAULT_LOGGER ) != log_config::get().logger_map.end() ) {
            log = log_config::get().logger_map[DEFAULT_LOGGER];
            log_config::get().logger_map.emplace( name, log );
         }
      }
   }

   void configure_logging( const fc::path& lc ) {
      configure_logging( fc::json::from_file<logging_config>(lc) );
   }
   bool configure_logging( const logging_config& cfg ) {
      return log_config::configure_logging( cfg );
   }

   bool log_config::configure_logging( const logging_config& cfg ) {
      try {
         std::lock_guard g( log_config::get().log_mutex );
         log_config::get().logger_map.clear();
         log_config::get().sink_map.clear();

         for ( size_t i = 0; i < cfg.sinks.size(); ++i ) {
            // create sink
            if (cfg.sinks[i].type == "stderr_color_sink_st") {
               auto config = cfg.sinks[i].args.as<sink::stderr_color_sink_st_config>();
               auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
               for (auto& it : config.level_colors) {
                  if (it.color == "yellow")
                     sink->set_color(spdlog::level::from_str(it.level), sink->yellow);
                  else if (it.color == "red")
                     sink->set_color(spdlog::level::from_str(it.level), sink->red);
                  else if (it.color == "green")
                     sink->set_color(spdlog::level::from_str(it.level), sink->green);
                  else
                     sink->set_color(spdlog::level::from_str(it.level), sink->reset);
               }
               log_config::get().sink_map[cfg.sinks[i].name] = sink;
            } else if (cfg.sinks[i].type == "daily_file_sink_mt") {
               auto config = cfg.sinks[i].args.as<sink::daily_file_sink_mt_config>();
               auto sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                       config.base_filename, config.rotation_hour, config.rotation_minute, config.truncate, config.max_files);
               log_config::get().sink_map[cfg.sinks[i].name] = sink;
            } else if (cfg.sinks[i].type == "rotating_file_sink_mt") {
               auto config = cfg.sinks[i].args.as<sink::rotating_file_sink_mt_config>();
               auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                       config.base_filename, config.max_size*1024*1024, config.max_files);
               log_config::get().sink_map[cfg.sinks[i].name] = sink;
            }
         }

         for( size_t i = 0; i < cfg.loggers.size(); ++i ) {
            auto lgr = log_config::get().logger_map[cfg.loggers[i].name];
            lgr.set_name(cfg.loggers[i].name);
            if( cfg.loggers[i].level ) lgr.set_log_level( *cfg.loggers[i].level );

            for (auto s = cfg.loggers[i].sinks.begin(); s != cfg.loggers[i].sinks.end(); ++s) {
               auto sink_it = log_config::get().sink_map.find(*s);
               if (sink_it != log_config::get().sink_map.end() ) {
                  lgr.add_sink(sink_it->second);
               }
            }
            if (cfg.loggers[i].sinks.size() > 0)
               lgr.update_agent_logger(std::make_unique<spdlog::logger>("", lgr.get_sinks().begin(), lgr.get_sinks().end()));
         }
         return true;
      } catch ( exception& e ) {
         std::cerr<<e.to_detail_string()<<"\n";
      }
      return false;
   }

   logging_config logging_config::default_config() {
      //slog( "default cfg" );
      logging_config cfg;

     variants  c;
               c.push_back(  mutable_variant_object( "level","debug")("color", "green") );
               c.push_back(  mutable_variant_object( "level","warn")("color", "brown") );
               c.push_back(  mutable_variant_object( "level","error")("color", "red") );

      logger_config dlc;
      dlc.name = DEFAULT_LOGGER;
      dlc.level = log_level::info;
      cfg.loggers.push_back( dlc );
      return cfg;
   }

   static thread_local std::string thread_name;
   void set_os_thread_name( const string& name ) {
#ifdef FC_USE_PTHREAD_NAME_NP
      pthread_setname_np( pthread_self(), name.c_str() );
#endif
   }
   void set_thread_name( const string& name ) {
      thread_name = name;
   }
   const string& get_thread_name() {
      if( thread_name.empty() ) {
#ifdef FC_USE_PTHREAD_NAME_NP
         char thr_name[64];
         int rc = pthread_getname_np( pthread_self(), thr_name, 64 );
         if( rc == 0 ) {
            thread_name = thr_name;
         }
#else
         static int thread_count = 0;
         thread_name = string( "thread-" ) + fc::to_string( thread_count++ );
#endif
      }
      return thread_name;
   }
}
